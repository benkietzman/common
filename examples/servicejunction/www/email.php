<?php
include('../../../www/servicejunction/ServiceJunction.php');
$junction = new ServiceJunction();
$junction->setApplication('Test Application');
if ($junction->email('ben@kietzman.org', 'ben@kietzman.org', 'Test Message', 'This is a plain-text message.', '<html><body><b>This is an HTML message.</b></body></html>', 'example.odt'))
{
  echo 'Sent the email.'."\n";
}
else
{
  echo $junction->getError()."\n";
}
?>
