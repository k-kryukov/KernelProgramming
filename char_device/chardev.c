#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define DEVICE_NAME "chardev"

static int handle_open(struct inode *, struct file *) { return 0; }
static int handle_release(struct inode *, struct file *) { return 0; }
static ssize_t handle_read(struct file *, char __user *, size_t, loff_t *) { return 0; }
static ssize_t handle_write(struct file *, const char __user *, size_t, loff_t *) { return 0; }

static struct file_operations fops = {
    .read = handle_read,
    .write = handle_write,
    .open = handle_open,
    .release = handle_release,
};

static struct cdev *my_cdev;

static int __init chardev_init(void)
{
    dev_t dev_params;
    int start_minor_no;
    int err;

    pr_info("Initting chardev!\n");

    start_minor_no = 0;

    err = alloc_chrdev_region(&dev_params, start_minor_no, 1, DEVICE_NAME);
    if (err < 0) {
        pr_alert("alloc_chrdev_region ERROR: %d", err);
        return err;
    }
    pr_info("Major is %i", MAJOR(dev_params));
    pr_info("Minor is %i", MINOR(dev_params));

    err = register_chrdev_region(dev_params, 1, DEVICE_NAME);
    if (err < 0) {
        pr_alert("register_chrdev_region ERROR: %d", err);
        return err;
    }

    my_cdev = cdev_alloc();
    cdev_init(my_cdev, &fops);

    return 0;
}

static void __exit chardev_exit(void)
{
    pr_info("Goodbye, chardev!\n");
    cdev_del(my_cdev);
}


module_init(chardev_init);
module_exit(chardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("k.kryukov");
MODULE_DESCRIPTION("Character device driver");
