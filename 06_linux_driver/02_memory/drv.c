#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>

static int __init newchrdev_init(void)
{
    
    return 0;
}

static void __exit newchrdev_exit(void)
{
    pr_info("newchrdev exit done!\n");
    
}

module_init(newchrdev_init);
module_exit(newchrdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Canrad");

