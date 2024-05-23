#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

// https://elixir.bootlin.com/linux/v6.9.1/source/drivers/char/scx200_gpio.c#L102

#define DEVICE_NAME "chardev"

static int handle_open(struct inode *, struct file *) {
    pr_info("handle_open");
    return 0;
}
static int handle_release(struct inode *, struct file *) {
    pr_info("handle_release");
    return 0;
}
static ssize_t handle_read(struct file *, char __user *, size_t, loff_t *) {
    pr_info("handle_read");
    return 0;
}
static ssize_t handle_write(struct file *, const char __user *, size_t, loff_t *) {
    pr_info("handle_write");
    return 0;
}

static struct file_operations fops = {
    .read = handle_read,
    .write = handle_write,
    .open = handle_open,
    .release = handle_release,
};

static struct cdev *my_cdev;
static int major;

static int __init chardev_init(void)
{
    int start_minor_no;
    int err;
    dev_t dev_params;

    pr_info("Initting " DEVICE_NAME "!\n");

    start_minor_no = 0;

    err = alloc_chrdev_region(&dev_params, start_minor_no, 1, DEVICE_NAME);
    if (err < 0) {
        pr_alert("alloc_chrdev_region ERROR: %d", err);
        return err;
    }
    pr_info("Err is %i", err);
    pr_info("Major is %i", MAJOR(dev_params));
    pr_info("Minor is %i", MINOR(dev_params));
    major = MAJOR(dev_params);

    my_cdev = cdev_alloc();
    cdev_init(my_cdev, &fops);
	err = cdev_add(my_cdev, dev_params, 1);
	if (err < 0) {
		pr_alert("Error: %i", err);
		return err;
	}

    pr_info("Success!");

    return 0;
}

static void __exit chardev_exit(void)
{
    pr_info("Goodbye, chardev!\n");
    cdev_del(my_cdev);
	unregister_chrdev_region(MKDEV(major, 0), 1);
}


module_init(chardev_init);
module_exit(chardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("k.kryukov");
MODULE_DESCRIPTION("Character device driver");
