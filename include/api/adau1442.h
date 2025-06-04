#ifndef ADAU1442_H__
#define ADAU1442_H__

#include <api/dsp.h>

#define ADAU1442_INPUT_UHD	    0
#define ADAU1442_INPUT_SPARE    1
#define ADAU1442_INPUT_LTESPK   2
#define ADAU1442_INPUT_FM	    3
#define ADAU1442_INPUT_CPU	    4
#define ADAU1442_INPUT_TTS	    5

#define ADAU1442_OUTPUT_SPDIF       0
#define ADAU1442_OUTPUT_AMP	        1
#define ADAU1442_OUTPUT_SPARE       2
#define ADAU1442_OUTPUT_LTEMIC	    3
#define ADAU1442_OUTPUT_MOSPK       4
#define ADAU1442_OUTPUT_20W_AUDIO	5

#define ADAU_SELNCEQ_BYPASS	0
#define ADAU_SELNCEQ_NC	    1
#define ADAU_SELNCEQ_EQ	    2 

#define ADAU_EQSEL_NCEQ	    0
#define ADAU_EQSEL_EQ	    1

#define ADAU_EQOUT_BYPASS	0
#define ADAU_EQOUT_NC		1
#define ADAU_EQOUT_EQ		2

#define ADAU_OUTSEL_DIS_DSP	0
#define ADAU_OUTSEL_NCEQ	1
#define ADAU_OUTSEL_TEST	2

#define ADAU_ECHODLY_1ST	0
#define ADAU_ECHODLY_2ND	1
#define ADAU_ECHODLY_3RD	2
#define ADAU_ECHODLY_4TH	3

#define ADAU_ECHOVOL_1ST	0
#define ADAU_ECHOVOL_2ND	1
#define ADAU_ECHOVOL_3RD	2
#define ADAU_ECHOVOL_4TH	3

#define ADAU_DSP_SEL_DISABLE    0
#define ADAU_DSP_SEL_ENABLE     1

#define ADAU_PATH_ON     1
#define ADAU_PATH_OFF    0

int adau1442_register(dsp_adapter_t *adapter);

#endif

