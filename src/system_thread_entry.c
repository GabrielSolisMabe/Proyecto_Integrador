#include <system_thread.h>

#define FREC_CLK_TIMER (120000000U) // Timer Frecuency =  120 MHz
#define BTN_PRESSED IOPORT_LEVEL_LOW
#define BTN_RELEASED IOPORT_LEVEL_HIGH
#define LED_ON IOPORT_LEVEL_LOW
#define LED_OFF IOPORT_LEVEL_HIGH
#define MOTOR_ON IOPORT_LEVEL_HIGH
#define MOTOR_OFF IOPORT_LEVEL_LOW
#define C_FILTER_ORDER  8

uint16_t PwmPercent=50;
uint16_t Frec_PWM = 1000, Frec_Sensor_op1 = 0, Frec_Sensor_op2 = 0;
uint8_t Pulses = 0;
ioport_level_t Pin=0;
uint16_t RPM_SP = 1000, RPM = 0, DutyCycle_toPlot;
uint8_t Kp=64, Ki=32; // Gains for T=100ms

uint16_t Value_Filtered, RPM_Filtered;

uint16_t Send_DataToLCD[2] = {0};

void Motor_Control(void);
void Motor_Status(void);
uint16_t Filter(uint16_t Value);

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

    g_timer2.p_api->periodSet(g_timer2.p_ctrl, Frec_PWM, TIMER_UNIT_FREQUENCY_HZ); //used to change the frec manually


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
        //Pin = !Pin; g_ioport.p_api->pinWrite(IOPORT_PORT_01_PIN_14, Pin); //Pin used to check the sample time
        Motor_Status();
        Motor_Control();
        //PwmPercent= (uint16_t)((u16ADC_Data * 100)/982); // Convert data from ADC(0-982) to Duty_cycle (0-100)
        // g_timer2.p_api->dutyCycleSet(g_timer2.p_ctrl, PwmPercent, TIMER_PWM_UNIT_PERCENT, 0); //used to change the dutycycle manually
        tx_queue_flush(&Message_Queue); //Clean Queue to send the latest data
        tx_queue_send(&Message_Queue, Send_DataToLCD, TX_NO_WAIT);// send data to LCD Thread
        //g_ioport.p_api->pinWrite(IOPORT_PORT_01_PIN_14, IOPORT_LEVEL_LOW); //Pin used to check algorithm time
        tx_thread_sleep(10);
        }
}


void external_irq5_callback(external_irq_callback_args_t *p_args)
{
    static uint32_t counts = 0, counts_avg[4]={0};

    SSP_PARAMETER_NOT_USED(p_args);
    g_timer0.p_api->counterGet(g_timer0.p_ctrl, &counts); // read the period clocks
    g_timer0.p_api->reset(g_timer0.p_ctrl); // restart the Timer for a new period measurement
    counts_avg[0] = (counts + counts_avg[1] + counts_avg[2] + counts_avg[3])/4; // Counts average of the last 4 measurements
    Frec_Sensor_op1 = FREC_CLK_TIMER/counts_avg[0]; // conversion of time to frec
    counts_avg[3] = counts_avg[2]; //Update previous values
    counts_avg[2] = counts_avg[1];
    counts_avg[1] = counts;
    Pulses++;

}


void timer1_callback(timer_callback_args_t * p_args)
{
    static uint16_t Frec_n = 0, Frec_n1 = 0;

    if (TIMER_EVENT_EXPIRED  == p_args->event)
    {
        Frec_n = (uint16_t)(Pulses *10);
        Frec_Sensor_op2 = (uint16_t)((Frec_n + Frec_n1)/2);
        Frec_n1 = Frec_n;
        Pulses = 0; // Pulses every 100 ms
    }
}

