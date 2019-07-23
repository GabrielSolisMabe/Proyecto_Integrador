#include <lcd_thread.h>
#include <system_thread.h>
#include "gx_api.h"
#include "gui/gui_adc_specifications.h"
#include "gui/gui_adc_resources.h"
#include "lcd_setup/lcd.h"

GX_WINDOW_ROOT * p_window_root;
extern GX_CONST GX_STUDIO_WIDGET * gui_adc_widget_table[];

/* LCD Thread entry function */
void lcd_thread_entry(void)
{
    /* Initializes GUIX. */
    gx_system_initialize();
    /* Initializes GUIX drivers. */
    g_sf_el_gx.p_api->open(g_sf_el_gx.p_ctrl, g_sf_el_gx.p_cfg);

    gx_studio_display_configure(DISPLAY_1,
                                    g_sf_el_gx.p_api->setup,
                                    LANGUAGE_ENGLISH,
                                    DISPLAY_1_THEME_1,
                                    &p_window_root);

    g_sf_el_gx.p_api->canvasInit(g_sf_el_gx.p_ctrl, p_window_root);

    GX_CONST GX_STUDIO_WIDGET ** pp_studio_widget = &gui_adc_widget_table[0];
        GX_WIDGET * p_first_screen = NULL;

        while (GX_NULL != *pp_studio_widget)
        {

            if (0 == strcmp("window1", (char *) (*pp_studio_widget)->widget_name))
            {
                gx_studio_named_widget_create((*pp_studio_widget)->widget_name, (GX_WIDGET *) p_window_root, GX_NULL);
            }
            else
            {
                gx_studio_named_widget_create((*pp_studio_widget)->widget_name, GX_NULL, GX_NULL);
            }

            pp_studio_widget++;
        }

        gx_widget_attach(p_window_root, p_first_screen);
        gx_widget_show(p_window_root);

    /* Lets GUIX run. */
    gx_system_start();

    /** Open the SPI driver to initialize the LCD (SK-S7G2) **/
    g_spi_lcdc.p_api->open(g_spi_lcdc.p_ctrl, g_spi_lcdc.p_cfg);

    /** Setup the ILI9341V (SK-S7G2) **/
    ILI9341V_Init();

    while (1)
    {
        gx_prompt_text_set(&window1.window1_prompt, "10");
        tx_thread_sleep (1);
    }
}

//Comunicacion de pantalla
void g_lcd_spi_callback (spi_callback_args_t * p_args)
{
    if (p_args->event == SPI_EVENT_TRANSFER_COMPLETE)
    {
       tx_semaphore_ceiling_put(&g_main_semaphore_lcdc, 1);
    }
}
