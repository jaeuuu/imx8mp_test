#include <api/adau1442.h>
#include <api/dsp.h>

static dsp_adapter_t *adapter;

static dsp_adapter_t *get_ptr_dsp_adapter(void)
{
    return adapter;
}

#ifdef DSP_ADAU1442

static double convert_to_dsp_vol(unsigned char vol)
{
}

int dsp_init(void)
{
    if (adau1442_register(adapter) < 0)
        return -1;
    return 0;
}

void set_dsp_input(enum dsp_input_path path, unsigned char ctrl)
{
    dsp_adapter_t *adp = get_ptr_dsp_adapter();

    if (!adp)
        return;

    switch (path) {
    case DSP_IN_ALL:
        adp->set_input_path(ADAU1442_INPUT_UHD, ctrl);
        adp->set_input_path(ADAU1442_INPUT_SPARE, ctrl);
        adp->set_input_path(ADAU1442_INPUT_LTESPK, ctrl);
        adp->set_input_path(ADAU1442_INPUT_FM, ctrl);
        adp->set_input_path(ADAU1442_INPUT_CPU, ctrl);
        adp->set_input_path(ADAU1442_INPUT_TTS, ctrl);
        break;
    case DSP_IN_UHD:
        adp->set_input_path(ADAU1442_INPUT_UHD, ctrl);
        adp->set_input_detect(ADAU1442_INPUT_UHD);
        break;
    case DSP_IN_SPARE:
        adp->set_input_path(ADAU1442_INPUT_SPARE, ctrl);
        adp->set_input_detect(ADAU1442_INPUT_SPARE);
        break;
    case DSP_IN_LTESPK:
        adp->set_input_path(ADAU1442_INPUT_LTESPK, ctrl);
        adp->set_input_detect(ADAU1442_INPUT_LTESPK);
        break;
    case DSP_IN_FM:
        adp->set_input_path(ADAU1442_INPUT_FM, ctrl);
        adp->set_input_detect(ADAU1442_INPUT_FM);
        break;
    case DSP_IN_CPU:
        adp->set_input_path(ADAU1442_INPUT_CPU, ctrl);
        adp->set_input_detect(ADAU1442_INPUT_CPU);
        break;
    case DSP_IN_TTS:
        adp->set_input_path(ADAU1442_INPUT_TTS, ctrl);
        adp->set_input_detect(ADAU1442_INPUT_TTS);
        break;
    default:
        return;
    }
}

void set_dsp_input_vol(enum dsp_input_path path, unsigned char vol)
{
    dsp_adapter_t *adp = get_ptr_dsp_adapter();
    double cvol = convert_to_dsp_vol(vol);

    if (!adp)
        return;

    switch (path) {
    case DSP_IN_ALL:
        adp->set_input_vol(ADAU1442_INPUT_UHD, cvol);
        adp->set_input_vol(ADAU1442_INPUT_SPARE, cvol);
        adp->set_input_vol(ADAU1442_INPUT_LTESPK, cvol);
        adp->set_input_vol(ADAU1442_INPUT_FM, cvol);
        adp->set_input_vol(ADAU1442_INPUT_CPU, cvol);
        adp->set_input_vol(ADAU1442_INPUT_TTS, cvol);
        break;
    case DSP_IN_UHD:
        adp->set_input_vol(ADAU1442_INPUT_UHD, cvol);
        break;
    case DSP_IN_SPARE:
        adp->set_input_vol(ADAU1442_INPUT_SPARE, cvol);
        break;
    case DSP_IN_LTESPK:
        adp->set_input_vol(ADAU1442_INPUT_LTESPK, cvol);
        break;
    case DSP_IN_FM:
        adp->set_input_vol(ADAU1442_INPUT_FM, cvol);
        break;
    case DSP_IN_CPU:
        adp->set_input_vol(ADAU1442_INPUT_CPU, cvol);
        break;
    case DSP_IN_TTS:
        adp->set_input_vol(ADAU1442_INPUT_TTS, cvol);
        break;
    default:
        return;
    }
}

