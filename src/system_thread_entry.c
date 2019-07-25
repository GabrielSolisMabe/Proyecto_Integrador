#include <system_thread.h>

#define FREC_CLK_TIMER (120000000U) // Frecuency of the timer =  120 MHz
#define BTN_PRESSED IOPORT_LEVEL_LOW
#define BTN_RELEASED IOPORT_LEVEL_HIGH
#define LED_ON IOPORT_LEVEL_LOW
#define LED_OFF IOPORT_LEVEL_HIGH
#define MOTOR_ON IOPORT_LEVEL_HIGH
#define MOTOR_OFF IOPORT_LEVEL_LOW
#define C_FILTER_ORDER  8

uint16_t u16ADC_Data, PwmPercent=50;

uint16_t Frec = 1000, Frec_read = 0, Frec_read2 = 0, Frec_n = 0, Frec_n1 = 0;
uint8_t Pulses = 0;
uint32_t counts = 0, counts_avg[4]={0};
ioport_level_t Pin=0;

int16_t error, ie, ie_n1=0, Ctrl_Out, Ctrl_Plot;
uint16_t rpm_sp=1000,rpm=0;
//uint8_t kp=32, ki=16; // gains for T=10ms
uint8_t kp=64, ki=32; // gains for T=100ms

uint16_t Value_Filtered, Value_Filtered2, rpm_Filtered, rpm_Filtered2;


void control_motor(void);
void Motor_status(void);
uint16_t Filtro(uint16_t valor);
uint16_t FiltroPB(uint16_t Value);

void system_thread_entry(void)
{

    /******   CONFIGURING THE ADC    ******/
    g_adc0.p_api->open(g_adc0.p_ctrl, g_adc0.p_cfg);
    g_adc0.p_api->scanCfg(g_adc0.p_ctrl, g_adc0.p_channel_cfg);
    g_adc0.p_api->scanStart(g_adc0.p_ctrl);


    /******   CONFIGURING TIMERS AND IRQ    ******/
    g_timer0.p_api->open(g_timer0.p_ctrl, g_timer0.p_cfg); //Timer 0 to measure the period of the sensor
    g_timer1.p_api->open(g_timer1.p_ctrl, g_timer1.p_cfg); //Timer 1 to count pulses per every 100 ms
    g_timer2.p_api->open(g_timer2.p_ctrl, g_timer2.p_cfg); //Timer 2 to generate the PWM at 10 KHz
    g_external_irq5.p_api->open(g_external_irq5.p_ctrl, g_external_irq5.p_cfg); //Int 1 to read the counts of timer 1

    g_timer2.p_api->periodSet(g_timer2.p_ctrl, Frec, TIMER_UNIT_FREQUENCY_HZ); //used to change the frec manually


    /******   CONFIGURING THE ADC    ******/
    g_timer0.p_api->start(g_timer0.p_ctrl);
    g_timer1.p_api->start(g_timer1.p_ctrl);
    g_timer2.p_api->start(g_timer2.p_ctrl);

    /******      TURNING OFF THE LEDs    ******/
    g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_00, LED_OFF);
    g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_01, LED_OFF);
    g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_02, LED_OFF);

    while(1){
        Motor_status();
        //g_ioport.p_api->pinWrite(IOPORT_PORT_01_PIN_14, IOPORT_LEVEL_HIGH); //Pin used to check algorithm time
        //Pin = !Pin; g_ioport.p_api->pinWrite(IOPORT_PORT_01_PIN_14, Pin); //Pin used to check the sample time
        control_motor();
        //PwmPercent= (uint16_t)((u16ADC_Data * 100)/982); // Convert data from ADC(0-982) to Duty_cycle (0-100)
        // g_timer2.p_api->dutyCycleSet(g_timer2.p_ctrl, PwmPercent, TIMER_PWM_UNIT_PERCENT, 0); //used to change the dutycycle manually
        //g_ioport.p_api->pinWrite(IOPORT_PORT_01_PIN_14, IOPORT_LEVEL_LOW); //Pin used to check algorithm time
        tx_thread_sleep(10);
        }
}


