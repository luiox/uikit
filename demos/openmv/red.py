import pyb
from machine import LED
import sensor, image, time
from pyb import Servo
from pid import PID
from machine import I2C,Pin #从 machine 模块导入 I2C、 Pin 子模块
from ssd1306 import SSD1306_I2C #从 ssd1306 模块中导入 SSD1306_I2C 子模块
# 初始化  sda--p1   scl--p0

i2c = I2C(sda=Pin("P5"), scl=Pin("P4"),freq=80000)
#OLED 显示屏初始化： 128*64 分辨率,OLED 的 I2C 地址是 0x3c
oled = SSD1306_I2C(128, 64, i2c, addr=0x3c)
led = LED("LED_BLUE")
global num,mode
num=0
mode=0
function=0
pencil_points = []  #铅笔画的方形的定点坐标和中心点坐标，共5组
A4_points= []
jiguang_threshold_on_pencil_value=[]#激光在铅笔上的坐标角度
now_points=[]#记录现在的角度
pan_servo=Servo(1)#左右
tilt_servo=Servo(2)#上下

pan_servo.calibration(500,1950,500)
tilt_servo.calibration(500, 1950,500)

jiguang_threshold=[(16, 59, 10, 71, -7, 22)]
black_threshold = [(0, 10, -9, 8, -11, 127)]
#pan_pid = PID(p=0.07, i=0, imax=90) #脱机运行或者禁用图像传输，使用这个PID，一般调整p量
#tilt_pid = PID(p=0.05, i=0, imax=90) #脱机运行或者禁用图像传输，使用这个PID，一般调整p量
pan_pid = PID(p=0.06, i=0, imax=90)#在线调试使用这个PID
tilt_pid = PID(p=0.06, i=0, imax=90)#在线调试使用这个PID


def senser_original():
    sensor.reset()
    sensor.set_auto_gain(False)
    sensor.set_pixformat(sensor.RGB565) # or sensor.RGB565
    sensor.set_framesize(sensor.  QVGA) # or sensor.QVGA (or others)
    sensor.set_auto_exposure(False)
    sensor.skip_frames(time=2000) # Let new settings take affect.
    sensor.set_auto_whitebal(False) # turn this off.
    sensor.set_auto_gain(False) # 关闭增益（色块识别时必须要关）
    sensor.set_contrast(6) #对比度

def senser_original2():
    sensor.reset()
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QVGA)
    sensor.skip_frames(time=2000)
    sensor.set_auto_gain(False)  # must be turned off for color tracking
    sensor.set_auto_whitebal(False)  # must be turned off for color tracking
    sensor.set_contrast(4)

def find_max(blobs):
    max_size=0
    for blob in blobs:
        if blob[2]*blob[3] > max_size:
            max_blob=blob
            max_size = blob[2]*blob[3]
    if max_size==0:
        return 0
    else:
        return max_blob

def find_A4_traget():
    global A4_points
    senser_original2()
    print("我在这一步了")
    max_blob=0
    img = sensor.snapshot()
    blobs = img.find_blobs(black_threshold)
    if blobs:#first,if blobs
        max_blob = find_max(blobs)
        flag3= True
        while(max_blob!=0 and flag3):
            max_blob = find_max(blobs)
            img.draw_rectangle(max_blob.rect())
            img.draw_cross(max_blob.cx(), max_blob.cy())
            A4_points= [[max_blob.x(),max_blob.y()],[max_blob.x()+max_blob.w(),max_blob.y()],[max_blob.x()+max_blob.w(),max_blob.y()+max_blob.h()],[max_blob.x(),max_blob.y()+max_blob.h()]]
            oled.fill(0)#清屏
            oled.text("x1: " + str(max_blob.x()) + " y1: " + str(max_blob.y()), 10, 5)
            oled.text("x2: " + str(max_blob.x()+max_blob.w())  + " y2: " + str(max_blob.y()), 10, 15)
            oled.text("x3: " + str(max_blob.x()+max_blob.w())  + " y3: " + str(max_blob.y()+max_blob.h()), 10, 25)
            oled.text("x4: " + str(max_blob.x())  + " y4: " + str(max_blob.y()+max_blob.h()), 10, 35)
            oled.show()#OLED显示生效


def move_to_traget(target_x, target_y, now_x, now_y):
    pan_error = now_x-target_x
    tilt_error = now_y-target_y

    pan_output=pan_pid.get_pid(pan_error,1)/2
    tilt_output=tilt_pid.get_pid(tilt_error,1)
    pan_servo.angle(pan_servo.angle()+pan_output)
    print("new pan_angle",pan_servo.angle())
    tilt_servo.angle(tilt_servo.angle()-tilt_output)
    print("new tilt_angle",tilt_servo.angle())


