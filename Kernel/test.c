#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>#include <linux/init.h>
#include <linux/fs.h>
#include <asm/io.h>

#define DEVICE_NAME "test"
#define SUCCESS 0
#define ERROR -1

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marcin Kondej <markondej@gmail.com>");
MODULE_DESCRIPTION("Kernel test module");

static unsigned char *data = NULL;
static int major;
static struct class *cls;

static int device_open(struct inode *, struct file *);
static int device_close(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);

static struct file_operations fops = {
    .open = device_open,
    .read = device_read,
    .release = device_close
};

static int __init test_init(void) {
    int major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        pr_alert("Registering char device failed\n");
        return major;
    }

    cls = class_create(THIS_MODULE, DEVICE_NAME);
    device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);

    data = (unsigned char *)kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!data) {
        device_destroy(cls, MKDEV(major, 0));
        class_destroy(cls);
        unregister_chrdev(major, DEVICE_NAME);
        return ERROR;
    }

    pr_info("Test module initialized: 0x%16llx\n", virt_to_phys(data));
    return SUCCESS;
}

static void __exit test_exit(void) {
    kfree(data);
    data = NULL;

    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);
    unregister_chrdev(major, DEVICE_NAME);
    pr_info("Test module exit\n");
}

static int device_open(struct inode *inode, struct file *file) {
    try_module_get(THIS_MODULE);
    return SUCCESS;
}
static int device_close(struct inode *inode, struct file *file) {
    module_put(THIS_MODULE);    return SUCCESS;
}

static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset) {
    int bytes_read = 0;
    if (*offset >= sizeof(uintptr_t)) {        *offset = 0;        return 0;    }    while ((*offset < sizeof(uintptr_t)) && length) {        char byte = virt_to_phys(data) >> (int)(*offset * 8);        put_user(byte, buffer++);        bytes_read++;        (*offset)++;        length--;    }    return bytes_read;
}

module_init(test_init);
module_exit(test_exit);