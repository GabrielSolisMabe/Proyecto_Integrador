#include <system_thread.h>



uint16_t u16ADC_Data, PwmPercent=50;
uint32_t frec=1000;


void system_thread_entry(void)
{

    g_adc0.p_api->open(g_adc0.p_ctrl, g_adc0.p_cfg);
    g_adc0.p_api->scanCfg(g_adc0.p_ctrl, g_adc0.p_channel_cfg);
    g_adc0.p_api->scanStart(g_adc0.p_ctrl);

    g_timer2.p_api->open(g_timer2.p_ctrl, g_timer2.p_cfg); //Timer 2 to generate the PWM at 10 KHz
    g_timer2.p_api->periodSet(g_timer2.p_ctrl, frec, TIMER_UNIT_FREQUENCY_HZ); //used to change the frec manually
    g_timer2.p_api->start(g_timer2.p_ctrl);

    while(1){
     g_adc0.p_api->read(g_adc0.p_ctrl, ADC_REG_CHANNEL_0, &u16ADC_Data);
     PwmPercent= (uint16_t)((u16ADC_Data * 100)/982); // Convert data from ADC(0-982) to Duty_cycle (0-100)
     g_timer2.p_api->dutyCycleSet(g_timer2.p_ctrl, PwmPercent, TIMER_PWM_UNIT_PERCENT, 0); //used to change the dutycycle manually
     tx_thread_sleep(10);
     }
}
