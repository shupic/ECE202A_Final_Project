import serial
import pyautogui
import numpy as np
ser = serial.Serial('/dev/tty.usbmodem1412', 9600, bytesize=8, timeout=2)
pyautogui.FAILSAFE = False
pyautogui.PAUSE = 0.01

MOVE_FACTOR = 0.1
#move factor to move the cursor.

print(ser.name)
data_serial = [0,0,0,0,0,0,0,0]
fl = 0
# ML coff
coffe_0 = [ 3.26058485e-03, -4.89987206e-03, -2.05877017e-02,  3.16671988e-03,\
   5.79273382e+00, -1.60204263e+00]

coffe_1 = [ 1.22703505e-03, -2.06662456e-04, -1.68982080e-02,  1.94460189e-02,\
  -4.50079549e+00,  1.09530008e+00]

coffe_2 = [-2.84085412e-04, -3.18166798e-03, -7.44055441e-03,  2.84450999e-02,\
  -1.19638051e+00,  5.25903292e+00]

coffe_3 = [ 7.09153276e-03, -4.21382224e-04, -1.51337127e-02,  5.92974483e-03,\
   2.22769372e+00, -8.17349130e+00]

bias_0 = -2.069
bias_1 = -2.005
bias_2 = -1.976
bias_3 = -2.617

# ML result
data_result = [0,0,0,0]

#record two consuctive reading 

#difference in x direction and y direction. 

vx = 0
vy = 0

roll_3_buffer = [0,0,0]
pitch_3_buffer = [0,0,0]
gx_buffer = [0,0,0]

#buffer index
t = 0

#in process tag
in_process = 0

while(ser.is_open):
    line = ser.readline()
    d_line = str(line,encoding='utf-8',errors='strict')
    cop = str(d_line)
                #print(d_line)
    s = cop.split(" ")
                #print(s)
    gx = float(s[0])
    gy = float(s[1])
    gz = float(s[2])
    roll = float(s[3])
    pitch = float(s[4])
    ax = float(s[5])
    ay = float(s[6])
    az = float(s[7])
    #print(az)
    if t<3 :
        gx_buffer[t] = gx
        t = t + 1
    else :
        gx_buffer[0] = gx
        t = 1
                
                #moving cursor by orientation 
    vx = roll
    vy = pitch

    #print(vx,' ',vy)
    #print('in_process ',in_process)

    data_pak = [gx,gy,roll,pitch,ax,ay]
    data_result[0] = np.dot(data_pak,coffe_0) + bias_0
    data_result[1] = np.dot(data_pak,coffe_1) + bias_1
    data_result[2] = np.dot(data_pak,coffe_2) + bias_2
    data_result[3] = np.dot(data_pak,coffe_3) + bias_3

    max_ind = 4
    #vx = gy
    #vy = gz

    if max(data_result) > 0 : 
        max_ind = data_result.index(max(data_result))
    print(gx,gy,gz,roll,pitch,ax,ay,az)
    #print(gx,gy,gz,ax,ay,az,roll,pitch)
    if  gx == 1 :
        #in_process = 1
        pyautogui.scroll(-5)
        print(11)
    elif gx == 2 :
        #in_process = 1
        pyautogui.scroll(5)
        print(22)  
    elif gx == 3 :
        #in_process = 1
        pyautogui.hotkey('ctrl','shift','+')
    elif gx == 4 :
        #in_process = 1
        pyautogui.hotkey('ctrl','shift','-')
    elif gx == 5:
        pyautogui.hotkey('Print')
                                
                                
    elif(abs(vx) > 10 or abs(vy) > 10):
            pyautogui.moveRel(vx*MOVE_FACTOR,-vy*MOVE_FACTOR)
            #pyautogui.moveRel(vx*MOVE_FACTOR,-vy*MOVE_FACTOR)
            #pyautogui.moveRel(vx*MOVE_FACTOR,-vy*MOVE_FACTOR)                    
ser.close()
		
