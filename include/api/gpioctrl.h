#ifndef GPIOCTRL_H__
#define GPIOCTRL_H__

#include <api/gpio.h>

typedef enum gpioctrl_index {
    /* write pin */
    DSP_RESET = 0x00,  /* active low */
    RELAY_IO_PORT1,     /* active low */
    RELAY_IO_PORT2,     /* active low */
    EQ_OR_CODEC_BYPASS, /* low: earth quake, high: codec */
    AMP485_ENABLE,  /* active high */
    EXT_AMP_PWR,    /* active low */
    MONSPK_MUTE,    /* active low */
    MONSPK_RESET,   /* active low */
    SIREN_BULB_PWR, /* active low */
    LCD_PWR_RESET,  /* active low */
    LCD_BACKLIGHT,  /* active high */
    TOUCH_IC_RESET,     /* not used */
    LVDS_DRV_ENABLE,    /* not used */
    LTE_MODEM_RESET,    /* active low */
    WDOG_MODE,
    WDOG_STROBE,
    FM_MODULE_CH_SEL,   /* low: ch1, high: ch2*/
    FM_MODULE_RESET,    /* active low */
    I2C_SWITCH_RESET,   /* active low */
    USB_TO_LAN1_RESET,  /* active low */
    USB_TO_LAN2_RESET,  /* active low */
    DMB_MODULE_CH1_EN,  /* active high */
    DMB_MODULE_CH2_EN,  /* active high */
    USB_TO_UART_RESET,  /* active low */
    USB_TO_UART_TERM_RESET, /* not used, opened */

    /* read pin */
    DSP_CLK_FB,        /* clk(high & low) */
    DSP_ACT_STATUS_FB,     /* low: dsp running */
    DSP_SB_FINISH_FB,      /* low: dsp self boot ok */
    DSP_SEL_AUDIO_IN_FB,   /* low: detect signal */
    DSP_SAT_AUDIO_IN_FB,   /* low: detect signal */
    DSP_SEL_AUDIO_OUT_FB,  /* low: detect signal */
    DSP_AMP_AUDIO_OUT_FB,  /* low: detect signal */
    AMP_PWR_FB,     /* low: amp power on */
    AMP_PWR_SWITCH_FB,  /* low: amp power switch on */
    SIREN_BULB_PWR_FB,  /* low: bulb power on */
    RELAY_IO_PORT1_FB,  /* low: relay on for active */
    RELAY_IO_PORT2_FB,  /* low: relay on for active */
    MONSPK_ERR_FB,
    EQ_OR_CODEC_BYPASS_FB,  /*low: eq, high: codec */
    EQ_FB,      /* low: detect */
    SPARE_IN_FB,    /*low: detect */
    AMP_PWR_RELAY_FB,   /* low: detect */
    AMP_AC220V_FB,      /* low: detect */
    SPARE_IN1_FB,
    SPARE_IN2_FB,
    LTE_MODEM_PWR_FB,       /* low: power off */
    LTE_MODEM_STATUS_FB,    /* low: no serivce, high: contect */
    FM_MODULE_FB,           /* low: contect */
} gpioctrl_idx_t;

void gpioctrl_init(void);
int gpioctrl_wrtie(gpioctrl_idx_t idx, unsigned int signal);
int gpioctrl_read(gpioctrl_idx_t idx);

#endif