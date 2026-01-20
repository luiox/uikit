import sensor, image, time
from pid import PID
from pyb import Servo
pan_servo=Servo(1)
tilt_servo=Servo(2)
pan_servo.calibration(500,2500,200)
tilt_servo.calibration(500,2500,200)
red_threshold  = (77, 100, -11, 16, -10, 7)
pan_pid = PID(p=0.015, i=0, imax=1)
tilt_pid = PID(p=0.02, i=0, imax=1)
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


sensor.skip_frames(time=2000)
clock = time.clock()
def find_max(blobs):
    max_size=0
    for blob in blobs:
        if blob[2]*blob[3] > max_size:
            max_blob=blob
            max_size = blob[2]*blob[3]
    return max_blob
while(True):
    clock.tick()
    img = sensor.snapshot()
    img.draw_line((0, 120, 320, 120), color=(255, 0, 0), thickness = 2)# 绘制垂直线
    img.draw_line((160, 0, 160, 240), color=(255, 0, 0), thickness = 2)
    blobs = img.find_blobs([red_threshold])
    if blobs:
        max_blob = find_max(blobs)
        pan_error = max_blob.cx()-img.width()/2
        tilt_error = max_blob.cy()-img.height()/2

        img.draw_rectangle(max_blob.rect())
        img.draw_cross(max_blob.cx(), max_blob.cy())
        if(tilt_error<0):
            pan_output=pan_pid.get_pid(pan_error,1)/2
            tilt_output=tilt_pid.get_pid(tilt_error,1)
        else:
            pan_output=pan_pid.get_pid(pan_error,1)/2
            tilt_output=(tilt_pid.get_pid(tilt_error,1))*1.5
        print("tilt_output",tilt_output)
        pan_servo.angle(pan_servo.angle()+pan_output)
        tilt_servo.angle(tilt_servo.angle()+tilt_output)
