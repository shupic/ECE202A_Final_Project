import bluepy.btle as btle
import binascii
import pyautogui
pyautogui.FAilSAFE=False
pyautogui.PAUSE = 0.001
move_factor = 0.1
dev = btle.Peripheral("00:1D:40:0B:00:45")
print("Connected to Hexiwear")
for svc in dev.services:
    print( str(svc))
#s = p.getServiceByUUID("0000180f-0000-1000-8000-00805f9b34fb")
s = dev.getServiceByUUID("00002000-0000-1000-8000-00805f9b34fb")
c = s.getCharacteristics()[0]

try:
	send_ex = -1
	while 1:
                #data = int(str(binascii.b2a_hex(c.read()), 'utf-8'), 16)
                #data_x = (data & int("00000000ffff", 16))
                #data_y = (data & int("0000ffff0000", 16)) >> 16
                #data_z = (data & int("ffff00000000", 16)) >> 32
                #data = int.from_bytes(c.read(), byteorder = 'big', signed = True)
                data = c.read()
                data_0 = data[:2]
                data_1 = data[2:4]
                data_2 = data[-2:]
                data_0_temp = int.from_bytes(data_0, byteorder = 'big', signed = True)
                data_1_temp = int.from_bytes(data_1, byteorder = 'big', signed = True)
                data_2_temp = int.from_bytes(data_2, byteorder = 'big', signed = True)
                #print(data_0_temp, " ", data_1_temp, " ", data_2_temp)  
                send = (data_0_temp & int("000000000001", 16))
                if (send != send_ex): 
                    if (data_0_temp == 2 or data_0_temp == 3):
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                        pyautogui.moveRel(data_1_temp*move_factor, -data_2_temp*move_factor)
                    elif (data_0_temp == 4 or data_0_temp == 5):
                        pyautogui.click()
                    elif (data_0_temp == 6 or data_0_temp == 7):
                        pyautogui.scroll(5)
                    elif (data_0_temp == 8 or data_0_temp == 9):
                        pyautogui.scroll(-5)
                    elif (data_0_temp == 10 or data_0_temp == 11):
                        pyautogui.hotkey('ctrl', 'shift', '+')
                    elif (data_0_temp == 12 or data_0_temp == 13):
                        pyautogui.hotkey('ctrl', 'shift', '-')
                    print(data_0_temp, " ", data_1_temp, " ", data_2_temp) 
                    
                send_ex = send


finally: 
	dev.disconnect()













































































