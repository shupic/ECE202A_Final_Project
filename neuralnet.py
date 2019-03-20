import os
import time
import numpy as np
import pyautogui
import serial
import scipy.stats as stats
import matplotlib.pyplot as plt
from sklearn.svm import SVR
from sklearn.neural_network import MLPRegressor

pyautogui.FAILSAFE = False

# log file index
i = 4
#sensor data
ax = []
ay = []
az = []
gx = []
gy = []
gz = []
roll = []
pitch = []
#mouse position
mx = []
my = []
#displacement in x and y direction
dx = []
dy = []
#read data file to get the sensor data
with open('data_log%s.txt' %i, encoding = "ISO-8859-1") as file:
    for line in file:
        line_data = line.split()
        #print(len(line_data))
        #print(line_data[0])
        if len(line_data) == 9:
            gx.append(float(line_data[1]))
            gy.append(float(line_data[2]))
            gz.append(float(line_data[3]))
            roll.append(float(line_data[4]))
            pitch.append(float(line_data[5]))
            ax.append(float(line_data[6]))
            ay.append(float(line_data[7]))
            az.append(float(line_data[8]))

#read mouse logger data to get mouse position.
with open('mouse_log%s.txt' %i, encoding = "ISO-8859-1") as file:
    for line in file:
        line_data = line.split()
        #print(line_data)
        mx.append(float(line_data[3][2:len(line_data[3])-1]))
        my.append(float(line_data[4][0:len(line_data[4])-1]))

        
#ser = serial.Serial('/dev/tty.usbmodem1452', 9600, bytesize=8, timeout=2)

for j in range(1,len(mx)):
    dx.append(mx[j]-mx[j-1])
    dy.append(my[j]-my[j-1])

length = len(dx)

train_length = int(length*0.7)

t = train_length-1


trainning_data_x = np.column_stack((gx[1:],\
                                  gy[1:],gz[1:],roll[1:],pitch[1:],\
                                  ax[1:],ay[1:],az[1:]))

trainning_data_y = np.column_stack((gx[1:],\
                                  gy[1:],gz[1:],roll[1:],pitch[1:],\
                                  ax[1:],ay[1:],az[1:]))




train_dx = dx[0:t-1]
train_dy = dy[0:t-1]

svr_rbf_x = SVR(kernel='rbf', C=1e3, gamma=0.1)
svr_lin_x = SVR(kernel='linear', C=1e3)
svr_poly_x = SVR(kernel='poly', C=1e3, degree=2)

svr_rbf_y = SVR(kernel='rbf', C=1e3, gamma=0.1)
svr_lin_y = SVR(kernel='linear', C=1e3)
svr_poly_y = SVR(kernel='poly', C=1e3, degree=2)

nn_x = MLPRegressor(hidden_layer_sizes = (100,100,80,80,50,50,30,30,10,10,)\
                    ,activation='tanh',max_iter=2000)
nn_y = MLPRegressor(hidden_layer_sizes = (100,100,80,80,50,50,30,30,10,10,)\
                    ,activation='tanh',max_iter=2000)


#svr_rbf_x.fit(trainning_data,dx)
s_x = svr_lin_x.fit(trainning_data_x,dx).predict(trainning_data_x)
#s_x = nn_x.fit(trainning_data_x,dx).predict(trainning_data_x)
print("done lin")
#svr_poly_x.fit(trainning_data,dx)

#svr_rbf_y.fit(trainning_data,dy)
s_y = svr_lin_y.fit(trainning_data_y,dy).predict(trainning_data_y)
#s_y = nn_y.fit(trainning_data_y,dy).predict(trainning_data_y)
#svr_poly_x.fit(trainning_data,dy)
print('length of s_x is ',len(s_x))

#calculate the predictied position of a mouse
px = [mx[1]]
py = [my[1]]
for k in range(len(s_x)):
    px.append(px[k]+s_x[k])
    py.append(py[k]+s_y[k])

print ('release reset')
#time.sleep(0.5)
###
#using the neural network do the real time command
###
#while(ser.is_open):
#    line = ser.readline()
#    d_line = str(line,encoding='utf-8',errors='strict')
#    cop = str(d_line)
#    s = cop.split(" ")
#    if len(s) == 9:
#        new_data = [[float(s[0]),float(s[1])\
#                    ,float(s[2]),float(s[3]),float(s[4]),float(s[5]),\
#                     float(s[6]),float(s[7])]]
#        new_x = nn_x.predict(new_data)
#        new_y = nn_y.predict(new_data)
#        print('dx is ',new_x)
#        print('dy is ',new_y)
#        pyautogui.moveRel(new_x+22,new_y-2)
    
# plot the predicted data vs. original data. 
lw = 2
plt.plot(mx,my,color = 'darkorange')
plt.plot(px,py,color = 'navy')
plt.title('Support Vector Regression')
plt.show()
print (len(dx))
print (len(ax[1:]))

