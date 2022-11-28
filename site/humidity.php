<?php
$db_user="my_db_admin";
$db_password="password";
$db_host="192.168.88.251";
$db_port="3306";
$db_name="climatic_db";
$db_table_name="sensor_data";
$db_dataColumn="data";
$db_temperatureColumn="temperature";
$db_humidityColumn="humidity";

$mysqli = mysqli_connect($db_host, $db_user, $db_password, $db_name, $db_port);
echo "Начало";
if (!$mysqli) {
    die("Не могу подключиться к Базе данных! " . mysqli_connect_error());
} else {
    try {
        $query="SELECT * FROM $db_table_name WHERE $db_dataColumn BETWEEN SYSDATE-1 AND SYSDATE;";
        $result = $mysqli->query($query);
        $array_data = $result->fetch_all();
        echo "$array_data";
    } catch (Exception $e) {
        $mesg = $e->getMessage();
        echo '<div class="alert alert-warning" role="alert">
        <?php echo $mesg; ?>
        </div>';
    }
}

//require 'humidity.html';
exit;
?>