#include <api/debug.h>
#include <api/gpio.h>
#include <api/gpioctrl.h>

static gpio_t gpio = {
    .dev = PATH_DEV_GPIO,
    .fd = -1,
};

static int gd_dsp_reset(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT6;
    ctrl.pin = GPIO_PIN16;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_relay_io_port1(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN26;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_relay_io_port2(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN9;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_eq_or_codec_bypass(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN18;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_amp485_enable(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT5;
    ctrl.pin = GPIO_PIN9;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_ext_amp_pwr(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN11;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_monspk_mute(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN29;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_monspk_reset(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN28;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_siren_bulb_pwr(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN27;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_lcd_pwr_reset(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT5;
    ctrl.pin = GPIO_PIN7;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_lcd_backlight(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT5;
    ctrl.pin = GPIO_PIN8;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_touch_ic_reset(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN31;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_lvds_drv_enable(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT5;
    ctrl.pin = GPIO_PIN5;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_lte_modem_reset(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN20;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_wdog_mode(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN6;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_wdog_strobe(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN1;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_fm_module_ch_sel(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT6;
    ctrl.pin = GPIO_PIN31;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_fm_module_reset(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN3;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_i2c_switch_reset(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN30;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_usb_to_lan1_reset(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT3;
    ctrl.pin = GPIO_PIN7;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_usb_to_lan2_reset(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT2;
    ctrl.pin = GPIO_PIN9;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_dmb_module_ch1_en(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN15;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_dmb_module_ch2_en(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT3;
    ctrl.pin = GPIO_PIN19;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_usb_to_uart_reset(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT3;
    ctrl.pin = GPIO_PIN8;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_usb_to_uart_term_reset(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN21;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_dsp_clk_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN28;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_dsp_act_status_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN29;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_dsp_sb_finish_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN30;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_dsp_sel_audio_in_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN13;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_dsp_sat_audio_in_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN4;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_dsp_sel_audio_out_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN15;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_dsp_amp_audio_out_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN14;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_amp_pwr_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN10;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_amp_pwr_switch_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN12;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_siren_bulb_pwr_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT2;
    ctrl.pin = GPIO_PIN14;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_relay_io_port1_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT2;
    ctrl.pin = GPIO_PIN8;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_relay_io_port2_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT2;
    ctrl.pin = GPIO_PIN9;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_monspk_err_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT2;
    ctrl.pin = GPIO_PIN10;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_eq_or_codec_bypass_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN16;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_eq_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN25;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_spare_in_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN24;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_amp_pwr_relay_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN23;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_amp_ac220v_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN22;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_spare_in1_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT2;
    ctrl.pin = GPIO_PIN11;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_spare_in2_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT2;
    ctrl.pin = GPIO_PIN12;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_lte_modem_pwr_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN5;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_lte_modem_status_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN16;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

static int gd_fm_module_fb(unsigned int direction)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT3;
    ctrl.pin = GPIO_PIN29;
    ctrl.data = direction;

    if (set_gpio_direction(&gpio, &ctrl) < 0)
        return -1;
    return 0;
}

int gpioctrl_direction(gpioctrl_idx_t idx, unsigned int direction)
{
    switch (idx) {
        /* write pins */
    case DSP_RESET:
        return gd_dsp_reset(direction);
    case RELAY_IO_PORT1:
        return gd_relay_io_port1(direction);
    case RELAY_IO_PORT2:
        return gd_relay_io_port2(direction);
    case EQ_OR_CODEC_BYPASS:
        return gd_eq_or_codec_bypass(direction);
    case AMP485_ENABLE:
        return gd_amp485_enable(direction);
    case EXT_AMP_PWR:
        return gd_ext_amp_pwr(direction);
    case MONSPK_MUTE:
        return gd_monspk_mute(direction);
    case MONSPK_RESET:
        return gd_monspk_reset(direction);
    case SIREN_BULB_PWR:
        return gd_siren_bulb_pwr(direction);
    case LCD_PWR_RESET:
        return gd_lcd_pwr_reset(direction);
    case LCD_BACKLIGHT:
        return gd_lcd_backlight(direction);
    case TOUCH_IC_RESET:
        return gd_touch_ic_reset(direction);
    case LVDS_DRV_ENABLE:
        return gd_lvds_drv_enable(direction);
    case LTE_MODEM_RESET:
        return gd_lte_modem_reset(direction);
    case WDOG_MODE:
        return gd_wdog_mode(direction);
    case WDOG_STROBE:
        return gd_wdog_strobe(direction);
    case FM_MODULE_CH_SEL:
        return gd_fm_module_ch_sel(direction);
    case FM_MODULE_RESET:
        return gd_fm_module_reset(direction);
    case I2C_SWITCH_RESET:
        return gd_i2c_switch_reset(direction);
    case USB_TO_LAN1_RESET:
        return gd_usb_to_lan1_reset(direction);
    case USB_TO_LAN2_RESET:
        return gd_usb_to_lan2_reset(direction);
    case DMB_MODULE_CH1_EN:
        return gd_dmb_module_ch1_en(direction);
    case DMB_MODULE_CH2_EN:
        return gd_dmb_module_ch2_en(direction);
    case USB_TO_UART_RESET:
        return gd_usb_to_uart_reset(direction);
    case USB_TO_UART_TERM_RESET:
        return gd_usb_to_uart_term_reset(direction);

        /* read pins */
    case DSP_CLK_FB:
        return gd_dsp_clk_fb(direction);
    case DSP_ACT_STATUS_FB:
        return gd_dsp_act_status_fb(direction);
    case DSP_SB_FINISH_FB:
        return gd_dsp_sb_finish_fb(direction);
    case DSP_SEL_AUDIO_IN_FB:
        return gd_dsp_sel_audio_in_fb(direction);
    case DSP_SAT_AUDIO_IN_FB:
        return gd_dsp_sat_audio_in_fb(direction);
    case DSP_SEL_AUDIO_OUT_FB:
        return gd_dsp_sel_audio_out_fb(direction);
    case DSP_AMP_AUDIO_OUT_FB:
        return gd_dsp_amp_audio_out_fb(direction);
    case AMP_PWR_FB:
        return gd_amp_pwr_fb(direction);
    case AMP_PWR_SWITCH_FB:
        return gd_amp_pwr_switch_fb(direction);
    case SIREN_BULB_PWR_FB:
        return gd_siren_bulb_pwr_fb(direction);
    case RELAY_IO_PORT1_FB:
        return gd_relay_io_port1_fb(direction);
    case RELAY_IO_PORT2_FB:
        return gd_relay_io_port2_fb(direction);
    case MONSPK_ERR_FB:
        return gd_monspk_err_fb(direction);
    case EQ_OR_CODEC_BYPASS_FB:
        return gd_eq_or_codec_bypass_fb(direction);
    case EQ_FB:
        return gd_eq_fb(direction);
    case SPARE_IN_FB:
        return gd_spare_in_fb(direction);
    case AMP_PWR_RELAY_FB:
        return gd_amp_pwr_relay_fb(direction);
    case AMP_AC220V_FB:
        return gd_amp_ac220v_fb(direction);
    case SPARE_IN1_FB:
        return gd_spare_in1_fb(direction);
    case SPARE_IN2_FB:
        return gd_spare_in2_fb(direction);
    case LTE_MODEM_PWR_FB:
        return gd_lte_modem_pwr_fb(direction);
    case LTE_MODEM_STATUS_FB:
        return gd_lte_modem_status_fb(direction);
    case FM_MODULE_FB:
        return gd_fm_module_fb(direction);
    default:
        dlp_warn("unknown gpio control index! (index: [%d])", idx);
        return -1;
    }
}

static int gw_dsp_reset(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT6;
    ctrl.pin = GPIO_PIN16;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_relay_io_port1(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN26;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_relay_io_port2(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN9;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_eq_or_codec_bypass(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN18;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_amp485_enable(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT5;
    ctrl.pin = GPIO_PIN9;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_ext_amp_pwr(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN11;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_monspk_mute(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN29;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_monspk_reset(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN28;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_siren_bulb_pwr(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN27;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_lcd_pwr_reset(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT5;
    ctrl.pin = GPIO_PIN7;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_lcd_backlight(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT5;
    ctrl.pin = GPIO_PIN8;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_touch_ic_reset(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN31;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_lvds_drv_enable(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT5;
    ctrl.pin = GPIO_PIN5;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_lte_modem_reset(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN20;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_wdog_mode(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN6;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_wdog_strobe(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN1;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_fm_module_ch_sel(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT6;
    ctrl.pin = GPIO_PIN31;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_fm_module_reset(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN3;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_i2c_switch_reset(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN30;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_usb_to_lan1_reset(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT3;
    ctrl.pin = GPIO_PIN7;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_usb_to_lan2_reset(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT2;
    ctrl.pin = GPIO_PIN9;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_dmb_module_ch1_en(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN15;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_dmb_module_ch2_en(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT3;
    ctrl.pin = GPIO_PIN19;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_usb_to_uart_reset(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT3;
    ctrl.pin = GPIO_PIN8;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

static int gw_usb_to_uart_term_reset(unsigned int signal)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN21;
    ctrl.data = signal;

    return write_gpio(&gpio, &ctrl);
}

int gpioctrl_write(gpioctrl_idx_t idx, unsigned int signal)
{
    switch (idx) {
        /* write pins */
    case DSP_RESET:
        return gw_dsp_reset(signal);
    case RELAY_IO_PORT1:
        return gw_relay_io_port1(signal);
    case RELAY_IO_PORT2:
        return gw_relay_io_port2(signal);
    case EQ_OR_CODEC_BYPASS:
        return gw_eq_or_codec_bypass(signal);
    case AMP485_ENABLE:
        return gw_amp485_enable(signal);
    case EXT_AMP_PWR:
        return gw_ext_amp_pwr(signal);
    case MONSPK_MUTE:
        return gw_monspk_mute(signal);
    case MONSPK_RESET:
        return gw_monspk_reset(signal);
    case SIREN_BULB_PWR:
        return gw_siren_bulb_pwr(signal);
    case LCD_PWR_RESET:
        return gw_lcd_pwr_reset(signal);
    case LCD_BACKLIGHT:
        return gw_lcd_backlight(signal);
    case TOUCH_IC_RESET:
        return gw_touch_ic_reset(signal);
    case LVDS_DRV_ENABLE:
        return gw_lvds_drv_enable(signal);
    case LTE_MODEM_RESET:
        return gw_lte_modem_reset(signal);
    case WDOG_MODE:
        return gw_wdog_mode(signal);
    case WDOG_STROBE:
        return gw_wdog_strobe(signal);
    case FM_MODULE_CH_SEL:
        return gw_fm_module_ch_sel(signal);
    case FM_MODULE_RESET:
        return gw_fm_module_reset(signal);
    case I2C_SWITCH_RESET:
        return gw_i2c_switch_reset(signal);
    case USB_TO_LAN1_RESET:
        return gw_usb_to_lan1_reset(signal);
    case USB_TO_LAN2_RESET:
        return gw_usb_to_lan2_reset(signal);
    case DMB_MODULE_CH1_EN:
        return gw_dmb_module_ch1_en(signal);
    case DMB_MODULE_CH2_EN:
        return gw_dmb_module_ch2_en(signal);
    case USB_TO_UART_RESET:
        return gw_usb_to_uart_reset(signal);
    case USB_TO_UART_TERM_RESET:
        return gw_usb_to_uart_term_reset(signal);
    default:
        dlp_warn("unknown gpio control index! (index: [%d])", idx);
        return -1;
    }
}

static int gr_dsp_clk_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN28;

    return read_gpio(&gpio, &ctrl);
}

static int gr_dsp_act_status_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN29;

    return read_gpio(&gpio, &ctrl);
}

static int gr_dsp_sb_finish_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN30;

    return read_gpio(&gpio, &ctrl);
}

static int gr_dsp_sel_audio_in_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN13;

    return read_gpio(&gpio, &ctrl);
}

static int gr_dsp_sat_audio_in_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN4;

    return read_gpio(&gpio, &ctrl);
}

static int gr_dsp_sel_audio_out_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN15;

    return read_gpio(&gpio, &ctrl);
}

static int gr_dsp_amp_audio_out_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN14;

    return read_gpio(&gpio, &ctrl);
}

static int gr_amp_pwr_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN10;

    return read_gpio(&gpio, &ctrl);
}

static int gr_amp_pwr_switch_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN12;

    return read_gpio(&gpio, &ctrl);
}

static int gr_siren_bulb_pwr_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT2;
    ctrl.pin = GPIO_PIN14;

    return read_gpio(&gpio, &ctrl);
}

static int gr_relay_io_port1_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT2;
    ctrl.pin = GPIO_PIN8;

    return read_gpio(&gpio, &ctrl);
}

static int gr_relay_io_port2_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT2;
    ctrl.pin = GPIO_PIN19;

    return read_gpio(&gpio, &ctrl);
}

static int gr_monspk_err_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT2;
    ctrl.pin = GPIO_PIN10;

    return read_gpio(&gpio, &ctrl);
}

static int gr_eq_or_codec_bypass_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN16;

    return read_gpio(&gpio, &ctrl);
}

static int gr_eq_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN25;

    return read_gpio(&gpio, &ctrl);
}

static int gr_spare_in_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN24;

    return read_gpio(&gpio, &ctrl);
}

static int gr_amp_pwr_relay_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN23;

    return read_gpio(&gpio, &ctrl);
}

static int gr_amp_ac220v_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT4;
    ctrl.pin = GPIO_PIN22;

    return read_gpio(&gpio, &ctrl);
}

static int gr_spare_in1_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT2;
    ctrl.pin = GPIO_PIN11;

    return read_gpio(&gpio, &ctrl);
}

static int gr_spare_in2_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT2;
    ctrl.pin = GPIO_PIN12;

    return read_gpio(&gpio, &ctrl);
}

static int gr_lte_modem_pwr_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN5;

    return read_gpio(&gpio, &ctrl);
}

static int gr_lte_modem_status_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT1;
    ctrl.pin = GPIO_PIN16;

    return read_gpio(&gpio, &ctrl);
}

static int gr_fm_module_fb(void)
{
    gpio_ctrl_t ctrl;

    ctrl.port = GPIO_PORT3;
    ctrl.pin = GPIO_PIN29;

    return read_gpio(&gpio, &ctrl);
}

int gpioctrl_read(gpioctrl_idx_t idx)
{
    switch (idx) {
        /* read pins */
    case DSP_CLK_FB:
        return gr_dsp_clk_fb();
    case DSP_ACT_STATUS_FB:
        return gr_dsp_act_status_fb();
    case DSP_SB_FINISH_FB:
        return gr_dsp_sb_finish_fb();
    case DSP_SEL_AUDIO_IN_FB:
        return gr_dsp_sel_audio_in_fb();
    case DSP_SAT_AUDIO_IN_FB:
        return gr_dsp_sat_audio_in_fb();
    case DSP_SEL_AUDIO_OUT_FB:
        return gr_dsp_sel_audio_out_fb();
    case DSP_AMP_AUDIO_OUT_FB:
        return gr_dsp_amp_audio_out_fb();
    case AMP_PWR_FB:
        return gr_amp_pwr_fb();
    case AMP_PWR_SWITCH_FB:
        return gr_amp_pwr_switch_fb();
    case SIREN_BULB_PWR_FB:
        return gr_siren_bulb_pwr_fb();
    case RELAY_IO_PORT1_FB:
        return gr_relay_io_port1_fb();
    case RELAY_IO_PORT2_FB:
        return gr_relay_io_port2_fb();
    case MONSPK_ERR_FB:
        return gr_monspk_err_fb();
    case EQ_OR_CODEC_BYPASS_FB:
        return gr_eq_or_codec_bypass_fb();
    case EQ_FB:
        return gr_eq_fb();
    case SPARE_IN_FB:
        return gr_spare_in_fb();
    case AMP_PWR_RELAY_FB:
        return gr_amp_pwr_relay_fb();
    case AMP_AC220V_FB:
        return gr_amp_ac220v_fb();
    case SPARE_IN1_FB:
        return gr_spare_in1_fb();
    case SPARE_IN2_FB:
        return gr_spare_in2_fb();
    case LTE_MODEM_PWR_FB:
        return gr_lte_modem_pwr_fb();
    case LTE_MODEM_STATUS_FB:
        return gr_lte_modem_status_fb();
    case FM_MODULE_FB:
        return gr_fm_module_fb();
    default:
        dlp_warn("unknown gpio control index! (index: [%d])", idx);
        return -1;
    }
}

static int gpioctrl_init_direction(void)
{
    /* write pins */
    if (gpioctrl_direction(DSP_RESET, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(RELAY_IO_PORT1, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(RELAY_IO_PORT2, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(EQ_OR_CODEC_BYPASS, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(AMP485_ENABLE, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(EXT_AMP_PWR, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(MONSPK_MUTE, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(MONSPK_RESET, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(SIREN_BULB_PWR, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(LCD_PWR_RESET, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(LCD_BACKLIGHT, GPIO_DIR_OUTPUT) < 0)
        return -1;
    /***
     * NOTE - touch ic reset 초기화 시 ic 먹통 이슈, 제어 X
     * if (gpioctrl_direction(TOUCH_IC_RESET, GPIO_DIR_OUTPUT) < 0)
     * return -1;
    */
    if (gpioctrl_direction(LVDS_DRV_ENABLE, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(LTE_MODEM_RESET, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(WDOG_MODE, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(WDOG_STROBE, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(FM_MODULE_CH_SEL, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(FM_MODULE_RESET, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(I2C_SWITCH_RESET, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(USB_TO_LAN1_RESET, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(USB_TO_LAN2_RESET, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(DMB_MODULE_CH1_EN, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(DMB_MODULE_CH2_EN, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(USB_TO_UART_RESET, GPIO_DIR_OUTPUT) < 0)
        return -1;
    if (gpioctrl_direction(USB_TO_UART_TERM_RESET, GPIO_DIR_OUTPUT) < 0)
        return -1;

    /* read pins */
    if (gpioctrl_direction(DSP_CLK_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(DSP_ACT_STATUS_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(DSP_SB_FINISH_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(DSP_SEL_AUDIO_IN_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(DSP_SAT_AUDIO_IN_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(DSP_SEL_AUDIO_OUT_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(DSP_AMP_AUDIO_OUT_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(AMP_PWR_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(AMP_PWR_SWITCH_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(SIREN_BULB_PWR_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(RELAY_IO_PORT1_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(RELAY_IO_PORT2_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(MONSPK_ERR_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(EQ_OR_CODEC_BYPASS_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(EQ_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(SPARE_IN_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(AMP_PWR_RELAY_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(AMP_AC220V_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(SPARE_IN1_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(SPARE_IN2_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(LTE_MODEM_PWR_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(LTE_MODEM_STATUS_FB, GPIO_DIR_INPUT) < 0)
        return -1;
    if (gpioctrl_direction(FM_MODULE_FB, GPIO_DIR_INPUT) < 0)
        return -1;
}

// TODO
// [ ] GPIO 초기화 후 초기 값 설정 필요
static void gpioctrl_init_value(void)
{
    dlp_dbg("write 1----------------------------");
    gpioctrl_write(DSP_RESET, GPIO_LOW);
    gpioctrl_write(RELAY_IO_PORT1, GPIO_LOW);
    gpioctrl_write(RELAY_IO_PORT2, GPIO_LOW);
    gpioctrl_write(EQ_OR_CODEC_BYPASS, GPIO_LOW);
    gpioctrl_write(AMP485_ENABLE, GPIO_LOW);
    gpioctrl_write(EXT_AMP_PWR, GPIO_LOW);
    gpioctrl_write(MONSPK_MUTE, GPIO_LOW);
    gpioctrl_write(MONSPK_RESET, GPIO_LOW);
    gpioctrl_write(SIREN_BULB_PWR, GPIO_LOW);
    dlp_dbg("read 1----------------------------");
    gpioctrl_read(DSP_CLK_FB);
    gpioctrl_read(DSP_ACT_STATUS_FB);
    gpioctrl_read(DSP_SB_FINISH_FB);
    gpioctrl_read(DSP_SEL_AUDIO_IN_FB);
    gpioctrl_read(DSP_SAT_AUDIO_IN_FB);
    gpioctrl_read(DSP_SEL_AUDIO_OUT_FB);
    gpioctrl_read(DSP_AMP_AUDIO_OUT_FB);
    gpioctrl_read(AMP_PWR_FB);
    gpioctrl_read(AMP_PWR_SWITCH_FB);
    gpioctrl_read(SIREN_BULB_PWR_FB);
    gpioctrl_read(RELAY_IO_PORT1_FB);
    gpioctrl_read(RELAY_IO_PORT2_FB);
    gpioctrl_read(MONSPK_ERR_FB);
    gpioctrl_read(EQ_OR_CODEC_BYPASS_FB);
    gpioctrl_read(EQ_FB);
    gpioctrl_read(SPARE_IN_FB);
    gpioctrl_read(AMP_PWR_RELAY_FB);
    gpioctrl_read(AMP_AC220V_FB);
    gpioctrl_read(SPARE_IN1_FB);
    gpioctrl_read(SPARE_IN2_FB);
    dlp_dbg("write 2----------------------------");
    gpioctrl_write(DSP_RESET, GPIO_HIGH);
    gpioctrl_write(RELAY_IO_PORT1, GPIO_HIGH);
    gpioctrl_write(RELAY_IO_PORT2, GPIO_HIGH);
    gpioctrl_write(EQ_OR_CODEC_BYPASS, GPIO_HIGH);
    gpioctrl_write(AMP485_ENABLE, GPIO_HIGH);
    gpioctrl_write(EXT_AMP_PWR, GPIO_HIGH);
    gpioctrl_write(MONSPK_MUTE, GPIO_HIGH);
    gpioctrl_write(MONSPK_RESET, GPIO_HIGH);
    gpioctrl_write(SIREN_BULB_PWR, GPIO_HIGH);
    dlp_dbg("read 2----------------------------");
    gpioctrl_read(DSP_CLK_FB);
    gpioctrl_read(DSP_ACT_STATUS_FB);
    gpioctrl_read(DSP_SB_FINISH_FB);
    gpioctrl_read(DSP_SEL_AUDIO_IN_FB);
    gpioctrl_read(DSP_SAT_AUDIO_IN_FB);
    gpioctrl_read(DSP_SEL_AUDIO_OUT_FB);
    gpioctrl_read(DSP_AMP_AUDIO_OUT_FB);
    gpioctrl_read(AMP_PWR_FB);
    gpioctrl_read(AMP_PWR_SWITCH_FB);
    gpioctrl_read(SIREN_BULB_PWR_FB);
    gpioctrl_read(RELAY_IO_PORT1_FB);
    gpioctrl_read(RELAY_IO_PORT2_FB);
    gpioctrl_read(MONSPK_ERR_FB);
    gpioctrl_read(EQ_OR_CODEC_BYPASS_FB);
    gpioctrl_read(EQ_FB);
    gpioctrl_read(SPARE_IN_FB);
    gpioctrl_read(AMP_PWR_RELAY_FB);
    gpioctrl_read(AMP_AC220V_FB);
    gpioctrl_read(SPARE_IN1_FB);
    gpioctrl_read(SPARE_IN2_FB);

    //gpioctrl_write(TOUCH_IC_RESET, GPIO_LOW);
    //usleep(10 * 1000);
    //gpioctrl_write(TOUCH_IC_RESET, GPIO_HIGH);
    gpioctrl_write(LVDS_DRV_ENABLE, GPIO_LOW);
    usleep(10 * 1000);
    gpioctrl_write(LVDS_DRV_ENABLE, GPIO_HIGH);
    gpioctrl_write(LCD_PWR_RESET, GPIO_LOW);
    usleep(10 * 1000);
    gpioctrl_write(LCD_PWR_RESET, GPIO_HIGH);

    gpioctrl_write(LCD_BACKLIGHT, GPIO_LOW);
    sleep(2);
    gpioctrl_write(LCD_BACKLIGHT, GPIO_HIGH);
}

void gpioctrl_init(void)
{
    if (open_gpio(&gpio) < 0) {
        dlp_err("not opend gpio device!");
        exit(EXIT_FAILURE);
    }
    if (gpioctrl_init_direction() < 0) {
        dlp_err("not set gpio direction!");
        exit(EXIT_FAILURE);
    }
    gpioctrl_init_value();
    dlp_note("initialized gpio!");
}