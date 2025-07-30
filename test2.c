#include <sndfile.h>

int main(int argc, char *argv[])
{
	SF_INFO sfinfo = {0};
	sfinfo.samplerate = 96000;
	sfinfo.channels = 1;
	sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

	SNDFILE *sndf = sf_open(argv[1], SFM_WRITE, &sfinfo);
	if (!sndf) {
		printf("sf_open() error!\n");
		return -1;
	}

	sf_close(sndf);
	return 0;
}
