# ECE202A Final Project
Control the mouse cursor movement with smartwatch.
## Introduction 
In this project we are going to use the on board accelerometer and gyroscope to measure the movement of Hexiwear and hence make more decision about the command made to the mouse. 
## Problem Statement 
For this project, the problem contains 4 major parts. The first part is to build a regression model using the sensor data that control the movement of mouse cursor. The second part is build a classifier that classify a movement to a certain predefined movement sets that maps to a certain command to mouse. The third part is to send the command over the BLE link in a timely manner. The fourth part is to operative the system in real time. 
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
### Configure sensor and draw samples.
First, we need to configure the sensors to operate at the maximum rate in order to get best estimate of the current position and orientation of the Hexiwear. The maximum output data rate is set to be 800Hz for both accelerometer and gyroscope. We set the sensor output data rate at 800Hz for both of the sensor using the library provided by Hexiwear site.  
<img src="https://github.com/shupic/ECE202A_Final_Project/blob/master/image/image_1.png" width="500" /><br>   
*example register map[1]*  

<img src="https://github.com/shupic/ECE202A_Final_Project/blob/master/image/image_2.png" width="500" /><br> 
*out put data rate[1]*   

The next mission is process the data, the data we measure is linear acceleration and angular velocity, but the actual data we want is position and orientation in world reference frame. In order to doing this, we implement a complementary filter to get the data we desired. 
The accelerometer measures the acceleration due to gravity and other forces. if we want to use the accelerometer to get the accurate linear acceleration, we have to eliminate the influence due to gravity. And that means we need to get the accurate measurement of object’s orientation. To do so, we have to filter out the short-term force applied. 
On the other hand, gyroscope can get the accurate instant velocity data but the measurement is subject to some constant drift, so we have to apply a high pass filter to filter out the drift. 
<img src="https://github.com/shupic/ECE202A_Final_Project/blob/master/image/image_3.png" width="500" /><br>     
*complementary filter block diagram[2]*    

The complementary filter is add a low pass filter to accelerometer data and a high pass filter to gyroscope data and combine those two to get a better measurement of  the object orientation.   

After this we tried to collect data for out motion sets via a serial link in maximum speed. First option we tried is to using the Hexiwear control the mouse cursor as a conventional mouse (using the position in a plane to map the cursor position). This method ends up failed and then other method is implemented (detail in next chapter). 
The sensor data is more capable to measure the orientation than measure the linear velocity of the object. The situation is much worse if we use BLE as communication method (due to the delay of the system). We finally choose the using the orientation as the variable to build a regression model to control the mouse cursor. The model is simply a linear model as the displacement of the move cursor is proportional to the rotation angle of the Hexiwear around x and y axis.    
### Decision Tree and classifier
Then we build the classifier to make more complicated movement set, the decision making process is following the below decision tree.<br>
<img src="https://github.com/shupic/ECE202A_Final_Project/blob/master/image/image_8.png" width="500" /><br>     
*Decision tree*    
Spicifically, the to make decision about quick turning to a direction is done by a linear binary SVM classifier that taking the angular velocity and current oritation as input. by doing this combine with the two part motion decision. we have a reletaively good seperation between the movement set and move the cursor.
below is complete command set.
<img src="https://github.com/shupic/ECE202A_Final_Project/blob/master/image/image_9.png" width="500" /><br>     
*Command table*    

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
### Regression trail
We first tried to implement a regression model using the linear velocity that enable the Hexiwear works exactly same as the conventional mouse. The challenge for this mission is to collect sensor data that match the movement of actual mouse. During our experiment, we have to bind the docking station to my waist while operating a normal mouse.  Then we using the normal mouse to draw a certain picture and log the mouse location and sensor data. For sync mouse log and data log, we log mouse location when we receive new sensor data from serial link. 
Using that data set to build a regression model. Here are some experimental result.  
#### Linear SVM    
<img src="https://github.com/shupic/ECE202A_Final_Project/blob/master/image/image_5.png" width="500" /><br> 
*result for linear SVM model*   
#### Neuron network 
<img src="https://github.com/shupic/ECE202A_Final_Project/blob/master/image/image_6.png" width="500" /><br>  
*result for neural network model*  
<p>The model trained by linear SVM is not useable to get the result we want. The neural network implementation is much better but is cannot implemented in Mbed system. And it also maybe overfitting due to the nature of the neural network, on the hardware test, its also fail to produce desired result. At this point, we decide not using the linear displacement but using the angular displacement to build the control model of the mouse cursor.</p>  

## Analysis and Results
## Future Directions
## References  
[Pyautogui](https://pyautogui.readthedocs.io/en/latest/index.html)   
[RTOS Mbed](https://os.mbed.com/handbook/RTOS)
