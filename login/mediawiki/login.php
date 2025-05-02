<?php
//////////////////////////////////////////////////////////////////////////
// Central Login
// -------------------
// begin                : 2020-07-15
// copyright            : Ben Kietzman
// email                : ben@kietzman.org
//////////////////////////////////////////////////////////////////////////
include_once(dirname(__FILE__).'/../../www/auth/Secure.php');
if (!isset($wgSessionName))
{
  $wgSessionName = 'PHPSESSID';
}
if (!isset($wgCentralSite))
{
  $wgCentralSite = 'MediaWiki';
}
if (!isset($wgCentralHost))
{
  $wgCentralHost = 'localhost';
}
if (!isset($wgCentralUser))
{
  $wgCentralUser = 'central';
}
if (!isset($wgCentralPassword))
{
  $wgCentralPassword = 'central';
}
if (!isset($wgCentralDatabase))
{
  $wgCentralDatabase = 'central';
}
if (!isset($wgBasePath))
{
  $wgBasePath = '/wiki/';
}
if (!isset($wgReturnPath))
{
  $wgReturnPath = null;
}
session_name($wgSessionName);
session_start();
$secure = new Secure($wgCentralUser, $wgCentralPassword, $wgCentralHost, $wgCentralDatabase, $wgReturnPath);
$secure->setApplication($wgCentralSite);
$strPassThru = $secure->getParam('pass');
if (!$secure->isValid())
{
  $strError = null;
  $secure->processLogin($_POST, $strError);
  if ($secure->isValid())
  {
    echo '<html><head><script type="text/javascript">document.location.href="'.(($strPassThru != '')?$strPassThru:$strBasePath).'";</script></head></html>';
    exit();
  }
}
if ($secure->isValid())
{
  if ($strPassThru == '')
  {
    $secure->logout('http'.((isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] == 'on')?'s':'').'://'.$_SERVER['SERVER_NAME'].$strBasePath);
    echo "<h1 class='center'>System Notification</h1>";
    echo "<p style='text-align:center;'>";
    echo "You have been logged out of the website.<br><br>";
    echo "<a href='".$strBasePath."login.php'>Login Again</a>";
    echo "</p>";
  }
  else
  {
    echo '<html><head><script type="text/javascript">document.location.href="'.(($strPassThru != '')?$strPassThru:'/').'";</script></head></html>';
  }
}
else
{
  echo "<div align='center'>";
  echo "<center>";
  echo "<br><br>";
  $secure->login();
  echo "</center>";
  echo "</div>";
}
?>
