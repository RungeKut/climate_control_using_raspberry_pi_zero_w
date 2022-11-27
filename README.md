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

Степень сжатия я оставил LZ4, но наврядли это поможет оптимально сжимать БД. Наверное в будущем стоит поставить значение none, чтобы немного увеличить производительность. Этот размер записи соответствует размеру [стандартной страницы](https://mariadb.com/kb/en/innodb-system-variables/#innodb_page_size), используемой движком InnoDB базы данных MariaDB.

Размер блока я поменял на 16K т.к. это хорошо отразиться на производительности БД MySQL, установленной на этот пул.

[Вот неплохой гайд по настройке рабочей нагрузки](https://openzfs.github.io/openzfs-docs/Performance%20and%20Tuning/Workload%20Tuning.html#mysql).

Создаем пул, заходим в jail и выбираем его для хранения тюрьм. Создаем новую тюрьму:
+ Имя - я назвал MySQL
+ Выпуск - выбрал 13.1-RELEASE-p4
+ Интерфейс IPv4 - Выбрал свой единственный интерфейс re0
+ Адрес IPv4 - указал 192.168.88.251 не входящий в диапазон DHCP сервера моего роутера (100-200)
+ Ставим галочку Автоматический старт и нажимаем Создать.

Далее система скачает образ, установит его и все это время будет висеть окошко "Обождите.." А это примерно минут 15 у меня заняло.

Подключимся к нашему TrueNAS по ssh и введем команду ***jls***, чтобы просмотреть доступные тюрьмы:

    root@freenas:~ # jls
    JID  IP Address      Hostname                    Path
    1  192.168.88.251  MySQL                         /mnt/Share/iocage/jails/MySQL/root

Видим, что тюрьма установилась и запущена. После этого стоит пробежаться по всем созданным вложенным датасетам и проверить настройки для производительности.

Переключимся на консоль нашей тюрьмы указав ее номер ***jexec <номер(JID)> tcsh***:

    root@freenas:~ # jexec 1 tcsh
    root@MySQL:/ #

Обратимся к [руководству](https://www.ricalo.com/blog/mariadb-server-freenas/#) по установке MariaDB на TrueNAS.

Выполним ***pkg update***, потому что вероятно этот менеджер пакетов не установлен:

    pkg update

Посмотрим доступные пакеты MariaDB :

    root@MySQL:~ # pkg search mariadb
    mariadb-connector-c-3.3.2      MariaDB database connector for C
    mariadb-connector-odbc-3.1.15  MariaDB database connector for odbc
    mariadb103-client-10.3.36      Multithreaded SQL database (client)
    mariadb103-server-10.3.36      Multithreaded SQL database (server)
    mariadb104-client-10.4.26      Multithreaded SQL database (client)
    mariadb104-server-10.4.26      Multithreaded SQL database (server)
    mariadb105-client-10.5.17      Multithreaded SQL database (client)
    mariadb105-server-10.5.17      Multithreaded SQL database (server)
    mariadb106-client-10.6.10      Multithreaded SQL database (client)
    mariadb106-server-10.6.10      Multithreaded SQL database (server)

Выполним установку MariaDB :

    pkg install --yes mariadb106-server-10.6.10

Создадим папки в джейле, куда будем монтировать наборы данных. Назначим mysql пользователя как владельца:

    mkdir -p /var/db/mysql/innodb_data
    mkdir -p /var/db/mysql/innodb_log
    mkdir -p /var/db/mysql/datadir

    chown -R mysql:mysql /var/db/mysql/innodb_data
    chown -R mysql:mysql /var/db/mysql/innodb_log
    chown -R mysql:mysql /var/db/mysql/datadir

Закроем сеанс в тюрьме, чтобы мы могли смонтировать наборы данных с нашего FreeNAS:

    exit

Остановим тюрьму:

    iocage stop MySQL

Создадим наборы данных innodb_data, innodb_log и datadir в Share/iocage для монтирования:

![Image alt](https://github.com/RungeKut/climate_control_using_raspberry_pi_zero_w/blob/main/supplementary_files/5.jpg "general view")​

Добавим точки монтирования для innodb_data, innodb_log и datadir:

![Image alt](https://github.com/RungeKut/climate_control_using_raspberry_pi_zero_w/blob/main/supplementary_files/6.jpg "general view")​

Запустим тюрьму:

    iocage start MySQL

Снова зайдем в ее консоль используя еще одну команду:

    iocage console MySQL

Нам понадобиться nano, поэтому установим его:

    pkg nano

Создадим или изменим файл /usr/local/etc/mysql/my.cnf

    nano /usr/local/etc/mysql/my.cnf

И добавим в него:

    [mysqld]
    # Uncomment the following line to enable access from remote hosts.
    # bind-address    = 0.0.0.0
    innodb_data_home_dir      = /var/db/mysql/innodb_data
    innodb_log_group_home_dir = /var/db/mysql/innodb_log
    datadir                   = /var/db/mysql/datadir
    skip-innodb_doublewrite

Настроим автозапуск службы и запустим службу:

    sysrc mysql_enable=yes
    service mysql-server start

Выполним первоначальную настройку выполнив:

    mysql_secure_installation

Далее следуем инструкциям:

    NOTE: RUNNING ALL PARTS OF THIS SCRIPT IS RECOMMENDED FOR ALL MariaDB
    SERVERS IN PRODUCTION USE!  PLEASE READ EACH STEP CAREFULLY!

    ПРИМЕЧАНИЕ: РЕКОМЕНДУЕТСЯ ЗАПУСКАТЬ ВСЕ ЧАСТИ ЭТОГО СКРИПТА ДЛЯ ВСЕХ MariaDB
    СЕРВЕРОВ В ПРОИЗВОДСТВЕННОМ ИСПОЛЬЗОВАНИИ! ПОЖАЛУЙСТА, ВНИМАТЕЛЬНО ПРОЧИТАЙТЕ КАЖДЫЙ ШАГ!

    In order to log into MariaDB to secure it, we'll need the current
    password for the root user. If you've just installed MariaDB, and
    haven't set the root password yet, you should just press enter here.

    Чтобы войти в MariaDB и обезопасить его, нам понадобится текущий
    пароль для пользователя root. Если вы только что установили MariaDB и
    еще не установили пароль root, вам следует просто нажать enter здесь.

    Enter current password for root (enter for none):
    OK, successfully used password, moving on...

    Введите текущий пароль для root (enter если без пароля):
    ОК, успешно использованный пароль, двигаемся дальше...

    Setting the root password or using the unix_socket ensures that nobody
    can log into the MariaDB root user without the proper authorisation.

    Установка пароля root или использование unix_socket гарантирует, что никто
    не сможет войти в систему пользователя MariaDB root без надлежащей авторизации.

    You already have your root account protected, so you can safely answer 'n'.

    Ваша учетная запись root уже защищена, поэтому вы можете смело отвечать "n".

Переключаемся на аутентификацию:

    Switch to unix_socket authentication [Y/n] y
    Enabled successfully!
    Reloading privilege tables..
    ... Success!

    Change the root password? [Y/n] y
    New password:
    Re-enter new password:
    Password updated successfully!
    Reloading privilege tables..
    ... Success!

    By default, a MariaDB installation has an anonymous user, allowing anyone
    to log into MariaDB without having to have a user account created for
    them.  This is intended only for testing, and to make the installation
    go a bit smoother.  You should remove them before moving into a
    production environment.

    По умолчанию установка MariaDB имеет анонимного пользователя, что позволяет любому пользователю
    входить в MariaDB без необходимости создавать для
    него учетную запись пользователя. Это предназначено только для тестирования и для того, 
    чтобы сделать установку немного более плавной. Вы должны удалить их перед переходом в
    производственную среду.

    Remove anonymous users? [Y/n] y
    ... Success!

    Normally, root should only be allowed to connect from 'localhost'.  This
    ensures that someone cannot guess at the root password from the network.

    Обычно root должно быть разрешено подключаться только с 'localhost'. Этот
    гарантирует, что кто-то не сможет угадать пароль root из сети.

    Запретить удаленный вход в систему с правами суперпользователя?
    Disallow root login remotely? [Y/n] n
    ... Success!

    Remove test database and access to it? [Y/n] y
    Dropping test database...
    ... Success!

    Reloading the privilege tables will ensure that all changes made so far
    will take effect immediately.

    Перезагрузка таблиц привилегий гарантирует, что все внесенные до сих пор изменения
    вступят в силу немедленно.

    Removing privileges on test database...
    ... Success!

### Протестируем установку
    mysql --user=root --password

Увидим следующий вывод:

    Enter password:
    Welcome to the MariaDB monitor.  Commands end with ; or \g.
    Your MariaDB connection id is 23
    Server version: 10.6.10-MariaDB FreeBSD Ports

    Copyright (c) 2000, 2018, Oracle, MariaDB Corporation Ab and others.

    Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

В приглашении MariaDB вы можете перечислить существующие базы данных:

    root@localhost [(none)]> show databases;

Увидим что-то вроде этого:

    root@localhost [(none)]> show databases;
    +--------------------+
    | Database           |
    +--------------------+
    | datadir            |
    | information_schema |
    | innodb_data        |
    | innodb_log         |
    | mysql              |
    | performance_schema |
    +--------------------+
    6 rows in set (0.002 sec)

Вход с помощью пользователя root в последних версиях заблокирован, поэтому:

    root@localhost [(none)]> CREATE database climatic_db;
    Query OK, 1 rows affected (0.011 sec)

    root@localhost [(none)]> CREATE USER 'my_db_admin'@'localhost' IDENTIFIED BY 'password';
    Query OK, 0 rows affected (0.018 sec)

    root@localhost [(none)]> GRANT ALL PRIVILEGES ON climatic_db.* TO 'my_db_admin'@'localhost';
    Query OK, 0 rows affected (0.013 sec)

    root@localhost [(none)]> CREATE USER 'my_db_admin'@'%' IDENTIFIED BY 'password';
    Query OK, 0 rows affected (0.016 sec)

    root@localhost [(none)]> GRANT ALL PRIVILEGES ON climatic_db.* TO 'my_db_admin'@'%';
    Query OK, 0 rows affected (0.022 sec)

## Проверяем доступ к БД, подключаемся и настраиваем:

![Image alt](https://github.com/RungeKut/climate_control_using_raspberry_pi_zero_w/blob/main/supplementary_files/7.jpg "general view")​
![Image alt](https://github.com/RungeKut/climate_control_using_raspberry_pi_zero_w/blob/main/supplementary_files/8.jpg "general view")​

Добавим новую таблицу sensor_data в нашу бд:

![Image alt](https://github.com/RungeKut/climate_control_using_raspberry_pi_zero_w/blob/main/supplementary_files/9.jpg "general view")​

Импортируем старые данные, записанные в формате csv:

    Data;Temperature;Humidity
    2021-11-29 23:33:18;25,6;49,8
    2021-11-29 23:33:40;25,6;49,8
    2021-11-29 23:33:45;25,6;49,8

![Image alt](https://github.com/RungeKut/climate_control_using_raspberry_pi_zero_w/blob/main/supplementary_files/10.jpg "general view")​
![Image alt](https://github.com/RungeKut/climate_control_using_raspberry_pi_zero_w/blob/main/supplementary_files/11.jpg "general view")​