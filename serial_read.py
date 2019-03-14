import serial
import pyautogui
import numpy as np
ser = serial.Serial('/dev/tty.usbmodem1412', 9600, bytesize=8, timeout=2)
pyautogui.FAILSAFE = False
pyautogui.PAUSE = 0.01

MOVE_FACTOR = 0.3
#move factor to move the cursor.

print(ser.name)

#dummy variable
in_process = 0

while(ser.is_open):
    line = ser.readline()
    d_line = str(line,encoding='utf-8',errors='strict')
    cop = str(d_line)
                #print(d_line)
    s = cop.split(" ")
                #print(s)
    if len(s) == 9:
        gx = float(s[0])
        gy = float(s[1])
        gz = float(s[2])
        roll = float(s[3])
        pitch = float(s[4])
        ax = float(s[5])
        ay = float(s[6])
        az = float(s[7])
        #moving cursor by orientation 
        vx = roll
        vy = pitch

        #print(vx,' ',vy)
        #print('in_process ',in_process

        max_ind = 4
        #vx = gy
        #vy = gz
        print(gx,gy,gz,roll,pitch,ax,ay,az)
        #print(gx,gy,gz,ax,ay,az,roll,pitch)
        if  gx == 6 or gx == 7 :
            in_process = 1
            pyautogui.scroll(-5)
            #print(11)
        elif gx == 8 or gx == 9 :
            in_process = 1
            pyautogui.scroll(5)
            #print(22)  
        elif gx == 10 or gx == 11 :
            in_process = 1
            pyautogui.hotkey('command','+')
        elif gx == 12 or gx == 13 :
             in_process = 1
             pyautogui.hotkey('command','-')
        elif gx == 14 or gx == 15:
             in_process = 1
             pyautogui.hotkey('command','shift','3')
                                    
        elif gx == 4 or gx == 5 :
             pyautogui.click()
        elif(abs(vx) > 10 or abs(vy) > 10):
            in_process = 1;
            pyautogui.moveRel(vx*MOVE_FACTOR,-vy*MOVE_FACTOR)
                #pyautogui.moveRel(vx*MOVE_FACTOR,-vy*MOVE_FACTOR)
                #pyautogui.moveRel(vx*MOVE_FACTOR,-vy*MOVE_FACTOR)                    
ser.close()
		
