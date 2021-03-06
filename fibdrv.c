#include "fibdrv.h"

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/timekeeping.h>
#include <linux/uaccess.h>

#include "bigint.h"

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("National Cheng Kung University, Taiwan");
MODULE_DESCRIPTION("Fibonacci engine driver");
MODULE_VERSION("0.1");

#define DEV_FIBONACCI_NAME "fibonacci"

static dev_t fib_dev = 0;
static struct cdev *fib_cdev;
static struct class *fib_class;
static DEFINE_MUTEX(fib_mutex);
uint128_t (*fib_func)(long long);

static uint128_t fib_sequence(long long k)
{
    /* FIXME: use clz/ctz and fast algorithms to speed up */
    uint128_t f[k + 2];

    f[0] = (uint128_t){.upper = 0, .lower = 0};
    f[1] = (uint128_t){.upper = 0, .lower = 1};

    for (int i = 2; i <= k; i++) {
        add128(f + i, f[i - 1], f[i - 2]);
    }

    return f[k];
}

static uint128_t fib_sequence_ll(long long k)
{
    /* FIXME: use clz/ctz and fast algorithms to speed up */
    u64 f[k + 2];
    uint128_t ret;
    ret.upper = 0;

    f[0] = 0;
    f[1] = 1;

    for (int i = 2; i <= k; i++) {
        f[i] = f[i - 1] + f[i - 2];
    }
    ret.lower = f[k];

    return ret;
}

static uint128_t fibseq_doubling(long long k)
{
    uint128_t a = {.lower = 0, .upper = 0};
    uint128_t b = {.lower = 1, .upper = 0};
    if (k <= 1) {
        return k == 0 ? a : b;
    }

    int clz = __builtin_clzll(k);

    for (int i = (64 - clz); i > 0; i--) {
        uint128_t t1, t2, tmp;
        lsft128(&tmp, b, 1);   // tmp = 2b
        sub128(&tmp, tmp, a);  // tmp = tmp -a
        mul128(&t1, a, tmp);   // t1 = a*tmp

        mul128(&a, a, a);   // a = a^2
        mul128(&b, b, b);   // b = b^2
        add128(&t2, a, b);  // t2 = a^2 + b^2
        a = t1;
        b = t2;
        if (k & (1ull << (i - 1))) {  // current bit == 1
            add128(&t1, a, b);
            a = b;
            b = t1;
        }
    }
    return a;
}

static uint128_t fibseq_doubling_ll(long long k)
{
    u64 a = 0;
    u64 b = 1;
    uint128_t ret = {0, 0};
    if (k <= 1) {
        return k == 0 ? (uint128_t){.lower = 0, .upper = 0}
                      : (uint128_t){.lower = 1, .upper = 0};
    }

    int clz = __builtin_clzll(k);

    for (int i = (64 - clz); i > 0; i--) {
        u64 t1, t2;
        t1 = a * ((b << 1) - a);
        t2 = a * a + b * b;

        a = t1;
        b = t2;
        if (k & (1ull << (i - 1))) {  // current bit == 1
            t1 = a + b;
            a = b;
            b = t1;
        }
    }
    ret.lower = a;
    return ret;
}

static int fib_open(struct inode *inode, struct file *file)
{
    printk("#########xxxxxx##########\n");
    if (!mutex_trylock(&fib_mutex)) {
        printk(KERN_ALERT "fibdrv is in use");
        return -EBUSY;
    }
    return 0;
}

static int fib_release(struct inode *inode, struct file *file)
{
    mutex_unlock(&fib_mutex);
    return 0;
}

/* calculate the fibonacci number at given offset */
static ssize_t fib_read(struct file *file,
                        char *buf,
                        size_t size,
                        loff_t *offset)
{
    u64 t1, t2;
    t1 = ktime_get_ns();
    uint128_t result = fib_func(*offset);
    t2 = ktime_get_ns();
    copy_to_user(buf, &result, sizeof(uint128_t));
    return t2 - t1;
}

