#ifndef DSP_H__
#define DSP_H__

#define DSP_ADAU1442

enum dsp_input_path {
    DSP_IN_ALL = 0x00,
    DSP_IN_UHD,
    DSP_IN_SPARE,
    DSP_IN_LTESPK,
    DSP_IN_FM,
    DSP_IN_CPU,
    DSP_IN_TTS,
};

enum dsp_output_path {
    DSP_OUT_ALL = 0x00,
    DSP_OUT_SPDIF,
    DSP_OUT_AMP,
    DSP_OUT_SPARE,
    DSP_OUT_LTEMIC,
    DSP_OUT_MONSPK,
};

#define PATH_OFF    0
#define PATH_ON     1

typedef struct st_dsp_adapter {
    int (*set_input_path)(char path, char ctrl);
    int (*set_input_vol)(char path, double vol);
    int (*set_input_detect)(char path);
    int (*set_output_path)(char path, char ctrl);
    int (*set_output_vol)(char path, double vol);
    int (*set_output_detect)(char path);
    void (*release)(void);
} dsp_adapter_t;

int dsp_init(void);
void set_dsp_input(enum dsp_input_path path, unsigned char ctrl);
void set_dsp_input_vol(enum dsp_input_path path, unsigned char vol);
void set_dsp_output(enum dsp_output_path path, unsigned char ctrl);
void set_dsp_output_vol(enum dsp_output_path path, unsigned char vol);
void set_dsp_input_on(enum dsp_input_path path, unsigned char vol);
void set_dsp_input_off(enum dsp_input_path path);
void set_dsp_output_on(enum dsp_output_path path, unsigned char vol);
void set_dsp_output_off(enum dsp_output_path path);

#endif