import serial
import time
from time import sleep
import csv
import matplotlib
matplotlib.use("tkAgg")
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np

ser = serial.Serial('COM4')
ser.flushInput()

fig, ax = plt.subplots()
line, = ax.plot([],[], lw=2)
ax.grid()
xdata, ydata = [], []
ax.set_xlabel('Time')
ax.set_ylabel('Pressure')

startTime = time.time()

def getData():
    while True:
        ser_bytes = ser.readline()
        decoded_bytes = float(ser_bytes[0:len(ser_bytes)-2].decode("utf-8"))
        print(decoded_bytes)
        with open("test_data.csv","a") as f:
            writer = csv.writer(f,delimiter=",")
            writer.writerow([time.time()-startTime,decoded_bytes])
            decoded_bytes = [time.time()-startTime,decoded_bytes]
            yield decoded_bytes


def init():
    ax.set_ylim(1900, 2200)
    ax.set_xlim(0, 60)
    del xdata[:]
    del ydata[:]
    line.set_data(xdata, ydata)
    return line,

def run(data):
    t, y = data
    xdata.append(t)
    ydata.append(y)
    xmin, xmax = ax.get_xlim()
    
    if t >= xmax:
        ax.set_xlim(xmin, 2*xmax)
        ax.figure.canvas.draw()
    line.set_data(xdata, ydata)


ani = animation.FuncAnimation(fig, run, getData, blit=False, repeat=False, init_func=init)

plt.show()