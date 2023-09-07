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
    $divider = 30;
    $data = null;
    $temperature = null;
    $humidity = null;
    $t = null;
    $h = null;
    $datafr = null;
    $temperaturefr = null;
    $humidityfr = null;
    $tfr = null;
    $hfr = null;
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
    $da = filter($data, $temperature, $humidity, $divider);
    $res = $pdo->query("SELECT * FROM sensor_data2 WHERE data BETWEEN '$data1' AND '$data2';");

    while ($row = $res->fetch())
    {
        //$data[$row['data']] = array('temperature' => $row['temperature'], 'humidity' => $row['humidity']);
        //$data[$row['data']] = $row['temperature'];
        $datafr[] = $row['data'];
        $temperaturefr[] = $tfr = $row['temperature'];
        $humidityfr[] = $hfr = $row['humidity'];
    }
    $dafr = filter($datafr, $temperaturefr, $humidityfr, $divider);
    return array($da[0], $da[1], $da[2], $t, $h, $dafr[0], $dafr[1], $dafr[2], $tfr, $hfr);
}

function separator($data, $temperature, $humidity, $div)
{
    $step = count($data);
    $summ_t = 0;
    $summ_h = 0;
    for ( $i = 0 ; $i <= $step / $div - 1 ; $i++ )
    {
        for ( $j = 1 ; $j <= $div ; $j++ )
        {
            $summ_t += $temperature[$j + $i * $div];
            $summ_h += $humidity[$j + $i * $div];
        }
        $da[$i] = $data[$i * $div];
        $te[$i] = $summ_t / $div;
        $hu[$i] = $summ_h / $div;
        $summ_t = 0;
        $summ_h = 0;
    }
    return array($da, $te, $hu);
}

function sred($data, $temperature, $humidity, $k) //Скользящее среднее
{
    $step = count($data) - $k - 1;
    if ($step <= $k) return null;
    $summ_t = 0;
    $summ_h = 0;
    for ( $i = 0 ; $i <= $k - 1 ; $i++ )
    {
        $summ_t += $temperature[$i];
        $summ_h += $humidity[$i];
    }
    $da[0] = $data[$k / 2 - 1];
    $te[0] = $summ_t / $k;
    $hu[0] = $summ_h / $k;
    for ( $i = 1 ; $i <= $step ; $i++ )
    {
        $summ_t -= $temperature[$i - 1];
        $summ_t += $temperature[$i + $k];
        $summ_h -= $humidity[$i - 1];
        $summ_h += $humidity[$i + $k];
        $da[$i] = $data[$i + $k / 2 - 1];
        $te[$i] = $summ_t / $k;
        $hu[$i] = $summ_h / $k;
    }
    return array($da, $te, $hu);
}

function filter($data, $temperature, $humidity, $separate) //Y(n)=A*Y(n-1)+B*X(n)
{
    $A = 0.99;
    $B = 0.01;
    $step = count($data) - 1;
    //echo $step;
    //echo "_";
    if ($step <= 0) return null;
    $da[0] = $data[0];
    $te[0] = $temperature[0];
    $hu[0] = $humidity[0];
    for ( $i = 1 ; $i <= $step ; $i++ )
    {
        if (($temperature[$i] > 40) || ($temperature[$i] < 0) || ($humidity[$i] < 0) || ($humidity[$i] > 100))
        {
            $da[$i] = $data[$i];
            $te[$i] = $A * $te[$i - 1] + $B * $temperature[$i - 1];
            $hu[$i] = $A * $hu[$i - 1] + $B * $humidity[$i - 1];
        }
        else
        {
            $da[$i] = $data[$i];
            $te[$i] = $A * $te[$i - 1] + $B * $temperature[$i];
            $hu[$i] = $A * $hu[$i - 1] + $B * $humidity[$i];
        }
        $te[$i - 1] = round( $te[$i - 1], 2, PHP_ROUND_HALF_EVEN);
        $hu[$i - 1] = round( $hu[$i - 1], 2, PHP_ROUND_HALF_EVEN);
    }
    $te[$step] = round( $te[$step], 2, PHP_ROUND_HALF_EVEN);
    $hu[$step] = round( $hu[$step], 2, PHP_ROUND_HALF_EVEN);
    
    $count = intdiv( $step, $separate);
    //echo $count;
    for ( $i = 0 ; $i <= $count ; $i++ )
    {
        $dat[$i] = $da[$i * $separate];
        $tem[$i] = $te[$i * $separate];
        $hum[$i] = $hu[$i * $separate];
    }
    return array($dat, $tem, $hum);
}
