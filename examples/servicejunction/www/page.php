<?php
include('../../../www/servicejunction/ServiceJunction.php');
$junction = new ServiceJunction();
$junction->setApplication('Test Application');
if ($junction->page('bk6471', 'Test Message'))
{
  echo 'Paged the message.'."\n";
}
else
{
  echo $junction->getError()."\n";
}
?>
