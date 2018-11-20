import serial
import time
import csv

ser = serial.Serial('COM4')
ser.flushInput()
startTime = time.time()
while True:
    try:
        ser_bytes = ser.readline()
        decoded_bytes = float(ser_bytes[0:len(ser_bytes)-2].decode("utf-8"))
        print(decoded_bytes)
        with open("test_data_16.csv","a") as f:
            writer = csv.writer(f,delimiter=",")
            writer.writerow([time.time()-startTime,decoded_bytes])
    except:
        print("Keyboard Interrupt")
        break