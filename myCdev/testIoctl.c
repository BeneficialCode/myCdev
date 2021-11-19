#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CDEV_IOC_MAGIC	'z'
#define CDEV_QUEUE_RESET _IO(CDEV_IOC_MAGIC,0)
#define CDEV_QUEUE_CHANGE_IN _IOW(CDEV_IOC_MAGIC,1,int)
#define CDEV_QUEUE_CHANGE_OUT _IOW(CDEV_IOC_MAGIC,2,int)
#define CDEV_QUEUE_CHANGE_SIZE _IOW(CDEV_IOC_MAGIC,3,int)

int main() {
	int fd = open("/dev/myCdev", O_RDWR);
	int arg = 0;
	if (fd == -1)
		return -1;
	// ioctl(fd, CDEV_QUEUE_RESET);
	
	/*arg = 5;
	ioctl(fd, CDEV_QUEUE_CHANGE_IN, &arg);*/
	
	//arg = 2;
	//ioctl(fd, CDEV_QUEUE_CHANGE_OUT, &arg);

	arg = 512;
	ioctl(fd, CDEV_QUEUE_CHANGE_SIZE, &arg);
	return 0;
}
