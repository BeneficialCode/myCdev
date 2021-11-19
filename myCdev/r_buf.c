#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SIZE 1024
char buffer[SIZE] = { 0 };

int main() {
	int fd = open("/dev/myCdev", O_RDONLY);
	if (-1 == fd)
		return -1;
	read(fd, buffer, 3);
	close(fd);
	return 0;
}