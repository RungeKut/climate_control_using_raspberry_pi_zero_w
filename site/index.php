<?php
$db_user="root";
$db_password="root";
$db_host="127.0.0.1";
$db_port="3306";
$db_name="climatic_db";
$db_table_name="sensor_data";
$db_dataColumn="data";
$db_temperatureColumn="temperature";
$db_humidityColumn="humidity";

$mysqli = mysqli_connect($db_host, $db_user, $db_password, $db_name, $db_port);
if (!$mysqli) {
    die("Не могу подключиться к Базе данных! " . mysqli_connect_error());
} else {
    try {
       // $query="SELECT * FROM $db_table_name WHERE $db_dataColumn BETWEEN SYSDATE-1 AND SYSDATE;";
        $query="SELECT * FROM $db_table_name WHERE $db_dataColumn BETWEEN '2022-11-28 18:00:00' AND '2022-11-28 18:01:00';";
        $result = $mysqli->query($query);
        //echo "Количество записей: ". $result->num_rows ."<br>";
        //$u=0;
        //while(($row = $result->fetch_assoc()) != FALSE){
        //    $u++;
        //    echo $u." запись: ". $row['data']." _ ". $row['temperature']." °C _ ".  $row['humidity']." %<br>";
        //}
        $inputArray = array(array($result->num_rows), array(3));
        $u=0;
        while(($row = $result->fetch_assoc()) != FALSE){
            $inputArray[$u][0] = $row['data'];
            $inputArray[$u][1] = $row['temperature'];
            $inputArray[$u][2] = $row['humidity'];
            $u++;
        }
        //print_r($inputArray);
        //echo json_encode($inputArray)."<br>";
    } catch (Exception $e) {
        $mesg = $e->getMessage();
        echo '<div class="alert alert-warning" role="alert">
        <?php echo $mesg; ?>
        </div>';
    }
}

require 'index.html';
exit;
?>