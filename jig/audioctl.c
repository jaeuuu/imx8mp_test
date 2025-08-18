#include <api/debug.h>
#include <api/pcm_audio.h>
#include <api/menu.h>
#include <sndfile.h>
#include <ncurses.h>

#define MAX_AUDIO_MENU_DEPTH	2

static WINDOW *pr_win_audio[MAX_AUDIO_MENU_DEPTH];
static int pr_win_audio_depth = 0;

static int set_pcm_hw_params(pcm_info_t *pcm)
{
	int err;

	if ((err = snd_pcm_hw_params_malloc(&pcm->hw_params)) < 0) {
		dlp("snd_pcm_hw_params_malloc() fail : %s", snd_strerror(err));
		snd_pcm_hw_params_free(pcm->hw_params);
		return -1;
	}

	if ((err = snd_pcm_hw_params_any(pcm->handle, pcm->hw_params)) < 0) {
		dlp("snd_pcm_hw_params_any() fail : %s", snd_strerror(err));
		snd_pcm_hw_params_free(pcm->hw_params);
		return -1;
	}

	if ((err = snd_pcm_hw_params_set_access(pcm->handle, pcm->hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		dlp("snd_pcm_hw_params_set_access() fail : %s", snd_strerror(err));
		snd_pcm_hw_params_free(pcm->hw_params);
		return -1;
	}

	if ((err = snd_pcm_hw_params_set_format(pcm->handle, pcm->hw_params, pcm->format)) < 0) {
		dlp("snd_pcm_hw_params_set_format() fail : %s", snd_strerror(err));
		snd_pcm_hw_params_free(pcm->hw_params);
		return -1;
	}

	if ((err = snd_pcm_hw_params_set_channels(pcm->handle, pcm->hw_params, pcm->channels)) < 0) {
		dlp("snd_pcm_hw_params_set_channels() fail : %s", snd_strerror(err));
		snd_pcm_hw_params_free(pcm->hw_params);
		return -1;
	}

	if ((err = snd_pcm_hw_params_set_rate(pcm->handle, pcm->hw_params, pcm->sample_rate, 0)) < 0) {
		dlp("snd_pcm_hw_params_set_rate() fail : %s", snd_strerror(err));
		snd_pcm_hw_params_free(pcm->hw_params);
		return -1;
	}

	if ((err = snd_pcm_hw_params_set_periods_near(pcm->handle, pcm->hw_params, &pcm->periods, NULL)) < 0) {
		dlp("snd_pcm_hw_params_set_periods_near() fail : %s", snd_strerror(err));
		snd_pcm_hw_params_free(pcm->hw_params);
		return -1;
	}

	snd_pcm_uframes_t period_size_max;
	if ((err = snd_pcm_hw_params_get_period_size_max(pcm->hw_params, &period_size_max, NULL)) < 0) {
		dlp("snd_pcm_hw_params_get_period_size_max() fail : %s", snd_strerror(err));
		snd_pcm_hw_params_free(pcm->hw_params);
		return -1;
	}
	dlp("period_size_max : %lu", period_size_max);
	pcm->period_size = period_size_max;
	pcm->buffer_size = pcm->period_size * pcm->periods;
	if ((err = snd_pcm_hw_params_set_buffer_size_near(pcm->handle, pcm->hw_params, &pcm->buffer_size)) < 0) {
		dlp("snd_pcm_hw_params_set_buffer_size_near() fail : %s", snd_strerror(err));
		snd_pcm_hw_params_free(pcm->hw_params);
		return -1;
	}

	snd_pcm_hw_params_get_buffer_size(pcm->hw_params, &pcm->buffer_size);
	snd_pcm_hw_params_get_period_size(pcm->hw_params, &pcm->period_size, NULL);
	if ((pcm->periods * pcm->period_size) > pcm->buffer_size) {
		dlp("buffer size too small");
		snd_pcm_hw_params_free(pcm->hw_params);
		return -1;
	}

	if ((err = snd_pcm_hw_params(pcm->handle, pcm->hw_params)) < 0) {
		dlp("snd_pcm_hw_params() fail : %s", snd_strerror(err));
		snd_pcm_hw_params_free(pcm->hw_params);
		return -1;
	}

	snd_pcm_hw_params_free(pcm->hw_params);
	return 0;
}

static int set_pcm_sw_params(pcm_info_t *pcm)
{
	int err;

	if ((err = snd_pcm_sw_params_malloc(&pcm->sw_params)) < 0) {
		dlp("snd_pcm_sw_params_malloc() fail : %s", snd_strerror(err));
		return -1;
	}

	if ((err = snd_pcm_sw_params_current(pcm->handle, pcm->sw_params)) < 0) {
		dlp("snd_pcm_sw_params_current() fail : %s", snd_strerror(err));
		snd_pcm_sw_params_free(pcm->sw_params);
		return -1;
	}

	if ((err = snd_pcm_sw_params_set_start_threshold(pcm->handle, pcm->sw_params, pcm->period_size)) < 0) {
		dlp("snd_pcm_sw_params_set_start_threshold() : %s", snd_strerror(err));
		snd_pcm_sw_params_free(pcm->sw_params);
		return -1;
	}

	if ((err = snd_pcm_sw_params_set_avail_min(pcm->handle, pcm->sw_params, pcm->period_size)) < 0) {
		dlp("snd_pcm_sw_params_set_avail_min() : %s", snd_strerror(err));
		snd_pcm_sw_params_free(pcm->sw_params);
		return -1;
	}

	if ((err = snd_pcm_sw_params(pcm->handle, pcm->sw_params)) < 0) {
		dlp("snd_pcm_sw_params() : %s", snd_strerror(err));
		snd_pcm_sw_params_free(pcm->sw_params);
		return -1;
	}

	snd_pcm_sw_params_free(pcm->sw_params);
	return 0;
}

#define AUDIO_FORMAT_PCM 1
static int check_pcm_format(pcm_info_t *pcm)
{
	int fd;
	wav_header_t header;
	struct stat file_info;
	const char *path = pcm->file_path;

	if (!path) {
		return -1;
	}

	if (stat(path, &file_info) < 0) {
		perror("stat() fail");
		return -1;
	}

	if ((fd = open(path, O_RDONLY)) < 0) {
		perror("open() fail");
		dlp("open fail wav file : [%s]", path);
		return -1;
	}

	if (read(fd, &header, sizeof(wav_header_t)) < sizeof(wav_header_t)) {
		dlp("invalid wav file : mismatch header size");
		close(fd);
		return -1;
	};

	if (memcmp(header.riff_id, "RIFF", 4)) {
		dlp("invalid wav file : not a riff field");
		close(fd);
		return -1;
	}

	if (memcmp(header.wav_fmt, "WAVE", 4)) {
		dlp("invalid wav file : not a wave field");
		close(fd);
		return -1;
	}

	if (memcmp(header.fmt_id, "fmt ", 4)) {
		dlp("invalid wav file : not a fmt field");
		close(fd);
		return -1;
	}

	if (header.audio_fmt != AUDIO_FORMAT_PCM) {
		dlp("invalid wav file : not a pcm format");
		close(fd);
		return -1;
	}

	if (header.bit_per_sample == 8) {
		pcm->format = SND_PCM_FORMAT_U8;
	} else if (header.bit_per_sample == 16) {
		pcm->format = SND_PCM_FORMAT_S16;
	} else {
		dlp("invalid wav file : unknown pcm format");
		close(fd);
		return -1;
	}
	pcm->channels = header.channels;
	pcm->sample_rate = header.sample_rate;
	pcm->raw_size = header.data_len;
	pcm->periods = 2;
	return 0;
}

static int stream_pcm_u8(pcm_info_t *pcm)
{
	unsigned int fragment_size = snd_pcm_frames_to_bytes(pcm->handle, pcm->period_size);
	unsigned int fragments = pcm->raw_size / fragment_size;
	FILE *file = fopen(pcm->file_path, "rb");
	unsigned char *u8;

	if (!file) {
		perror("open() fail");
		return -1;
	}

	if (!(pcm->raw_data = calloc(fragment_size, sizeof(char)))) {
		perror("calloc() fail");
		return -1;
	}
	u8 = (unsigned char *)pcm->raw_data;

	pcm->status = PCM_PLAY;

	int i, len, err;
	for (i = 0; i < fragments; i++) {
		if ((len = fread(u8, sizeof(char), fragment_size, file)) > 0) {
			if ((err = snd_pcm_writei(pcm->handle, u8, len)) < 0) {
				if (err == -EAGAIN) {
					continue;
				} else if (err == -EPIPE) {
					dlp("snd_pcm_writei() underrun occur");
					snd_pcm_prepare(pcm->handle);
				} else if (err == -ESTRPIPE) {
					dlp("snd_pcm_writei() suspend occur");
					snd_pcm_resume(pcm->handle);
					snd_pcm_prepare(pcm->handle);
				} else {
					dlp("snd_pcm_writei() fail : %s", snd_strerror(err));
				}
			}
		}
		if (pcm->status == PCM_STOP) {
			dlp("pcm stop occur");
			snd_pcm_drain(pcm->handle);
			snd_pcm_prepare(pcm->handle);
			break;
		}

		// add PCM_PAUSE
		if (pcm->status == PCM_PAUSE) {
			dlp("note: pcm pause occur!");
			while (pcm->status == PCM_PAUSE)
				usleep(100 * 1000);
		}
	}

	pcm->status = PCM_NONE;

	free(u8);
	return 0;
}

static int stream_pcm_s16(pcm_info_t *pcm)
{
	unsigned int fragment_size = snd_pcm_frames_to_bytes(pcm->handle, pcm->period_size);
	unsigned int fragments = pcm->raw_size / fragment_size;
	FILE *file = fopen(pcm->file_path, "rb");
	short *s16;

	if (!file) {
		perror("open() fail");
		return -1;
	}

	if (!(pcm->raw_data = calloc(fragment_size, sizeof(short)))) {
		perror("calloc() fail");
		return -1;
	}
	s16 = (short *)pcm->raw_data;

	pcm->status = PCM_PLAY;

	int i, len, err;
	for (i = 0; i < fragments; i++) {
		if ((len = fread(s16, sizeof(short), fragment_size, file)) > 0) {
			if ((err = snd_pcm_writei(pcm->handle, s16, len)) < 0) {
				if (err == -EAGAIN) {
					continue;
				} else if (err == -EPIPE) {
					dlp("snd_pcm_writei() underrun occur");
					snd_pcm_prepare(pcm->handle);
				} else if (err == -ESTRPIPE) {
					dlp("snd_pcm_writei() suspend occur");
					snd_pcm_resume(pcm->handle);
					snd_pcm_prepare(pcm->handle);
				} else {
					dlp("snd_pcm_writei() fail : %s", snd_strerror(err));
				}
			}
		}
		if (pcm->status == PCM_STOP) {
			dlp("pcm stop occur");
			snd_pcm_drain(pcm->handle);
			snd_pcm_prepare(pcm->handle);
			break;
		}

		// add PCM_PAUSE
		if (pcm->status == PCM_PAUSE) {
			dlp("note: pcm pause occur!");
			while (pcm->status == PCM_PAUSE)
				usleep(100 * 1000);
		}
	}

	pcm->status = PCM_NONE;

	free(s16);
	return 0;
}

int stream_pcm(pcm_info_t *pcm)
{
	int err;

	if (!pcm->handle) {
		dlp_err("pcm handle is null! please check device");
		return -1;
	}

	if (check_pcm_format(pcm) < 0) {
		dlp_warn("wav check fail");
		return -1;
	}

	if ((err = set_pcm_hw_params(pcm)) < 0) {
		dlp_warn("set_pcm_hw_params() fail");
		return -2;
	}

	if ((err = set_pcm_sw_params(pcm)) < 0) {
		dlp_warn("set_pcm_sw_params() fail");
		return -2;
	}

	switch (pcm->format) {
	case SND_PCM_FORMAT_U8:
		dlp_dbg("u8");
		return stream_pcm_u8(pcm);
	case SND_PCM_FORMAT_S16:
		dlp_dbg("s16");
		return stream_pcm_s16(pcm);
	default:
		dlp_warn("unknown pcm format");
		return -1;
	}
}

static int pcm_read()

int record_pcm(pcm_info_t *pcm)
{
	/* make wave file */
	SF_INFO sfinfo = { 0 };
	SNDFILE *sf;
	int count;


	sfinfo.samplerate = pcm->sample_rate;
	sfinfo.channels = pcm->channels;
	sfinfo.format = SF_FORMAT_WAV | pcm->format;

	sf = sf_open("test.wav", SFM_WRITE, &sfinfo);
	if (!sf) {
		pr_win(pr_win_audio[pr_win_audio_depth], "sf_open() fail! path: [test.wav]\n");
		return -1;
	}
	/* read pcm data */
	count = snd_pcm_format_size(pcm->format, pcm->sample_rate * pcm->channels);
	count *= 10;		// for 10s

	if (set_pcm_hw_params(pcm) < 0) {
		pr_win(pr_win_audio[pr_win_audio_depth], "set_pcm_hw_params() fail\n");
		return -1;
	}

	if (set_pcm_sw_params(pcm) < 0) {
		pr_win(pr_win_audio[pr_win_audio_depth], "set_pcm_sw_params() fail\n");
		return -1;
	}

	while (count > 0) {
		int f = (count * 8) / (pcm->sample_rate * pcm->channels);
		int r = snd_pcm_readi(pcm->handle, )
	}

	/* write pcm to file */
	/* size over check */
}

int init_pcm(pcm_info_t *pcm, snd_pcm_stream_t stream)
{
	int err;

	//memset(pcm, 0x00, sizeof(pcm_info_t));
	//strcpy(pcm->dev_name, "default");

	if ((err = snd_pcm_open(&pcm->handle, pcm->dev_name, stream, 0)) < 0) {
		dlp_err("snd_pcm_open() fail : %s", snd_strerror(err));
		free(pcm);
		return -1;
	}
	return 0;
}

void free_pcm(pcm_info_t *pcm)
{
	free(pcm);
}
