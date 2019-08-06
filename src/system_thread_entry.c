#include <system_thread.h>
#include <RAM.h>
#include <system.h>


void system_thread_entry(void){


    SR_Conf_System();


    while(1){
        //g_ioport.p_api->pinWrite(IOPORT_PORT_01_PIN_14, IOPORT_LEVEL_HIGH); //Pin used to check algorithm time
        u1Pin = !u1Pin; g_ioport.p_api->pinWrite(IOPORT_PORT_01_PIN_14, u1Pin); //Pin used to check the sample time
        SR_Fault_handle(); //Check if there is Fault, if not, check the switches status.
        SR_Blinking_LED(); // Red LED at 1 Hz: working correctly, Red LED 10 Hz: There is a Motor Fault
        if(1 == u8Mot_status)
            SR_Motor_Control();
        SError = tx_queue_flush(&Message_Queue); //Clean Queue to send the latest data
        SError = tx_queue_send(&Message_Queue, au16Send_DataToLCD, TX_NO_WAIT);// send data to LCD Thread
        //g_ioport.p_api->pinWrite(IOPORT_PORT_01_PIN_14, IOPORT_LEVEL_LOW); //Pin used to check algorithm time
        SError = tx_thread_sleep(10);

        /****** Refresh IWDT ******/
        SError = g_wdt0.p_api->counterGet(g_wdt0.p_ctrl, &u32WDT_Counter); //Counter don't working, it seems like disabled by the OS
        SError = g_wdt0.p_api->refresh(g_wdt0.p_ctrl);
        SError = g_wdt0.p_api->statusGet(g_wdt0.p_ctrl, &u1WDT_Status);

        SR_Error_handle();
    }
}



void SR_Conf_System(void){

    /******   CONFIGURING THE ADC    ******/
    SError = g_adc0.p_api->open(g_adc0.p_ctrl, g_adc0.p_cfg);
    SError = g_adc0.p_api->scanCfg(g_adc0.p_ctrl, g_adc0.p_channel_cfg);
    SError = g_adc0.p_api->scanStart(g_adc0.p_ctrl);


    /******   CONFIGURING TIMERS AND IRQ    ******/
    SError = g_timer0.p_api->open(g_timer0.p_ctrl, g_timer0.p_cfg); //Timer 0 to measure the period of the sensor
    SError = g_timer1.p_api->open(g_timer1.p_ctrl, g_timer1.p_cfg); //Timer 1 to count pulses every 100 ms
    SError = g_timer2.p_api->open(g_timer2.p_ctrl, g_timer2.p_cfg); //Timer 2 to generate the PWM at 1 KHz
    SError = g_external_irq5.p_api->open(g_external_irq5.p_ctrl, g_external_irq5.p_cfg); //Int 5 detects the sensor pulse raise

    SError = g_timer2.p_api->periodSet(g_timer2.p_ctrl, u16Frec_PWM, TIMER_UNIT_FREQUENCY_HZ); //used to change the frec manually


    /******   STARTING THE TIMERS    ******/
    SError = g_timer0.p_api->start(g_timer0.p_ctrl);
    SError = g_timer1.p_api->start(g_timer1.p_ctrl);
    SError = g_timer2.p_api->start(g_timer2.p_ctrl);

    /******      TURNING OFF THE LEDs    ******/
    SError = g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_00, LED_OFF);
    SError = g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_01, LED_OFF);
    SError = g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_02, LED_OFF);

    /****** Initialize WDT  ******/
    SError = g_wdt0.p_api->open(g_wdt0.p_ctrl, g_wdt0.p_cfg);

    SR_Error_handle();
}

