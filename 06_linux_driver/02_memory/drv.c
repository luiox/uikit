#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>

static int __init newchrdev_init(void)
{
    u8* buffer;
    s32 i;

    printk(KERN_INFO "My First Kernel module is loaded!\n");
    // 申请1024个字节的内存作为buffer
    buffer = (u8*)kmalloc(1024, GFP_KERNEL);
    if(buffer == NULL) {
        printk(KERN_WARNING "Memory allocation failed\n");
        return -ENOMEM;
    }
    printk(KERN_INFO "Memory allocated successfully!\n");

    // 使用内存，往里面循环写入数据
    for(i = 0; i < 1024; i++) {
        buffer[i] = (u8)(i % 256);
        printk(KERN_DEBUG "buffer[%d] = %u\n", i, buffer[i]);
    }
    printk(KERN_INFO "Data written to buffer successfully!\n");

    // 释放内存
    kfree(buffer);
    printk(KERN_INFO "Memory freed successfully!\n");

    return 0;
}

static void __exit newchrdev_exit(void)
{
    printk("My First Kernel module exit!\n");
}

module_init(newchrdev_init);
module_exit(newchrdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Canrad");

