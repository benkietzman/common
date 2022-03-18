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

/*! \file Windows.php
* \brief Windows Class
*
* This module plugs into the Secure class.
*/

include_once(dirname(__FILE__).'/../../servicejunction/ServiceJunction.php');

//! Windows Class
/*!
* This module plugs into the Secure class.
*/
class bk_Windows
{
  // {{{ variable
  //! Stores the application name
  private $m_strApplication;
  //! Stores the login title
  private $m_strLoginTitle;
  //! Stores the bool check account value
  private $m_bCheckAccount;
  //! Stores the database handle
  private $m_db;
  //! Stores the read-only database handle
  private $m_readdb;
  //! Stores the Service Junction instantiation
  private $m_junction;
  // }}}
  // {{{ __construct()
  /*! \fn __construct($db = null, $strApplication = null, $bCheckAccount = true)
  * \brief Instantiates the member variables.
  * \param $db Contains the database handle.
  * \param $strApplication Contains the application name.
  * \param $bCheckAccount Determines whether to perform an account check.
  */
  public function __construct($db = null, $strApplication = null, $bCheckAccount = true)
  {
    if (is_array($db))
    {
      $this->m_db = $db['write'];
      $this->m_readdb = $db['read'];
    }
    else
    {
      $this->m_db = $this->m_readdb = $db;
    }
    $this->m_strApplication = $strApplication;
    $this->m_strLoginTitle = 'Windows Login';
    $this->m_bCheckAccount = $bCheckAccount;
    $this->m_junction = new ServiceJunction;
  }
  // }}}
  // {{{ __destruct()
  /*! \fn __destruct()
  * \brief Closes the database.
  */
  public function __destruct()
  {
  }
  // }}}
  // {{{ getLoginTitle()
  public function getLoginTitle()
  {
    return $this->m_strLoginTitle;
  }
  // }}}
  // {{{ setDatabase()
  /*! \fn setDatabase($db)
  * \brief Sets the database handle.
  * \param $db Contains the database handle.
  */
  public function setDatabase($db)
  {
    if (is_array($db))
    {
      $this->m_db = $db['write'];
      $this->m_readdb = $db['read'];
    }
    else
    {
      $this->m_db = $this->m_readdb = $db;
    }
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

    return $bResult;
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
    }

