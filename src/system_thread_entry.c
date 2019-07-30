#include <system_thread.h>

#define FREC_CLK_TIMER (120000000U) // Timer Frecuency =  120 MHz
#define BTN_PRESSED IOPORT_LEVEL_LOW
#define BTN_RELEASED IOPORT_LEVEL_HIGH
#define LED_ON IOPORT_LEVEL_LOW
#define LED_OFF IOPORT_LEVEL_HIGH
#define MOTOR_ON IOPORT_LEVEL_HIGH
#define MOTOR_OFF IOPORT_LEVEL_LOW
#define ON IOPORT_LEVEL_HIGH
#define OFF IOPORT_LEVEL_LOW
#define C_FILTER_ORDER  8
#define TS_NUMBER 5

uint16_t u16PwmPercent=50;
uint16_t u16Frec_PWM = 1000, u16Frec_Sensor_op1 = 0, u16Frec_Sensor_op2 = 0;
uint8_t u8Pulses = 0;
ioport_level_t u1Pin=0;
uint16_t u16RPM_SP = 1000, u16RPM = 0;
uint8_t u8Kp=64, u8Ki=32; // Gains for T=100ms

uint16_t au16Send_DataToLCD[2] = {0};

uint8_t u8Mot_status=0;
uint8_t u8Faults_Counter = 0;
uint8_t U8Ts_Number = 5;

uint32_t u32WDT_Counter ;
wdt_status_t u1WDT_Status;

void SR_Conf_System(void);
void SR_Motor_Control(void);
void SR_Motor_Status(void);
uint16_t FN_u16PI_Control(int16_t lu16Error);
uint16_t FN_u16Filter(uint16_t lu16Value);
void FN_Enable_Motor(bool lu1Status);
uint16_t FN_u16Read_RPM_SP(void);
void SR_Fault_handle(void);
void SR_Toggling_LED(void);


void system_thread_entry(void){


    SR_Conf_System();


    while(1){
        //g_ioport.p_api->pinWrite(IOPORT_PORT_01_PIN_14, IOPORT_LEVEL_HIGH); //Pin used to check algorithm time
        u1Pin = !u1Pin; g_ioport.p_api->pinWrite(IOPORT_PORT_01_PIN_14, u1Pin); //Pin used to check the sample time
        SR_Fault_handle();
        SR_Toggling_LED(); // Red LED at 1 Hz: working correctly, Red LED 10 Hz: There is a Motor Fault
        SR_Motor_Control();
        tx_queue_flush(&Message_Queue); //Clean Queue to send the latest data
        tx_queue_send(&Message_Queue, au16Send_DataToLCD, TX_NO_WAIT);// send data to LCD Thread
        //g_ioport.p_api->pinWrite(IOPORT_PORT_01_PIN_14, IOPORT_LEVEL_LOW); //Pin used to check algorithm time
        tx_thread_sleep(10);

        /****** Refresh IWDT ******/
        g_wdt0.p_api->counterGet(g_wdt0.p_ctrl, &u32WDT_Counter); //Counter don't working, it seems like disabled by the OS
        g_wdt0.p_api->refresh(g_wdt0.p_ctrl);
        g_wdt0.p_api->statusGet( g_wdt0.p_ctrl, &u1WDT_Status);

        }
}


void SR_Conf_System(void){

    /******   CONFIGURING THE ADC    ******/
    g_adc0.p_api->open(g_adc0.p_ctrl, g_adc0.p_cfg);
    g_adc0.p_api->scanCfg(g_adc0.p_ctrl, g_adc0.p_channel_cfg);
    g_adc0.p_api->scanStart(g_adc0.p_ctrl);


    /******   CONFIGURING TIMERS AND IRQ    ******/
    g_timer0.p_api->open(g_timer0.p_ctrl, g_timer0.p_cfg); //Timer 0 to measure the period of the sensor
    g_timer1.p_api->open(g_timer1.p_ctrl, g_timer1.p_cfg); //Timer 1 to count pulses every 100 ms
    g_timer2.p_api->open(g_timer2.p_ctrl, g_timer2.p_cfg); //Timer 2 to generate the PWM at 1 KHz
    g_external_irq5.p_api->open(g_external_irq5.p_ctrl, g_external_irq5.p_cfg); //Int 5 detects the sensor pulse raise

    g_timer2.p_api->periodSet(g_timer2.p_ctrl, u16Frec_PWM, TIMER_UNIT_FREQUENCY_HZ); //used to change the frec manually


    /******   STARTING THE TIMERS    ******/
    g_timer0.p_api->start(g_timer0.p_ctrl);
    g_timer1.p_api->start(g_timer1.p_ctrl);
    g_timer2.p_api->start(g_timer2.p_ctrl);

    /******      TURNING OFF THE LEDs    ******/
    g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_00, LED_OFF);
    g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_01, LED_OFF);
    g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_02, LED_OFF);

    /****** Initialize WDT  ******/
    g_wdt0.p_api->open(g_wdt0.p_ctrl, g_wdt0.p_cfg);
}