/* write operation is skipped */
static ssize_t fib_write(struct file *file,
                         const char *buf,
                         size_t size,
                         loff_t *offset)
{
    char kbuf[10];
    copy_from_user(kbuf, buf, 10);
    switch (kbuf[0]) {
    case FIB_REG_128:
        printk("changing fib_func to regular one (128)\n");
        fib_func = fib_sequence;
        break;
    case FIB_DOUB_128:
        printk("changing fib_func to doubling one (128)\n");
        fib_func = fibseq_doubling;
        break;
    case FIB_DOUB_LL:
        printk("changing fib_func to doubling one (64)\n");
        fib_func = fibseq_doubling_ll;
        break;
    case FIB_REG_LL:
        printk("changing fib_func to regular one (64)\n");
        fib_func = fib_sequence_ll;
        break;
    default:
        printk("Invalid argument\n");
        return -EINVAL;
    }
    return 1;
}

static loff_t fib_device_lseek(struct file *file, loff_t offset, int orig)
{
    loff_t new_pos = 0;
    switch (orig) {
    case 0: /* SEEK_SET: */
        new_pos = offset;
        break;
    case 1: /* SEEK_CUR: */
        new_pos = file->f_pos + offset;
        break;
    case 2: /* SEEK_END: */
        new_pos = MAX_LENGTH - offset;
        break;
    }

    if (new_pos > MAX_LENGTH)
        new_pos = MAX_LENGTH;  // max case
    if (new_pos < 0)
        new_pos = 0;        // min case
    file->f_pos = new_pos;  // This is what we'll use now
    return new_pos;
}

const struct file_operations fib_fops = {
    .owner = THIS_MODULE,
    .read = fib_read,
    .write = fib_write,
    .open = fib_open,
    .release = fib_release,
    .llseek = fib_device_lseek,
};

static int __init init_fib_dev(void)
{
    int rc = 0;

    mutex_init(&fib_mutex);

    fib_func = fib_sequence;

    // Let's register the device
    // This will dynamically allocate the major number
    rc = alloc_chrdev_region(&fib_dev, 0, 1, DEV_FIBONACCI_NAME);

    if (rc < 0) {
        printk(KERN_ALERT
               "Failed to register the fibonacci char device. rc = %i",
               rc);
        return rc;
    }

    fib_cdev = cdev_alloc();
    if (fib_cdev == NULL) {
        printk(KERN_ALERT "Failed to alloc cdev");
        rc = -1;
        goto failed_cdev;
    }
    cdev_init(fib_cdev, &fib_fops);
    rc = cdev_add(fib_cdev, fib_dev, 1);

    if (rc < 0) {
        printk(KERN_ALERT "Failed to add cdev");
        rc = -2;
        goto failed_cdev;
    }

    fib_class = class_create(THIS_MODULE, DEV_FIBONACCI_NAME);

    if (!fib_class) {
        printk(KERN_ALERT "Failed to create device class");
        rc = -3;
        goto failed_class_create;
    }

    if (!device_create(fib_class, NULL, fib_dev, NULL, DEV_FIBONACCI_NAME)) {
        printk(KERN_ALERT "Failed to create device");
        rc = -4;
        goto failed_device_create;
    }
    return rc;
failed_device_create:
    class_destroy(fib_class);
failed_class_create:
    cdev_del(fib_cdev);
failed_cdev:
    unregister_chrdev_region(fib_dev, 1);
    return rc;
}

static void __exit exit_fib_dev(void)
{
    mutex_destroy(&fib_mutex);
    device_destroy(fib_class, fib_dev);
    class_destroy(fib_class);
    cdev_del(fib_cdev);
    unregister_chrdev_region(fib_dev, 1);
}

module_init(init_fib_dev);
module_exit(exit_fib_dev);
