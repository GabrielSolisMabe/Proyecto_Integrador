#include "lcd_thread.h"
#include "gx_api.h"


GX_WINDOW_ROOT * p_window_root;
/* LCD Thread entry function */
void lcd_thread_entry(void)
{
    /* Initializes GUIX. */
    gx_system_initialize();
    /* Initializes GUIX drivers. */
    g_sf_el_gx.p_api->open(g_sf_el_gx.p_ctrl, g_sf_el_gx.p_cfg);

    g_sf_el_gx.p_api->canvasInit(g_sf_el_gx.p_ctrl, p_window_root);

    /* Lets GUIX run. */
    gx_system_start();

    /** Open the SPI driver to initialize the LCD (SK-S7G2) **/
    g_spi_lcdc.p_api->open(g_spi_lcdc.p_ctrl, g_spi_lcdc.p_cfg);

    /** Setup the ILI9341V (SK-S7G2) **/
    ILI9341V_Init();

    while (1)
    {
        g_spi_lcdc.p_api->open(g_spi_lcdc.p_ctrl, g_spi_lcdc.p_cfg);

        tx_thread_sleep (1);
    }
}

//Comunicacion de pantalla
void g_lcd_spi_callback (spi_callback_args_t * p_args)
{
    if (p_args->event == SPI_EVENT_TRANSFER_COMPLETE)
    {
    }
}
