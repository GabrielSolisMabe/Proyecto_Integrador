#include "lcd_thread.h"

/* LCD Thread entry function */
void lcd_thread_entry(void)
{
    /* TODO: add your own code here */
    while (1)
    {
        gx_system_start();
        g_spi_lcdc.p_api->open(g_spi_lcdc.p_ctrl, g_spi_lcdc.p_cfg);

        tx_thread_sleep (1);
    }
}
