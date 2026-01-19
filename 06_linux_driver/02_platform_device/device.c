#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

/* 
 * 模拟硬件资源：比如寄存器地址、中断号等
 * 在实际开发中，这些信息现在通常写在设备树（DTS）里
 */
static struct resource my_resources[] = {
    [0] = {
        .start = 0x020C406C, /* 模拟一个寄存器起始地址 */
        .end   = 0x020C406C + 4 - 1,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = 3,          /* 模拟一个中断号 */
        .end   = 3,
        .flags = IORESOURCE_IRQ,
    },
};

static void my_device_release(struct device *dev)
{
    printk("platform: device released\n");
}

/* 定义平台设备 */
static struct platform_device my_platform_device = {
    .name = "my-demo-device", /* 名字！这是匹配的关键 */
    .id = -1,
    .num_resources = ARRAY_SIZE(my_resources),
    .resource = my_resources,
    .dev = {
        .release = my_device_release,
    },
};

static int __init my_device_init(void)
{
    int ret;
    /* 注册平台设备 */
    ret = platform_device_register(&my_platform_device);
    if (ret) {
        printk("platform: device register failed\n");
        return ret;
    }
    printk("platform: device module loaded\n");
    return 0;
}

static void __exit my_device_exit(void)
{
    /* 注销平台设备 */
    platform_device_unregister(&my_platform_device);
    printk("platform: device module unloaded\n");
}

module_init(my_device_init);
module_exit(my_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Canrad");
MODULE_DESCRIPTION("Platform Device Demo");
