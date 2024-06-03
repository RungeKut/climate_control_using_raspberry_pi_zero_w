import pymysql
import datetime
import pytz
class DataBase:
    def __init__(self):
        self.timezone = pytz.timezone('Europe/Moscow')
        self.db_host='192.168.88.3'
        self.db_port=3306
        self.db_user='my_db_admin'
        self.db_password='password'
        self.db_name='climatic_db'
        self.db_table_name1='sensor_data'
        self.db_table_name2='sensor_data2'
        self.db_dataColumn='data'
        self.db_temperatureColumn='temperature'
        self.db_humidityColumn='humidity'
        
    def SaveToDataBase(self, name, t, h):
        mdate=datetime.datetime.now(self.timezone).strftime("%Y-%m-%d %H:%M:%S")
        insertDataQuery = "INSERT INTO %s.%s (%s,%s,%s) VALUES ('%s','%s','%s');" % (self.db_name, name, self.db_dataColumn, self.db_temperatureColumn, self.db_humidityColumn, mdate, t, h)
        connection = pymysql.connect(host=self.db_host,port=self.db_port,user=self.db_user,password=self.db_password)
        try:
            
            cursor = connection.cursor()
            cursor.execute(insertDataQuery)
            connection.commit()
            return 0
        except Exception as e:
            connection.rollback()
            currentTime = datetime.datetime.now(pytz.timezone('Europe/Moscow')).strftime("%Y-%m-%d %H:%M:%S")
            print(currentTime + " SaveToDataBase_Error:",e)
            #print("SaveToDataBase_Error:",e)
            return -1