void Motor_Control(void){

    /********** PI Control formula **********
    Ctrl_Out= (1/kp)*e + ie; (ie= error integral)
    where:
    Error = RPM_SP - RPM;
    ie=(1/ki)*ie*T + ie_n1; (T = sample time, ie_n1= error integral accumulated)
    Where T= 1/10 = 100 ms;
     */
    static int16_t Ctrl_Out, Error, ie, ie_n1 = 0;
    static uint16_t u16ADC_Data;

    g_adc0.p_api->read(g_adc0.p_ctrl, ADC_REG_CHANNEL_0, &u16ADC_Data);
    RPM_SP= (uint16_t)((u16ADC_Data * 3000)/982); // Convert data from ADC_10Bits(0-982) to rpm (0-3000)
    RPM = (uint16_t)(15 * Frec_Sensor_op2); // Conversion from Frec to RPM (Frec*60/4)
    RPM_Filtered = Filter(RPM);
    Error = (int16_t)(RPM_SP - RPM_Filtered);
    ie = (int16_t)((Error/Ki) + ie_n1);
    ie_n1 = ie;
    ie = ie / 10;// Integral multiplied x 0.1 ms (Sample time)
    if(ie > 100) ie = 100;
    if(ie < 0) ie = 0;
    Ctrl_Out = (int16_t)((Error/Kp) + ie);
    if(Ctrl_Out > 100) Ctrl_Out = 100;
    if(Ctrl_Out < 0) Ctrl_Out = 0;
    DutyCycle_toPlot = (uint16_t)(Ctrl_Out * 30); //Duty Cycle to plot on debugging with RPMs
    PwmPercent = (uint16_t) (100 - Ctrl_Out);  // Driver has reverse logical
    g_timer2.p_api->dutyCycleSet(g_timer2.p_ctrl, PwmPercent, TIMER_PWM_UNIT_PERCENT, 0); //Send the result of the Control to the Motor Driver
    Send_DataToLCD[0] = (uint16_t)Ctrl_Out; //Duty Cycle to be sent to LCD_Thread
    Send_DataToLCD[1] = RPM;                //RPM to be sent to LCD_Thread
}

void Motor_Status(void)
{
    static ioport_level_t P05_status, P06_status;
    static uint8_t Mot_status=0;

    /******      READ INPUTS    ******/
    g_ioport.p_api->pinRead(IOPORT_PORT_00_PIN_05, &P05_status);
    g_ioport.p_api->pinRead(IOPORT_PORT_00_PIN_06, &P06_status);
    switch(Mot_status)
      {
          case 0:
                if(P05_status == BTN_PRESSED && P06_status == BTN_PRESSED){
                    g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_00, LED_ON);
                    g_ioport.p_api->pinWrite(IOPORT_PORT_04_PIN_12, MOTOR_ON); //Enable Motor
                    Mot_status=1;
                    tx_thread_sleep(100);
                }

                else {
                    g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_00, LED_OFF);
                    g_ioport.p_api->pinWrite(IOPORT_PORT_04_PIN_12, MOTOR_OFF);  //Disable Motor
                    Mot_status=0;
                 }
                break;

          case 1:
              if(P05_status == BTN_RELEASED && P06_status == BTN_RELEASED){
                      g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_00, LED_ON);
                      g_ioport.p_api->pinWrite(IOPORT_PORT_04_PIN_12, MOTOR_ON);  //Enable Motor
                      Mot_status=1;
              }
              else {
                  g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_00, LED_OFF);
                  g_ioport.p_api->pinWrite(IOPORT_PORT_04_PIN_12, MOTOR_OFF); //Disable Motor
                  Mot_status=0;
              }
              break;
          default:
              Mot_status=0;
              break;
      }
}

uint16_t Filter(uint16_t Value){

    if(C_FILTER_ORDER > 1)
        {
        Value_Filtered = (uint16_t)((Value + (C_FILTER_ORDER - 1) * Value_Filtered) / C_FILTER_ORDER);
        }
    else
        {
        Value_Filtered = Value;
        }
    return Value_Filtered;
}
