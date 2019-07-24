#include <system_thread.h>

#define FREC_CLK_TIMER (120000000U) // Frecuency of the timer =  120 MHz


uint16_t u16ADC_Data, PwmPercent=50;

uint16_t Frec = 1000, Frec_read = 0, Frec_read2 = 0, Frec_n = 0, Frec_n1 = 0;
uint8_t Pulses = 0;
uint32_t counts = 0, counts_avg[4]={0};
ioport_level_t Pin=0;

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

    g_timer0.p_api->start(g_timer0.p_ctrl);
    g_timer1.p_api->start(g_timer1.p_ctrl);
    g_timer2.p_api->start(g_timer2.p_ctrl);

    while(1){
     g_adc0.p_api->read(g_adc0.p_ctrl, ADC_REG_CHANNEL_0, &u16ADC_Data);
     PwmPercent= (uint16_t)((u16ADC_Data * 100)/982); // Convert data from ADC(0-982) to Duty_cycle (0-100)
     g_timer2.p_api->dutyCycleSet(g_timer2.p_ctrl, PwmPercent, TIMER_PWM_UNIT_PERCENT, 0); //used to change the dutycycle manually
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


