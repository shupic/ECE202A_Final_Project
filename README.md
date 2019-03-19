# ECE202A Final Project
Control the mouse cursor movement with smartwatch.
## Introduction 
in this project we are going to implement a system that using a Hexiwear smartwatch to move mouse cursor and do more comlicated instructions.  
## Problem Statement 
## Prior Works
### Connect Hexiwear and Raspberry pi through BLE
[Tutorial: BLE Pairing the Raspberry Pi 3 Model B with Hexiwear](https://mcuoneclipse.com/2016/12/19/tutorial-ble-pairing-the-raspberry-pi-3-model-b-with-hexiwear/)     
[Raspberry Pi Zero W and Hexiwear Bluetooth Experiment](https://github.com/Klamath233/ecexxx/blob/master/docs/btle.md)  
### Hexiwear BLE
[Hexiwear_BLE_Example](https://os.mbed.com/teams/Hexiwear/code/Hexi_BLE_Example/)
### Hexiwear Sensor Data Reading 
[Hexi_Accelero_Magneto_Example](https://os.mbed.com/teams/Hexiwear/code/Hexi_Accelero_Magneto_Example/)  
[Hexi_Gyro_Example](https://os.mbed.com/teams/Hexiwear/code/Hexi_Gyro_Example/)
## Technical Approaches
### BLE communication in Raspberry pi side
Pair with Hexiwear by command line (MAC address can be known through scan or BLE APP in phone)      
`bluetoothctl`      
`pair MAC_ADDRESS`    
Exit from bluetoothctl after pairing 

Use bluepy package and the main functions are listed below    
Connect Raspberry pi with Hexiwear through python script (read_data.py)
`dev = btle.Peripheral("MAC_ADDRESS")`  
Get the services from Hexiwear through its UUID 
`s = dev.getServiceByUUID("UUID")`  
Read the data from that service 
`c = s.getCharacteristics()[0]` 
`data = c.read()` 
### BLE communication in Hexiwear side
Toggle Hexiwear BLE (its default is in advertisement mode)
`kw40z_device.ToggleAdvertisementMode();`   
Send instruction, move_x and move_y to Raspberry pi
`kw40z_device.SendAccel(instruction, move_x, move_y);`
## Experimental Methods
## Analysis and Results
## Future Directions
## References  
[Pyautogui](https://pyautogui.readthedocs.io/en/latest/index.html)   
[RTOS Mbed](https://os.mbed.com/handbook/RTOS)