    return $bResult;
  }
  // }}}
  // {{{ isLocalAdmin()
  /*! \fn isLocalAdmin($strApplication = null, $bAny = false)
  * \brief Determines whether the current user is a local admin for the given application.
  * \param $strApplication Contains the application name.
  * \param $bAny Determines whether any application is acceptable.
  * \return Returns a boolean value.
  */
  public function isLocalAdmin($strApplication = null, $bAny = false)
  {
    $bResult = false;

    if ($strApplication == "")
    {
      $strApplication = $this->m_strApplication;
    }
    if ($this->isGlobalAdmin())
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

    return $bResult;
  }
  // }}}
  // {{{ processLogin()
  /*! \fn processLogin()
  * \brief Processes a user login in order to set their security status.
  */
  public function processLogin($request, &$strError)
  {
    $strUserID = ((isset($request['userid']))?$request['userid']:((isset($request['bk_login_user_id']))?$request['bk_login_user_id']:''));
    $strPassword = ((isset($request['password']))?$request['password']:((isset($request['bk_login_password']))?$request['bk_login_password']:''));

    if (!$this->m_bCheckAccount || $this->m_strApplication != "")
    {
      if ($strUserID != '' && $strPassword != '')
      {
        if (isset($_SESSION['sl_login']))
        {
          unset($_SESSION['sl_login']);
        }
        if (isset($_SESSION['sl_auth']))
        {
          unset($_SESSION['sl_auth']);
        }
        if (isset($_SESSION['sl_admin']))
        {
          unset($_SESSION['sl_admin']);
        }
        $getAccount = $this->m_readdb->parse("select * from person where userid='".$strUserID."'");
        if (($getAccountRow = $getAccount->fetch()) || !$this->m_bCheckAccount)
        {
          $getApplicationID = $this->m_readdb->parse("select a.id application_id, b.type type_name from application a, login_type b where a.login_type_id=b.id and a.name='".$this->m_strApplication."'");
          if (!$this->m_bCheckAccount || ($getApplicationIDRow = $getApplicationID->fetch()))
          {
            if ($this->m_bCheckAccount)
            {
              $getAccess = $this->m_readdb->parse("select * from application_contact where contact_id=".$getAccountRow['id']." and application_id=".$getApplicationIDRow['application_id']);
            }
            $bAuthenticated = false;
            $in = array();
            $out = null;
            $request = array();
            $request['Service'] = 'samba';
            $request['Function'] = 'login';
            $request['User'] = $strUserID;
            $request['Password'] = $strPassword;
            if (($strJson = file_get_contents(dirname(__FILE__).'/../../../../common_addons/www/auth/modules.json')) !== false)
            {
              $modules = json_decode($strJson, true);
              if (isset($modules['windows']) && isset($modules['windows']['domain']) && $modules['windows']['domain'] != '')
              {
                $request['Domain'] = $modules['windows']['domain'];
              }
              unset($modules);
            }
            $request['reqApp'] = 'Common Library';
            $in[] = json_encode($request);
            unset($request);
            if ($this->m_junction->request($in, $out))
            {
              if (sizeof($out) > 0)
              {
                $response = json_decode($out[0], true);
                if (isset($response['Status']) && $response['Status'] == 'okay')
                {
                  $bAuthenticated = true;
                }
                else if (isset($response['Error']) && $response['Error'] != '')
                {
                  $strError = $response['Error'];
                }
              }
            }
            else
            {
              $strError = $this->m_junction->getError();
            }
            unset($in);
            unset($out);
            if (!$this->m_bCheckAccount || $getAccountRow['admin'] || ($getAccessRow = $getAccess->fetch()))
            {
              if ($bAuthenticated && (!$this->m_bCheckAccount || ($getAccountRow['active'] && !$getAccountRow['locked'] && ($getAccountRow['admin'] || !$getAccessRow['locked']))))
              {
                if ($this->m_bCheckAccount)
                {
                  $getPerson = $this->m_readdb->parse('select * from person where id='.$getAccountRow['id']);
                }
                if (isset($getAccountRow['admin']) && $getAccountRow['admin'])
                {
                  $_SESSION['sl_login'] = $strUserID;
                  $_SESSION['sl_admin'] = true;
                }
                if (!$this->m_bCheckAccount || (($getPersonRow = $getPerson->fetch()) && $getPersonRow['userid'] == $strUserID))
                {
                  $_SESSION['sl_login'] = $strUserID;
                  $_SESSION['sl_auth'] = array();
                  if (isset($getAccountRow['id']))
                  {
                    $getApplication = $this->m_readdb->parse("select * from application where login_type_id is not null order by name");
                    while (($getApplicationRow = $getApplication->fetch()))
                    {
                      $getAccessable = $this->m_readdb->parse("select * from application_contact where contact_id=".$getAccountRow['id']." and application_id=".$getApplicationRow['id']);
                      if (($getAccessableRow = $getAccessable->fetch()))
                      {
                        $_SESSION['sl_auth'][$getApplicationRow['name']] = (($getAccessableRow['admin'])?true:false);
                      }
                      $this->m_readdb->free($getAccessable);
                    }
                    $this->m_readdb->free($getApplication);
                  }
                  if (sizeof($_SESSION['sl_auth']) == 0)
                  {
                    unset($_SESSION['sl_auth']);
                  }
                }
                else
                {
                  $strError = 'Data Error';
                }
                if ($this->m_bCheckAccount)
                {
                  $this->m_readdb->free($getPerson);
                }
              }
              else
              {
                if ($this->m_bCheckAccount && !$getAccountRow['active'])
                {
                  $strError = 'Account Not Active';
                }
                else if ($this->m_bCheckAccount && ($getAccountRow['locked'] || (isset($getAccessRow) && $getAccessRow['locked'])))
                {
                  $strError = 'Account Locked';
                }
                else if ($strError == '')
                {
                  $strError = 'Unknown Error';
                }
              }
            }
            else
            {
              $strError = 'Application Access Failed';
            }
            if ($this->m_bCheckAccount)
            {
              $this->m_readdb->free($getAccess);
            }
          }
          else
          {
            $strError = 'Invalid Application Name';
          }
          $this->m_readdb->free($getApplicationID);
        }
        else
        {
          $strError = 'Not Valid User ID';
        }
        $this->m_readdb->free($getAccount);
      }
    }
    else
    {
      $strError = 'Application Not Registered';
    }
  }
  // }}}
  // {{{ login()
  /*! \fn login($strRetPath = null)
  * \brief Write the HTML login page.
  * \param $strRetPath Contains the return path from a successful login.
  */
  public function login($strRetPath = null, $bJson = false)
  {
    $response = array();
    $strUserID = (isset($_POST['bk_login_user_id']))?$_POST['bk_login_user_id']:"";
    $strPassword = (isset($_POST['bk_login_password']))?$_POST['bk_login_password']:"";
    $strMessage = (isset($_SESSION['sl_message']))?$_SESSION['sl_message']:"&nbsp;";

    if ($bJson)
    {
      $response['Status'] = 'okay';
    }
    else
    {
      echo "<div style='font-size:16px;font-weight:bold;'>Windows Login</div><br>";
      echo "<form name='bk_login_formlogin' method='post' onSubmit='return bk_login_checklogin();'>";
      echo "<table class='std_form'>";
      echo "<tr><td>";
      echo "<table class='std_pad'>";
      echo "<tr><td nowrap>";
      echo "<label for='bk_login_user_id' accesskey='u'>";
      echo "<span class='std_bold_font'><u>U</u>serID:&nbsp;&nbsp;</span>";
      echo "</label>";
      echo "</td><td>";
      echo "<input id='bk_login_user_id' name='bk_login_user_id' type='text' size='6' maxlength='8' value='" . $strUserID . "'>";
      echo "</td></tr><tr><td nowrap>";
      echo "<label for='bk_login_password' accesskey='p'>";
      echo "<span class='std_bold_font'><u>P</u>assword:&nbsp;&nbsp;</span>";
      echo "</label>";
      echo "</td><td>";
      echo "<input id='bk_login_password' name='bk_login_password' type='password' size='6' maxlength='20' value='".$strPassword."'>";
      echo "</td></tr><tr><td align='center' colspan='2'>";
      echo "<span id='bk_login_message' class='std_error'>".$strMessage."</span>";
      echo "<hr width='95%'>";
      echo "</td></tr><tr><td colspan='2'>";
      echo "<input type='submit' class='std_button' style='float:right;' value='Login'>";
      echo "</td></tr>";
      echo "</table>";
      echo "</td></tr>";
      echo "</table>";
      echo "</form>";
    }

    return $response;
  }
  // }}}
  // {{{ logout()
  /*! \fn logout($strRetPath = null)
  * \brief Unsets all security authorization for a user.
  * \param $strRetPath Contains the return path from a successful login.
  */
  public function logout($strRetPath = null, $bJson = false)
  {
    return null;
  }
  // }}}
}
?>
