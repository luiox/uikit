#include "key_util.h"

key_t keys[5] = {0};
void  key_read_pin(key_t* key)
{
    // 读取按键的GPIO引脚状态
    // 把数据存到keyState中
    // key->keyState = HAL_GPIO_ReadPin(key->gpioPort, key->gpioPin);
    key->keyState = 0;
}

void key_scan()
{
    for (u32 i = 0; i < array_size(keys); i++) {
        key_read_pin(&keys[i]);
    }
    for (u32 i = 0; i < array_size(keys); i++) {
        // 按键状态判断
        switch (keys[i].judgeState) {
        case 0:
            if (keys[i].keyState == 0) {
                keys[i].judgeState = 1;
                keys[i].time       = 0;
            }
            break;
        case 1:
            if (keys[i].keyState == 0) {
                keys[i].judgeState = 2;
            }
            else {
                keys[i].judgeState = 0;
            }
            break;
        case 2:
            if (keys[i].keyState == 0) {
                keys[i].time++;
            }
            else {
                if (keys[i].time < 70) {
                    keys[i].singleFlag = 1;
                }
                else if (keys[i].time > 200) {
                    keys[i].longFlag = 1;
                }
                keys[i].judgeState = 0;
            }
            break;
        }
    }
}
