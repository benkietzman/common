<?php
// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2020-01-03
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
include(dirname(__FILE__).'/../Acorn.php');
$acorn = new common\Acorn;
$request = json_decode(file_get_contents('php://input'), true);
$response = [];
header('Access-Control-Allow-Origin: *');
if (!$acorn->request($request, $response))
{
  $response['Status'] = 'error';
  $response['Error'] = $acorn->getError();
}
echo json_encode($response);
unset($request);
unset($response);
?>
