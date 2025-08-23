#include <linux/module.h>
#include <linux/kernel.h>

int __init first_lkm_init(void){
	printk(KERN_INFO "My First Kernel module is loaded!\n");
	return 0;
}

void __exit first_lkm_exit(void){
	printk(KERN_INFO "My First Kernel module exit!\n");
}

module_init(first_lkm_init);
module_exit(first_lkm_exit);

/* 模块信息 */
MODULE_LICENSE("MIT");
MODULE_VERSION("V1.0");
MODULE_DESCRIPTION("Canrad's first LKM");
MODULE_AUTHOR("Canrad");
