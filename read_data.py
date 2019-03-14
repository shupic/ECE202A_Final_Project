import bluepy.btle as btle
#import binascii
import pyautogui
pyautogui.FAILSAFE=False
pyautogui.PAUSE = 0.001
move_factor = 0.4

# connect to hexiwear with known MAC address 
dev = btle.Peripheral("00:1D:40:0B:00:45")
print("Connected to Hexiwear")

#print out the hexiwear's services
for svc in dev.services:
    print( str(svc))
    
#get the service for the accelerometer 
s = dev.getServiceByUUID("00002000-0000-1000-8000-00805f9b34fb")

#read out the data for this service 
c = s.getCharacteristics()[0]

try:
    send_ex = -1
    while(True):
        #read the accelerometer data
        data = c.read()
        data_0 = data[:2]
        data_1 = data[2:4]
        data_2 = data[-2:]
        #read out each byte from the 3 bytes accelerometer data
        data_0_temp = int.from_bytes(data_0, byteorder = 'big', signed = True)
        data_1_temp = int.from_bytes(data_1, byteorder = 'big', signed = True)
        data_2_temp = int.from_bytes(data_2, byteorder = 'big', signed = True)
        #read the last bit of the most significant bytes for only collecting the new data 
        send = (data_0_temp & int("000000000001", 16))
        if (send != send_ex):
            # move mouse cursor 
            if (data_0_temp == 2 or data_0_temp == 3):
                pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
            # perform click  
            elif (data_0_temp == 4 or data_0_temp == 5):
                pyautogui.click()
            # perform scroll up by 5 
            elif (data_0_temp == 6 or data_0_temp == 7):
                pyautogui.scroll(5)
            # perform scroll down by 5
            elif (data_0_temp == 8 or data_0_temp == 9):
                pyautogui.scroll(-5)
            # perform zoom in 
            elif (data_0_temp == 10 or data_0_temp == 11):
                pyautogui.hotkey('ctrl', 'shift', '+')
            # perform zoom out
            elif (data_0_temp == 12 or data_0_temp == 13):
                pyautogui.hotkey('ctrl', 'shift', '-')
            # screen shot hotkey 
            elif (data_0_temp == 14 or data_0_temp == 15):
                pyautogui.hotkey('Print')
            # print the acclerometer data out 
            print(data_0_temp, " ", data_1_temp, " ", data_2_temp) 
                        
        send_ex = send
except KeyboardInterrupt:
    print("Keyboard Interrupt")
    dev.disconnect()

finally: 
    dev.disconnect()

