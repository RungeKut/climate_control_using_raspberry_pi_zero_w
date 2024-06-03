import serial
import time
import datetime
import pytz
import re
class Sensor:
    def __init__(self):
        self.Temp_control=serial.Serial(port="/dev/rfcomm7", baudrate=38400, timeout=3)
        self.Vent_control=serial.Serial(port="/dev/rfcomm8", baudrate=38400, timeout=3)
        
    def GetTemperature(self):
        k = 0
        while (k < 3):
            with open('/sys/devices/platform/dht11@4/iio:device0/in_temp_input') as file:
                try:
                    temp = file.readline()
                    temp = float(temp)/1000
#                    return '{0}'.format(temp)
                    return temp
                except Exception as err:
                    k = k + 1
                    currentTime = datetime.datetime.now(pytz.timezone('Europe/Moscow')).strftime("%Y-%m-%d %H:%M:%S")
                    print(currentTime + " GetTemperature_Error:",err)
                    #print("GetTemperature_Error:",err)
        
    def GetHumidity(self):
        k = 0
        while (k < 3):
            with open('/sys/devices/platform/dht11@4/iio:device0/in_humidityrelative_input') as file:
                try:
                    hum = file.readline()
                    hum = float(hum)/1000
#                    return '{0}'.format(hum)
                    return hum
                except Exception as err:
                    k = k + 1
                    currentTime = datetime.datetime.now(pytz.timezone('Europe/Moscow')).strftime("%Y-%m-%d %H:%M:%S")
                    print(currentTime + " GetHumidity_Error:",err)
                    #print("GetHumidity_Error:",err)

    def VentControlSend(self, string):
        k = 0
        while (k < 3):
            with self.Vent_control as ser:
                try:
                    ser.write(string.encode("utf-8"))
                    ser.close()
                    time.sleep(1) #stm не успевает обрабатывать
                    return
                except Exception as err:
                    k = k + 1
                    ser.close()
                    print("VentControlSend_Error:",err)
                    
    def VentControlRead(self):
        k = 0
        while (k < 3):
            with self.Vent_control as ser:
                try:
                    x = ser.readline().decode("utf-8")
                    ser.close()
                    return x
                except Exception as err:
                    k = k + 1
                    ser.close()
                    print("VentControlRead_Error:",err)
                    
    def VentControlGetSpeed(self):
        k = 0
        while (k < 3):
            with self.Vent_control as ser:
                try:
                    string='VentSpeed?'
                    ser.write(string.encode("utf-8"))
                    time.sleep(1)
                    x = ser.readline().decode("utf-8")
                    ser.close()
                    if (len(x) > 0):
                        tempData = re.findall(r"[-+]?(?:\d*\.*\d+)", x)
                        return int(float(tempData[0]))
                except Exception as err:
                    k = k + 1
                    ser.close()
                    currentTime = datetime.datetime.now(pytz.timezone('Europe/Moscow')).strftime("%Y-%m-%d %H:%M:%S")
                    print(currentTime + " VentControlGetSpeed_Error:",err)
                    print("VentControlGetSpeed_Error:",err)
                    time.sleep(1)
        return -1
                    
    def TempControlGetClimate(self):
        k = 0
        while (k < 3):
            with self.Temp_control as ser:
                try:
                    string='Climate?'
                    ser.write(string.encode("utf-8"))
                    time.sleep(1)
                    x = ser.readline().decode("utf-8")
                    ser.close()
                    if (len(x) >= 17):
                        tempData = re.findall(r"[-+]?(?:\d*\.*\d+)", x)
                        return (float(tempData[0]), float(tempData[1])) #Кортеж
                except Exception as err:
                    k = k + 1
                    ser.close()
                    currentTime = datetime.datetime.now(pytz.timezone('Europe/Moscow')).strftime("%Y-%m-%d %H:%M:%S")
                    print(currentTime + " TempControlGetClimate_Error:",err)
                    #print("TempControlGetClimate_Error:",err)
                    time.sleep(1)
        return -1
                    
    def TempControlGetIrMessage(self):
        k = 0
        while (k < 3):
            with self.Temp_control as ser:
                try:
                    string='IrGetMessage?'
                    ser.write(string.encode("utf-8"))
                    time.sleep(1)
                    x = ser.readline().decode("utf-8")
                    ser.close()
                    return x
                except Exception as err:
                    k = k + 1
                    ser.close()
                    currentTime = datetime.datetime.now(pytz.timezone('Europe/Moscow')).strftime("%Y-%m-%d %H:%M:%S")
                    print(currentTime + " TempControlGetIrMessage_Error:",err)
                    #print("TempControlGetIrMessage_Error:",err)
                    time.sleep(1)
        