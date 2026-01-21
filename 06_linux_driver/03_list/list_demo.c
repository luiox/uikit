#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/vmalloc.h>

struct student {
    int id;
    char* name;
    struct list_head list;
};

struct student* student_alloc(int id, const char* name)
{
    struct student* stu;

    stu = vmalloc(sizeof(struct student));
    if(stu == NULL){
        printk("vmalloc student failed\n");
        return NULL;
    }

    // 为字符串分配内存
    stu->name = vmalloc(strlen(name) + 1);
    if(stu->name == NULL){
        printk("vmalloc name failed\n");
        vfree(stu);
        return NULL;
    }

    // 初始化
    stu->id = id;
    strcpy(stu->name, name);
    INIT_LIST_HEAD(&stu->list);

    printk("Allocated student: id=%d, name=%s\n", stu->id, stu->name);

    return stu;
}

void student_free(struct student* stu)
{
    if(stu != NULL){
        if(stu->name != NULL){
            vfree(stu->name);
        }
        vfree(stu);
        printk("Freed student\n");
    }
}

// 链表的头节点
static LIST_HEAD(student_list);

// void list_move(struct list_head* list, struct list_head* head)
// {
//     list_del(list);
//     list_add(list, head);
// }

static int __init list_demo_init(void)
{
    struct student* stu1, *stu2, *stu3;
    
    stu1 = student_alloc(1, "stu1");
    if(stu1 == NULL){
        goto ERR_3;
    }

    stu2 = student_alloc(2, "stu2");
    if(stu2 == NULL){
        goto ERR_2;
    }

    stu3 = student_alloc(3, "stu3");
    if(stu3 == NULL){
        goto ERR_1;
    }

    // 纯头插法
    list_add(&stu3->list, &student_list);
    list_add(&stu2->list, &student_list);
    list_add(&stu1->list, &student_list);

    // 纯尾插法
    // list_add_tail(&stu1->list, &student_list);
    // list_add_tail(&stu2->list, &student_list);
    // list_add_tail(&stu3->list, &student_list);

    // 挨个尾插法
    // 第一步插入到当前的尾
    // list_add(&stu1->list, &student_list);
    // // 然后在插入到尾部的尾部
    // list_add(&stu2->list, &stu1->list);
    // // 然后在插入到尾部的尾部
    // list_add(&stu3->list, &stu2->list);

    // 删除stu2
    // list_del(&stu2->list);

    // 遍历
    {
        struct list_head* pos;
        list_for_each(pos, &student_list){
            struct student* stu = list_entry(pos, struct student, list);
            printk("Student: id=%d, name=%s\n", stu->id, stu->name);
        }
    }
    // 简化版遍历
    {
        struct student* stu;
        list_for_each_entry(stu, &student_list, list){
            printk("Student: id=%d, name=%s\n", stu->id, stu->name);
        }
    }

    // 循环删除所有
    {
        struct list_head *pos, *n;
        list_for_each_safe(pos, n, &student_list){
            struct student* stu = list_entry(pos, struct student, list);
            printk("Deleting Student: id=%d, name=%s\n", stu->id, stu->name);
            list_del(pos);
            student_free(stu);
        }
    }

    return 0;
ERR_1:
    student_free(stu2);
    
ERR_2:
    student_free(stu1);

ERR_3:
    return -ENOMEM;
}

static void __exit list_demo_exit(void)
{

}

module_init(list_demo_init);
module_exit(list_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Canrad");
