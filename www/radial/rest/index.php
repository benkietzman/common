<?php
// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-01-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
ini_set('max_execution_time', '300');
include(dirname(__FILE__).'/../Radial.php');
$radial = new common\Radial;
$request = json_decode(file_get_contents('php://input'), true);
$response = [];
header('Access-Control-Allow-Origin: *');
if (!$radial->request($request, $response))
{
  $response['Status'] = 'error';
  $response['Error'] = $radial->getError();
}
echo json_encode($response);
unset($request);
unset($response);
?>
