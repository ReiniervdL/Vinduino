#!/usr/bin/python

import sqlite3

conn = sqlite3.connect('vinduino.db')
print ("Opened database successfully",)

conn.execute("INSERT INTO VINDUINO (TSKEY, SENSOR1,SENSOR2, SENSOR3, SENSOR4, VBAT, TEMP, HUMID, AUX) \
      VALUES (8, 105, 150, 310, 420, 0, 0, 0, 0)");


conn.commit()
print ("Records created successfully",)
conn.close()
