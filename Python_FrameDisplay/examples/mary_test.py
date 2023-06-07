#!/usr/bin/python
# -*- coding:utf-8 -*-
import sys
import os
import serial
import time
import datetime
import pytz
import re
from getpass import getpass
import pymysql
import schedule
picdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'pic')
libdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'lib')
if os.path.exists(libdir):
    sys.path.append(libdir)

import logging
from waveshare_epd import epd7in5
import time
from PIL import Image,ImageDraw,ImageFont
import traceback

logging.basicConfig(level=logging.DEBUG)

db_host='192.168.88.102'
db_port=3306
db_user='my_db_admin'
db_password='password'
db_name='climatic_db'
db_table_name1='sensor_data'
db_table_name2='sensor_data2'
db_dataColumn='data'
db_temperatureColumn='temperature'
db_humidityColumn='humidity'
Temp_control=serial.Serial(port="/dev/rfcomm7", baudrate=38400, timeout=10)
Vent_control=serial.Serial(port="/dev/rfcomm8", baudrate=38400, timeout=10)
ventOn = "08:00"
ventOff = "01:30"
condOn = "06:00"
condOff = "08:00"

epd = epd7in5.EPD()
epd.init()
#epd.Clear()

def GetTemperature():
    while (True):
        with open('/sys/devices/platform/dht11@4/iio:device0/in_temp_input') as file:
            try:
                temp = file.readline()
                temp = float(temp)/1000
                return '{0}°C'.format(temp)
            except Exception as err:
                print("Температура:",err)
        
def GetHumidity():
    while (True):
        with open('/sys/devices/platform/dht11@4/iio:device0/in_humidityrelative_input') as file:
            try:
                hum = file.readline()
                hum = float(hum)/1000
                return '{0}%'.format(hum)
            except Exception as err:
                print("Влажность:",err)

def VentControlSend(string):
    while (True):
        with Vent_control as ser:
            try:
                ser.write(string.encode("utf-8"))
                ser.close()
                return
            except Exception as err:
                ser.close()
                print("VentControlSendError:",err)
                
def VentControlRead():
    while (True):
        with Vent_control as ser:
            try:
                x = ser.readline().decode("utf-8")
                ser.close()
                return x
            except Exception as err:
                ser.close()
                print("VentControlReadError:",err)
                
def VentControlGetSpeed():
    while (True):
        with Vent_control as ser:
            try:
                string='VentSpeed?'
                ser.write(string.encode("utf-8"))
                #time.sleep(1)
                x = ser.readline().decode("utf-8")
                ser.close()
                if (len(x) > 0):
                    return x
            except Exception as err:
                ser.close()
                print("VentControlGetSpeed:",err)
                #time.sleep(1)
                
def TempControlGetClimate():
    while (True):
        with Temp_control as ser:
            try:
                string='Climate?'
                ser.write(string.encode("utf-8"))
                #time.sleep(1)
                x = ser.readline().decode("utf-8")
                ser.close()
                if (len(x) >= 17):
                    return x
            except Exception as err:
                ser.close()
                print("TempControlGetClimateError:",err)
                #time.sleep(1)
                
def SaveToDataBase(name, t, h):
    mdate=datetime.datetime.now(pytz.timezone('Europe/Moscow')).strftime("%Y-%m-%d %H:%M:%S")
    
    insertDataQuery = "INSERT INTO %s.%s (%s,%s,%s) VALUES ('%s','%s','%s');" % (db_name, name, db_dataColumn, db_temperatureColumn, db_humidityColumn, mdate, t, h)
    
    try:
        connection = pymysql.connect(host=db_host,port=db_port,user=db_user,password=db_password)
        cursor = connection.cursor()
        cursor.execute(insertDataQuery)
        connection.commit()
        #print("MySQL:OK")
    except Exception as e:
        connection.rollback()
        print("MySQL:",e)

