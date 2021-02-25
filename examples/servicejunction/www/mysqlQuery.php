<?php
include('../../../www/servicejunction/ServiceJunction.php');
$junction = new ServiceJunction();
$junction->setApplication('Test Application');
$result = null;
if ($junction->mysqlQuery('******', '******', '******', '******', 'select * from table', $result))
{
  for ($i = 0; $i < sizeof($result); $i++)
  {
    echo '--- Row ---'."\n";
    foreach ($result[$i] as $key => $value)
    {
      echo $key.':  '.$value."\n";
    }
    unset($result[$i]);
  }
}
else
{
  echo $junction->getError()."\n";
}
unset($result);
?>
