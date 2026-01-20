# PWM 控制例子
#
# 这个例子展示了如何使用OpenMV的PWM

import sensor, image, time
from pyb import Pin, Timer
from pyb import Servo

sensor.reset()
sensor.set_auto_gain(False)
sensor.set_pixformat(sensor.RGB565) # or sensor.RGB565
sensor.set_framesize(sensor.  QVGA) # or sensor.QVGA (or others)
sensor.set_auto_exposure(False)
sensor.set_vflip(True)
sensor.set_hmirror(True)
sensor.set_auto_whitebal(False) # turn this off.
sensor.set_auto_gain(False) # 关闭增益（色块识别时必须要关）
sensor.set_auto_exposure(1000)
sensor.set_contrast(10) #对比度
sensor.skip_frames(time=2000) # Let new settings take affect.
clock = time.clock()
tim = Timer(4, freq=50) #设置频率，初始化定时器4，将其设置为50HZ，也就是说一个PWM周期为20ms


# 生成50HZ方波，使用TIM4，channels 1，2，3分别是 5% ， 10% ，15占空比。
tim.channel(1, Timer.PWM, pin=Pin("P7"), pulse_width=3290)#水平
tim.channel(2, Timer.PWM, pin=Pin("P8"), pulse_width=3140)#竖直
j=0



while (True):
    clock.tick()
    img = sensor.snapshot()
    for i in range(3280, 2620, -10):#水平运动
##
        tim.channel(1).pulse_width(i)
        time.sleep_ms(100)
    for i in range(3170, 2530, -10): # 竖直运动
        if (i==2950):
            tim.channel(1).pulse_width(2630)
            print("zhi")
            time.sleep_ms(100)
        if (i==2800):
            tim.channel(1).pulse_width(2620)
            print("li")
            time.sleep_ms(100)
        if (i==2750):
            tim.channel(1).pulse_width(2610)
            print("li")
            time.sleep_ms(100)
        if (i==2700):
            tim.channel(1).pulse_width(2600)
            print("li")
            time.sleep_ms(100)
        tim.channel(2).pulse_width(i) # 设置PWM通道1的脉冲宽度
        time.sleep_ms(100)
    for i in range(2600, 3270, 10): # 水平运动
        tim.channel(1).pulse_width(i) #
        time.sleep_ms(100)
    for i in range(2550,3180 , 10): # 竖直运动
        tim.channel(2).pulse_width(i) #
        time.sleep_ms(100)
##回中心点
    tim.channel(2).pulse_width(2800)
    tim.channel(1).pulse_width(2920)
    time.sleep_ms(2000)
#水平运动开始前的竖直
    tim.channel(2).pulse_width(3120)
    # for i in range(3250, 2650, -50): # 脉冲宽度从2000us到500us递减
    #     tim.channel(2).pulse_width(i) # 设置PWM通道1的脉冲宽度
    #     time.sleep_ms(800)