void set_dsp_output(enum dsp_output_path path, unsigned char ctrl)
{
    dsp_adapter_t *adp = get_ptr_dsp_adapter();

    if (!adp)
        return;

    switch (path) {
    case DSP_OUT_ALL:   /* when situation is off*/
        adp->set_output_path(ADAU1442_OUTPUT_SPDIF, ctrl);
        adp->set_output_path(ADAU1442_OUTPUT_AMP, ctrl);
        adp->set_output_path(ADAU1442_OUTPUT_SPARE, ctrl);
        adp->set_output_path(ADAU1442_OUTPUT_LTEMIC, ctrl);
        adp->set_output_path(ADAU1442_OUTPUT_MOSPK, ctrl);
        break;
    case DSP_OUT_SPDIF:
        adp->set_output_path(ADAU1442_OUTPUT_SPDIF, ctrl);
        adp->set_output_detect(ADAU1442_OUTPUT_SPDIF);
        break;
    case DSP_OUT_AMP:
        adp->set_output_path(ADAU1442_OUTPUT_AMP, ctrl);
        adp->set_output_detect(ADAU1442_OUTPUT_AMP);
        break;
    case DSP_OUT_SPARE:
        adp->set_output_path(ADAU1442_OUTPUT_SPARE, ctrl);
        adp->set_output_detect(ADAU1442_OUTPUT_SPARE);
        break;
    case DSP_OUT_LTEMIC:
        adp->set_output_path(ADAU1442_OUTPUT_LTEMIC, ctrl);
        adp->set_output_detect(ADAU1442_OUTPUT_LTEMIC);
        break;
    case DSP_OUT_MONSPK:
        adp->set_output_path(ADAU1442_OUTPUT_MOSPK, ctrl);
        adp->set_output_detect(ADAU1442_OUTPUT_MOSPK);
        break;
    default:
        return;
    }
}

void set_dsp_output_vol(enum dsp_output_path path, unsigned char vol)
{
    dsp_adapter_t *adp = get_ptr_dsp_adapter();
    double cvol = convert_to_dsp_vol(vol);

    if (!adp)
        return;

    switch (path) {
    case DSP_OUT_ALL:
        adp->set_output_vol(ADAU1442_OUTPUT_SPDIF, cvol);
        adp->set_output_vol(ADAU1442_OUTPUT_AMP, cvol);
        adp->set_output_vol(ADAU1442_OUTPUT_SPARE, cvol);
        adp->set_output_vol(ADAU1442_OUTPUT_LTEMIC, cvol);
        adp->set_output_vol(ADAU1442_OUTPUT_MOSPK, cvol);
        break;
    case DSP_OUT_SPDIF:
        adp->set_output_vol(ADAU1442_OUTPUT_SPDIF, cvol);
        break;
    case DSP_OUT_AMP:
        adp->set_output_vol(ADAU1442_OUTPUT_AMP, cvol);
        break;
    case DSP_OUT_SPARE:
        adp->set_output_vol(ADAU1442_OUTPUT_SPARE, cvol);
        break;
    case DSP_OUT_LTEMIC:
        adp->set_output_vol(ADAU1442_OUTPUT_LTEMIC, cvol);
        break;
    case DSP_OUT_MONSPK:
        adp->set_output_vol(ADAU1442_OUTPUT_MOSPK, cvol);
        break;
    default:
        return;
    }
}

void set_dsp_input_on(enum dsp_input_path path, unsigned char vol)
{
    set_dsp_input_vol(path, vol);
    set_dsp_input(path, PATH_ON);
}

void set_dsp_input_off(void)
{
    set_dsp_input(DSP_IN_ALL, PATH_OFF);
}

void set_dsp_output_on(enum dsp_output_path path, unsigned char vol)
{
    set_dsp_output_vol(path, vol);
    set_dsp_output(path, PATH_ON);
}

void set_dsp_output_off(void)
{
    set_dsp_output(DSP_OUT_ALL, PATH_OFF);
}

#endif