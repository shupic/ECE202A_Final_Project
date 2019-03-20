import os
import time
import numpy as np
import pyautogui
import serial
import scipy.stats as stats
import matplotlib.pyplot as plt
import pandas as pd
from sklearn import svm
from sklearn.neural_network import MLPRegressor

pyautogui.FAILSAFE = False
pd.set_option('display.expand_frame_repr',False)
T_larger = 150
T_smaller = 100

# log file index
i = 0
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

#result
#1
scoll_down = []
#2
scoll_up = []
#3
zoom_out = []
#4
zoom_in = []
# multi class result
result = []
# read data from text file. 
with open('scoll_log%s.txt' %i, encoding = "ISO-8859-1") as file:
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

# debug parameters. 
count = 0
count_1 = 0
count_2 = 0
count_3 = 0
count_4 = 0
count_5 = 0
count_6 = 0
count_7 = 0

# label data. 
for i in range(0,len(gx)):
    if gx[i] > T_larger and gx[i] > gy[i] :
        scoll_down.append(1)
    else:
        scoll_down.append(0)
for i in range(0,len(ax)):        
    if gx[i] < -T_larger and gx[i] < gy[i] :
        scoll_up.append(1)
    else:
        scoll_up.append(0)
for i in range(0,len(ax)):        
    if gy[i] > T_larger and gy[i] > gx[i] :
        zoom_in.append(1)
    else:
        zoom_in.append(0)
for i in range(0,len(ax)):        
    if gy[i] < -T_larger and gy[i] < gx[i] :
        zoom_out.append(1)
    else :
        zoom_out.append(0)

# count the number of 1's in result, for debug use.       
for i in range(0,len(gx)):
    if gx[i] > T_larger :
        #print(ax[i],ay[i])
        count = count + 1
        
    if gx[i] < -T_larger :
        count_1 = count_1 + 1
        
    if gy[i] > T_larger :
        count_2 = count_2 + 1
        
    if gy[i] < -T_larger:
        count_3 = count_3 + 1
        
    if scoll_down[i] == 1 :
        count_4 = count_4 + 1
        
    if scoll_up[i] == 1 :
        count_5 = count_5 + 1
        
    if zoom_in[i] == 1 :
        count_6 = count_6 + 1
        
    if zoom_out[i] == 1 :
        count_7 = count_7 + 1



# making the trainning data
trainning_data = np.column_stack((gx,gy,ax,ay,az))

#print(trainning_data)
print(count)
print(count_1)
print(count_2)
print(count_3)
print(count_4)
print(count_5)
print('zoom in ',count_6)
print('zoom out',count_7)
print(len(trainning_data))
print(len(gx))

# train the classifier 
clf_0 = svm.LinearSVC(max_iter=100000)
clf_0.fit(trainning_data,scoll_down)

clf_1 = svm.LinearSVC(max_iter=100000)
clf_1.fit(trainning_data,scoll_up)

clf_2 = svm.LinearSVC(max_iter=100000)
clf_2.fit(trainning_data,zoom_in)

clf_3 = svm.LinearSVC(max_iter=100000)
clf_3.fit(trainning_data,zoom_out)

clf_4 = svm.NuSVC(nu = 0.1)
clf_4.fit(trainning_data,scoll_down)

#get the decision parameters. 

print(clf_0.score(trainning_data,scoll_down))
print("scoll_down ",clf_0.coef_)
print("scoll_down ",clf_0.intercept_)

print(clf_1.score(trainning_data,scoll_up))
print("scoll_up ",clf_1.coef_)
print("scoll_up ",clf_1.intercept_)

print(clf_2.score(trainning_data,zoom_in))
print("zoom_in ",clf_2.coef_)
print("zoom_in ",clf_2.intercept_)

print(clf_3.score(trainning_data,zoom_out))
print("zoom_out ",clf_3.coef_)
print("zoom_out ",clf_3.intercept_)

        
        

