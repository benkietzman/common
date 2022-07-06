<?php
// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2014-01-09
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//////////////////////////////////////////////////////////////////////////

/*! \file Secure.php
* \brief Secure Class
*
* Provides security functionality with module support.
*/

// {{{ includes
include_once(dirname(__FILE__).'/../Basic.php');
include_once(dirname(__FILE__).'/../database/CommonDatabase.php');
include_once(dirname(__FILE__).'/../Syslog.php');
include_once(dirname(__FILE__).'/../servicejunction/ServiceJunction.php');
include_once(dirname(__FILE__).'/../warden/Warden.php');
if (($mod_handle = opendir(dirname(__FILE__).'/module/')))
{
  while (($mod_file = readdir($mod_handle)) !== false)
  {
    if (strlen($mod_file) > 4 && (substr($mod_file, strlen($mod_file) - 4, 4) == '.php' || substr($mod_file, strlen($mod_file) - 4, 4) == '.inc') && !file_exists(dirname(__FILE__).'/module/'.substr($mod_file, 0, strlen($mod_file) - 4).'.disable'))
    {
      include_once(dirname(__FILE__).'/module/'.$mod_file);
    }
  }
  closedir($mod_handle);
}
if (file_exists(dirname(__FILE__).'/../../../common_addons/www/auth/module') && ($mod_handle = opendir(dirname(__FILE__).'/../../../common_addons/www/auth/module/')))
{
  while (($mod_file = readdir($mod_handle)) !== false)
  {
    if (strlen($mod_file) > 4 && (substr($mod_file, strlen($mod_file) - 4, 4) == '.php' || substr($mod_file, strlen($mod_file) - 4, 4) == '.inc') && !file_exists(dirname(__FILE__).'/../../../common_addons/www/auth/module/'.substr($mod_file, 0, strlen($mod_file) - 4).'.disable'))
    {
      include_once(dirname(__FILE__).'/../../../common_addons/www/auth/module/'.$mod_file);
    }
  }
  closedir($mod_handle);
}
// }}}
//! Secure Class
/*!
* Provides security functionality with module support.
*/
class Secure extends Basic
{
  // {{{ variable
  //! Stores the bool check account value
  protected $m_bCheckAccount;
  //! Stores the ServiceJunction handle.
  protected $m_junction;
  //! Stores the application name
  protected $m_strApplication;
  //! Stores the login title
  protected $m_strLoginTitle;
  //! Stores the login type
  protected $m_strLoginType;
  //! Stores the return path of the resultant data
  protected $m_strReturnPath;
  //! storage the unique prefix
  protected $m_strUniquePrefix;
  //! Stores the module
  protected $m_module;
  //! Stores the database class instantiation
  private $m_database;
  //! Stores the database handle
  private $m_db;
  //! Stores the read-only database handle
  private $m_readdb;
  //! Stores the syslog class instantiation
  private $m_syslog;
  // }}}
  // {{{ __construct()
  /*! \fn __construct($strUser, $strPassword, $strHost, $strDB, $strReturnPath = 'secure')
  * \brief Instantiates the member variables.
  * \param $strUser Contains the database user.
  * \param $strPassword Contains the database password.
  * \param $strHost Contains the database host.
  * \param $strDB Contains the database name.
  * \param $strReturnPath Contains the return path of the resultant data.
  */
  public function __construct($strUser = 'central', $strPassword = 'central', $strHost = 'localhost', $strDB = 'central', $strReturnPath = 'secure')
  {
    parent::__construct();
    $this->m_bCheckAccount = true;
    if ($strHost != '')
    {
      if (!is_array($strHost))
      {
        $strHostTemp = $strHost;
        $strHost = array();
        $strHost['read'] = $strHostTemp;
        $strHost['write'] = $strHostTemp;
      }
      $this->m_database = new CommonDatabase('MySql');
      $this->m_db = $this->m_database->connect($strUser, $strPassword, $strHost['write']);
      $this->m_db->selectDatabase($strDB);
      $this->m_readdb = $this->m_database->connect($strUser, $strPassword, $strHost['read']);
      $this->m_readdb->selectDatabase($strDB);
    }
    else
    {
      if (!is_array($strDB))
      {
        $strDBTemp = $strDB;
        $strDB = array();
        $strDB['read'] = $strDBTemp;
        $strDB['write'] = $strDBTemp;
      }
      $this->m_database = new CommonDatabase('Bridge');
      $this->m_db = $this->m_database->connect($strUser, $strPassword, $strDB['write']);
      $this->m_readdb = $this->m_database->connect($strUser, $strPassword, $strDB['read']);
    }
    if ($this->paramExists('bk_login_choice') && $this->getParam('bk_login_choice') != '')
    {
      $_SESSION['sl_login_choice'] = $this->getParam('bk_login_choice');
    }
    $this->m_junction = new ServiceJunction;
    $this->m_junction->useSecureJunction(false);
    $this->m_junction->setApplication('Common');
    $this->m_syslog = new Syslog('Common Library', 'Secure.php');
    $this->m_strReturnPath = $strReturnPath;
  }
  // }}}
  // {{{ __destruct()
  /*! \fn __destruct()
  * \brief Does nothing.
  */
  public function __destruct()
  {
    $this->m_database->disconnect($this->m_db);
    $this->m_database->disconnect($this->m_readdb);
    parent::__destruct();
  }
  // }}}
  // {{{ enforceAccountCheck()
  /*! \fn enforceAccountCheck($bCheckAccount = true)
  * \brief Sets the account checking feature.
  * \param $bCheckAccount Determines whether to perform an account check.
  */
  public function enforceAccountCheck($bCheckAccount = true)
  {
    $this->m_bCheckAccount = $bCheckAccount;
    if ($this->m_module != null)
    {
      $this->m_module->enforceAccountCheck($bCheckAccount);
    }
  }
  // }}}
  // {{{ getApplication()
  /*! \fn getApplication()
  * \brief Gets the application name.
  * \return Returns the application name.
  */
  public function getApplication()
  {
    return $this->m_strApplication;
  }
  // }}}
  // {{{ getLoginTitle()
  public function getLoginTitle()
  {
    return $this->m_strLoginTitle;
  }
  // }}}
  // {{{ getUserEmail()
  /*! \fn getUserEmail()
  * \brief Fetches the current user email.
  * \return Returns the current user email.
  */
  public function getUserEmail()
  {
    $strEmail = '';
    if (isset($_SESSION['sl_login']))
    {
      $getEmail = $this->m_readdb->parse('select email from person where userid=\''.$_SESSION['sl_login'].'\'');
      if (($getEmailRow = $getEmail->fetch()))
      {
        $strEmail = $getEmailRow['email'];
      }
      $this->m_readdb->free($getEmail);
    }

    return $strEmail;
  }
  // }}}
  // {{{ getUserFirstName()
  /*! \fn getUserFirstName()
  * \brief Fetches the current user first name.
  * \return Returns the current user first name.
  */
  public function getUserFirstName()
  {
    $strName = '';
    if (isset($_SESSION['sl_login']))
    {
      $strName = $_SESSION['sl_login'];
      $getName = $this->m_readdb->parse('select first_name from person where userid=\''.$_SESSION['sl_login'].'\'');
      if (($getNameRow = $getName->fetch()))
      {
        $strName = $getNameRow['first_name'];
      }
      $this->m_readdb->free($getName);
    }

    return $strName;
  }
  // }}}
  // {{{ getUserID()
  /*! \fn getUserID()
  * \brief Fetches the current user id.
  * \return Returns the current user id.
  */
  public function getUserID()
  {
    $strUserID = null;

    if (isset($_SESSION['sl_login']))
    {
      $strUserID = $_SESSION['sl_login'];
    }

    return $strUserID;
  }
  // }}}
  // {{{ getUserLastName()
  /*! \fn getUserLastName()
  * \brief Fetches the current user last name.
  * \return Returns the current user last name.
  */
  public function getUserLastName()
  {
    $strName = '';
    if (isset($_SESSION['sl_login']))
    {
      $strName = $_SESSION['sl_login'];
      $getName = $this->m_readdb->parse('select last_name from person where userid=\''.$_SESSION['sl_login'].'\'');
      if (($getNameRow = $getName->fetch()))
      {
        $strName = $getNameRow['last_name'];
      }
      $this->m_readdb->free($getName);
    }

    return $strName;
  }
  // }}}
  // {{{ getUserName()
  /*! \fn getUserName()
  * \brief Fetches the current user name.
  * \return Returns the current user name.
  */
  public function getUserName()
  {
    $strName = '';
    if (isset($_SESSION['sl_login']))
    {
      $strName = $_SESSION['sl_login'];
      $getName = $this->m_readdb->parse('select first_name, last_name from person where userid=\''.$_SESSION['sl_login'].'\'');
      if (($getNameRow = $getName->fetch()))
      {
        $strName = $getNameRow['first_name'].' '.$getNameRow['last_name'];
      }
      $this->m_readdb->free($getName);
    }

    return $strName;
  }
  // }}}
  // {{{ isGlobalAdmin()
  /*! \fn isGlobalAdmin()
  * \brief Determines whether the current user is a global admin.
  * \return Returns a boolean value.
  */
  public function isGlobalAdmin()
  {
    $bResult = false;

    if (isset($_SESSION['sl_login']) && isset($_SESSION['sl_admin']) && $_SESSION['sl_admin'])
    {
      $bResult = true;
      $this->m_syslog->logon('Authorized the user as a global administrator.', $this->getUserID());
    }
    else
    {
      $this->m_syslog->logon('Failed to authorize the user as a global administrator.', $this->getUserID(), false);
    }

    return $bResult;
  }
  // }}}
  // {{{ isLocalAdmin()
  /*! \fn isLocalAdmin($strApplication = null, $bAny = false, $bLocal = false)
  * \brief Determines whether the current user is a local admin for the given application.
  * \param $strApplication Contains the application name.
  * \param $bAny Determines whether any application is acceptable.
  * \param $bLocal Determines whether it shouuld test only for local admin and exclude global admins.
  * \return Returns a boolean value.
  */
  public function isLocalAdmin($strApplication = null, $bAny = false, $bLocal = false)
  {
    $bResult = false;

    if ($strApplication == "")
    {
      $strApplication = $this->m_strApplication;
    }
    if (!$bLocal && $this->isGlobalAdmin())
    {
      $bResult = true;
    }
    else if (($bAny || $this->m_strApplication != "") && isset($_SESSION['sl_login']) && isset($_SESSION['sl_auth']))
    {
      foreach ($_SESSION['sl_auth'] as $key => $value)
      {
        if (($bAny || $key == $strApplication) && $value)
        {
          $bResult = true;
        }
      }
    }
    if ($bResult)
    {
      $this->m_syslog->logon('Authorized the user as a local administrator for the '.$this->m_strApplication.' application.', $this->getUserID());
    }
    else
    {
      $this->m_syslog->logon('Failed to authorize the user as a local administrator for the '.$this->m_strApplication.' application.', $this->getUserID(), false);
    }

    return $bResult;
  }
  // }}}
  // {{{ isValid()
  /*! \fn isValid($strApplication = null)
  * \brief Determines whether the current user has access to the given application.
  * \param $strApplication Contains the application name.
  * \return Returns a boolean value.
  */
  public function isValid($strApplication = null)
  {
    $bResult = false;
    $bCheckAccount = (($strApplication != '')?true:false);

    if ($strApplication == '')
    {
      $strApplication = $this->m_strApplication;
    }
    if ($this->isGlobalAdmin())
    {
      $bResult = true;
    }
    else if (isset($_SESSION['sl_login']))
    {
      if (!$this->m_bCheckAccount && !$bCheckAccount)
      {
        $bResult = true;
      }
      else if (isset($_SESSION['sl_auth']) && $strApplication != '')
      {
        foreach ($_SESSION['sl_auth'] as $key => $value)
        {
          if ($key == $strApplication)
          {
            $bResult = true;
          }
        }
      }
    }
    if ($bResult)
    {
      $this->m_syslog->logon('Authorized the user for the '.$this->m_strApplication.' application.', $this->getUserID());
    }
    else
    {
      $this->m_syslog->logon('Failed to authorize the user for the '.$this->m_strApplication.' application.', $this->getUserID(), false);
    }

    return $bResult;
  }
  // }}}
  // {{{ loadJwt()
  /*! \fn loadJwt($strJwt, &$strError)
  * \brief Fetches and loads the session variables.
  * \param $strJwt Contains the encoded JWT.
  * \param $strError Contains the error.
  * \return Returns a boolean value.
  */
  public function loadJwt($strJwt, &$strError)
  {
    $bResult = false;

    if ($strJwt != '')
    {
      $warden = new Warden('Bridge', '/data/warden/socket');
      $secret = null;
      if ($warden->vaultRetrieve(['jwt'], $secret))
      {
        if (isset($secret['Secret']) && $secret['Secret'] != '')
        {
          if (isset($secret['Signer']) && $secret['Signer'] != '')
          {
            $strPayload = null;
            $strEncrypted = base64_decode($strJwt);
            $this->m_junction->aes($secret['Secret'], $strPayload, $strEncrypted);
            if ($strPayload == '')
            {
              $strPayload = $strJwt;
            }
            $payload = null;
            if ($this->m_junction->jwt($secret['Signer'], $secret['Secret'], $strPayload, $payload))
            {
              $bResult = true;
              $_SESSION = $payload;
            }
            else
            {
              $strError = $this->m_junction->getError();
            }
          }
          else
          {
            $strError = 'Failed to find JWT signer.';
          }
        }
        else
        {
          $strError = 'Failed to find JWT secret.';
        }
      }
      else
      {
        $strError = $warden->getError();
      }
      unset($secret);
    }
    else
    {
      $strError = 'Please provide the Jwt.';
    }

    return $bResult;
  }
  // }}}
  // {{{ loadSession()
  /*! \fn loadSession($strSessionID, &$strError)
  * \brief Fetches and loads the session variables.
  * \param $strSessionID Contains the session ID.
  * \param $strError Contains the error.
  * \return Returns a boolean value.
  */
  public function loadSession($strSessionID, &$strError)
  {
    $bResult = false;

    if ($strSessionID != '')
    {
      $getSession = $this->m_db->parse('select session_json from php_session where session_id = \''.$strSessionID.'\'');
      if (($getSessionRow = $getSession->fetch()))
      {
        $bResult = true;
        $_SESSION = json_decode($getSessionRow['session_json'], true);
      }
      else
      {
        $strError = 'Failed to find the session.';
      }
      $this->m_db->free($getSession);
    }
    else
    {
      $strError = 'Please provide the session ID.';
    }

    return $bResult;
  }
  // }}}
  // {{{ login()
  /*! \fn login($strRetPath = null)
  * \brief Write the HTML login page.
  * \param $strRetPath Contains the return path from a successful login.
  */
  public function login($strRetPath = null, $bJson = false)
  {
    $response = null;

    if ($this->m_module != null)
    {
      $response = $this->m_module->login($strRetPath, $bJson);
    }
    else
    {
      echo '<form name="bk_login_formlogin" method="post">';
      echo '<table class="std_form">';
      echo '<tr><td>';
      echo '<table class="std_pad">';
      echo '<tr><td style="white-space:nowrap;">';
      echo '<label for="bk_login_choice" accesskey="c"><b><u>C</u>hoose login type:</b></label>';
      echo '</td><td>';
      echo '<select name="bk_login_choice">';
      $getLoginType = $this->m_readdb->parse('select type from login_type order by type');
      while (($getLoginTypeRow = $getLoginType->fetch()))
      {
        echo '<option value="'.$getLoginTypeRow['type'].'">'.$getLoginTypeRow['type'].'</option>';
      }
      $this->m_readdb->free($getLoginType);
      echo '</td><td>';
      echo '<input type="submit" class="std_button" value="Submit">';
      echo '</td></tr>';
      echo '</table>';
      echo '</td></tr>';
      echo '</table>';
      echo '</form>';
    }

    return $response;
  }
  // }}}
  // {{{ logout()
  /*! \fn logout($strRetPath = null)
  * \brief Unsets all security authorization for a user.  This function uses session_destroy() which appears to keep you from using the $_SESSION array.
  * \param $strRetPath Contains the return path from a successful login.
  */
  public function logout($strRetPath = null, $bJson = false)
  {
    $response = null;

    if (isset($_SESSION['sl_login_choice']))
    {
      unset($_SESSION['sl_login_choice']);
    }
    if (isset($_SESSION['sl_login']))
    {
      unset($_SESSION['sl_login']);
    }
    if (isset($_SESSION['sl_admin']))
    {
      unset($_SESSION['sl_admin']);
    }
    if (isset($_SESSION['sl_auth']))
    {
      unset($_SESSION['sl_auth']);
    }
    if (isset($_SESSION['sl_change']))
    {
      unset($_SESSION['sl_change']);
    }
    if ($this->m_module != null)
    {
      $response = $this->m_module->logout($strRetPath, $bJson);
    }
    $_SESSION = array();
    if (isset($_COOKIE[session_name()]))
    {
      setcookie(session_name(), '', time()-42000, '/');
    }
    session_destroy();
    if ($bJson)
    {
      if (!is_array($response))
      {
        $response = array();
      }
      if (!isset($response['Status']))
      {
        $response['Status'] = 'okay';
      }
      if ($strRetPath != '' && !isset($response['Redirect']))
      {
        $response['Redirect'] = $strRetPath;
      }
    }
    else if ($strRetPath != '')
    {
      echo '<script type="text/javascript">document.location.href="'.$strRetPath.'";</script>';
    }

    return $response;
  }
  // }}}
  // {{{ processLogin()
  /*! \fn processLogin()
  * \brief Processes a user login in order to set their security status.
  */
  public function processLogin($request, &$strError)
  {
    $bResult = false;

    if ($request == null && isset($_POST))
    {
      $request = $_POST;
    }
    $getApplication = $this->m_readdb->parse('select * from application where name = \''.$this->m_strApplication.'\'');
    if ($getApplication->execute())
    {
      if (($getApplicationRow = $getApplication->fetch()))
      {
        $bResult = true;
        if (!$this->isValid())
        {
          if ($this->m_module != null)
          {
            if (($bResult = $this->m_module->processLogin($request, $strError)))
            {
              $this->m_syslog->logon('Authenticated the user against the '.$this->m_strLoginType.' module.', $this->getUserID());
            }
            else
            {
              $this->m_syslog->logon('Failed to authenticate the user against the '.$this->m_strLoginType.' module.', $this->getUserID(), false);
            }
          }
          else
          {
            $strError = $_SESSION['sl_message'] = 'Please load a security module.';
          }
        }
        if ($getApplicationRow['auto_register'])
        {
          $bRevertAccountCheck = $this->m_bCheckAccount;
          if (!$this->isValid())
          {
            $this->enforceAccountCheck(false);
            if ($this->m_module != null)
            {
              if (($bResult = $this->m_module->processLogin($request, $strError)))
              {
                $this->m_syslog->logon('Authenticated the user against the '.$this->m_strLoginType.' module.', $this->getUserID());
              }
              else
              {
                $this->m_syslog->logon('Failed to authenticate the user against the '.$this->m_strLoginType.' module.', $this->getUserID(), false);
              }
            }
          }
          if ($this->isValid())
          {
            $_SESSION['sl_message'] = '&nbsp;';
            $getPerson = $this->m_readdb->parse('select id from person where userid = \''.$this->getUserID().'\'');
            if ($getPerson->execute())
            {
              if (!$getPerson->fetch())
              {
                $insertPerson = $this->m_db->parse('insert into person (userid, active, admin, locked) values (\''.$this->getUserID().'\', 1, 0, 0)');
                if ($insertPerson->execute())
                {
                  $this->syslog()->userAccountCreated('Created the user.', $this->getUserID());
                }
                else
                {
                  $strError = $_SESSION['sl_message'] = 'Auto-Register Failed:  Could not insert user into central database ('.$insertPerson->getError().')';
                  $this->syslog()->userAccountCreated('Failed to create the user.', $this->getUserID(), false);
                }
              }
            }
            else
            {
              $strError = $_SESSION['sl_message'] = $getPerson->getError();
            }
            $this->m_readdb->free($getPerson);
            $getPerson = $this->m_readdb->parse('select id from person where userid = \''.$this->getUserID().'\'');
            if ($getPerson->execute())
            {
              if (($getPersonRow = $getPerson->fetch()))
              {
                $getContactType = $this->m_readdb->parse('select id from contact_type where type = \'Contact\'');
                if ($getContactType->execute())
                {
                  if (($getContactTypeRow = $getContactType->fetch()))
                  {
                    $getApplicationContact = $this->m_readdb->parse('select * from application_contact where application_id = '.$getApplicationRow['id'].' and contact_id = '.$getPersonRow['id']);
                    if ($getApplicationContact->execute())
                    {
                      if (!$getApplicationContact->fetch())
                      {
                        $insertApplicationContact = $this->m_db->parse('insert into application_contact (type_id, application_id, contact_id, notify, admin, locked) values ('.$getContactTypeRow['id'].', '.$getApplicationRow['id'].', '.$getPersonRow['id'].', 1, 0, 0)');
                        if ($insertApplicationContact->execute())
                        {
                          $this->syslog()->userAccountCreated('Created the user for the '.$this->m_strApplication.' application.', $this->getUserID());
                        }
                        else
                        {
                          $_SESSION['sl_message'] = 'Auto-Register Failed:  Could not register user with application ('.$insertApplicationContact->getError().')';
                          $this->syslog()->userAccountCreated('Failed to create the user for the '.$this->m_strApplication.' application.', $this->getUserID(), false);
                        }
                      }
                    }
                    else
                    {
                      $strError = $_SESSION['sl_message'] = $getApplicationContact->getError();
                    }
                    $this->m_readdb->free($getApplicationContact);
                  }
                }
                else
                {
                  $strError = $_SESSION['sl_message'] = $getContactType->getError();
                }
                $this->m_readdb->free($getContactType);
              }
            }
            else
            {
              $strError = $_SESSION['sl_message'] = $getPerson->getError();
            }
            $this->m_readdb->free($getPerson);
            $this->enforceAccountCheck($bRevertAccountCheck);
            if ($_SESSION['sl_message'] == '&nbsp;' && $this->m_module != null)
            {
              if (($bResult = $this->m_module->processLogin($request, $strError)))
              {
                $this->m_syslog->logon('Authenticated the user against the '.$this->m_strLoginType.' module.', $this->getUserID());
              }
              else
              {
                $this->m_syslog->logon('Failed to authenticate the user against the '.$this->m_strLoginType.' module.', $this->getUserID(), false);
              }
            }
          }
          else
          {
            $this->enforceAccountCheck($bRevertAccountCheck);
          }
        }
      }
      else
      {
        $strError = $_SESSION['sl_message'] = 'Application does not exist in central database.';
      }
    }
    else
    {
      $strError = $_SESSION['sl_message'] = $getApplication->getError();
    }
    $this->m_readdb->free($getApplication);

    return $bResult;
  }
  // }}}
  // {{{ setApplication()
  /*! \fn setApplication($strApplication = null)
  * \brief Sets the application name.
  * \param $strApplication Contains the application name.
  */
  public function setApplication($strApplication = null)
  {
    $this->m_strApplication = $strApplication;
    if ($this->m_module == null || $this->m_strApplication != $strApplication)
    {
      $getApplicationType = $this->m_readdb->parse('select id, account_check, login_type_id from application where name=\''.$strApplication.'\'');
      if (($getApplicationTypeRow = $getApplicationType->fetch()))
      {
        $this->m_bCheckAccount = (($getApplicationTypeRow['account_check'] == 1)?true:false);
        if ($getApplicationTypeRow['login_type_id'] != '')
        {
          $getLoginType = $this->m_readdb->parse('select type from login_type where id = '.$getApplicationTypeRow['login_type_id']);
          if (($getLoginTypeRow = $getLoginType->fetch()))
          {
            $this->setSecurity($getLoginTypeRow['type']);
          }
          $this->m_readdb->free($getLoginType);
        }
        else if (isset($_SESSION['sl_login_choice']) && $_SESSION['sl_login_choice'] != '')
        {
          $this->setSecurity($_SESSION['sl_login_choice']);
        }
      }
      $this->m_readdb->free($getApplicationType);
    }
  }
  // }}}
  // {{{ setSecurity()
  /*! \fn setSecurity($strLoginType = null)
  * \brief Sets the application security module.
  * \param $strLoginType Contains the application security module name.
  */
  public function setSecurity($strLoginType = null)
  {
    if ($strLoginType != null)
    {
      unset($this->m_module);
      $this->m_strLoginType = $strLoginType;
      $strLoginType = 'bk_'.$strLoginType;
      $this->m_module = new $strLoginType(array('write'=>$this->m_db, 'read'=>$this->m_readdb), $this->m_strApplication, $this->m_bCheckAccount, $this->m_strReturnPath);
      $this->m_strLoginTitle = $this->m_module->getLoginTitle();
    }
  }
  // }}}
  // {{{ setUniquePrefix()
  /*! \fn setUniquePrefix($strPrefix)
  * \brief Sets the unique prefix.
  * \param $strUniquePrefix Contains the unique prefix.
  */
  public function setUniquePrefix($strPrefix)
  {
    $this->m_strUniquePrefix = $strPrefix;
  }
  // }}}
  // {{{ syslog()
  /*! \fn syslog()
  * \brief Accesses the syslog instantiation.
  */
  public function syslog()
  {
    return $this->m_syslog;
  }
  // }}}
}
?>
