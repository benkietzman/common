<?php
namespace common;
// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2018-12-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
class Acorn
{
  // {{{ variables
  protected $m_conf;
  protected $m_strConf;
  protected $m_strError;
  protected $m_ulModifyTime;
  // }}}
  // {{{ __construct()
  public function __construct($strConf = null)
  {
    $this->m_ulModifyTime = 0;
    $this->m_strConf = '/etc/central.conf';
    if ($strConf != '')
    {
      $this->m_strConf = $strConf;
    }
  }
  // }}}
  // {{{ __destruct()
  public function __destruct()
  {
  }
  // }}}
  // {{{ getConf()
  public function getConf()
  {
    return $this->m_conf;
  }
  // }}}
  // {{{ getError()
  public function getError()
  {
    return $this->m_strError;
  }
  // }}}
  // {{{ mysqlQuery()
  public function mysqlQuery($strUser, $strPassword, $strServer, $strDatabase, $strQuery, &$result)
  {
    $bResult = false;

    $request = []; 
    $request['Acorn'] = 'mysql';
    $request['User'] = $strUser;
    $request['Password'] = $strPassword;
    $request['Server'] = $strServer;
    $request['Database'] = $strDatabase;
    $request['Request'] = [];
    $request['Request']['Query'] = $strQuery;
    $response = null;
    if ($this->request($request, $response))
    {
      if (isset($response['Status']) && $response['Status'] == 'okay')
      {
        $bResult = true;
        if (isset($response['Response']) && is_array($response['Response']))
        {
          $result = $response['Response'];
        }
      }
      else if (isset($response['Error']) && $response['Error'] != '')
      {
        $this->setError($response['Error']);
      }
      else
      {
        $this->setError('Encountered an unknown error.');
      }
    }

    return $bResult;
  }
  // }}}
  // {{{ mysqlUpdateWithID()
  public function mysqlUpdateWithID($strUser, $strPassword, $strServer, $strDatabase, $strUpdate, &$nID)
  {
    $bResult = false;

    $request = []; 
    $request['Acorn'] = 'mysql';
    $request['User'] = $strUser;
    $request['Password'] = $strPassword;
    $request['Server'] = $strServer;
    $request['Database'] = $strDatabase;
    $request['Request'] = [];
    $request['Request']['Update'] = $strQuery;
    $response = null;
    if ($this->request($request, $response))
    {
      if (isset($response['Status']) && $response['Status'] == 'okay')
      {
        $bResult = true;
        if (isset($response['Response']) && isset($response['Response']['ID']))
        {
          $nID = $response['Response']['ID'];
        }
      }
      else if (isset($response['Error']) && $response['Error'] != '')
      {
        $this->setError($response['Error']);
      }
      else
      {
        $this->setError('Encountered an unknown error.');
      }
    }

    return $bResult;
  }
  // }}}
  // {{{ mysqlUpdate()
  public function mysqlUpdate($strUser, $strPassword, $strServer, $strDatabase, $strUpdate)
  {
    $nID = null;

    return $this->mysqlUpdateWithID($strUser, $strPassword, $strServer, $strDatabase, $strUpdate, $nID);
  }
  // }}}
  // {{{ readConf()
  public function readConf()
  {
    $bResult = false;

    if (($ulModifyTime = filemtime($this->m_strConf)) != $this->m_ulModifyTime)
    {
      $this->m_ulModifyTime = $ulModifyTime;
      if (($handle = fopen($this->m_strConf, 'r')) != null)
      {
        $bResult = true;
        $this->m_conf = json_decode(fgets($handle), true);
        fclose($handle);
      }
    }

    return $bResult;
  }
  // }}}
  // {{{ request()
  public function request($request, &$response)
  {
    $bResult = false;

    $this->readConf();
    if (isset($this->m_conf['Load Balancer']) && $this->m_conf['Load Balancer'] != '')
    {
      if (!defined('AF_INET'))
      {
        define('AF_INET', 2);
      }
      if (!defined('SOCK_STREAM'))
      {
        if (PHP_OS == 'Linux')
        {
          define('SOCK_STREAM', 1);
        }
        else if (PHP_OS == 'SunOS')
        {
          define('SOCK_STREAM', 2);
        }
      }
      if (!defined('SOL_TCP'))
      {
        define('SOL_TCP', 6);
      }
      if (($fdSocket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP)) !== false)
      {
        if (socket_connect($fdSocket, $this->m_conf['Load Balancer'], 22676) !== false)
        {
          $bExit = false;
          $strBuffer = array(null, json_encode($request)."\n");
          while (!$bExit)
          {
            $readfds = array($fdSocket);
            $writefds = array();
            if ($strBuffer[1] != '')
            {
              $writefds[] = $fdSocket;
            }
            $errorfds = null;
            if (($nReturn = socket_select($readfds, $writefds, $errorfds, 2)) > 0)
            {
              if (in_array($fdSocket, $readfds))
              {
                if (($strData = socket_read($fdSocket, 1024)) !== false)
                {
                  if ($strData != '')
                  {
                    $strBuffer[0] .= $strData;
                    if (($unPosition = strpos($strBuffer[0], "\n")) !== false)
                    {
                      $bExit = $bResult = true;
                      $response = json_decode(substr($strBuffer[0], 0, $unPosition), true);
                      $strBuffer[0] = substr($strBuffer[0], ($unPosition + 1), (strlen($strBuffer[0]) - ($unPosition + 1)));
                    }
                  }
                  else
                  {
                    $bExit = true;
                  }
                }
                else
                {
                  $bExit = true;
                  $this->setError(socket_strerror(socket_last_error()));
                }
              }
              if (in_array($fdSocket, $writefds))
              {
                if (($nReturn = socket_write($fdSocket, $strBuffer[1])) !== false)
                {
                  $strBuffer[1] = substr($strBuffer[1], $nReturn, (strlen($strBuffer[1]) - $nReturn));
                }
                else
                {
                  $bExit = true;
                  $this->setError(socket_strerror(socket_last_error()));
                }
              }
            }
            else if ($nReturn === false)
            {
              $bExit = true;
              $this->setError(socket_strerror(socket_last_error()));
            }
            unset($read);
            unset($write);
          }
        }
        else
        {
          $this->setError(socket_strerror(socket_last_error()));
        }
        socket_close($fdSocket);
      }
      else
      {
        $this->setError(socket_strerror(socket_last_error()));
      }
    }
    else
    {
      $this->setError('Please provide the Server.');
    }

    return $bResult;
  }
  // }}}
  // {{{ setError()
  public function setError($strError)
  {
    $this->m_strError = $strError;
  }
  // }}}
}
?>
