# Климат в комнате
Это самое важное особенно если есть маленький ребенок.

Мне стало иинтересно и необходимо знать температуру и влажность в детской комнате, т.к. у ребенка появились шелушки на коже, сильно пересыхал носик под утро и появлялся кашель.

Задача была сделать не просто монитор, полноченный логер. Чтобы можно было сравнить температуру например год назад. Так же необходимо было знать мгновенные показания не заходя в комнату.

Для решения задачи были выбраны уже имеющиеся и лежащие без дела железки:
1. Raspberry Pi Zero W
![Image alt](https://github.com/RungeKut/climate_control_using_raspberry_pi_zero_w/blob/main/supplementary_files/3.jpg "general view")​
2. Флешка на 4Gb
3. Датчик температуры и влажности
![Image alt](https://github.com/RungeKut/climate_control_using_raspberry_pi_zero_w/blob/main/supplementary_files/1.jpg "general view")​
![Image alt](https://github.com/RungeKut/climate_control_using_raspberry_pi_zero_w/blob/main/supplementary_files/2.jpg "general view")​
4. В дополнение TrueNAS сервер.

## Установка MySQL базы данных в jail на сервер TrueNAS
Для этого нас понадобиться Пул на котором будет храниться наша тюрьма.

Т.к. все пулы у меня созданы на засыпающих дисках, то я сделаю отдельный пул на отдельно дополнительном диске, чтобы остальным эта тюрьма не мешала засыпать.

Подключаем диск, включаем NAS и проверяем настройки подключенного диска:
![Image alt](https://github.com/RungeKut/climate_control_using_raspberry_pi_zero_w/blob/main/supplementary_files/4.jpg "general view")​