void external_irq5_callback(external_irq_callback_args_t *p_args)
{
    SSP_PARAMETER_NOT_USED(p_args);
    Pin = !Pin; g_ioport.p_api->pinWrite(IOPORT_PORT_01_PIN_14, Pin); //Pin used to check the sample time

    g_timer0.p_api->counterGet(g_timer0.p_ctrl, &counts); // read the period clocks
    g_timer0.p_api->reset(g_timer0.p_ctrl); // restart the Timer for a new period measurement
    counts_avg[0] = (counts + counts_avg[1] + counts_avg[2] + counts_avg[3])/4; // Counts average of the last 4 measurements
    Frec_read = FREC_CLK_TIMER/counts_avg[0]; // conversion of time to frec
    counts_avg[3] = counts_avg[2];
    counts_avg[2] = counts_avg[1];
    counts_avg[1] = counts;
    Pulses++;

}


void timer1_callback(timer_callback_args_t * p_args)
{

    if (TIMER_EVENT_EXPIRED  == p_args->event)
    {
        Pin = !Pin; g_ioport.p_api->pinWrite(IOPORT_PORT_01_PIN_14, Pin); //Pin used to check the sample time
        Frec_n = (uint16_t)(Pulses *10);
        Frec_read2 = (uint16_t)((Frec_n + Frec_n1)/2);
        Frec_n1 = Frec_n;
        Pulses = 0; // Pulses every 100 ms
    }
}

void control_motor(void){

    /********** PI Control formula **********
    Ctrl_Out= (1/kp)*e + ie; (ie= error integral)
    where:
    e = rpm_sp - rpm;
    ie=(1/ki)*ie*T + ie_n1; (T = sample time, ie_n1= error integral accumulated)
    Where T= 1/100= 10 ms;
     */


    g_adc0.p_api->read(g_adc0.p_ctrl, ADC_REG_CHANNEL_0, &u16ADC_Data);
    rpm_sp= (uint16_t)((u16ADC_Data * 3000)/982); // Convert data from ADC_10Bits(0-982) to rpm (0-3000)
    rpm = (uint16_t)(15 * Frec_read2); // conversion of frec to rpm (frec*60/4)
    rpm_Filtered = Filtro(rpm);
    //rpm_Filtered2 = FiltroPB(rpm);
    error = (int16_t)(rpm_sp - rpm_Filtered);
    //ie = (int16_t)((ki * error) + ie_n1); // For Gains greater than 1
    ie = (int16_t)((error/ki) + ie_n1);  // For Gains smaller than 1
    ie_n1 = ie;
    //ie = ie / 100;// T for 10ms
    ie = ie / 10;// T for 100ms
    if(ie > 100) ie = 100;
    if(ie < 0) ie = 0;
    //Ctrl_Out = (int16_t)((kp * error) + ie); // For Gains greater than 1
    Ctrl_Out = (int16_t)((error/kp) + ie); // For Gains smaller than 1
    if(Ctrl_Out > 100) Ctrl_Out = 100;
    if(Ctrl_Out < 10) Ctrl_Out = 10;
    Ctrl_Plot = (int16_t)(Ctrl_Out * 30);
    PwmPercent = (uint16_t) (100 - Ctrl_Out);  // Inverse logic
    g_timer2.p_api->dutyCycleSet(g_timer2.p_ctrl, PwmPercent, TIMER_PWM_UNIT_PERCENT, 0); //Send the result of the Control to the Motor Driver
}

void Motor_status(void)
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
                    g_ioport.p_api->pinWrite(IOPORT_PORT_04_PIN_12, MOTOR_ON); //Pin used to enable the Motor
                    Mot_status=1;
                    tx_thread_sleep(100);
                }

                else {
                    g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_00, LED_OFF);
                    g_ioport.p_api->pinWrite(IOPORT_PORT_04_PIN_12, MOTOR_OFF); //Pin used to enable the Motor
                    Mot_status=0;
                 }
                break;

          case 1:
              if(P05_status == BTN_RELEASED && P06_status == BTN_RELEASED){
                      g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_00, LED_ON);
                      g_ioport.p_api->pinWrite(IOPORT_PORT_04_PIN_12, MOTOR_ON); //Pin used to enable the Motor
                      Mot_status=1;
              }
              else {
                  g_ioport.p_api->pinWrite(IOPORT_PORT_06_PIN_00, LED_OFF);
                  g_ioport.p_api->pinWrite(IOPORT_PORT_04_PIN_12, MOTOR_OFF); //Pin used to enable the Motor
                  Mot_status=0;
              }
              break;
          default:
              Mot_status=0;
              break;
      }
}

uint16_t Filtro(uint16_t Value){

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

uint16_t FiltroPB(uint16_t Value){ //Filtro Pasa Bajas Fc=0.5Hz

    Value_Filtered2 = (uint16_t)((Value_Filtered2 + (3 * Value))/4);

              return Value_Filtered2;
}
