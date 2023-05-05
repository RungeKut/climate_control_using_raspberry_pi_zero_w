<h1 align="center">Управление вентилятором</h1>
<h2>Содержание</h2>
<ol>
<li><a href="#bluetooth-модуль-hc05">Bluetooth модуль HC05</a></li>
</ol>

<h2 align="center">Bluetooth модуль HC05</h2>

Напряжение питания модуля: 3.3 В

Потребляяемый ток: 50 мА

[Datasheet](supplementary_files/HC-05 Datasheet.pdf)

<p align="center"><img src="*supplementary_files/1.jpg"></p>

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

+ [Bluetooth консоль для Raspberry](https://hacks.mozilla.org/2017/02/headless-raspberry-pi-configuration-over-bluetooth/)
+ [Cервер беспроводной консоли Bluetooth на Raspberry Pi Zero W](https://www.historiantech.com/create-a-bluetooth-wireless-console-server-with-raspberry-pi-zero-w/)
