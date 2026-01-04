#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>

/* 
 * 当设备树中的节点与驱动匹配成功时，调用 probe
 */
static int my_dts_driver_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    const char *str;
    int value;

    printk("platform_dts: driver matched via Device Tree!\n");

    /* 1. 演示如何从设备树节点中读取自定义字符串属性 */
    if (of_property_read_string(np, "my-property-str", &str) == 0) {
        printk("platform_dts: found string property: %s\n", str);
    }

    /* 2. 演示如何读取自定义数字属性 */
    if (of_property_read_u32(np, "my-property-num", &value) == 0) {
        printk("platform_dts: found numeric property: %d\n", value);
    }

    /* 3. 演示如何获取标准的资源（如寄存器地址）
     * 内核会自动把设备树里的 'reg' 属性转换成 IORESOURCE_MEM 资源
     */
    struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (res) {
        printk("platform_dts: got reg resource: start=0x%llx\n", res->start);
    }

    return 0;
}

static int my_dts_driver_remove(struct platform_device *pdev)
{
    printk("platform_dts: driver removed\n");
    return 0;
}

/* 
 * 设备树匹配表
 * 这里的 compatible 字符串必须和 .dts 文件里的完全一致
 */
static const struct of_device_id my_of_match[] = {
    { .compatible = "alientek,my-demo-device" },
    { /* 哨兵，必须为空 */ }
};
MODULE_DEVICE_TABLE(of, my_of_match);

static struct platform_driver my_dts_platform_driver = {
    .driver = {
        .name = "my-dts-demo",
        .of_match_table = my_of_match, /* 使用设备树匹配表 */
        .owner = THIS_MODULE,
    },
    .probe = my_dts_driver_probe,
    .remove = my_dts_driver_remove,
};

static int __init my_dts_driver_init(void)
{
    return platform_driver_register(&my_dts_platform_driver);
}

static void __exit my_dts_driver_exit(void)
{
    platform_driver_unregister(&my_dts_platform_driver);
}

module_init(my_dts_driver_init);
module_exit(my_dts_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Canrad");
MODULE_DESCRIPTION("Device Tree Platform Driver Demo");
