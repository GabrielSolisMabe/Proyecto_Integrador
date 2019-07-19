#include "lcd_thread.h"

/* LCD Thread entry function */
void lcd_thread_entry(void)
{
    /* TODO: add your own code here */
    while (1)
    {
        g_spi_lcdc.p_api->open(g_spi_lcdc.p_ctrl, g_spi_lcdc.p_cfg);

        tx_thread_sleep (1);
    }
}

void g_lcd_spi_callback (spi_callback_args_t * p_args)
{
    if (p_args->event == SPI_EVENT_TRANSFER_COMPLETE)
    {
    }
}
