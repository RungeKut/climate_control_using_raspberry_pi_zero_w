<h1 align="center">Управление вентилятором</h1>
<h2>Содержание</h2>
<ol>
<li><a href="#bluetooth-модуль-hc05">Bluetooth модуль HC05</a></li>
</ol>

<h2 align="center">Bluetooth модуль HC05</h2>

Напряжение питания модуля: 3.3 В

Потребляяемый ток: 50 мА

<a href="supplementary_files/HC-05 Datasheet.pdf">Datasheet</a>

<p align="center"><img src="supplementary_files/1.jpg"></p>

Этот модуль может работать в 3-х режимах:
1. **Master** (или сервер) — в этом режиме модуль может сам подключиться к какому-нибудь Bluetooth девайсу. AT+ROLE=1
2. **Slave** — в этом режиме другой мастер может подключиться к модулю. AT+ROLE=0
3. **Slave-loop** — здесь модуль отправляет обратно все байты, которые ему прислали. AT+ROLE=2

### Конфигурация модуля:

Для того, чтобы настроить BT модуль нужно:
1. Перевести модуль в режим приема AT команд, замкнув PIO11 на +3.3 V
2. Подключиться к UART'у модуля проводами, настроить COM порт вот так:

    Baudrate = 38400
    StopBits = 1
    DataBits = 8
    Parity = none
    Handshaking = none

В терминале настроить отправку символов конца строки CR+LF

И отправить ему команду: **AT**

После каждой успешно выполненной команды модуль отвечает: **OK**

[Деталька](http://we.easyelectronics.ru/part/rabota-s-bluetooth-modulem-hc05.html)

# Полезные ссылки

+ [Таймер ШИМ HAL.](https://narodstream.ru/stm-urok-10-hal-izuchaem-pwm-shim-migaem-svetodiodami-plavno/)
+ [HAL Таймер в режиме захвата. Енкодер.](https://istarik.ru/blog/stm32/121.html)
+ [Подключаем bluetooth-модуль HC-05](https://narodstream.ru/stm-urok-97-podklyuchaem-bluetooth-modul-hc-05/)

0100 1101 1011 0010 1111 1001 0000 0110

17_low_cool = 4D B2 F9 06 00 FF = 0000 0000 1111 1111
18_low_cool = 4D B2 F9 06 08 F7 = 0000 1000 1111 0111
19_low_cool = 4D B2 F9 06 0C F3 = 0000 1100 1111 0011
20_low_cool = 4D B2 F9 06 04 FB = 0000 0100 1111 1011
21_low_cool = 4D B2 F9 06 06 F9 = 0000 0110 1111 1001
22_low_cool = 4D B2 F9 06 0E F1 = 0000 1110 1111 0001
23_low_cool = 4D B2 F9 06 0A F5 = 0000 1010 1111 0101
24_low_cool = 4D B2 F9 06 02 FD = 0000 0010 1111 1101
25_low_cool = 4D B2 F9 06 03 FC = 0000 0011 1111 1100
26_low_cool = 4D B2 F9 06 0B F4 = 0000 1011 1111 0100
27_low_cool = 4D B2 F9 06 09 F6 = 0000 1001 1111 0110
28_low_cool = 4D B2 F9 06 01 FE = 0000 0001 1111 1110
29_low_cool = 4D B2 F9 06 05 FA = 0000 0101 1111 1010
30_low_cool = 4D B2 F9 06 0D F2 = 0000 1101 1111 0010

38 кГц  35% скважность 72Мгц PSC=0x0766 CCR=0x5999

21имп преамбула=167