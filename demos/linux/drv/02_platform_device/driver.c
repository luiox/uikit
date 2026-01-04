#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

/* 
 * 当设备和驱动匹配成功时，内核会调用这个 probe 函数
 */
static int my_driver_probe(struct platform_device *pdev)
{
    struct resource *res;
    printk("platform: driver matched and probe called!\n");

    /* 演示如何从设备中获取资源 */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (res) {
        printk("platform: got MEM resource: start=0x%llx\n", res->start);
    }

    res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    if (res) {
        printk("platform: got IRQ resource: %lld\n", res->start);
    }

    return 0;
}

/* 当设备或驱动被卸载时，调用 remove */
static int my_driver_remove(struct platform_device *pdev)
{
    printk("platform: driver removed\n");
    return 0;
}

/* 定义平台驱动 */
static struct platform_driver my_platform_driver = {
    .driver = {
        .name = "my-demo-device", /* 必须和 device.c 里的名字完全一致才能匹配 */
        .owner = THIS_MODULE,
    },
    .probe = my_driver_probe,
    .remove = my_driver_remove,
};

static int __init my_driver_init(void)
{
    int ret;
    /* 注册平台驱动 */
    ret = platform_driver_register(&my_platform_driver);
    if (ret) {
        printk("platform: driver register failed\n");
        return ret;
    }
    printk("platform: driver module loaded\n");
    return 0;
}

static void __exit my_driver_exit(void)
{
    /* 注销平台驱动 */
    platform_driver_unregister(&my_platform_driver);
    printk("platform: driver module unloaded\n");
}

module_init(my_driver_init);
module_exit(my_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Canrad");
MODULE_DESCRIPTION("Platform Driver Demo");
