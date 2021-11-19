#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#define SIZE 1024
char buffer[SIZE] = { 0 };

int main() {
	int fd = open("/dev/myCdev", O_RDWR);
	int i = 0;

	if (-1 == fd)
		return -1;
	for (;i<10; i++) {
		read(fd, buffer, 3);
		srand(i);
		sleep(rand() % 5);
		write(fd, buffer, 3);
	}
	close(fd);
	return 0;
}

