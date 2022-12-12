<?php

#$db_name = "climatic_db";
#$db_table_name = "sensor_data";
#$db_dataColumn="data";
#$db_temperatureColumn="temperature";
#$db_humidityColumn="humidity";

function debug($data)
{
    echo '<pre>' . print_r($data, 1) . '</pre>';
}

function get_data_by($data1, $data2)
{
    global $pdo;
    $res = $pdo->query("SELECT * FROM sensor_data WHERE data BETWEEN '$data1' AND '$data2';");

    while ($row = $res->fetch())
    {
        //$data[$row['data']] = array('temperature' => $row['temperature'], 'humidity' => $row['humidity']);
        //$data[$row['data']] = $row['temperature'];
        $data[] = $row['data'];
        $temperature[] = $t = $row['temperature'];
        $humidity[] = $h = $row['humidity'];
    }
    return array($data, $temperature, $humidity, $t, $h);
}
