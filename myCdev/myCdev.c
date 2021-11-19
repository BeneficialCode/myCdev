#ifndef __KERNEL__
#define __KERNEL__
#endif // !__KERNEL__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <linux/wait.h>
#include <linux/kfifo.h>
#include <linux/proc_fs.h>

// 模块作者
MODULE_AUTHOR("VirtualCC");
MODULE_LICENSE("GPL"); // 指定模块采用的协议
MODULE_DESCRIPTION("Char Device Driver");
MODULE_VERSION("V1.1");

#define CDEV_DEVICE_NAME	"Cdev"
#define CDEV_NODE_NAME		"myCdev"
#define CDEV_CLASS_NAME		"myCdevClass"

#define CDEV_IOC_MAGIC	'z'
#define CDEV_QUEUE_RESET _IO(CDEV_IOC_MAGIC,0)
#define CDEV_QUEUE_CHANGE_IN _IOW(CDEV_IOC_MAGIC,1,int)
#define CDEV_QUEUE_CHANGE_OUT _IOW(CDEV_IOC_MAGIC,2,int)
#define CDEV_QUEUE_CHANGE_SIZE _IOW(CDEV_IOC_MAGIC,3,int)
#define CDEV_IOC_MAXNR 3

struct class *myCdevClass;
static int major = 0;
static int queueSize = 1 << 10;	// 1 kb
module_param(queueSize, int, S_IRUGO);

int cdevOpen(struct inode* inode, struct file *filp);
int cdevIoctl(struct inode* inode, struct file * filp, unsigned int cmd, unsigned long arg);
int cdevRead(struct file* filp, char __user* buf, size_t count, loff_t* f_pos);
int cdevWrite(struct file* filp, const char __user* buff, size_t count, loff_t *f_pos);
int cdevRelease(struct inode* inode, struct file *filp);
loff_t cdevLlseek(struct file *filp, loff_t offset, int whence);
int queryInfoProc(char* page, char ** start, off_t offset, int count, int* eof, void *data);
static int __init cdevInit(void);
static void __exit cdevExit(void);

// 由于2.6.32没有定义，于是扩展kfifo函数
int kfifo_is_empty(struct kfifo *fifo);
int kfifo_is_full(struct kfifo *fifo);

// 指定模块初始化函数
module_init(cdevInit);
// 指定模块退出函数
module_exit(cdevExit);

struct file_operations myCdevFops = {
	.owner = THIS_MODULE,
	.open = cdevOpen,
	.release = cdevRelease,
	.read = cdevRead,
	.write = cdevWrite,
	.ioctl = cdevIoctl,
	.llseek = cdevLlseek,
};

struct myCdev {
	unsigned char* buffer;	// 用于临时缓冲
	struct kfifo* myQueue;
	wait_queue_head_t writeQueue;
	wait_queue_head_t readQueue;
	struct semaphore sem;
	spinlock_t lock;
	struct cdev cdev;
}myCdev;

int cdevOpen(struct inode* inode, struct file *filp) {
	struct myCdev* dev;
	dev = container_of(inode->i_cdev, struct myCdev, cdev);
	filp->private_data = dev;

	printk("myCdev device open.\n");
	return 0;
}

int cdevIoctl(struct inode* inode, struct file * filp, unsigned int cmd, unsigned long arg) {
	int err = 0;
	int ret = 0;

	struct myCdev* dev = filp->private_data;
	if (_IOC_TYPE(cmd) != CDEV_IOC_MAGIC)
		return -EINVAL;

	if (_IOC_NR(cmd) > CDEV_IOC_MAXNR)
		return -EINVAL;

	if (_IOC_DIR(cmd)&_IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user*)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd)&_IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void __user*)arg, _IOC_SIZE(cmd));

	if (err)
		return -EFAULT;
	
	switch (cmd) {
		case CDEV_QUEUE_RESET:
			kfifo_reset(dev->myQueue);
			printk("Reset queue initialize status successfully!\n");
			break;
		case CDEV_QUEUE_CHANGE_IN:
			get_user(dev->myQueue->in, (int __user*)arg);
			printk("Change queue in successfully!\n");
			break;
		case CDEV_QUEUE_CHANGE_OUT:
			get_user(dev->myQueue->out, (int __user*)arg);
			printk("Change queue out successfully!\n");
			break;
		case CDEV_QUEUE_CHANGE_SIZE:
			kfifo_free(dev->myQueue);
			kfree(dev->buffer);
			get_user(queueSize, (int __user*)arg);
			myCdev.myQueue = kfifo_alloc(queueSize, GFP_KERNEL, &myCdev.lock);
			dev->buffer = kmalloc(queueSize, GFP_KERNEL);
			printk("Change queue size successfully!\n");
			break;
		default:
			return -ENOTTY;
	}

	return ret;
}

int cdevRead(struct file* filp, char __user* buf, size_t count, loff_t* f_pos) {
	struct myCdev* dev = filp->private_data;
	int len = 0;

	if (down_interruptible(&dev->sem))
		return -ERESTART;

	while (kfifo_is_empty(dev->myQueue)) {
		up(&dev->sem);
		if (filp->f_flags&O_NONBLOCK)
			return -EAGAIN;
		if (wait_event_interruptible(dev->readQueue, !kfifo_is_empty(dev->myQueue)))
			return -ERESTART;
		if (down_interruptible(&dev->sem))
			return -ERESTART;
	}

	/* 设备已就绪 */
	len = kfifo_len(dev->myQueue);
	if (count > len)
		count = len;
	kfifo_get(dev->myQueue, dev->buffer, count);
	if (copy_to_user(buf, dev->buffer, count)) {
		up(&dev->sem);
		return -EFAULT;
	}

	up(&dev->sem);
	wake_up_interruptible(&dev->writeQueue);
	printk("%s did read %d bytes!\n", current->comm, count);
	
	return count;
}

