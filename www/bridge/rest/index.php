<?php
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2020-01-03
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
ini_set('max_execution_time', '300');
include(dirname(__FILE__).'/../Bridge.php');
$bridge = new common\Bridge;
$request = json_decode(file_get_contents('php://input'), true);
$response = [];
header('Access-Control-Allow-Origin: *');
if (!$bridge->request($request, $response))
{
  $response['Status'] = 'error';
  $response['Error'] = $bridge->getError();
}
echo json_encode($response);
unset($request);
unset($response);
?>
