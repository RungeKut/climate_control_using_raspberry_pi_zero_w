<?php
require __DIR__ . '/db.php';
require __DIR__ . '/myfunc.php';
date_default_timezone_set("Europe/Moscow");
#debug(get_data_by('2022-12-02 22:00:00', '2022-12-02 22:01:00'));
$startData = date('Y-m-d H:i:s', strtotime('-2 days'));
$finishData = date('Y-m-d H:i:s');
//$startData = "2022-12-10 00:00:00";
//$finishData = "2022-12-15 00:00:00";
$input = get_data_by($startData, $finishData);

require 'index.html';
exit;
?>