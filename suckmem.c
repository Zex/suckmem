/*
 * Author: Zex <top_zlynch@yahoo.com>
 *
 * Driver module for suckmem character device
 *
 * Copyright (c) by Zex Li <top_zlynch@yahoo.com>, Hytera Communications Co., Ltd.
 *
 * Sample command to load the module
 *	mknod /dev/suckmem 60 0
 * 	insmod suckmem.ko
 *	echo runaway > /dev/suckmem
 *	dmesg|tail
 *	cat /dev/suckmem
 *	dmesg|tail
 *	rmmod suckmem
 *	dmesg|tail
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zex");
MODULE_DESCRIPTION("Driver for suckmem character device");
MODULE_SUPPORTED_DEVICE("suckmem");
MODULE_VERSION("0.0.1");

int suckmem_major = 60;
char *suckbuf;

// buffer size for character device is always 1
static int bufsize = 1;
//module_param(bufsize, int, 0644);
//MODULE_PARM_DESC(bufsize, "size of device buffer");

/*
int suckmem_open(struct inode *inodep, struct file *filp);
ssize_t suckmem_read(struct file *filp, char* buf, size_t count, loff_t *f_pos);
ssize_t suckmem_write(struct file *filp, char* buf, size_t count, loff_t *f_pos);
int suckmem_release(struct inode *inodep, struct file *filp);
*/
int suckmem_open(struct inode *inodep, struct file *filp)
{
	printk("suckmem: %s operation\n", __func__);
	return 0;
}

ssize_t suckmem_read(
	struct file *filp, char* buf, size_t count, loff_t *f_pos)
{
	int ret;
	printk("suckmem: %s operation\n", __func__);

	ret = copy_to_user(buf, suckbuf, bufsize);
	printk("suckmem: read size %d\n", ret);

	if (*f_pos == 0) {
		*f_pos += 1;
		return 1;
	}
	
	return 0;
}

ssize_t suckmem_write(
	struct file *filp, const char* buf, size_t count, loff_t *f_pos)
{
	char *tmp;
	int ret;
	printk("suckmem: %s operation\n", __func__);

	tmp = buf+count-1;
	ret = copy_from_user(suckbuf, tmp, bufsize);
	printk("suckmem: write size %d\n", ret);

	return 1;
}

int suckmem_release(struct inode *inodep, struct file *filp)
{
	printk("suckmem: %s operation\n", __func__);
	return 0;
}

struct file_operations suckmem_fops = {
	read: suckmem_read,
	write: suckmem_write,
	open: suckmem_open,
	release: suckmem_release,
};

void suckmem_exit(void)
{
	unregister_chrdev(suckmem_major, "suckmem");

	if (suckbuf) {
		kfree(suckbuf);
	}

	printk("removing suckmem module\n");
}

int suckmem_init(void)
{
	int ret;

	ret = register_chrdev(suckmem_major, "suckmem", &suckmem_fops);

	if (ret) {
		printk("suckmem: failed to obtain major number %d\n", suckmem_major);
		return ret;
	}

	suckbuf = kmalloc(bufsize, GFP_KERNEL);
	if (!suckbuf) {
		ret = -ENOMEM;
		goto fail;
	}
	memset(suckbuf, 0, 1);

	printk("inserting suckmem module\n");
	return 0;

	fail:
		suckmem_exit();
		return ret;
}

int init_module()
{
	return suckmem_init();
}

void cleanup_module()
{
	suckmem_exit();
}
