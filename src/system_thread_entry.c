#include <system_thread.h>

#define FREC_CLK_TIMER (120000000U) // Timer Frecuency =  120 MHz
#define BTN_PRESSED IOPORT_LEVEL_LOW
#define BTN_RELEASED IOPORT_LEVEL_HIGH
#define LED_ON IOPORT_LEVEL_LOW
#define LED_OFF IOPORT_LEVEL_HIGH
#define MOTOR_ON IOPORT_LEVEL_HIGH
#define MOTOR_OFF IOPORT_LEVEL_LOW
#define C_FILTER_ORDER  8

uint16_t u16PwmPercent=50;
uint16_t u16Frec_PWM = 1000, u16Frec_Sensor_op1 = 0, u16Frec_Sensor_op2 = 0;
uint8_t u8Pulses = 0;
ioport_level_t u1Pin=0;
uint16_t u16RPM_SP = 1000, u16RPM = 0, u16DutyCycle_toPlot;
uint8_t u8Kp=64, u8Ki=32; // Gains for T=100ms

uint16_t u16Value_Filtered, u16RPM_Filtered;

uint16_t au16Send_DataToLCD[2] = {0};

void SR_Motor_Control(void);
void SR_Motor_Status(void);
uint16_t FN_Filter(uint16_t lu16Value);

void system_thread_entry(void)
{

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

    while(1){
        //g_ioport.p_api->pinWrite(IOPORT_PORT_01_PIN_14, IOPORT_LEVEL_HIGH); //Pin used to check algorithm time
        //u1Pin = !u1Pin; g_ioport.p_api->pinWrite(IOPORT_PORT_01_PIN_14, Pin); //Pin used to check the sample time
        SR_Motor_Status();
        SR_Motor_Control();
        //PwmPercent= (uint16_t)((lu16ADC_Data * 100)/982); // Convert data from ADC(0-982) to Duty_cycle (0-100)
        // g_timer2.p_api->dutyCycleSet(g_timer2.p_ctrl, u16PwmPercent, TIMER_PWM_UNIT_PERCENT, 0); //used to change the dutycycle manually
        tx_queue_flush(&Message_Queue); //Clean Queue to send the latest data
        tx_queue_send(&Message_Queue, au16Send_DataToLCD, TX_NO_WAIT);// send data to LCD Thread
        //g_ioport.p_api->pinWrite(IOPORT_PORT_01_PIN_14, IOPORT_LEVEL_LOW); //Pin used to check algorithm time
        tx_thread_sleep(10);
        }
}


void external_irq5_callback(external_irq_callback_args_t *p_args)
{
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


void timer1_callback(timer_callback_args_t * p_args)
{
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

    /********** PI Control formula **********
    Ctrl_Out= (1/kp)*e + Integr_Error;
    where:
    Error = RPM_SP - RPM;
    Integr_Error=(1/ki)*Integr_Error*Ts + Integr_Error_n1;
    Where: Ts = sample time, Integr_Error_n1= Error integral accumulated
     */
    static int16_t li16Ctrl_Out, li16Error, li16Integr_Error, li16Integr_Error_n1 = 0;
    static uint16_t lu16ADC_Data;
    static uint8_t u8Ts_Inv = 10;

    g_adc0.p_api->read(g_adc0.p_ctrl, ADC_REG_CHANNEL_0, &lu16ADC_Data);
    u16RPM_SP= (uint16_t)((lu16ADC_Data * 3000)/982); // Convert data from ADC_10Bits(0-982) to rpm (0-3000)
    u16RPM = (uint16_t)(15 * u16Frec_Sensor_op2); // Conversion from Frec to RPM (Frec*60/4)
    u16RPM_Filtered = FN_Filter(u16RPM);
    li16Error = (int16_t)(u16RPM_SP - u16RPM_Filtered);
    li16Integr_Error = (int16_t)((li16Error/u8Ki) + li16Integr_Error_n1);
    li16Integr_Error_n1 = li16Integr_Error;
    li16Integr_Error = (int16_t)(li16Integr_Error / u8Ts_Inv);// Integral multiplied x 0.1 ms (Sample time)
    if(li16Integr_Error > 100) li16Integr_Error = 100;
    if(li16Integr_Error < 0) li16Integr_Error = 0;
    li16Ctrl_Out = (int16_t)((li16Error/u8Kp) + li16Integr_Error);
    if(li16Ctrl_Out > 100) li16Ctrl_Out = 100;
    if(li16Ctrl_Out < 0) li16Ctrl_Out = 0;
    u16DutyCycle_toPlot = (uint16_t)(li16Ctrl_Out * 30); //Duty Cycle to plot on debugging with RPMs
    u16PwmPercent = (uint16_t) (100 - li16Ctrl_Out);  // Driver has reverse logical
    g_timer2.p_api->dutyCycleSet(g_timer2.p_ctrl, u16PwmPercent, TIMER_PWM_UNIT_PERCENT, 0); //Send the result of the Control to the Motor Driver
    au16Send_DataToLCD[0] = (uint16_t)li16Ctrl_Out; //Duty Cycle to be sent to LCD_Thread
    au16Send_DataToLCD[1] = u16RPM_Filtered;                //RPM to be sent to LCD_Thread
}

void SR_Motor_Status(void)
{
    static ioport_level_t lu1P05_status, lu1P06_status;
    static uint8_t lu8Mot_status=0;

    /******      READ INPUTS    ******/
    g_ioport.p_api->pinRead(IOPORT_PORT_00_PIN_05, &lu1P05_status);
    g_ioport.p_api->pinRead(IOPORT_PORT_00_PIN_06, &lu1P06_status);
    switch(lu8Mot_status)
      {
          case 0:
                if(lu1P05_status == BTN_PRESSED && lu1P06_status == BTN_PRESSED){
                    g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_00, LED_ON);
                    g_ioport.p_api->pinWrite(IOPORT_PORT_04_PIN_12, MOTOR_ON); //Enable Motor
                    lu8Mot_status=1;
                    tx_thread_sleep(100);
                }

                else {
                    g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_00, LED_OFF);
                    g_ioport.p_api->pinWrite(IOPORT_PORT_04_PIN_12, MOTOR_OFF);  //Disable Motor
                    lu8Mot_status=0;
                 }
                break;

          case 1:
              if(lu1P05_status == BTN_RELEASED && lu1P06_status == BTN_RELEASED){
                      g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_00, LED_ON);
                      g_ioport.p_api->pinWrite(IOPORT_PORT_04_PIN_12, MOTOR_ON);  //Enable Motor
                      lu8Mot_status=1;
              }
              else {
                  g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_00, LED_OFF);
                  g_ioport.p_api->pinWrite(IOPORT_PORT_04_PIN_12, MOTOR_OFF); //Disable Motor
                  lu8Mot_status=0;
              }
              break;
          default:
              lu8Mot_status=0;
              break;
      }
}

uint16_t FN_Filter(uint16_t lu16Value){

    if(C_FILTER_ORDER > 1)
        {
        u16Value_Filtered = (uint16_t)((lu16Value + (C_FILTER_ORDER - 1) * u16Value_Filtered) / C_FILTER_ORDER);
        }
    else
        {
        u16Value_Filtered = lu16Value;
        }
    return u16Value_Filtered;
}
