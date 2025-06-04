#ifndef API_PCM_AUDIO_H_
#define API_PCM_AUDIO_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include <sys/stat.h>

enum pcm_status {
    PCM_NONE = 0x00,
    PCM_PLAY,
    PCM_STOP,
    PCM_PAUSE,
};

typedef struct st_pcm_info {
    char dev_name[64];
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_sw_params_t *sw_params;
    snd_pcm_format_t format;

    unsigned int channels;
    unsigned int sample_rate;
    unsigned int periods;
    snd_pcm_uframes_t period_size;
    unsigned int buffers;
    snd_pcm_uframes_t buffer_size;
    const char *file_path;
    void *raw_data;
    unsigned int raw_size;
    enum pcm_status status;
} pcm_info_t;

typedef struct st_wav_header {
    unsigned char   riff_id[4];
    unsigned int    riff_len;
    unsigned char   wav_fmt[4];
    unsigned char   fmt_id[4];
    unsigned int    fmt_len;
    unsigned short  audio_fmt;
    unsigned short  channels;
    unsigned int    sample_rate;
    unsigned int    byte_rate;
    unsigned short  block_align;
    unsigned short  bit_per_sample;
    unsigned char   data_id[4];
    unsigned int    data_len;
} __attribute__((packed)) wav_header_t;

int init_pcm(pcm_info_t *pcm);
int stream_pcm(pcm_info_t *pcm);
void free_pcm(pcm_info_t *pcm);

#endif
