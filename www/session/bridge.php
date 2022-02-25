<?php
// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2016-03-11
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//////////////////////////////////////////////////////////////////////////

/*! \file bridge.php
* \brief Session handling functions
*
* Provides functions for load balancing session variables via a database. 
*/
// {{{ includes
// include_once('../Utility.php');
// }}}
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
  return true;
}
// }}}
// {{{ sessionRead()
/*! \fn sessionRead($strSessionID, $sessionStreamContext)
* \brief Read the session
* \param $strSessionID Contains the session id.
* \param $sessionStreamContext Contains stream context needed for stream connection.
*/
function sessionRead($strSessionID, $sessionStreamContext)
{
  $strResult = '';
  $port = 12199;

  if ($sessionStreamContext === NULL)
  {
    echo 'sessionRead()->error:  stream context is NULL.';
    unset($port);
    return $strResult;
  }

  $sessionUtility = new Utility;
  if (($handle = $sessionUtility->createClientStream($GLOBALS['sess_server'], $port, $sessionStreamContext, $strError)) !== false)
  {
    $request = array();
    $request['Section'] = 'session';
    $request['Function'] = 'read';
    $request['User'] = $GLOBALS['sess_user'];
    $request['Password'] = $GLOBALS['sess_password'];
    $request['reqApp'] = $GLOBALS['sess_application'];
    $request['Request'] = array('ID'=>$strSessionID);
    $strRequest = json_encode($request)."\n";
    if ((fwrite($handle, $strRequest)) != false)
    {
      if (($strResponse = fgets($handle)) !== false)
      {
        $response = json_decode($strResponse, true);
        if (isset($response['Status']) && $response['Status'] == 'okay')
        {
          if (isset($response['Response']) && is_array($response['Response']) && isset($response['Response']['Data']))
          {
            $strResult = $response['Response']['Data'];
          }
        }
        else if (isset($response['Error']) && is_array($response['Error']))
        {
          echo 'sessionRead() error:  '.json_encode($response['Error']);
        }
        else
        {
          echo 'sessionRead() error:  Encountered an unknown error.';
        }
        unset($response);
      }
      else
      {
        echo 'sessionRead()->fgets() error:  Failed to read response.';
      }
    }
    else
    {
      echo 'sessionRead()->fwrite() error:  Failed to write request.';
    }
    fclose($handle);
  }
  else
  {
    echo 'sessionRead()->createClientStream() error:  Failed to open stream connection to '.$GLOBALS['sess_server'].'(port '.$port.').';
  }
  unset($port);
  unset($request);
  unset($sessionUtility);
  
  return $strResult;
}
// }}}
// {{{ sessionWrite()
/*! \fn sessionWrite($strSessionID, $strSessionData, $sessionStreamContext)
* \brief Write the session
* \param $strSessionID Contains the session id.
* \param $strSessionData Contains the session data.
* \param $sessionStreamContext Contains stream context needed for stream connection.
*/
function sessionWrite($strSessionID, $strSessionData, $sessionStreamContext)
{
  $bResult = false;
  $port = 12199;

  if ($sessionStreamContext === NULL)
  {
    echo 'sessionWrite()->error:  stream context is NULL.';
    unset($port);
    return $bResult;
  }

  $sessionUtility = new Utility;
  if (($handle = $sessionUtility->createClientStream($GLOBALS['sess_server'], $port, $sessionStreamContext, $strError)) !== false)
  {
    $request = array();
    $request['Section'] = 'session';
    $request['Function'] = 'write';
    $request['User'] = $GLOBALS['sess_user'];
    $request['Password'] = $GLOBALS['sess_password'];
    $request['reqApp'] = $GLOBALS['sess_application'];
    $request['Request'] = array('ID'=>$strSessionID, 'Data'=>$strSessionData, 'Json'=>json_encode($_SESSION));
    $strRequest = json_encode($request)."\n";
    if ((fwrite($handle, $strRequest)) != false)
    {
      if (($strResponse = fgets($handle)) !== false)
      {
        $response = json_decode($strResponse, true);
        if (isset($response['Status']) && $response['Status'] == 'okay')
        {
          $bResult = true;
        }
        else if (isset($response['Error']) && is_array($response['Error']))
        {
          echo 'sessionWrite() error:  '.json_encode($response['Error']);
        }
        else
        {
          echo 'sessionWrite() error:  Encountered an unknown error.';
        }
        unset($response);
      }
      else
      {
        echo 'sessionWrite()->fgets() error:  Failed to read response.';
      }
    }
    else
    {
      echo 'sessionWrite()->fwrite() error:  Failed to write request.';
    }
    fclose($handle);
  }
  else
  {
    echo 'sessionWrite()->createClientStream() error:  Failed to open stream connection to '.$GLOBALS['sess_server'].'(port '.$port.').';
  }
  unset($port);
  unset($request);
  unset($sessionUtility);

  return $bResult;
}
// }}}
// {{{ sessionDestroy()
/*! \fn sessionDestroy($strSessionID, $sessionStreamContext)
* \brief Destroy the session (stream or socket).
* \param $strSessionID Contains the session id.
* \param $sessionStreamContext Contains stream context needed for stream connection.
*/
function sessionDestroy($strSessionID, $sessionStreamContext)
{
  $bResult = false;
  $port = 12199;

  if ($sessionStreamContext === NULL)
  {
    echo 'sessionDestroy()->error:  stream context is NULL.';
    unset($port);
    return $bResult;
  }

  $sessionUtility = new Utility;
  if (($handle = $sessionUtility->createClientStream($GLOBALS['sess_server'], $port, $sessionStreamContext, $strError)) !== false)
  {
    $request = array();
    $request['Section'] = 'session';
    $request['Function'] = 'destroy';
    $request['User'] = $GLOBALS['sess_user'];
    $request['Password'] = $GLOBALS['sess_password'];
    $request['reqApp'] = $GLOBALS['sess_application'];
    $request['Request'] = array('ID'=>$strSessionID);
    $strRequest = json_encode($request)."\n";
    if ((fwrite($handle, $strRequest)) != false)
    {
      if (($strResponse = fgets($handle)) !== false)
      {
        $response = json_decode($strResponse, true);
        if (isset($response['Status']) && $response['Status'] == 'okay')
        {
          $bResult = true;
        }
        else if (isset($response['Error']) && is_array($response['Error']))
        {
          echo 'sessionDestroy() error:  '.json_encode($response['Error']);
        }
        else
        {
          echo 'sessionDestroy() error:  Encountered an unknown error.';
        }
        unset($response);
      }
      else
      {
        echo 'sessionDestroy()->fgets() error:  Failed to read response.';
      }
    }
    else
    {
      echo 'sessionDestroy()->fwrite() error:  Failed to write request.';
    }
    fclose($handle);
  }
  else
  {
    echo 'sessionDestroy()->createClientStream() error:  Failed to open stream connection to '.$GLOBALS['sess_server'].'(port '.$port.').';
  }
  unset($port);
  unset($request);
  unset($sessionUtility);

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
  return true;
}
// }}}

if (session_set_save_handler('sessionOpen', 'sessionClose', 'sessionRead', 'sessionWrite', 'sessionDestroy', 'sessionGC') === false)
{
  echo 'session_set_save_hander() error:  Failed to set session handler.<br>';
}
?>
