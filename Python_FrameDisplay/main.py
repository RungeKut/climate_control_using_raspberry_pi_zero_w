import serial
import time

Temp_control=serial.Serial(port="/dev/rfcomm7", baudrate=38400, timeout=10)
Vent_control=serial.Serial(port="/dev/rfcomm8", baudrate=38400, timeout=10)
temperature = ""
humidity = ""
i = 0
while True:
    string='X{0}'.format(i)
    Temp_control.write(string.encode("utf-8"))
#    string='VentSpeed=60'
#    Vent_control.write(string.encode("utf-8"))
    i=i+1
    time.sleep(10)
    try:
        x = Temp_control.readline()
        y = x.decode("utf-8")
        print(y)
    except Exception as err:
        print("SerialError:",err)
    
    string='X{0}'.format(i)
    Temp_control.write(string.encode("utf-8"))
#    string='VentSpeed=50'
#    Vent_control.write(string.encode("utf-8"))
    i=i+1
    time.sleep(10)
    
    with open('/sys/devices/platform/dht11@0/iio:device0/in_temp_input') as file:
        try:
            temperature = file.readline()
            temperature = float(temperature)/1000
        except OSError as err:
            print("****",err)
                
    
    
    with open('/sys/devices/platform/dht11@0/iio:device0/in_humidityrelative_input') as file:
        try:
            humidity = file.readline()
            humidity = float(humidity)/1000
        except OSError as err:
            print("****",err)
    
    
    print('Temp={0}°C Humidity={1}%'.format(temperature, humidity))
    