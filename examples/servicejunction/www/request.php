<?php
include('../../../www/servicejunction/ServiceJunction.php');
$junction = new ServiceJunction();
$junction->setApplication('Test Application');
$request = array();
$request['Service'] = 'addrInfo';
$request['Server'] = 'kietzman.org';
$response = null;
if ($junction->request(array(json_encode($request)), $response))
{
  $data = json_decode($response[0], true);
  if ($data['Status'] == 'okay')
  {
    echo 'IPv4:  '.$data['IPv4']."\n";
    echo 'IPv6:  '.$data['IPv6']."\n";
  }
  else
  {
    echo $data['Error']."\n";
  }
  unset($data);
}
else
{
  echo $junction->getError()."\n";
}
unset($response);
unset($request);
?>
