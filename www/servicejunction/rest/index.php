<?php
// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2020-01-03
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
ini_set('max_execution_time', '300');
include(dirname(__FILE__).'/../ServiceJunction.php');
$junction = new ServiceJunction;
$request = [];
$response = [];
$bRaw = ((isset($_GET['raw']))?true:false);
$data = json_decode(file_get_contents('php://input'), true);
if ($data == null)
{
  $data = json_decode(file_get_contents('php://stdin'), true);
}
header('Access-Control-Allow-Origin: *');
if ($bRaw)
{
  $request = $data;
}
else if (is_array($data))
{
  $nSize = sizeof($data);
  for ($i = 0; $i < $nSize; $i++)
  {
    $request[] = json_encode($data[$i]);
  }
}
unset($data);
if (!$junction->request($request, $response))
{
  $response[] = ['Status'=>'error','Error'=>$junction->getError()];
}
if ($bRaw)
{
  $data = $response;
}
else
{
  $data = [];
  $nSize = sizeof($response);
  for ($i = 0; $i < $nSize; $i++)
  {
    $data[] = json_decode($response[$i], true);
  }
}
unset($response);
echo json_encode($data);
unset($data);
?>
