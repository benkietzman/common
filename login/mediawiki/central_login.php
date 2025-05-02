<?php
//////////////////////////////////////////////////////////////////////////
// Central Login
// -------------------
// begin                : 2009-01-13
// copyright            : Ben Kietzman
// email                : ben@kietzman.org
//////////////////////////////////////////////////////////////////////////
if (!defined('MEDIAWIKI'))
{
  die();
}

///////////////////////////////////////
// Custom Settings
if (!isset($wgCentralSite))
{
  $wgCentralSite = 'MediaWiki';
}
if (!isset($wgCentralLoginURL))
{
  $wgCentralLoginURL = 'https://'.$_SERVER['SERVER_NAME'].'/login.php?pass='.urlencode('https://'.$_SERVER['SERVER_NAME'].'/central/wiki/'.((isset($_GET['returnto']) && $_GET['returnto'] != '')?'index.php/'.$_GET['returnto']:''));
}
if (!isset($wgCentralLogoutURL))
{
  $wgCentralLogoutURL = 'https://'.$_SERVER['SERVER_NAME'].'/login.php';
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
///////////////////////////////////////

$wgExtensionCredits['other'][] = array('name'=>'Central Login Authentication', 'description' => '[Hopefully] authenticates using central database', 'author' => 'Ben Kietzman');
$wgHooks['UserLoadAfterLoadFromSession'][] = 'AutoAuthenticateCentral';

function AutoAuthenticateCentral($user)
{
  global $wgContLang;
  //wfSetupSession();
  session_start();
  if (isset($_SESSION['sl_login']) && $_SESSION['sl_login'] != '')
  {
    $name = ucfirst($_SESSION['sl_login']);
    $t = Title::newFromText($name);
    if (is_null($t))
    {
      return true;
    }
    $canonicalName = $t->getText();
    global $wgAuth;
    $canonicalName = $wgAuth->getCanonicalName($t->getText());
    if (!User::isValidUserName($canonicalName))
    {
      return true;
    }
    $user->setName($name);
    $user_id = $user->idFromName($name);
    $user->setId($user_id);
    if ($user->getID() == 0)
    {
      global $wgCentralSite, $wgCentralHost, $wgCentralUser, $wgCentralPassword, $wgCentralDatabase;
      $canonicalName = $t->getText();
      $user->setName($canonicalName);
      $user->addToDatabase();
      $db = mysqli_connect($wgCentralHost, $wgCentralUser, $wgCentralPassword, $wgCentralDatabase);
      $getPerson = mysqli_query($db, 'select id, first_name, last_name, email from person where userid = \''.$_SESSION['sl_login'].'\'');
      if (($getPersonRow = mysqli_fetch_array($getPerson, MYSQLI_ASSOC)))
      {
        $user->setEmail($getPersonRow['email']);
        $user->setRealName($getPersonRow['first_name'].' '.$getPersonRow['last_name']);
        $getApplication = mysqli_query('select id from application where name = \''.$wgCentralSite.'\'', $db);
        if (($getApplicationRow = mysqli_fetch_array($getApplication, MYSQLI_ASSOC)))
        {
          $getContactType = mysqli_query('select id from central.contact_type where type = \'Contact\'', $db);
          if (($getContactTypeRow = mysqli_fetch_array($getContactType, MYSQLI_ASSOC)))
          {
            $getApplicationContact = mysqli_query('select * from application_contact where application_id = '.$getApplicationRow['id'].' and contact_id = '.$getPersonRow['id'], $db);
            if (!mysqli_fetch_array($getApplicationContact, MYSQLI_ASSOC))
            {
              $insertApplicationContact = mysqli_query('insert into application_contact (type_id, application_id, contact_id, notify, admin, locked) values ('.$getContactTypeRow['id'].', '.$getApplicationRow['id'].', '.$getPersonRow['id'].', 1, 0, 0)', $db);
            }
            mysqli_free_result($getApplicationContact);
          }
          mysqli_free_result($getContactType);
        }
        mysqli_free_result($getApplication);
      }
      mysqli_free_result($getPerson);
      mysqli_close($db);
      $user->setToken();
      $bLocalAdmin = false;
      if (isset($_SESSION['sl_login']) && isset($_SESSION['sl_admin']) && $_SESSION['sl_admin'])
      {
        $bLocalAdmin = true;
      }
      else if (isset($_SESSION['sl_login']) && isset($_SESSION['sl_auth']))
      {
        foreach ($_SESSION['sl_auth'] as $key => $value)
        {
          if ($key == $wgCentralSite && $value)
          {
            $bLocalAdmin = true;
          }
        }
      }
      if ($bLocalAdmin)
      {
        $user->addGroup("sysop");
      }
      $user->saveSettings();
    }
    else
    {
      if ($user->loadFromDatabase())
      {
        return true;
      }
      $user->saveToCache();
    }
  }
  else
  {
    global $wgCentralSite, $wgCentralHost, $wgCentralUser, $wgCentralPassword, $wgCentralDatabase, $wsReturnPath;
    if (isset($_GET['returnto']) && $_GET['returnto'] != '')
    {
      if (!isset($_SESSION['sl_login']) || $_SESSION['sl_login'] == '')
      {
        include_once(dirname(__FILE__).'/../../www/auth/Secure.php');
        $secure = new Secure($wgCentralUser, $wgCentralPassword, $wgCentralHost, $wgCentralDatabase, $wsReturnPath);
        $secure->setApplication($wgCentralSite);
        if (!$secure->isValid())
        {
          $strError = null;
          $secure->processLogin($_POST, $strError);
          if ($secure->isValid())
          {
            echo '<html><head><script type="text/javascript">document.location.href="'.$wgBasePath.'";</script></head></html>';
            exit();
          }
        }
        if ($secure->isValid())
        {
          echo '<html><head><script type="text/javascript">document.location.href="'.$wgBasePath.'";</script></head></html>';
          exit();
        }
        else
        {
          $secure->login();
        }
      }
    }
    else
    {
      return true;
    }
  }

  return false;
}
?>
