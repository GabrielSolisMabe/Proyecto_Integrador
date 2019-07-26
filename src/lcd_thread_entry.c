#include <lcd_thread.h>
#include <system_thread.h>
#include "gx_api.h"
#include "gui/gui_adc_specifications.h"
#include "gui/gui_adc_resources.h"
#include "lcd_setup/lcd.h"

GX_WINDOW_ROOT * psWindowRoot;// = NULL
extern GX_CONST GX_STUDIO_WIDGET * gui_adc_widget_table[];
uint16_t au16ReceiveBuffer[2] = {0};
GX_VALUE i16ReceiveBuffer360;
GX_VALUE i16ReceiveBufferRpm;

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
                                    &psWindowRoot);

    g_sf_el_gx.p_api->canvasInit(g_sf_el_gx.p_ctrl, psWindowRoot);

    GX_CONST GX_STUDIO_WIDGET ** ppsStudioWidget = &gui_adc_widget_table[0];
    GX_WIDGET * psFirstScreen = NULL;

        while (GX_NULL != *ppsStudioWidget)
        {

            if (0 == strcmp("window1", (char *) (*ppsStudioWidget)->widget_name))
            {
                gx_studio_named_widget_create((*ppsStudioWidget)->widget_name, (GX_WIDGET *) psWindowRoot, GX_NULL);
            }
            else
            {
                gx_studio_named_widget_create((*ppsStudioWidget)->widget_name, GX_NULL, GX_NULL);
            }

            ppsStudioWidget++;
        }

        gx_widget_attach(psWindowRoot, psFirstScreen);
        gx_widget_show(psWindowRoot);

    /* Lets GUIX run. */
    gx_system_start();

    /** Open the SPI driver to initialize the LCD (SK-S7G2) **/
    g_spi_lcdc.p_api->open(g_spi_lcdc.p_ctrl, g_spi_lcdc.p_cfg);

    /** Setup the ILI9341V (SK-S7G2) **/
    ILI9341V_Init();

    while (1)
    {
        tx_thread_sleep (10);
        //Receive queue message from system thread
        tx_queue_receive(&Message_Queue, au16ReceiveBuffer, TX_WAIT_FOREVER);//upt Message_Queue

        //Assign data to send to the widgets
        char u8Text[8];

        //Convert data type to the required by the prompt
        gx_utility_ltoa((LONG) au16ReceiveBuffer[1], u8Text, 8);

        //Set the new value to the prompt
        gx_prompt_text_set(&window1.window1_prompt_1, u8Text);

        //Convert data type to the required by the radial bar, and to degrees
        i16ReceiveBuffer360 = (GX_VALUE)(((LONG)(au16ReceiveBuffer[0])*-360/100));//SIGNED SHORT [−32,767, +32,767] - UNSIGNED INT 16 [0, 65536]
        i16ReceiveBufferRpm = (GX_VALUE)(((LONG)(au16ReceiveBuffer[1])*-360/3000));

        //Set the value to the radial bar
        gx_radial_progress_bar_value_set(&window1.window1_radial_progress_bar, i16ReceiveBuffer360);
        gx_radial_progress_bar_value_set(&window1.window1_radial_progress_bar_1, i16ReceiveBufferRpm);

        //Refresh widgets
        gx_system_dirty_mark((GX_WIDGET *) &window1.window1_prompt_1);
        gx_system_dirty_mark((GX_WIDGET *) &window1.window1_radial_progress_bar);
        gx_system_dirty_mark((GX_WIDGET *) &window1.window1_radial_progress_bar_1);
        gx_system_canvas_refresh();

        tx_thread_sleep(10);
    }
}

//Display irq
void g_lcd_spi_callback (spi_callback_args_t * p_args)
{
    if (p_args->event == SPI_EVENT_TRANSFER_COMPLETE)
    {
       tx_semaphore_ceiling_put(&g_main_semaphore_lcdc, 1);
    }
}
