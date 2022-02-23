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
/*! \fn sessionRead($strSessionID)
* \brief Read the session (socket only).
* \param $strSessionID Contains the session id.
*/
function sessionRead($strSessionID)
{
  $strResult = sessionReadData($strSessionID, false, NULL);
  return $strResult;
}
// }}}
// {{{ sessionReadData()
/*! \fn sessionReadData($strSessionID, $bSessionUseStream, $sessionStreamContext)
* \brief Read the session (stream or socket).
* \param $strSessionID Contains the session id.
* \param $bSessionUseStream Contains boolean for stream (true) or socket (false) connections.
* \param $sessionStreamContext Contains stream context needed for stream connection.
*/
function sessionReadData($strSessionID, $bSessionUseStream, $sessionStreamContext)
{
  $strResult = '';
  $port = 12199;

  if ($bSessionUseStream)
  {
    if ( $sessionStreamContext !== NULL )
    {
      $sessionUtility = new Utility; // Note: The calling program needs to include 'common/www/Utility.php'.
    }
    else
    {
      echo 'sessionReadData()->error:  stream context is NULL.';
      unset($port);
      return $strResult;
    }
  }

  if ((!$bSessionUseStream && ($handle = fsockopen($GLOBALS['sess_server'], $port)) !== false) || ( $bSessionUseStream && ($handle = $sessionUtility->createClientStream($GLOBALS['sess_server'], $port, $sessionStreamContext, $strError)) !== false)) 
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
          echo 'sessionReadData() error:  '.json_encode($response['Error']);
        }
        else
        {
          echo 'sessionReadData() error:  Encountered an unknown error.';
        }
        unset($response);
      }
      else
      {
        echo 'sessionReadData()->fgets() error:  Failed to read response.';
      }
    }
    else
    {
      echo 'sessionReadData()->fwrite() error:  Failed to write request.';
    }
    fclose($handle);
  }
  else
  {
    if ($bSessionUseStream)
    {
      echo 'sessionReadData()->createClientStream() error:  Failed to open stream connection to '.$GLOBALS['sess_server'].'(port '.$port.').';
    }
    else
    {
      echo 'sessionReadData()->fsockopen() error:  Failed to open socket connection to '.$GLOBALS['sess_server'].'(port '.$port.').';
    }
  }
  unset($port);
  unset($request);
  if ($bSessionUseStream)
  {
    unset($sessionUtility);
  }
  
  return $strResult;
}
// }}}
// {{{ sessionWrite()
/*! \fn sessionWrite($strSessionID, $strSessionData)
* \brief Write the session (socket only).
* \param $strSessionID Contains the session id.
* \param $strSessionData Contains the session data.
*/
function sessionWrite($strSessionID, $strSessionData)
{
  $bResult = sessionWriteData($strSessionID, $strSessionData, false, NULL);
  return $bResult;
}
// }}}
// {{{ sessionWriteData()
/*! \fn sessionWriteData($strSessionID, $strSessionData, $bSessionUseStream, $sessionStreamContext)
* \brief Write the session (stream or socket).
* \param $strSessionID Contains the session id.
* \param $strSessionData Contains the session data.
* \param $bSessionUseStream Contains boolean for stream (true) or socket (false) connections.
* \param $sessionStreamContext Contains stream context needed for stream connection.
*/
function sessionWriteData($strSessionID, $strSessionData, $bSessionUseStream, $sessionStreamContext)
{
  $bResult = false;
  $port = 12199;

  if ( $bSessionUseStream )
  {
    if ( $sessionStreamContext !== NULL )
    {
      $sessionUtility = new Utility;
    }
    else
    {
      echo 'sessionWriteData()->error:  stream context is NULL.';
      unset($port);
      return $bResult;
    }
  }

  if ((!$bSessionUseStream && ($handle = fsockopen($GLOBALS['sess_server'], $port)) !== false) || ( $bSessionUseStream && ($handle = $sessionUtility->createClientStream($GLOBALS['sess_server'], $port, $sessionStreamContext, $strError)) !== false))
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
          echo 'sessionWriteData() error:  '.json_encode($response['Error']);
        }
        else
        {
          echo 'sessionWriteData() error:  Encountered an unknown error.';
        }
        unset($response);
      }
      else
      {
        echo 'sessionWriteData()->fgets() error:  Failed to read response.';
      }
    }
    else
    {
      echo 'sessionWriteData()->fwrite() error:  Failed to write request.';
    }
    fclose($handle);
  }
  else
  {
    if ($bSessionUseStream)
    {
      echo 'sessionWriteData()->createClientStream() error:  Failed to open stream connection to '.$GLOBALS['sess_server'].'(port '.$port.').';
    }
    else
    {
      echo 'sessionWriteData()->fsockopen() error:  Failed to open socket connection to '.$GLOBALS['sess_server'].'(port '.$port.').';
    }
  }
  unset($port);
  unset($request);
  if ($bSessionUseStream)
  {
    unset($sessionUtility);
  }

  return $bResult;
}
// }}}
// {{{ sessionDestroy()
/*! \fn sessionDestroy($strSessionID)
* \brief Destroy the session (socket only).
* \param $strSessionID Contains the session id.
*/
function sessionDestroy($strSessionID)
{
  $bResult = sessionDestroyAll($strSessionID, false, NULL);
  return $bResult;
}
// }}}
// {{{ sessionDestroyAll()
/*! \fn sessionDestroy($strSessionID, $bSessionUseStream, $sessionStreamContext)
* \brief Destroy the session (stream or socket).
* \param $strSessionID Contains the session id.
* \param $bSessionUseStream Contains boolean for stream (true) or socket (false) connections.
* \param $sessionStreamContext Contains stream context needed for stream connection.
*/
function sessionDestroyAll($strSessionID, $bSessionUseStream, $sessionStreamContext)
{
  $bResult = false;
  $port = 12199;

  if ( $bSessionUseStream )
  {
    if ( $sessionStreamContext !== NULL )
    {
      $sessionUtility = new Utility;
    }
    else
    {
      echo 'sessionDestroyAll()->error:  stream context is NULL.';
      unset($port);
      return $bResult;
    }
  }

  if ((!$bSessionUseStream && ($handle = fsockopen($GLOBALS['sess_server'], $port)) !== false) || ( $bSessionUseStream && ($handle = $sessionUtility->createClientStream($GLOBALS['sess_server'], $port, $sessionStreamContext, $strError)) !== false))
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
          echo 'sessionDestroyAll() error:  '.json_encode($response['Error']);
        }
        else
        {
          echo 'sessionDestroyAll() error:  Encountered an unknown error.';
        }
        unset($response);
      }
      else
      {
        echo 'sessionDestroyAll()->fgets() error:  Failed to read response.';
      }
    }
    else
    {
      echo 'sessionDestroyAll()->fwrite() error:  Failed to write request.';
    }
    fclose($handle);
  }
  else
  {
    if ($bSessionUseStream)
    {
      echo 'sessionDestroyAll()->createClientStream() error:  Failed to open stream connection to '.$GLOBALS['sess_server'].'(port '.$port.').';
    }
    else
    {
      echo 'sessionDestroyAll()->fsockopen() error:  Failed to open socket connection to '.$GLOBALS['sess_server'].'(port '.$port.').';
    }
  }
  unset($port);
  unset($request);
  if ($bSessionUseStream)
  {
    unset($sessionUtility);
  }

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

if (session_set_save_handler('sessionOpen', 'sessionClose', 'sessionReadData', 'sessionWriteData', 'sessionDestroyAll', 'sessionGC') === false)
{
  echo 'session_set_save_hander() error:  Failed to set session handler.<br>';
}
?>
