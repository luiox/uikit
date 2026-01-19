#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define NEWCHRDEV_CNT 1            /* 设备号个数 */
#define NEWCHRDEV_NAME "newchrdev" /* 名字 */

/* 设备结构体 */
struct newchrdev_dev
{
    dev_t       devid;     /* 设备号 	 */
    struct cdev cdev;      /* cdev 	 */
    struct class* class;   /* 类 		 */
    struct device* device; /* 设备 	 */
    int            major;  /* 主设备号	 */
    int            minor;  /* 次设备号  */
};

struct newchrdev_dev newchrdev; /* 设备实例 */

static int newchrdev_open(struct inode* inode, struct file* filp)
{
    filp->private_data = &newchrdev; /* 设置私有数据 */
    printk("newchrdev open!\r\n");
    return 0;
}

static ssize_t newchrdev_read(struct file* filp, char __user* buf, size_t cnt, loff_t* offt)
{
    printk("newchrdev read!\r\n");
    return 0;
}

static ssize_t newchrdev_write(struct file* filp, const char __user* buf, size_t cnt, loff_t* offt)
{
    int           retvalue;
    unsigned char databuf[1024];

    if (cnt > 1024){
		cnt = 1024;
	}
	
    retvalue = copy_from_user(databuf, buf, cnt);
    if (retvalue == 0) {
        printk("kernel received data: %s\r\n", databuf);
    }
    else {
        printk("kernel received data failed!\r\n");
    }

    return cnt;
}

static int newchrdev_release(struct inode* inode, struct file* filp)
{
    printk("newchrdev release!\r\n");
    return 0;
}

static struct file_operations newchrdev_fops = {
    .owner   = THIS_MODULE,
    .open    = newchrdev_open,
    .read    = newchrdev_read,
    .write   = newchrdev_write,
    .release = newchrdev_release,
};

static int __init newchrdev_init(void)
{
    int ret;

    /* 1、分配设备号 */
    if (newchrdev.major) { /* 定义了主设备号 */
        newchrdev.devid = MKDEV(newchrdev.major, 0);
        ret             = register_chrdev_region(newchrdev.devid, NEWCHRDEV_CNT, NEWCHRDEV_NAME);
    }
    else { /* 没有定义主设备号，动态分配 */
        ret             = alloc_chrdev_region(&newchrdev.devid, 0, NEWCHRDEV_CNT, NEWCHRDEV_NAME);
        newchrdev.major = MAJOR(newchrdev.devid); /* 获取分配号的主设备号 */
        newchrdev.minor = MINOR(newchrdev.devid); /* 获取分配号的次设备号 */
    }

    if (ret < 0) {
        printk("newchrdev region error!\r\n");
        return -EINVAL;
    }
    printk("newchrdev major=%d, minor=%d\r\n", newchrdev.major, newchrdev.minor);

    /* 2、初始化cdev */
    newchrdev.cdev.owner = THIS_MODULE;
    cdev_init(&newchrdev.cdev, &newchrdev_fops);

    /* 3、添加一个cdev */
    ret = cdev_add(&newchrdev.cdev, newchrdev.devid, NEWCHRDEV_CNT);
    if (ret < 0) {
        goto free_region;
    }

    /* 4、创建类 */
    newchrdev.class = class_create(THIS_MODULE, NEWCHRDEV_NAME);
    if (IS_ERR(newchrdev.class)) {
        ret = PTR_ERR(newchrdev.class);
        goto del_cdev;
    }

    /* 5、创建设备 */
    newchrdev.device = device_create(newchrdev.class, NULL, newchrdev.devid, NULL, NEWCHRDEV_NAME);
    if (IS_ERR(newchrdev.device)) {
        ret = PTR_ERR(newchrdev.device);
        goto destroy_class;
    }

    printk("newchrdev init done!\r\n");
    return 0;

destroy_class:
    class_destroy(newchrdev.class);
del_cdev:
    cdev_del(&newchrdev.cdev);
free_region:
    unregister_chrdev_region(newchrdev.devid, NEWCHRDEV_CNT);
    return ret;
}

static void __exit newchrdev_exit(void)
{
    /* 注销字符设备驱动 */
    cdev_del(&newchrdev.cdev);                                /*  删除cdev */
    unregister_chrdev_region(newchrdev.devid, NEWCHRDEV_CNT); /* 注销设备号 */

    device_destroy(newchrdev.class, newchrdev.devid);
    class_destroy(newchrdev.class);

    printk("newchrdev exit done!\r\n");
}

module_init(newchrdev_init);
module_exit(newchrdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Canrad");
