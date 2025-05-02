<?php
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2013-11-12
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
include(dirname(__FILE__).'/../ServiceJunction.php');
$junction = new ServiceJunction();
$bResult = false;
$strError = null;
$data = array();
$response = null;
$post = json_decode(file_get_contents('php://input'), true);
if (isset($post['Transport']) && $post['Transport'] == 'standard')
{
  $junction->useSecureJunction(false);
}
if (isset($post['Function']) && $post['Function'] != '')
{
  $subrequest = array();
  $data['Function'] = $post['Function'];
  if (isset($post['Request']) && is_array($post['Request']) && sizeof($post['Request']) > 0)
  {
    $request = $post['Request'];
    $nSize = sizeof($request);
    for ($i = 0; $i < $nSize; $i++)
    {
      $subrequest[] = json_encode($request[$i]);
    }
    unset($request);
  }
  $subresponse = null;
  if (method_exists($junction, $post['Function']))
  {
    if (($bResult = $junction->{$post['Function']}($subrequest, $subresponse)))
    {
      $bResult = true;
      $nSize = sizeof($subresponse);
      for ($i = 0; $i < $nSize; $i++)
      {
        $response[] = json_decode($subresponse[$i], true);
      }
    }
    else
    {
      $strError = $junction->getError();
    }
  }
  else
  {
    $strError = 'Please provide a valid Function.';
  }
  unset($subrequest);
  unset($subresponse);
}
else
{
  $strError = 'Please provide the Function.';
}
if ($bResult)
{
  $data['Status'] = 'okay';
  $data['Response'] = $response;
}
else
{
  $data['Status'] = 'error';
  if ($strError != '')
  {
    $data['Error'] = $strError;
  }
}
echo json_encode($data);
unset($data);
unset($response);
?>
