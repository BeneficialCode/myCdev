#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
	char buffer[1024] = { 0 };
	int fd = open("/dev/myCdev", O_WRONLY);
	if (-1 == fd)
		return -1;
	write(fd, buffer, 20);
	close(fd);
	return 0;
}