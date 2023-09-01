import time
import datetime
import pytz
import re

import DBProvider
import DataProvider
import DataVisualizer
import CondeyCtrl

ventOn = "08:30"
ventOff = "01:30" 

mdb = DBProvider.DataBase()
mdp = DataProvider.Sensor()
mdv = DataVisualizer.Visualizer()
condey = CondeyCtrl.Condey()

#Чтобы включить управление кондеем
condeyRemoteControlIsActive = True

if (condeyRemoteControlIsActive):
    condey.ForseOff()
    condey.On()
    
#mdp.VentControlSend("VentSpeed=50")
loop = 100
t1 = -400
h1 = -400
t2 = -400
h2 = -400
s1 = -400

x1=25
y1=0
x2=27
y2=50
x3=30
y3=100

a=(y3-(x3*(y2-y1)+x2*y1-x1*y2)/(x2-x1))/(x3*(x3-x1-x2)+x1*x2)
b=(y2-y1)/(x2-x1)-a*(x1+x2)
c=(x2*y1-x1*y2)/(x2-x1)+a*x1*x2
print("%.1f" % (a) + "%.1f" % (b) + "%.1f" % (c))

while (True):
    try:
        currentTime = datetime.datetime.now(pytz.timezone('Europe/Moscow'))
#Чтение собственной температуры
        tempData = mdp.GetTemperature()
        t1 = t1 if tempData is None else tempData
        tempData = mdp.GetHumidity()
        h1 = h1 if tempData is None else tempData
#Чтение bluetooth датчика в детской (STM32 + HC05)
        tempData = mdp.TempControlGetClimate()
        t2 = tempData[0] if isinstance(tempData, tuple) else t2
        h2 = tempData[1] if isinstance(tempData, tuple) else h2
        
#Сохраняем показания датчиков в базу
        if (t2 >= -60) and (t2 <= 60) and (h2 >= 0) and (h2 <= 100):
            mdb.SaveToDataBase('sensor_data', t2, h2)
            
        if (t1 >= -60) and (t1 <= 60) and (h1 >= 0) and (h1 <= 100):
            mdb.SaveToDataBase('sensor_data2', t1, h1)

        if (condeyRemoteControlIsActive):
            condey.Executor(t2)
            
        if (loop >= 10):
#Управление вентиляцией
            tempData = a * t1 * t1 + b * t1 + c
            if tempData > 100: tempData = 100
            elif tempData < 0: tempData = 0
            y = str(int(tempData))
            mdp.VentControlSend("VentSpeed=" + y)
#Чтение параметров вентиляции
            tempData = mdp.VentControlGetSpeed()
            s1 = tempData if tempData >=0 else s1 # Тернарный оператор
#Рисование
            if (condeyRemoteControlIsActive):
                mdv.Draw(t1, h1, t2, h2, s1, condey.Status())
            else:
                mdv.Draw(t1, h1, t2, h2, s1, "Управление откл.")
            loop = 0
#            print("Инфракрасный приемник:", dp.TempControlGetIrMessage())
        else:
            loop=loop+1
            print(loop)
#        time.sleep(5)
    except Exception as e:
        currentTime = datetime.datetime.now(pytz.timezone('Europe/Moscow')).strftime("%Y-%m-%d %H:%M:%S")
        print(currentTime + "While:",e)
        
