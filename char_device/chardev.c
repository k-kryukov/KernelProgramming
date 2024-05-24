#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

// https://elixir.bootlin.com/linux/v6.9.1/source/drivers/char/scx200_gpio.c#L102

#define DEVICE_NAME "chardev"
#define BUF_LEN 128

static int counter = 0;

static char msg[BUF_LEN];

static char writeBuf[BUF_LEN];

static int handle_open(struct inode *, struct file *) {
    pr_info("handle_open");
    sprintf(msg, "I already told you %d times Hello world!\n", counter++);

    return 0;
}
static int handle_release(struct inode *, struct file *) {
    pr_info("handle_release");
    return 0;
}

static ssize_t handle_read(struct file * f, char __user * buf, size_t length, loff_t * offset) {
    size_t ret = 0;
    const char* it;

    pr_info("Offset is %i", (int)(*offset));

    it = msg;

    if (!*(it + *offset)) { /* мы находимся в конце сообщения. */
        *offset = 0; /* сброс смещения. */
        return 0; /* обозначение конца файла. */
    }
    it += *offset;

    while (length && *it) {
        put_user(*(it++), buf++);
        --length;
        ++ret;
    }

    *offset += ret;

    return ret;
}
static ssize_t handle_write(struct file *, const char __user * buff, size_t size, loff_t * offset) {
    if (copy_from_user(writeBuf, buff, size))
        return -EFAULT;

    pr_info("Write offset is %i", (int)(*offset));

    if (size < sizeof(writeBuf)) {
        writeBuf[size] = '\0';
        pr_info("Write buf is %s", writeBuf);
    }

    return size;
}

static struct file_operations fops = {
    .read = handle_read,
    .write = handle_write,
    .open = handle_open,
    .release = handle_release,
};

static int major;
static int minor;
static struct cdev my_cdev;
static struct class *cls;

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
    minor = MINOR(dev_params);

    cdev_init(&my_cdev, &fops);
	err = cdev_add(&my_cdev, dev_params, 1);
	if (err < 0) {
		pr_alert("Error: %i", err);
		return err;
	}

    // now creating a block device
    cls = class_create(THIS_MODULE, DEVICE_NAME);
    device_create(cls, NULL, MKDEV(major, minor), NULL, DEVICE_NAME);

    pr_info("Success!");

    return 0;
}

static void __exit chardev_exit(void)
{
    pr_info("Goodbye, chardev!\n");
    device_destroy(cls, MKDEV(major, minor));
    class_destroy(cls);

    cdev_del(&my_cdev);
	unregister_chrdev_region(MKDEV(major, minor), 1);
}


module_init(chardev_init);
module_exit(chardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("k.kryukov");
MODULE_DESCRIPTION("Character device driver");