def cam_calibration(num,flag):
    global pencil_points,laser_threshold
    senser_original()
    loop = True
    while loop:
        img = sensor.snapshot()
        blobs = img.find_blobs(jiguang_threshold)
        if blobs:
            jiguang_max_blob=find_max(blobs)
            img.draw_cross(blobs[0].cx(),blobs[0].cy(),color=(0,0,0))
            img.draw_rectangle( jiguang_max_blob.rect())
            print("flag=",flag)
            if (num==1 and flag==0):
                pencil_points.append([blobs[0].cx(),blobs[0].cy()])
                print(pencil_points)
                oled.text(str(pencil_points[0]) , 5, 0)
                oled.show()#OLED显示生效
                if (pencil_points[0]!=0):
                    flag=1
                    loop=False
            elif (num==2 and flag==1):
                pencil_points.append([blobs[0].cx(),blobs[0].cy()])
                print(pencil_points)
                oled.text(str(pencil_points[1]) , 15, 0)
                oled.show()#OLED显示生效
                if (pencil_points[1]!=0):
                    flag=2
                    loop=False
            elif (num==3 and flag==2):
                pencil_points.append([blobs[0].cx(),blobs[0].cy()])
                print(pencil_points)
                oled.text(str(pencil_points[2]) , 5, 5)
                oled.show()#OLED显示生效
                if (pencil_points[2]!=0):
                    flag=3
                    loop=False
            elif (num==4 and flag==3):
                pencil_points.append([blobs[0].cx(),blobs[0].cy()])
                oled.text(str(pencil_points[3]) , 15, 5)
                oled.show()#OLED显示生效
                print(pencil_points)
                if (pencil_points[3]!=0):
                    flag=4
                    loop=False
            elif (num==5 and flag==4):
                pencil_points.append([blobs[0].cx(),blobs[0].cy()])
                oled.text(str(pencil_points[4]) , 5, 10)
                oled.show()#OLED显示生效
                print(pencil_points)
                if (pencil_points[4]!=0):
                    flag=5
                    loop=False
            else :
                loop=False

def jiguang_calibration(traget):
    global pencil_points,jiguang_threshold,jiguang_threshol, d_on_pencil_value
    senser_original()

    move_pencil_step_tot = 30 #每个边细分成多少份进行移动
    move_pencil_point_n = 0  #临时存储变量，用于存储当前执行到铅笔方形的哪个角
    move_pencil_step_n = 0  #临时存储变量，用于存储在当前边的第几步。步总数参考上述变量。
    loop = True
    while loop:
        img = sensor.snapshot()
        blobs = img.find_blobs(jiguang_threshold)
        if blobs:
            max_blob = find_max(blobs)

            img.draw_rectangle(max_blob.rect()) # rect
            img.draw_cross(max_blob.cx(), max_blob.cy()) # cx, cy
            if len(traget>3):
                if move_pencil_point_n == 3: #最后一定点，需要和第一个点作差
                    x = (traget[0][0] - traget[3][0]) / move_pencil_step_tot *move_pencil_step_n + traget[3][0]
                    y = (traget[0][1] - traget[3][1]) / move_pencil_step_tot *move_pencil_step_n + traget[3][1]
                else:
                    x = (traget[move_pencil_point_n + 1][0] - traget[move_pencil_point_n][0]) / move_pencil_step_tot *move_pencil_step_n +traget[move_pencil_point_n][0]
                    y = (traget[move_pencil_point_n + 1][1] - traget[move_pencil_point_n][1]) / move_pencil_step_tot *move_pencil_step_n +traget[move_pencil_point_n][1]
                #print('x:'+str(x)+',y:'+str(y))
                move_to_traget(x, y, blobs[0].cx(),blobs[0].cy())
                move_pencil_step_n += 1 #每边细分的步骤加1
                if move_pencil_step_n > move_pencil_step_tot-1:   #每边加满
                    move_pencil_step_n = 0  #步数清零
                    move_pencil_point_n += 1    #边序号加一
                    if move_pencil_point_n > 3: #边序号满
                        move_pencil_point_n = 0 #边序号清零
            if len(traget<=2):
                x = (traget[0][0] - traget[1][0]) / move_pencil_step_tot *move_pencil_step_n + traget[1][0]
                y = (traget[0][1] - traget[1][1]) / move_pencil_step_tot *move_pencil_step_n + traget[1][1]
                move_to_traget(traget[0][0],traget[0][1], blobs[0].cx(),blobs[0].cy())
                move_pencil_step_n += 1 #每边细分的步骤加1
                if move_pencil_step_n > move_pencil_step_tot-1:   #每边加满
                    move_pencil_step_n = 0  #步数清零
                    move_pencil_point_n += 1    #边序号加一
                    if move_pencil_point_n > 0: #边序号满
                        move_pencil_point_n = 0 #边序号清零
