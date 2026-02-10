#include <stdio.h>

/* Call into the LVGL demo entry */
extern int main_lvgl(void);

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    return main_lvgl();
}
