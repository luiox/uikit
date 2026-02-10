#include "lvgl.h"
#include <stdio.h>
#include <unistd.h>

/* External init from fbdev driver */
int fbdev_init(const char *fbpath);

int main_lvgl(void)
{
    if (fbdev_init("/dev/fb0") != 0) {
        printf("fbdev init failed\n");
        return -1;
    }

    lv_init();

    /* create a simple label */
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello LVGL on 7\" RGB LCD");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    /* main loop */
    while (1) {
        lv_timer_handler(); /* v8+ LVGL uses lv_timer_handler / lv_task_handler compatibility */
        usleep(5000);
    }
    return 0;
}

/* main is defined in src/main.c to avoid duplicate symbol */