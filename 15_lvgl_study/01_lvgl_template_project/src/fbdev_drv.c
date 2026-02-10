#include "lvgl.h"
#include "../lvgl/src/drivers/display/fb/lv_linux_fbdev.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

static void *tick_thread(void *arg)
{
    (void)arg;
    while (1) {
        lv_tick_inc(5);
        usleep(5000);
    }
    return NULL;
}

int fbdev_init(const char *fbpath)
{
    /* Use LVGL's built-in Linux framebuffer driver (lv_linux_fbdev) */
    lv_display_t * disp = lv_linux_fbdev_create();
    if (!disp) {
        fprintf(stderr, "lv_linux_fbdev_create failed\n");
        return -1;
    }
    if (fbpath) {
        lv_linux_fbdev_set_file(disp, fbpath);
    }

    /* start tick thread for LVGL */
    pthread_t tid;
    pthread_create(&tid, NULL, tick_thread, NULL);

    return 0;
}
