#include <api/ini.h>
#include <api/debug.h>

int create_ini(const char *filename)
{
}

int read_ini(const char *filename, const char *sec, const char *key, char *buf, unsigned int size)
{
	FILE *file = NULL;
	char line[128];
	char cur_sec[64];
	char cur_key[64];
	char cur_val[64];

	if (!filename || !sec || !key || !buf)
		return -1;

	if ((file = fopen(filename, "r")) == NULL) {
		perror("fopen() fail");
		dlp_err("filename : \"%s\"\r\n", filename);
		return -1;
	}

	memset(line, 0x00, sizeof(line));
	memset(cur_sec, 0x00, sizeof(cur_sec));
	memset(cur_key, 0x00, sizeof(cur_key));
	memset(cur_val, 0x00, sizeof(cur_val));
	memset(buf, 0x00, size);

	while (fgets(line, sizeof(line), file)) {
		if (line[0] == ';')
			continue;

		if (line[0] == '[') {
			sscanf(line, "[%[^]]", cur_sec);
			continue;
		}

		if (!strcmp(cur_sec, sec)) {
			if (sscanf(line, "%[^=]=%127[^\n]", cur_key, cur_val) == 2) {
				if (!strcmp(cur_key, key)) {
					sprintf(buf, "%s", cur_val);
					fclose(file);
					return 0;
				}
			}
		}
	}

	fclose(file);
	dlp_warn("not found ini property! (filename: [%s] sector: [%s] key: [%s])", filename, sec, key);
	return -1;
}

int write_ini(const char *filename, const char *sec, const char *key, const char *buf, unsigned int size)
{
	FILE *file = NULL;
	char line[128];
	char cur_sec[64];
	char cur_key[64];

	if ((file = fopen(filename, "r+")) == NULL) {
		perror("fopen() fail");
		dlp_err("filename : \"%s\"", filename);
		return -1;
	}

	if (size != strlen(buf)) {
		dlp_err("included gabage data in buffer");
		dbp(buf, size, "BUFFER DEBUG");
		return -1;
	}

	memset(line, 0x00, sizeof(line));
	memset(cur_sec, 0x00, sizeof(cur_sec));
	memset(cur_key, 0x00, sizeof(cur_key));

	while (fgets(line, sizeof(line), file)) {
		if (line[0] == ';')
			continue;

		if (line[0] == '[') {
			sscanf(line, "[%[^]]", cur_sec);
			continue;
		}

		if (!strcmp(cur_sec, sec)) {
			if (sscanf(line, "%[^=]=", cur_key) == 1) {
				if (!strcmp(cur_key, key)) {
					dlp_dbg("ini trace \"%s=%s\"", key, buf);	/* ini 꺠지는 이슈 디버깅 */
					fseek(file, -strlen(line), SEEK_CUR);
					fprintf(file, "%s=%s\n", key, buf);
					fclose(file);
					return 0;
				}
			}
		}
	}

	fclose(file);
	dlp_warn("not found ini property! (filename: [%s] sector: [%s] key: [%s])", filename, sec, key);
	return -1;
}
