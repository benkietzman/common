<?php
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-07-11
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
/*! \file radial.php
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
* \brief Read the session
* \param $strSessionID Contains the session id.
*/
function sessionRead($strSessionID)
{
  $strResult = '';

  $context = array();
  $context['ssl'] = array();
  $context['ssl']['allow_self_signed'] = true;
  $context['ssl']['verify_peer'] = false;
  $context['ssl']['verify_peer_name'] = false;
  $streamContext = stream_context_create($context);

  $handle = null;
  $nErrorNo = null;
  $strError = null;
  if ($handle = stream_socket_client('ssl://'.$GLOBALS['sess_server'].':7234', $nErrorNo, $strError, 10, STREAM_CLIENT_CONNECT, $streamContext))
  {
    $request = array();
    $request['Interface'] = 'session';
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
    echo 'sessionRead()->stream_socket_client() error:  Failed to open stream connection to '.$GLOBALS['sess_server'].'.';
  }
  unset($context);
  unset($request);
  
  return $strResult;
}
// }}}
// {{{ sessionWrite()
/*! \fn sessionWrite($strSessionID, $strSessionData)
* \brief Write the session
* \param $strSessionID Contains the session id.
* \param $strSessionData Contains the session data.
*/
function sessionWrite($strSessionID, $strSessionData)
{
  $bResult = false;

  $context = array();
  $context['ssl'] = array();
  $context['ssl']['verify_peer'] = false;
  $context['ssl']['verify_peer_name'] = false;
  $context['ssl']['allow_self_signed'] = true;
  $streamContext = stream_context_create($context);

  $handle = null;
  $nErrorNo = null;
  $strError = null;
  if ($handle = stream_socket_client('ssl://'.$GLOBALS['sess_server'].':7234', $nErrorNo, $strError, 10, STREAM_CLIENT_CONNECT, $streamContext))
  {
    $request = array();
    $request['Interface'] = 'session';
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
    echo 'sessionWrite()->stream_socket_client() error:  Failed to open stream connection to '.$GLOBALS['sess_server'].'.';
  }
  unset($context);
  unset($request);

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

  $context = array();
  $context['ssl'] = array();
  $context['ssl']['verify_peer'] = false;
  $context['ssl']['verify_peer_name'] = false;
  $context['ssl']['allow_self_signed'] = true;
  $streamContext = stream_context_create($context);

  $handle = null;
  $nErrorNo = null;
  $strError = null;
  if ($handle = stream_socket_client('ssl://'.$GLOBALS['sess_server'].':7234', $nErrorNo, $strError, 10, STREAM_CLIENT_CONNECT, $streamContext))
  {
    $request = array();
    $request['Interface'] = 'session';
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
    echo 'sessionDestroy()->stream_socket_client() error:  Failed to open stream connection to '.$GLOBALS['sess_server'].'.';
  }
  unset($context);
  unset($request);

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