void external_irq5_callback(external_irq_callback_args_t *p_args){
    static uint32_t lu32Counts = 0, alu32Counts_avg[3]={0};

    SSP_PARAMETER_NOT_USED(p_args);
    SError = g_timer0.p_api->counterGet(g_timer0.p_ctrl, &lu32Counts); // read the period clocks
    SError = g_timer0.p_api->reset(g_timer0.p_ctrl); // restart the Timer for a new period measurement
    lu32Counts = (uint32_t)((lu32Counts + alu32Counts_avg[0] + alu32Counts_avg[1] + alu32Counts_avg[2])/4); // Average of the last 4 measurements
    u16Frec_Sensor_op1 = (uint16_t)(FREC_CLK_TIMER/alu32Counts_avg[0]); // conversion of time to frec
    alu32Counts_avg[2] = alu32Counts_avg[1]; //Update previous values
    alu32Counts_avg[1] = alu32Counts_avg[0];
    u8Pulses++;
}


void timer1_callback(timer_callback_args_t * p_args){
    static uint16_t lu16Frec_n = 0, lu16Frec_n1 = 0;

    if (TIMER_EVENT_EXPIRED  == p_args->event)
    {
        lu16Frec_n = (uint16_t)(u8Pulses *10);
        u16Frec_Sensor_op2 = (uint16_t)((lu16Frec_n + lu16Frec_n1)/2);
        lu16Frec_n1 = lu16Frec_n;
        u8Pulses = 0; // Pulses every 100 ms
    }
}

void SR_Motor_Control(void){

    static uint16_t lu16Ctrl_Out, lu16RPM_Filtered;
    static int16_t li16Error;

    u16RPM_SP = FN_u16Read_RPM_SP();

    u16RPM = (uint16_t)(15 * u16Frec_Sensor_op2); // Conversion from Frec to RPM (Frec*60/4)

    lu16RPM_Filtered = FN_u16Filter(u16RPM);

    li16Error = (int16_t)(u16RPM_SP - lu16RPM_Filtered);

    lu16Ctrl_Out = FN_u16PI_Control(li16Error);

    u16PwmPercent = (uint16_t) (100 - lu16Ctrl_Out);  // Driver has reverse logical

    SError = g_timer2.p_api->dutyCycleSet(g_timer2.p_ctrl, u16PwmPercent, TIMER_PWM_UNIT_PERCENT, 0); //Send the result of the Control to the Motor Driver

    au16Send_DataToLCD[0] = lu16Ctrl_Out; //Duty Cycle to be sent to LCD_Thread

    au16Send_DataToLCD[1] = lu16RPM_Filtered; //RPM to be sent to LCD_Thread
}

uint16_t FN_u16PI_Control(int16_t li16Error){

    /********** PI Control formula **********
    Ctrl_Out= (1/kp)*e + Integr_Error;
    where:
    Error = RPM_SP - RPM;
    Integr_Error=(1/ki)*Integr_Error*Ts + Integr_Error_n1;
    Where: Ts = sample time, Integr_Error_n1= Error integral accumulated
     */
    static int16_t li16Ctrl_Out, li16Integr_Error, li16Integr_Error_n1 = 0;
    static uint8_t lu8Ts_Inv = 10;

    li16Integr_Error = (int16_t)((li16Error/u8Ki) + li16Integr_Error_n1);

    li16Integr_Error_n1 = li16Integr_Error;

    li16Integr_Error = (int16_t)(li16Integr_Error / lu8Ts_Inv);// Integral multiplied x 0.1 ms (Sample time)

    if(li16Integr_Error > 100) li16Integr_Error = 100;

    if(li16Integr_Error < 0) li16Integr_Error = 0;

    li16Ctrl_Out = (int16_t)((li16Error/u8Kp) + li16Integr_Error);

    if(li16Ctrl_Out > 100) li16Ctrl_Out = 100;

    if(li16Ctrl_Out < 0) li16Ctrl_Out = 0;

    return (uint16_t)li16Ctrl_Out;
}

uint16_t FN_u16Read_RPM_SP(void){

    static uint16_t lu16ADC_Data, alu16ADC_Data[2], lu16RPM_SP;

    SError = g_adc0.p_api->read(g_adc0.p_ctrl, ADC_REG_CHANNEL_0, &lu16ADC_Data);

    lu16ADC_Data = (uint16_t)((lu16ADC_Data + alu16ADC_Data[0] + alu16ADC_Data[1])/3);
    alu16ADC_Data[1] = alu16ADC_Data[0];
    alu16ADC_Data[0] = lu16ADC_Data;

    lu16RPM_SP = (uint16_t)((lu16ADC_Data * 3000)/982); // Convert data from ADC_10Bits(0-982) to rpm (0-3000)

    return lu16RPM_SP;
}

