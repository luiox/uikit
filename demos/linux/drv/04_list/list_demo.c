/*
 * list_demo.c - simple kernel linked-list demo
 * Demonstrates basic kernel list APIs in module __init as "main".
 *
 * Build: make (provided Makefile)
 * Test: insmod list_demo.ko; dmesg -w
 * Remove: rmmod list_demo
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/gfp.h>

struct demo_node {
    int val;
    struct list_head list;
};

static LIST_HEAD(head);
static int nr_nodes = 0;

static int __init list_demo_init(void)
{
    int i;
    struct demo_node *node, *tmp;

    pr_info("list_demo: init start\n");

    /* create and add nodes */
    for (i = 0; i < 6; i++) {
        node = kmalloc(sizeof(*node), GFP_KERNEL);
        if (!node) {
            pr_err("list_demo: kmalloc failed at i=%d\n", i);
            goto err_alloc;
        }
        node->val = i + 1;
        list_add_tail(&node->list, &head);
        nr_nodes++;
        pr_info("list_demo: added node val=%d\n", node->val);
    }

    /* iterate and print using list_for_each_entry */
    pr_info("list_demo: iterate forward\n");
    list_for_each_entry(node, &head, list)
        pr_info("list_demo: node val=%d\n", node->val);

    /* demonstrate safe removal during iteration */
    pr_info("list_demo: remove even nodes during iteration\n");
    list_for_each_entry_safe(node, tmp, &head, list) {
        if ((node->val & 1) == 0) {
            pr_info("list_demo: removing node val=%d\n", node->val);
            list_del(&node->list);
            kfree(node);
            nr_nodes--;
        }
    }

    /* show remaining nodes */
    pr_info("list_demo: remaining nodes\n");
    list_for_each_entry(node, &head, list)
        pr_info("list_demo: node val=%d\n", node->val);

    /* demonstrate splice: move nodes to a new list then back */
    {
        LIST_HEAD(tmp_list);
        pr_info("list_demo: splice to tmp_list and back\n");
        list_splice_init(&head, &tmp_list); /* moves head -> tmp_list, head becomes empty */
        pr_info("list_demo: head empty after splice? %s\n", list_empty(&head) ? "yes" : "no");
        list_splice_tail_init(&tmp_list, &head); /* move back */
        pr_info("list_demo: head empty after splice_back? %s\n", list_empty(&tmp_list) ? "yes" : "no");
    }

    pr_info("list_demo: init done, nr_nodes=%d\n", nr_nodes);
    return 0;

err_alloc:
    /* cleanup any allocated nodes */
    list_for_each_entry_safe(node, tmp, &head, list) {
        list_del(&node->list);
        kfree(node);
    }
    nr_nodes = 0;
    return -ENOMEM;
}

static void __exit list_demo_exit(void)
{
    struct demo_node *node, *tmp;

    pr_info("list_demo: exit start, cleaning %d nodes\n", nr_nodes);
    list_for_each_entry_safe(node, tmp, &head, list) {
        pr_info("list_demo: freeing node val=%d\n", node->val);
        list_del(&node->list);
        kfree(node);
        nr_nodes--;
    }

    pr_info("list_demo: exit done\n");
}

module_init(list_demo_init);
module_exit(list_demo_exit);
MODULE_LICENSE("GPL");
