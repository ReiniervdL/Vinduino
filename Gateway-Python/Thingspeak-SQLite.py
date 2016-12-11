#!/usr/bin/python

import sqlite3
from time import localtime, strftime
import serial
import time
import thingspeak

channel_id = ()
write_key  = ()

#ser = serial.Serial('/dev/ttyACM0', 9600, timeout=900)
ser = serial.Serial('/dev/cu.usbmodem1411', 9600, timeout=900)

while True:
    line=ser.readline()
    if len(line)==0:
        print("Time Out")
        sys.exit()
    line=line.decode("utf-8")
    print (line)
    print (len(line.split(",")))

    try:
       
        start_char = line.split(",")[0]
        write_key = line.split(",")[1]
        field_1 = line.split(",")[2]
        field_2 = line.split(",")[3]
        field_3 = line.split(",")[4]
        field_4 = line.split(",")[5]
        v_batt = (line.split(",")[6])
        temperature = line.split(",")[7]
        humidity = line.split(",")[8]
        aux = line.split(",")[9]
        stop_char = line.split(",")[10]
        
    except:
        if start_char != "$":
            print ("no start character detected")
       

    try:
        channel = thingspeak.Channel(id=channel_id,write_key=write_key)
        print (write_key, field_1, field_2, field_3, field_4, v_batt, temperature , humidity, aux)
        response = channel.update({1:field_1, 2:field_2, 3:field_3, 4:field_4, 5:v_batt})
        print (strftime("%a, %d %b %Y %H:%M:%S", localtime()))
        print (response)

    except:        print ("connection failed",)

    conn = sqlite3.connect('vinduino.db')
    print ("Opened database successfully",)

    conn.execute('INSERT INTO VINDUINO (TSKEY, STATION_ID, SENSOR1,SENSOR2, SENSOR3, SENSOR4, VBAT, TEMP, HUMID, AUX) \
               VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?,?)', ((strftime("%d %b %Y %H:%M:%S", localtime())), write_key, field_1, field_2, field_3, field_4, v_batt, temperature, humidity, aux))
    
    conn.commit()
    print ("Records created successfully",)
    conn.close()
    
