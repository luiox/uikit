#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/string.h>
#include <linux/vmalloc.h>

static int __init kfifo_demo_init(void)
{
    struct kfifo my_kfifo;
    int ret;
    u32 i;

    ret = kfifo_alloc(&my_kfifo, 1024, GFP_KERNEL);
    if (ret != 0){
        printk(KERN_ERR "kfifo_alloc failed\n");
        return -ENOMEM;
    }

    // 往队列里面写数据
    for(i = 0; i < 10; i++) {
        // 我们一次写1个u32，对于kfifo，尽可能用大小明确的类型
        kfifo_in(&my_kfifo, &i, sizeof(u32));
        printk(KERN_INFO "kfifo_in: %u\n", i);
    }

    // 从队列里面读数据
    while(kfifo_len(&my_kfifo) > 0) {
        u32 val;
        kfifo_out(&my_kfifo, &val, sizeof(u32));
        printk(KERN_INFO "kfifo_out: %u\n", val);
    }

    kfifo_free(&my_kfifo);
    return 0;
}

static void __exit kfifo_demo_exit(void)
{

}

module_init(kfifo_demo_init);
module_exit(kfifo_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Canrad");
