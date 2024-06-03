from PIL import Image,ImageDraw,ImageFont
import os
import sys
import datetime
import pytz
picdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'pic')
libdir = os.path.join(os.path.dirname(os.path.dirname(os.path.realpath(__file__))), 'lib')
if os.path.exists(libdir):
    sys.path.append(libdir)
from waveshare_epd import epd7in5
epd = epd7in5.EPD()

class Visualizer:
    def __init__(self):
        epd.init()
#        epd.Clear()
        
    def Draw(self, t1, h1, t2, h2, s1, k1):
        try:
            epd.Clear()
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
            draw.text((p2, 0), '%s' % (t1) + '°C  ' + '%s' % (h1) + '%', font = font1, fill = 0)
            #Вывод bluetooth датчика в детской (STM32 + HC05)
            draw.text((p1, fontClimate+cl), 'Детская', font = font1, fill = 0)
            draw.text((p2, fontClimate+cl), '%s' % (t2) + '°C  ' + '%s' % (h2) + '%', font = font1, fill = 0)
            #
            #draw.text((p1, 2*(fontClimate+cl)), 'Кухня', font = font1, fill = 0)
            #draw.text((p2, 2*(fontClimate+cl)), '--.-°C  --.-%', font = font1, fill = 0)
            draw.text((p1, 3*(fontClimate+cl)), 'Кондиционер ' + k1, font = font1, fill = 0)
            #Чтение параметров вентиляции
            draw.text((p1, 4*(fontClimate+cl)), 'Вентиляция %d' % (s1) + '%', font = font1, fill = 0)
            #Вывод времени
            currentTime = datetime.datetime.now(pytz.timezone('Europe/Moscow'))
            strTime = currentTime.strftime("%d . %m . %Y")
            draw.text((10, 228), strTime, font = fontData, fill = 0)
            strTime = currentTime.strftime("%H:%M")
            draw.text((10, 228), strTime, font = fontTime, fill = 0)
            #Из-за особенностей отображения на экране для равномерной контрасности выводим рамку по периметру экрана
            #draw.line((0, 0, 639, 0), fill = 0)
            #draw.line((639, 0, 639, 383), fill = 0)
            #draw.line((0, 0, 0, 383), fill = 0)
            #draw.line((0, 383, 639, 383), fill = 0)
            epd.display(epd.getbuffer(Himage))
            
        except Exception as e:
            print("Visualiser_Error:",e)
            
