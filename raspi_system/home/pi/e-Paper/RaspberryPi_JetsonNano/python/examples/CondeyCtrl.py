import DataProvider
import time
import datetime
import pytz
import enum
import schedule

@enum.unique
class Mode(enum.Enum):
    auto = 1
    cool = 2
    dry = 3
    heat = 4
    fan = 5

@enum.unique
class FanSpeed(enum.Enum):
    low = 1
    medium = 2
    high = 3
    
class Temp(dict):
    def __init__(self):
        self._dict = {}

    def add(self, id, val):
        self._dict[id] = val

class Condey:
    def __init__(self):
        self.dp = DataProvider.Sensor()
        self.condOn = "05:20"
        self.condOff = "09:00"
        self.targetTemp = 26.5
        self.addr = "4D B2"
        self.setTemp = 0
        self.setMode = 0
        self.setFanSpeed = 0
        self.condIsOn = 0
        self.t2Hi = 0
        self.t2Lo = 0
        self.preOff = 0
        self.state = "Откл" #Строка состояния кондиционера
        self.sd = schedule
        self.sd.every().day.at(self.condOn).do(self.On)
        self.sd.every().day.at(self.condOff).do(self.Off)
        
    def On(self):
        self.dp.VentControlSend("IrSendMessage 6 4D B2 F9 06 0B F4 1") #26_low_cool
        time.sleep(10)
        self.LedDisplay()
        self.condIsOn = 1
        self.state = "Вкл default 26°C"
        
    def Off(self):
        if(self.preOff == 0):
            self.preOff = 1
            self.dp.VentControlSend("IrSendMessage 6 4D B2 F9 06 27 D8 1") #low_fan
            self.condIsOn = 0
            self.t2Hi = 0
            self.t2Lo = 0
            self.state = "Сушка"
            self.fOff = self.sd.every(15).minutes.do(self.ForseOff)
        
    def ForseOff(self):
        self.dp.VentControlSend("IrSendMessage 6 4D B2 DE 21 07 F8 1") #off
        self.preOff = 0
        self.condIsOn = 0
        self.t2Hi = 0
        self.t2Lo = 0
        self.state = "Откл"
        try:
            self.sd.cancel_job(self.fOff)
            self.fOff = None
        except Exception as e:
            print("CondeySchedule_Info:",e)
        
    def SleepOn(self):
        self.dp.VentControlSend("IrSendMessage 6 4D B2 07 F8 C0 3F 1")
        
    def SleepOff(self):
        self.dp.VentControlSend("IrSendMessage 6 4D B2 FD 02 0B F4 1")
        
    def Swing(self):
        self.dp.VentControlSend("IrSendMessage 6 4D B2 D6 29 07 F8 1")
        
    def Direct(self):
        self.dp.VentControlSend("IrSendMessage 6 4D B2 F0 0F 07 F8 1")
        
    def ShortCut(self):
        self.dp.VentControlSend("IrSendMessage 6 4D B2 F8 07 1B E4 1")
        
    def Clean(self):
        self.dp.VentControlSend("IrSendMessage 6 AD 52 AF 50 55 AA 1")
        
    def LedDisplay(self):
        self.dp.VentControlSend("IrSendMessage 6 AD 52 AF 50 A5 5A 1")
        
    def Turbo(self):
        self.dp.VentControlSend("IrSendMessage 6 AD 52 AF 50 45 BA 1")
        
    def Status(self):
        return self.state
        
    def Executor(self, temp):
        self.sd.run_pending()
        if (self.condIsOn):
            if (temp > self.targetTemp + 1) or (temp < self.targetTemp - 1):
                self.t2Hi = 0
                self.t2Lo = 0
                time.sleep(5)
            if (temp > self.targetTemp + 0.3) and (self.t2Hi == 0):
                self.dp.VentControlSend("IrSendMessage 6 4D B2 F9 06 0A F5 1") #23_low_cool
                self.t2Hi = 1
                self.t2Lo = 0
                self.state = 'Вкл cool %.1f' % (self.targetTemp) + '°C'
            elif (temp < self.targetTemp - 0.3) and (self.t2Lo == 0):
                self.dp.VentControlSend("IrSendMessage 6 4D B2 F9 06 27 D8 1") #low_fan
                self.t2Hi = 0
                self.t2Lo = 1
                self.state = "Вкл fan"
            
        