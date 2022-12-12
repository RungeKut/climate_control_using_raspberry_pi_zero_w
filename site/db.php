<?php
$user = 'my_db_admin';
$password = 'password';
$db_host = '192.168.88.102';
$db_port = '3306';
$db_name = 'climatic_db';
$db_table_name = 'sensor_data';
$db_charset = 'utf8';

try
{
    $dsn = "mysql:host=$db_host;dbname=$db_name;charset=$db_charset";
    $opt = [
        PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
        PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
    ];
    $pdo = new PDO($dsn, $user, $password, $opt);
}
catch (PDOException $db_e)
{
    die("Не могу подключиться к БД. " . $db_e->getMessage());
}
