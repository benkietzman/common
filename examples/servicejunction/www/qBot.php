<?php
include('../../../www/servicejunction/ServiceJunction.php');
$junction = new ServiceJunction();
$junction->setApplication('Test Application');
if ($junction->qBot('#nma.system', 'Test Message'))
{
  echo 'Posted the message.'."\n";
}
else
{
  echo $junction->getError()."\n";
}
?>