int cdevWrite(struct file* filp, const char __user* buf, size_t count, loff_t *f_pos) {
	struct myCdev* dev = filp->private_data;
	int len = 0;

	if (down_interruptible(&dev->sem))
		return -ERESTART;
	while (kfifo_is_full(dev->myQueue)) {
		up(&dev->sem);
		if (filp->f_flags&O_NONBLOCK)
			return -EAGAIN;
		if (wait_event_interruptible(dev->writeQueue, !kfifo_is_full(dev->myQueue)))
			return -ERESTART;
		if (down_interruptible(&dev->sem))
			return -ERESTART;
	}
	
	/* 有空间可用，接收数据 */
	len = kfifo_len(dev->myQueue);
	if (count + len > dev->myQueue->size) {
		count = dev->myQueue->size - len;
	}
	if (copy_from_user(dev->buffer, buf, count)) {	// 先拷贝到临时缓冲区
		up(&dev->sem);
		return -EFAULT;
	}
	kfifo_put(dev->myQueue, dev->buffer, count);	// 再将数据写入队列

	up(&dev->sem);
	wake_up_interruptible(&dev->readQueue);

	printk("%s did write %li bytes\n", current->comm, (long)count);
	return count;
}

int cdevRelease(struct inode* inode, struct file *filp) {
	printk("myCdev device release!\n");
	return 0;
}

loff_t cdevLlseek(struct file *filp, loff_t offset, int whence) {
	struct myCdev* dev = filp->private_data;
	loff_t newPos = offset;
	
	if (newPos < dev->myQueue->out)
		return -ENAVAIL;
	if (newPos < 0)
		return -ENAVAIL;

	if (newPos > dev->myQueue->in)
		newPos = dev->myQueue->in;

	dev->myQueue->in = newPos;
	return newPos;
}

int queryInfoProc(char* buf, char ** start, off_t offset, int count, int* eof, void *data) {
	int len = 0;
	len = sprintf(buf, "buffer address: %p,in: %u,out %u size: %u\n", myCdev.myQueue->buffer,
		myCdev.myQueue->in,myCdev.myQueue->out,myCdev.myQueue->size);
	*eof = 1;
	return len;
}

static void cdevSetup(struct cdev* dev, int minor, struct file_operations *fops) {
	int err, devno = MKDEV(major, minor);
	cdev_init(dev, fops);
	dev->owner = THIS_MODULE;
	dev->ops = fops;
	err = cdev_add(dev, devno, 1);
	if (err) {
		printk(KERN_NOTICE"Error %d adding myCdev %d\n", err, minor);
		return;
	}
	printk("myCdev device setup.\n");
}

static int __init cdevInit(void) {
	int ret = 0;
	dev_t dev = MKDEV(major, 0);
	if (major) {
		ret = register_chrdev_region(dev, 1, CDEV_DEVICE_NAME);
		printk("myCdev register_chrdev_region.\n");
	} else {
		ret = alloc_chrdev_region(&dev, 0, 1, CDEV_DEVICE_NAME);
		printk("myCdev alloc_chrdev_region.\n");
		major = MAJOR(dev);
	}

	if (ret < 0) {
		printk(KERN_WARNING"myCdev region fail.\n");
		return ret;
	}

	cdevSetup(&myCdev.cdev, 0, &myCdevFops);
	printk("The major of the myCdev device is %d.\n", major);

	myCdevClass = class_create(THIS_MODULE, CDEV_CLASS_NAME);
	if (IS_ERR(myCdevClass)) {
		printk("Err: failed in creating myCdev class.\n");
		return 0;
	}

	device_create(myCdevClass, NULL, dev, NULL, CDEV_NODE_NAME);

	init_waitqueue_head(&myCdev.readQueue);
	init_waitqueue_head(&myCdev.writeQueue);
	sema_init(&myCdev.sem, 1);
	spin_lock_init(&myCdev.lock);

	do {
		myCdev.myQueue = kfifo_alloc(queueSize,GFP_KERNEL,&myCdev.lock);
		if (!myCdev.myQueue) {
			printk(KERN_ERR"error kfifo_alloc\n");
			break;
		}
		myCdev.buffer = NULL;
		myCdev.buffer = kmalloc(queueSize, GFP_KERNEL);
		if (!myCdev.buffer) {
			printk(KERN_ERR"error malloc temp buffer\n");
			ret = -ENOMEM;
			break;
		}
		
		create_proc_read_entry("myCdev", 0, NULL, queryInfoProc, NULL); // 创建proc文件系统
	} while (0);

	if (ret < 0) {
		if (!myCdev.myQueue->buffer) {
			kfree(&myCdev.myQueue);
		}
	}
	return ret;
}

static void __exit cdevExit(void) {
	device_destroy(myCdevClass, MKDEV(major, 0));
	class_destroy(myCdevClass);
	cdev_del(&myCdev.cdev);
	unregister_chrdev_region(MKDEV(major, 0), 1);
	remove_proc_entry("myCdev", NULL);
	kfifo_free(myCdev.myQueue);
	kfree(myCdev.buffer);
	printk("myCdev device uninstalled.\n");
}

int kfifo_is_empty(struct kfifo *fifo) {
	return fifo->in == fifo->out ? 1 : 0;
}

int kfifo_is_full(struct kfifo *fifo) {
	return (fifo->in - fifo->out == fifo->size) ? 1 : 0;
}