uint16_t FN_u16Filter(uint16_t lu16Value){
    static float lf32Value_Filtered = 0;

    if(C_FILTER_ORDER > 1)
        lf32Value_Filtered = (float)((lu16Value + (float)((C_FILTER_ORDER - 1) * lf32Value_Filtered)) / C_FILTER_ORDER);
    else
        lf32Value_Filtered = lu16Value;

    return (uint16_t)lf32Value_Filtered;
}


void SR_Motor_Status(void){
    static ioport_level_t lu1P05_status, lu1P06_status;

    /******      READ INPUTS    ******/
    SError = g_ioport.p_api->pinRead(IOPORT_PORT_00_PIN_05, &lu1P05_status);
    SError = g_ioport.p_api->pinRead(IOPORT_PORT_00_PIN_06, &lu1P06_status);
    if(0 == u8Mot_status){
                if(lu1P05_status == BTN_PRESSED && lu1P06_status == BTN_PRESSED){
                    FN_Enable_Motor(ON);
                    tx_thread_sleep(100);
                }
                else FN_Enable_Motor(OFF);
    }
    else {
              if(lu1P05_status == BTN_RELEASED && lu1P06_status == BTN_RELEASED)
                  FN_Enable_Motor(ON);
              else FN_Enable_Motor(OFF);
    }
}

void FN_Enable_Motor(bool lu1Status){
    if(1==lu1Status){
        SError = g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_00, LED_ON);
        SError = g_ioport.p_api->pinWrite(IOPORT_PORT_04_PIN_12, MOTOR_ON);  //Enable Motor
        u8Mot_status=1;
    }
    else{
        SError = g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_00, LED_OFF);
        SError = g_ioport.p_api->pinWrite(IOPORT_PORT_04_PIN_12, MOTOR_OFF); //Disable Motor
        u8Mot_status=0;
        au16Send_DataToLCD[0] = 0; // To send 0 duty cycle
        au16Send_DataToLCD[1] = 0; // To send 0 RPM
    }
}

void SR_Fault_handle(void){
    static ioport_level_t lu1P411_status, lu8Fault_Status = 0;

    SError = g_ioport.p_api->pinRead(IOPORT_PORT_04_PIN_11, &lu1P411_status);
    if(IOPORT_LEVEL_LOW == lu1P411_status ){
        FN_Enable_Motor(OFF);
        U8Ts_Number = 0; //Blinking Red LED at 10 Hz
        lu8Fault_Status=1;
    }
    else{
        U8Ts_Number = 5; //Blinking Red LED at 1 Hz
        SR_Motor_Status();
        if(1 == lu8Fault_Status){
            u8Faults_Counter++;
            lu8Fault_Status = 0;
        }
    }

}

void SR_Blinking_LED(void){
    static ioport_level_t lu1LED=0;
    static uint8_t lu8Ts_Counter = 1;

    if(lu8Ts_Counter++ > U8Ts_Number){
        lu1LED = !lu1LED;
        SError = g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_01, lu1LED); // Toggling Red LED at 1 Hz
        lu8Ts_Counter = 1;
    }
}


void wdt0_callback(wdt_callback_args_t * p_args){
    SSP_PARAMETER_NOT_USED(p_args);
    FN_Enable_Motor(OFF); //Turn OFF the Motor
    SError = g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_02, LED_ON); //Turn ON Yellow LED
    SError = g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_01, LED_ON); //Turn ON Red LED
}

void SR_Error_handle(void)
{
    if(SSP_SUCCESS != SError) // if There is any error turn on all LEDs
    {
        SError = g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_00, LED_ON);
        SError = g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_01, LED_ON);
        SError = g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_02, LED_ON);
        while(1);
    }

}
