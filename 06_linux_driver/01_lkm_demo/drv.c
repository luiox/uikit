#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int __init my_drv_init(void)
{
    printk(KERN_INFO "my_drv init done!\n");
    return 0;
}

static void __exit my_drv_exit(void)
{
    printk("my_drv exit done!\n");
    pr_debug("my_drv exit done! (pr_debug)\n");
    trace_printk("my_drv exit done! (trace_printk)\n");
    spin_lock();
}

module_init(my_drv_init);
module_exit(my_drv_exit);

MODULE_LICENSE("GPL");
MODULE_VERSION("V1.0");
MODULE_DESCRIPTION("Canrad's first LKM");
MODULE_AUTHOR("Canrad");
