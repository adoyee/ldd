#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>

#include "scull.h"


struct scull_module module;

struct scull_dev {
	unsigned char *data;
	unsigned long size;       /* amount of data stored here     */
	unsigned int access_key;  /* used by sculluid and scullpriv */
	struct semaphore sem;     /* mutual exclusion semaphore     */
	struct cdev cdev;     	  /* Char device structure          */

	struct device *device;
};

static struct scull_dev *scull_device = NULL;

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

ssize_t scull_read(struct file *fp,  char __user *data, size_t size, loff_t *off)
{
	struct scull_dev *dev = scull_device;
	ssize_t count, retval = 0;

	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	if(*off > dev->size) {
		retval = -EFAULT;
		goto out;
	}

	count = size;
	if(*off + size > dev->size) {
		count = dev->size - *off;
	}

	if(copy_to_user(data, dev->data + *off, count)){
		retval = -EFAULT;
		goto out;
	}

	*off += count;
	retval = count;

out:
	up(&dev->sem);
	return retval;
}

ssize_t scull_write(struct file *fp, const char __user *data, size_t size, loff_t *off)
{
	struct scull_dev *dev = scull_device;
	ssize_t retval = 0;

	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	if(*off + size > SCULL_MEM_SIZE || size > SCULL_MEM_SIZE) {
		retval = -EFAULT;
		goto out;
	}

	if(copy_from_user(dev->data + *off, data, size)){
		retval = -EFAULT;
		goto out;
	}

	*off += size;
	retval = size;

	if(*off > dev->size)
		dev->size = *off;

out:
	up(&dev->sem);
	return retval;
}

struct file_operations scull_fops = {
	.owner	= THIS_MODULE,
	.read	= scull_read,
	.write	= scull_write,
	.open	= scull_open,
	.release = scull_release,
};

static int scull_device_init(void)
{
	int ret;
	dev_t dev;
	struct device *device;

	scull_device = kmalloc(sizeof(struct scull_dev), GFP_KERNEL);
	if(!scull_device){
		printk(KERN_ERR "scull: kmallo device failed\n");
		ret = -ENOMEM;
		return ret;
	}
	memset(scull_device, 0, sizeof(struct scull_dev));
	sema_init(&scull_device->sem, 1);
	scull_device->data = kvmalloc(SCULL_MEM_SIZE, GFP_KERNEL);
	if(!scull_device->data){
		printk(KERN_ERR "scull: kmallo device data failed\n");
		ret = -ENOMEM;
		goto data_fail;
	}
	printk(KERN_INFO "scull data address:%p\n", scull_device->data);

	cdev_init(&scull_device->cdev, &scull_fops);
	scull_device->cdev.owner = THIS_MODULE;

	dev = MKDEV(module.major, module.minor + SCULL_MINOR);
	ret = cdev_add(&scull_device->cdev, dev, 1);
	if(ret){
		printk(KERN_ERR "scull: cdev_add failed\n");
		goto cdev_add_fail;
	}

	device = device_create(module.scull_class, NULL, dev, scull_device, SCULL_DEV_NAME);
	if(!device){
		printk(KERN_ERR "scull: device create failed\n");
		goto device_create_fail;
	}
	scull_device->device = device;

	return ret;

device_create_fail:
	cdev_del(&scull_device->cdev);
cdev_add_fail:
	kvfree(scull_device->data);
data_fail:
	kfree(scull_device);
	return ret;
}

static void scull_device_uninit(void)
{
	device_destroy(module.scull_class, scull_device->cdev.dev);
	cdev_del(&scull_device->cdev);
	kvfree(scull_device->data);
	kfree(scull_device);
}


static int scull_module_init(void)
{
	int ret;
	dev_t dev;
	struct class *cls;

	ret = alloc_chrdev_region(&dev, 0, NUM_SCULL_DEV, SCULL_DEV_NAME);
	if(ret){
		printk(KERN_INFO "scull: alloc chrdev region failed\n");
		return ret;
	}

	module.dev = dev;
	module.major = MAJOR(dev);
	module.minor = MINOR(dev);
	printk(KERN_INFO "scull major:%d minor:%d\n", module.major, module.minor);

	cls = class_create(THIS_MODULE, SCULL_DEV_CLASS);
	if(!cls){
		printk(KERN_ERR "scull: create scull class failed\n");
		ret = -ENOMEM;
		goto class_fail;
	}

	module.scull_class = cls;

	return ret;
class_fail:
	unregister_chrdev_region(dev, NUM_SCULL_DEV);
	return ret;
}

static void scull_module_uninit(void)
{
	class_destroy(module.scull_class);
	unregister_chrdev_region(module.dev, NUM_SCULL_DEV);
}

static int __init scull_init(void)
{
	int ret;
	ret = scull_module_init();
	if(ret)
		return ret;

	ret =scull_device_init();
	if(ret){
		goto dev_fail;
	}

	return ret;
dev_fail:
	scull_module_uninit();
	return ret;
}

static void __exit scull_exit(void)
{
	scull_device_uninit();
	scull_module_uninit();
	printk(KERN_INFO "scull: good bye\n");
}

module_init(scull_init);
module_exit(scull_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("adoyee");
MODULE_VERSION("0.0.1");
