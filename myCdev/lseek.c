#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main() {
	int fd = open("/dev/myCdev", O_RDONLY);
	if (-1 == fd)
		return -1;
	lseek(fd, 5, SEEK_CUR);
	close(fd);
	return 0;
}