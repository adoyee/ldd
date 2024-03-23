#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>

#include "scull.h"

#define DEV_NAME "scull"

int scull_major = 0;
int scull_minor = 0;

struct scull_qset {
	void **data;
	struct scull_qset *next;
};

struct scull_dev {
	struct scull_qset *data;  /* Pointer to first quantum set   */
	int quantum;              /* the current quantum size       */
	int qset;                 /* the current array size         */
	unsigned long size;       /* amount of data stored here     */
	unsigned int access_key;  /* used by sculluid and scullpriv */
	struct semaphore sem;     /* mutual exclusion semaphore     */
	struct cdev cdev;     	  /* Char device structure          */
};

int scull_open(struct inode *inode, struct file *filep)
{
	struct scull_dev *dev;
	dev = container_of(inode->i_cdev, struct scull_dev, cdev);
	filep->private_data = dev;
	return 0;
}

int scull_release(struct inode *inode, struct file *filep)
{
	return 0;
}

ssize_t scull_read(struct file *fp, __user char *data, size_t size, loff_t *off)
{
	return 0;
}

ssize_t scull_write(struct file *fp, __user const char *data, size_t size, loff_t *off)
{
	return 0;
}

void scull_trim(struct scull_dev *dev)
{
	struct scull_qset *next, *dptr;
	int qset = dev->qset;
	int i;

	for (dptr = dev->data; dptr; dptr = next) {
		if (dptr->data) {
			for (i = 0; i < qset; i++) {
				kfree(dptr->data[i]);
			}
			kfree(dptr->data);
			dptr->data = NULL;
		}
		next = dptr->next;
		kfree(dptr);
	}

	dev->size = 0;
	dev->quantum = NUM_QUANTUM;
	dev->qset = NUM_QSET;
	dev->data = NULL;
}

struct file_operations scull_fops = {
	.owner	= THIS_MODULE,
	// .llseek	= scull_llseek,
	.read	= scull_read,
	.write	= scull_write,
	// .ioctl	= scull_ioctl,
	.open	= scull_open,
	.release = scull_release,
};

static int scull_setup_cdev(struct scull_dev *dev, int index)
{
	return 0;
}

static int __init scull_init(void)
{
	int ret;
	dev_t dev;

	ret = alloc_chrdev_region(&dev, 0, 1, "my-dev");
	ret = alloc_chrdev_region(&dev, 0, 1, DEV_NAME);

	if(ret){
		printk(KERN_INFO "alloc chrdev region failed\n");
		return ret;
	}

	printk(KERN_INFO "major:%d\n", MAJOR(dev));
	printk(KERN_INFO "minor:%d\n", MINOR(dev));

	printk(KERN_INFO "hello linux module\n");
	return 0;
}

static void __exit scull_exit(void)
{
	printk(KERN_INFO "good bye\n");
}

module_init(scull_init);
module_exit(scull_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("adoyee");
MODULE_VERSION("0.0.1");
