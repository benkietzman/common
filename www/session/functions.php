<?php
// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2007-08-01
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//////////////////////////////////////////////////////////////////////////

/*! \file functions.php
* \brief Session handling functions
*
* Provides functions for load balancing session variables via a database. 
*/

if (!isset($GLOBALS['sess_maxlifetime']))
{
  $GLOBALS['sess_maxlifetime'] = 28800;
}
ini_set('session.gc_maxlifetime', $GLOBALS['sess_maxlifetime']);
if (!isset($GLOBALS['sess_db_user']))
{
  $GLOBALS['sess_db_user'] = 'central';
}
if (!isset($GLOBALS['sess_db_password']))
{
  $GLOBALS['sess_db_password'] = 'central';
}
if (!isset($GLOBALS['sess_db_database']))
{
  $GLOBALS['sess_db_database'] = 'central';
}
if (!isset($GLOBALS['sess_db_server']))
{
  $GLOBALS['sess_db_server'] = 'server';
}
if (!isset($GLOBALS['sess_db_table']))
{
  $GLOBALS['sess_db_table'] = 'php_session';
}

// {{{ sessionOpen()
/*! \fn sessionOpen($strSavePath, $strSessionName)
* \brief Open the session.
* \param $strSavePath Contains the path for a session file.
* \param $strSessionName Contains the session name.
*/
function sessionOpen($strSavePath, $strSessionName)
{
  return true;
}
// }}}
// {{{ sessionClose()
/*! \fn sessionClose()
* \brief Close the session.
*/
function sessionClose()
{
  return sessionGC(ini_get('session.gc_maxlifetime'));
}
// }}}
// {{{ sessionRead()
/*! \fn sessionRead($strSessionID)
* \brief Read the session.
* \param $strSessionID Contains the session id.
*/
function sessionRead($strSessionID)
{
  $strResult = '';

  $db = new mysqli(((isset($GLOBALS['sess_dbread_server']) && $GLOBALS['sess_dbread_server'] != '')?$GLOBALS['sess_dbread_server']:$GLOBALS['sess_db_server']), $GLOBALS['sess_db_user'], $GLOBALS['sess_db_password'], $GLOBALS['sess_db_database']);
  if ($db->connect_error == '')
  {
    $strQuery = 'select * from '.$GLOBALS['sess_db_table'].' where session_id = \''.addslashes($strSessionID).'\' and session_data is not null and session_data != \'\'';
    if (($getSession = $db->query($strQuery)) !== false)
    {
      if (($getSessionRow = $getSession->fetch_array(MYSQLI_ASSOC)))
      {
        $strResult = $getSessionRow['session_data'];
      }
      $getSession->free();
    }
    else
    {
      echo 'sessionRead()->mysqli::query() error ['.$strQuery.']:  '.$db->error.'<br>';
    }
  }
  else
  {
    echo 'sessionRead()->mysqli::__construct() error:  '.$db->connect_error.'<br>';
  }
  $db->close();

  return $strResult;
}
// }}}
// {{{ sessionWrite()
/*! \fn sessionWrite($strSessionID, $strSessionData)
* \brief Write the session.
* \param $strSessionID Contains the session id.
* \param $strSessionData Contains the session data.
*/
function sessionWrite($strSessionID, $strSessionData)
{
  $bResult = false;

  $db = new mysqli($GLOBALS['sess_db_server'], $GLOBALS['sess_db_user'], $GLOBALS['sess_db_password'], $GLOBALS['sess_db_database']);
  if ($db->connect_error == '')
  {
    $strSessionJson = json_encode($_SESSION);
    $strQuery = 'insert into '.$GLOBALS['sess_db_table'].' (session_id, last_updated, session_data, session_json) values (\''.$strSessionID.'\', now(), \''.addslashes($strSessionData).'\', \''.addslashes($strSessionJson).'\') on duplicate key update last_updated = now(), session_data = \''.addslashes($strSessionData).'\', session_json = \''.addslashes($strSessionJson).'\'';
    if (($bResult = $db->query($strQuery)) === false)
    {
      echo 'sessionWrite()->mysqli::query() error ['.$strQuery.']:  '.$db->error.'<br>';
    }
  }
  else
  {
    echo 'sessionWrite()->mysqli::__construct() error: '.$db->connect_error.'<br>';
  }
  $db->close();

  return $bResult;
}
// }}}
// {{{ sessionDestroy()
/*! \fn sessionDestroy($strSessionID)
* \brief Destroy the session.
* \param $strSessionID Contains the session id.
*/
function sessionDestroy($strSessionID)
{
  $bResult = false;

  $db = new mysqli($GLOBALS['sess_db_server'], $GLOBALS['sess_db_user'], $GLOBALS['sess_db_password'], $GLOBALS['sess_db_database']);
  if ($db->connect_error == '')
  {
    $strQuery = 'delete from '.$GLOBALS['sess_db_table'].' where session_id = \''.$strSessionID.'\'';
    if (($bResult = $db->query($strQuery)) !== false)
    {
      $bResult = true;
    }
    else
    {
      echo 'sessionDestroy()->mysqli::query() error ['.$strQuery.']:  '.$db->error.'<br>';
    }
  }
  else
  {
    echo 'sessionDestroy()->mysqli::__consturct() error: '.$db->connect_error.'<br>';
  }
  $db->close();

  return $bResult;
}
// }}}
// {{{ sessionGC()
/*! \fn sessionGC($nMaxLifetime)
* \brief Garbage collection.
* \param $nMaxLifetime Contains the maximum lifetime in seconds.
*/
function sessionGC($nMaxLifetime)
{
  $bResult = false;

  $db = new mysqli($GLOBALS['sess_db_server'], $GLOBALS['sess_db_user'], $GLOBALS['sess_db_password'], $GLOBALS['sess_db_database']);
  if ($db->connect_error == '')
  {
    $strQuery = 'delete from '.$GLOBALS['sess_db_table'].' where last_updated < date_sub(now(), interval '.$nMaxLifetime.' second)';
    if (($bResult = $db->query($strQuery)) !== false)
    {
      $bResult = true;
    }
    else
    {
      echo 'sessionGC()->mysqli::query() error ['.$strQuery.']:  '.$db->error.'<br>';
    }
  }
  else
  {
    echo 'sessionGC()->mysqli::__construct() error:  '.$db->connect_error.'<br>';
  }
  $db->close();

  return $bResult;
}
// }}}

if (session_set_save_handler('sessionOpen', 'sessionClose', 'sessionRead', 'sessionWrite', 'sessionDestroy', 'sessionGC') === false)
{
  echo 'session_set_save_hander() error:  Failed to set session handler.<br>';
}
?>