def Draw(t1, h1, t2, h2, s1):
    try:
        fontClimate = 35
        font1 = ImageFont.truetype(os.path.join(picdir, 'cambria.ttc'), fontClimate)
        fontTime = ImageFont.truetype(os.path.join(picdir, 'cambria.ttc'), 150)
        fontData = ImageFont.truetype(os.path.join(picdir, 'cambria.ttc'), 30)
        Himage = Image.new('1', (epd.width, epd.height), 255)
        draw = ImageDraw.Draw(Himage)
        p1 = 10
        p2 = 170
        cl = 5
        #Вывод собственной температуры
        draw.text((p1, 0), 'Рамка', font = font1, fill = 0)
        draw.text((p2, 0), (t1 + '  ' + h1), font = font1, fill = 0)
        #Вывод bluetooth датчика в детской (STM32 + HC05)
        draw.text((p1, fontClimate+cl), 'Детская', font = font1, fill = 0)
        draw.text((p2, fontClimate+cl), t2 + '°C  ' + h2 + '%', font = font1, fill = 0)
        #
        draw.text((p1, 2*(fontClimate+cl)), 'Кухня', font = font1, fill = 0)
        draw.text((p2, 2*(fontClimate+cl)), '--.-°C  --.-%', font = font1, fill = 0)
        draw.text((p1, 3*(fontClimate+cl)), 'Ванная', font = font1, fill = 0)
        draw.text((p2, 3*(fontClimate+cl)), '--.-°C  --.-%', font = font1, fill = 0)
        #Чтение параметров вентиляции
        draw.text((p1, 4*(fontClimate+cl)), 'Вентиляция  ' + s1 + '%', font = font1, fill = 0)
        #Вывод времени
        strTime = datetime.datetime.now(pytz.timezone('Europe/Moscow')).strftime("%d . %m . %Y")
        draw.text((10, 228), strTime, font = fontData, fill = 0)
        strTime = datetime.datetime.now(pytz.timezone('Europe/Moscow')).strftime("%H:%M")
        draw.text((10, 228), strTime, font = fontTime, fill = 0)
        #Из-за особенностей отображения на экране для равномерной контрасности выводим рамку по периметру экрана
        draw.line((0, 0, 639, 0), fill = 0)
        draw.line((639, 0, 639, 383), fill = 0)
        draw.line((0, 0, 0, 383), fill = 0)
        draw.line((0, 383, 639, 383), fill = 0)
        epd.display(epd.getbuffer(Himage))
        
    except IOError as e:
        logging.info(e)
    
    except KeyboardInterrupt:    
        logging.info("ctrl + c:")
        epd7in5.epdconfig.module_exit()
        exit()

loop = 100
condIsOn = 0
t2Hi = 1
t2Lo = 1
while (True):
    try:
        #Чтение собственной температуры
        t1 = GetTemperature()
        h1 = GetHumidity()
        #Чтение bluetooth датчика в детской (STM32 + HC05)
        tempData = re.findall(r"[-+]?(?:\d*\.*\d+)", TempControlGetClimate())
        t2 = tempData[0]
        h2 = tempData[1]
        SaveToDataBase(db_table_name1, t2, h2)
        #Выключение вентилятора
        time = datetime.datetime.now(pytz.timezone('Europe/Moscow')).strftime("%H:%M")
        if (time == ventOff):
            VentControlSend("VentSpeed=0")
        elif (time == ventOn):
            VentControlSend("VentSpeed=100")
        if (time == condOn) and (condIsOn == 0):
            VentControlSend("IrSendMessage 6 AD 52 AF 50 A5 5A 1") #ledDisplay
            time.sleep(5)
            VentControlSend("IrSendMessage 6 4D B2 F9 06 09 F6 1") #27_low_cool
            condIsOn = 1
            t2Hi = 0
            t2Lo = 0
        elif (time == condOff) and (condIsOn == 1):
            VentControlSend("IrSendMessage 6 4D B2 DE 21 07 F8 1") #off
            time.sleep(5)
            VentControlSend("IrSendMessage 6 4D B2 DE 21 07 F8 1") #off
            condIsOn = 0
            t2Hi = 1
            t2Lo = 1
        elif (float(t2) > 29) and (t2Hi == 0):
            VentControlSend("IrSendMessage 6 4D B2 F9 06 0B F4 1") #26_low_cool
            t2Hi = 1
            t2Lo = 0
        elif (float(t2) < 24) and (t2Lo == 0):
            VentControlSend("IrSendMessage 6 4D B2 F9 06 27 D8 1") #low_fan
            t2Hi = 0
            t2Lo = 1
        
        if (loop >= 10):
            #Чтение параметров вентиляции
            tempData = re.findall(r"[-+]?(?:\d*\.*\d+)", VentControlGetSpeed())
            s1 = tempData[0]
            Draw(t1, h1, t2, h2, s1)
            loop = 0
        else:
            loop=loop+1
            print(loop)
        #time.sleep(5)
    except Exception as e:
        print("While:",e)
        