def key1_control(num,mode):#记录五个光点
    global flag
    p1 = pyb.Pin("P0", pyb.Pin.IN,pyb.Pin.PULL_UP)
    while(num<7 and mode==0):
        key1_value = p1.value() # Returns 0 or 1.
        while (num==0 and key1_value==1):
            key1_value = p1.value()
            if(key1_value==0):
                num=1
                flag=0
        while (num==1 and key1_value==1):
            key1_value = p1.value()
            if(key1_value==0):
                cam_calibration(num,flag)
                num=2
                flag=1
        while (num==2 and key1_value==1):
            key1_value = p1.value()
            if(key1_value==0):
                cam_calibration(num,flag)
                num=3
                flag=2
        while (num==3 and key1_value==1):
            key1_value = p1.value()
            if(key1_value==0):
                cam_calibration(num,flag)
                num=4
                flag=3
        while (num==4 and key1_value==1):
            key1_value = p1.value()
            if(key1_value==0):
                cam_calibration(num,flag)
                num=5
                flag=4
        while (num==5 and key1_value==1):
            key1_value = p1.value()
            if(key1_value==0):
                cam_calibration(num,flag)
                num=6
                flag=5
        while (num==6 and key1_value==1):
            key1_value = p1.value()
            if(key1_value==0):
                num=7
                mode=1
    if (num==7 and mode==1):
        return num


def key3_control(points,flag2):#实现复位（要输入两个坐标）
    global function,now_point,now_points;
    p3 = pyb.Pin("P2", pyb.Pin.IN,pyb.Pin.PULL_UP)
    key3_value = p3.value() # Returns 0 or 1.
    if(function<3 and flag2==True):
        now_points=[pan_servo.angle,tilt_servo.angle]
        if(function==0 and key3_value==1):#判断是否要进入循环，是否需要复位
            key3_value = p3.value()
            if(key3_value==0):
                function=1
                print("在这1")
        while (function==1 and key3_value==1):
            key3_value = p3.value()
            now_points=[ pan_servo.angle(),tilt_servo.angle() ]#记录现在的位置
            print(now_points)
            print("zaizheli")
            new_points=[now_points,pencil_points[4]]
            if(key3_value==0):
                print(pencil_points[4])#回到这个位置pencil_points[4]
                jiguang_calibration(new_points)
                function=2
        while(function==2 and key3_value==1):
            key3_value = p3.value()
            new_points2=[pencil_points[4],now_points]
            if(key3_value==0):
                function=0#回到原来的位置
                jiguang_calibration(new_points2)
                flag2=False



def key2_control(mode):#实现几个基础功能
    global k,num,function,flag2;

    p2 = pyb.Pin("P1", pyb.Pin.IN,pyb.Pin.PULL_UP)
    while(mode<=5):
        key2_value = p2.value() # Returns 0 or 1.
        while (mode==0 and key2_value==1):#记录五个激光点的位置
            k=key1_control(num,mode)
            if len(pencil_points)>=5:
                mode=1
                num=0
        while(mode==1 and key2_value==1):#实现复位功能
            flag2=True
            key2_value = p2.value()
            k=len(pencil_points)

            key3_control(pencil_points[k-1],flag2)
            if(key2_value==0):
                mode=2
                print(mode)
        while (mode==2 and key2_value==1):#实现铅笔绕圈功能
            key2_value = p2.value()
            flag2=True
            key3_control(pencil_points[k-1],flag2)
            jiguang_calibration(pencil_points)
            if(key2_value==0):
                mode=3
                print(mode)
        while (mode==3 and key2_value==1):#实现A4纸巡线功能
            find_A4_traget()
            key2_value = p2.value()
            if(key2_value==0):
                mode=4
                print(mode)
        while (mode==4 and key2_value==1):#实现A4纸随意角度巡线功能
            key2_value = p2.value()
            if(key2_value==0):
                mode=5
                print(mode)
        while (mode==5 and key2_value==1):
            key2_value = p2.value()
            if(key2_value==0):
                mode=6
                print(mode)
while True:
    key2_control(mode)
