#include <system_thread.h>


#define FREC_CLK_TIMER (120000000U) // Frecuency of the timer =  240 MHz

uint16_t u16ADC_Data;


void system_thread_entry(void)
{

    g_adc0.p_api->open(g_adc0.p_ctrl, g_adc0.p_cfg);
    g_adc0.p_api->scanCfg(g_adc0.p_ctrl, g_adc0.p_channel_cfg);
    g_adc0.p_api->scanStart(g_adc0.p_ctrl);
    g_adc0.p_api->read(g_adc0.p_ctrl, ADC_REG_CHANNEL_0, &u16ADC_Data);
}