void external_irq5_callback(external_irq_callback_args_t *p_args){
    static uint32_t lu32Counts = 0, alu32Counts_avg[4]={0};

    SSP_PARAMETER_NOT_USED(p_args);
    g_timer0.p_api->counterGet(g_timer0.p_ctrl, &lu32Counts); // read the period clocks
    g_timer0.p_api->reset(g_timer0.p_ctrl); // restart the Timer for a new period measurement
    alu32Counts_avg[0] = (uint32_t)((lu32Counts + alu32Counts_avg[1] + alu32Counts_avg[2] + alu32Counts_avg[3])/4); // Counts average of the last 4 measurements
    u16Frec_Sensor_op1 = (uint16_t)(FREC_CLK_TIMER/alu32Counts_avg[0]); // conversion of time to frec
    alu32Counts_avg[3] = alu32Counts_avg[2]; //Update previous values
    alu32Counts_avg[2] = alu32Counts_avg[1];
    alu32Counts_avg[1] = lu32Counts;
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

    u16RPM_SP= (uint16_t)((FN_u16Read_RPM_SP() * 3000)/982); // Convert data from ADC_10Bits(0-982) to rpm (0-3000)

    u16RPM = (uint16_t)(15 * u16Frec_Sensor_op2); // Conversion from Frec to RPM (Frec*60/4)

    lu16RPM_Filtered = FN_u16Filter(u16RPM);

    li16Error = (int16_t)(u16RPM_SP - lu16RPM_Filtered);

    lu16Ctrl_Out = FN_u16PI_Control(li16Error);

    u16PwmPercent = (uint16_t) (100 - lu16Ctrl_Out);  // Driver has reverse logical

    g_timer2.p_api->dutyCycleSet(g_timer2.p_ctrl, u16PwmPercent, TIMER_PWM_UNIT_PERCENT, 0); //Send the result of the Control to the Motor Driver

    au16Send_DataToLCD[0] = lu16Ctrl_Out; //Duty Cycle to be sent to LCD_Thread

    au16Send_DataToLCD[1] = lu16RPM_Filtered;                //RPM to be sent to LCD_Thread
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
    static uint8_t u8Ts_Inv = 10;

    li16Integr_Error = (int16_t)((li16Error/u8Ki) + li16Integr_Error_n1);

    li16Integr_Error_n1 = li16Integr_Error;

    li16Integr_Error = (int16_t)(li16Integr_Error / u8Ts_Inv);// Integral multiplied x 0.1 ms (Sample time)

    if(li16Integr_Error > 100) li16Integr_Error = 100;

    if(li16Integr_Error < 0) li16Integr_Error = 0;

    li16Ctrl_Out = (int16_t)((li16Error/u8Kp) + li16Integr_Error);

    if(li16Ctrl_Out > 100) li16Ctrl_Out = 100;

    if(li16Ctrl_Out < 0) li16Ctrl_Out = 0;

    return (uint16_t)li16Ctrl_Out;
}

uint16_t FN_u16Read_RPM_SP(void){

    static uint16_t lu16ADC_Data, alu16ADC_Data[2];

    g_adc0.p_api->read(g_adc0.p_ctrl, ADC_REG_CHANNEL_0, &lu16ADC_Data);

    lu16ADC_Data = (uint16_t)((lu16ADC_Data + alu16ADC_Data[0] + alu16ADC_Data[1])/3);
    alu16ADC_Data[1] = alu16ADC_Data[0];
    alu16ADC_Data[0] = lu16ADC_Data;

    return lu16ADC_Data;
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
    g_ioport.p_api->pinRead(IOPORT_PORT_00_PIN_05, &lu1P05_status);
    g_ioport.p_api->pinRead(IOPORT_PORT_00_PIN_06, &lu1P06_status);
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
        g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_00, LED_ON);
        g_ioport.p_api->pinWrite(IOPORT_PORT_04_PIN_12, MOTOR_ON);  //Enable Motor
        u8Mot_status=1;
    }
    else{
        g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_00, LED_OFF);
        g_ioport.p_api->pinWrite(IOPORT_PORT_04_PIN_12, MOTOR_OFF); //Disable Motor
        u8Mot_status=0;
    }
}

void SR_Fault_handle(void){
    static ioport_level_t lu1P411_status, lu8Fault_Status = 0;

    g_ioport.p_api->pinRead(IOPORT_PORT_04_PIN_11, &lu1P411_status);
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

void SR_Toggling_LED(void){
    static ioport_level_t lu1LED=0;
    static uint8_t lu8Ts_Counter = 1;

    if(lu8Ts_Counter++ > U8Ts_Number){
        lu1LED = !lu1LED;
        g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_01, lu1LED); // Toggling Red LED at 1 Hz
        lu8Ts_Counter = 1;
    }
}


void wdt0_callback(wdt_callback_args_t * p_args){
    SSP_PARAMETER_NOT_USED(p_args);
    g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_02, LED_ON); //Turn ON Yellow LED

}
