#include <lcd_thread.h>
#include <system_thread.h>
#include "gx_api.h"
#include "gui/gui_adc_specifications.h"
#include "gui/gui_adc_resources.h"
#include "lcd_setup/lcd.h"

GX_WINDOW_ROOT * psWindowRoot = NULL;
extern GX_CONST GX_STUDIO_WIDGET * gui_adc_widget_table[];
GX_CONST GX_STUDIO_WIDGET ** ppsStudioWidget = &gui_adc_widget_table[0];//global
uint16_t au16ReceiveBuffer[2] = {0};
GX_VALUE i16ReceiveBuffer360;
GX_VALUE i16ReceiveBufferRpm;

/* Subrutines */
void SR_Config(void);
void SR_CreateWidgets(void);
void SR_UpdateLcd(void);

/* LCD Thread entry function */
void lcd_thread_entry(void)     {
    /* Initializes GUIX. */
    gx_system_initialize();

    /* Initializes GUIX drivers. */
    g_sf_el_gx.p_api->open(g_sf_el_gx.p_ctrl, g_sf_el_gx.p_cfg);

    /* Lets GUIX run. */
    gx_system_start();

    /* Open the SPI driver to initialize the LCD (SK-S7G2) */
    g_spi_lcdc.p_api->open(g_spi_lcdc.p_ctrl, g_spi_lcdc.p_cfg);

    /* Function to configure the display */
    SR_Config();

    /* Setup the ILI9341V (SK-S7G2) */
    ILI9341V_Init();

    while (1)       {
        tx_thread_sleep (10);
        /* Receive queue message from system thread */
        //tx_queue_flush(&Message_Queue);
        tx_queue_receive(&Message_Queue, au16ReceiveBuffer, TX_WAIT_FOREVER);

        /* Assign data to send to the widgets */
        SR_UpdateLcd();

        tx_thread_sleep(10);
    }
}

/* Display irq */
void g_lcd_spi_callback (spi_callback_args_t * p_args)      {
    if (p_args->event == SPI_EVENT_TRANSFER_COMPLETE)
    {   tx_semaphore_ceiling_put(&g_main_semaphore_lcdc, 1);    }
}

void SR_Config()        {
    gx_studio_display_configure(DISPLAY_1,
                                        g_sf_el_gx.p_api->setup,
                                        LANGUAGE_ENGLISH,
                                        DISPLAY_1_THEME_1,
                                        &psWindowRoot);

        g_sf_el_gx.p_api->canvasInit(g_sf_el_gx.p_ctrl, psWindowRoot);

        GX_WIDGET * lpsFirstScreen = NULL;

        /* Function to create the guix widgets */
        SR_CreateWidgets();

        gx_widget_attach(psWindowRoot, lpsFirstScreen);
        gx_widget_show(psWindowRoot);
}

void SR_CreateWidgets()     {
    while (GX_NULL != *ppsStudioWidget)
                {
                    if (0 == strcmp("window1", (char *) (*ppsStudioWidget)->widget_name))
                    {  gx_studio_named_widget_create((*ppsStudioWidget)->widget_name, (GX_WIDGET *) psWindowRoot, GX_NULL); }

                    else
                    { gx_studio_named_widget_create((*ppsStudioWidget)->widget_name, GX_NULL, GX_NULL);     }

                    ppsStudioWidget++;     }
}

void SR_UpdateLcd()     {
    char lu8Text[8];

            //Convert data type to the required by the prompt
            gx_utility_ltoa((LONG) au16ReceiveBuffer[1], lu8Text, 8);

            //Set the new value to the prompt
            gx_prompt_text_set(&window1.window1_prompt_1, lu8Text);

            //Convert data type to the required by the radial bar, and to degrees
            LONG i32CastBuff = ((LONG)(au16ReceiveBuffer[0]*-360)/100);
            i16ReceiveBuffer360 = (GX_VALUE)(i32CastBuff);//SIGNED SHORT [−32,767, +32,767] - UNSIGNED INT 16 [0, 65536]
            i16ReceiveBufferRpm = (GX_VALUE)((LONG)(au16ReceiveBuffer[1]*-360/3000));

            //Set the value to the radial bar
            gx_radial_progress_bar_value_set(&window1.window1_radial_progress_bar, i16ReceiveBuffer360);
            gx_radial_progress_bar_value_set(&window1.window1_radial_progress_bar_1, i16ReceiveBufferRpm);

            //Refresh widgets
            gx_system_dirty_mark((GX_WIDGET *) &window1.window1_prompt_1);
            gx_system_dirty_mark((GX_WIDGET *) &window1.window1_radial_progress_bar);
            gx_system_dirty_mark((GX_WIDGET *) &window1.window1_radial_progress_bar_1);
            gx_system_canvas_refresh();
}
