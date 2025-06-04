#include "api_shm.h"

#define __FILENAME__ "api_shm.c"
#define D_API_SHM
#ifdef D_API_SHM
	#define dlp(fmt, args...) printf("[%s %s(), %d]"fmt"\r\n", __FILENAME__, __FUNCTION__, __LINE__, ##args)
#else
	#define dlp(fmt, args...)
#endif

int init_shm(const char *path, int size)
{
	int fd;

	if (!path || size < 0) {
		dlp("argument error");
		return -EARGS;
	}

	shm_unlink(path);

	fd = shm_open(path, O_RDWR|O_CREAT|O_EXCL, 0644);
	if (fd < 0) {
		perror("shm_open() fail");
		return -ESYSCALL;
	}

	if (ftruncate(fd, size) < 0) {
		perror("ftruncate() fail");
		return -ESYSCALL;
	}

	close(fd);
	return 0;
}

int open_shm(const char *path, void *map, int size)
{
	int fd;

	if (!path) {
		dlp("argument error");
		return -EARGS;
	}

	fd = shm_open(path, O_RDWR, 0644);
	if (fd < 0) {
		perror("shm_open() fail");
		return -ESYSCALL;
	}

	map = mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
	if (map < 0) {
		perror("mmap() fail");
		close(fd);
		return -ESYSCALL;
	}

	close(fd);
	return 0;
}

int close_shm(void *map, int size)
{
	if (!map || (size < 0)) {
		dlp("argument error");
		return -1;
	}

	msync(map, size, MS_SYNC);
	munmap(map, size);

	return 0;
}
