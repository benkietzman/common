<?php
// vim600: fdm=marker
//////////////////////////////////////////////////////////////////////////
// Central Login
// -------------------
// begin                : 2020-07-15
// copyright            : kietzman.org
// email                : ben@kietzman.org
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
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
  $secure->processLogin(null, $strError);
  if ($secure->isValid())
  {
    echo '<html><head><script type="text/javascript">document.location.href="'.(($strPassThru != '')?$strPassThru:'/wiki/').'";</script></head></html>';
    exit();
  }
}
if ($secure->isValid())
{
  if ($strPassThru == '')
  {
    $secure->logout('http'.((isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] == 'on')?'s':'').'://'.$_SERVER['SERVER_NAME'].'/wiki/');
    echo "<h1 class='center'>System Notification</h1>";
    echo "<p style='text-align:center;'>";
    echo "You have been logged out of the website.<br><br>";
    echo "<a href='/wiki/login.php'>Login Again</a>";
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
