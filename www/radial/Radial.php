<?php
namespace common;
// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-01-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
class Radial
{
  // {{{ variables
  protected $m_bAuthenticated;
  protected $m_buffer;
  protected $m_conf;
  protected $m_CTime;
  protected $m_bFailed;
  protected $m_handle;
  protected $m_streamContext;
  protected $m_strBuffer;
  protected $m_strConf;
  protected $m_strError;
  protected $m_strLine;
  protected $m_strPassword;
  protected $m_strUser;
  protected $m_strUserID;
  protected $m_ulModifyTime;
  // }}}
  // {{{ __construct()
  public function __construct($strConf = null)
  {
    $this->m_bAuthenticated = false;
    $this->m_bFailed = false;
    $this->m_bRegistered = false;
    $this->m_buffer = [[], []];
    $this->m_CTime = [0, 0];
    $this->m_handle = false;
    $this->m_strBuffer = [null, null];
    $this->m_strConf = (($strConf != '')?$strConf:'/etc/central.conf');
    $context = array();
    $context['ssl'] = array();
    $context['ssl']['allow_self_signed'] = true;
    $context['ssl']['verify_peer'] = false;
    $context['ssl']['verify_peer_name'] = false;
    $this->m_streamContext = stream_context_create($context);
    unset($context);
    $this->m_ulModifyTime = 0;
    if ($strConf != '')
    {
      $this->m_strConf = $strConf;
    }
  }
  // }}}
  // {{{ __destruct()
  public function __destruct()
  {
    if ($this->m_bRegistered)
    {
      $this->unregister();
    }
    if ($this->m_handle !== false)
    {
      $this->disconnect();
    }
    unset($this->m_buffer);
    unset($this->m_strBuffer);
  }
  // }}}
  // {{{ databaseQuery()
  public function databaseQuery($strDatabase, $strQuery, &$result)
  {
    $bResult = false;

    $request = [];
    $request['Interface'] = 'database';
    $request['Database'] = $strDatabase;
    $request['Query'] = $strQuery;
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
  // {{{ databaseUpdate()
  public function databaseUpdate($strDatabase, $strUpdate)
  {
    $nID = null;

    return $this->databaseUpdateWithID($strDatabase, $strUpdate, $nID);
  }
  // }}}
  // {{{ databaseUpdateWithID()
  public function databaseUpdateWithID($strDatabase, $strUpdate, &$nID)
  {
    $bResult = false;

    $request = [];
    $request['Interface'] = 'database';
    $request['Database'] = $strDatabase;
    $request['Update'] = $strQuery;
    $response = null;
    if ($this->request($request, $response))
    {
      if (isset($response['Status']) && $response['Status'] == 'okay')
      {
        $bResult = true;
        if (isset($response['ID']))
        {
          $nID = $response['ID'];
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
  // {{{ dbQuery()
  public function dbQuery($strDatabase, $strQuery, &$result)
  {
    return $this->databaseQuery($strDatabase, $strQuery, $result);
  }
  // }}}
  // {{{ dbUpdate()
  public function dbUpdate($strDatabase, $strUpdate)
  {
    return $this->databaseUpdate($strDatabase, $strUpdate);
  }
  // }}}
  // {{{ dbUpdateWithID()
  public function dbUpdateWithID($strDatabase, $strUpdate, &$nID)
  {
    return $this->databaseUpdateWithID($strDatabase, $strUpdate, $nID);
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
    $servers = [];

    if ($this->m_strUser != '' && (!isset($request['User']) || $request['User'] == ''))
    {
      $request['User'] = $this->m_strUser;
    }
    if ($this->m_strPassword != '' && (!isset($request['Password']) || $request['Password'] == ''))
    {
      $request['Password'] = $this->m_strPassword;
    }
    if ($this->m_strUserID != '' && (!isset($request['UserID']) || $request['UserID'] == ''))
    {
      $request['UserID'] = $this->m_strUserID;
    }
    $this->readConf();
    if (isset($this->m_conf['Load Balancer']) && $this->m_conf['Load Balancer'] != '')
    {
      $server = explode(',', $this->m_conf['Load Balancer']);
      while (sizeof($server) > 0)
      {
        $servers[] = array_shift($server);
      }
      unset($server);
    }
    if (isset($this->m_conf['Radial Server']) && $this->m_conf['Radial Server'] != '')
    {
      $server = explode(',', $this->m_conf['Radial Server']);
      while (sizeof($server) > 0)
      {
        $servers[] = array_shift($server);
      }
      unset($server);
    }
    if (($unSize = sizeof($servers)) > 0)
    {
      $bConnected = false;
      $handle = null;
      $nErrorNo = null;
      $port = 7234;
      $strError = null;
      $unAttempt = 0;
      $unPick = rand(0, ($unSize - 1));
      while (!$bConnected && $unAttempt++ < $unSize)
      {
        if ($unPick == $unSize)
        {
          $unPick = 0;
        }
        $strServer = $servers[$unPick];
        if ($handle = stream_socket_client('tls://'.$strServer.':'.$port, $nErrorNo, $strError, 10, STREAM_CLIENT_CONNECT, $this->m_streamContext))
        { 
          $bConnected = true;
        }
      }
      if ($bConnected)
      {
        $bExit = false;
        $strBuffer = array(null, json_encode($request)."\n");
        while (!$bExit)
        {
          $readfds = [$handle];
          $writefds = [];
          if ($strBuffer[1] != '')
          {
            $writefds[] = $handle;
          }
          $errorfds = null;
          if (($nReturn = stream_select($readfds, $writefds, $errorfds, 0, 250000)) > 0)
          {
            if (in_array($handle, $writefds))
            {
              if (($nReturn = fwrite($handle, $strBuffer[1])) !== false)
              {
                $strBuffer[1] = substr($strBuffer[1], $nReturn, (strlen($strBuffer[1]) - $nReturn));
              }
              else
              {
                $bExit = true;
                $strError = 'fwrite() Failed to write.';
                $this->setError($strError);
              }
            }
            if (in_array($handle, $readfds))
            {
              if (($strData = fread($handle, 65536)) !== false)
              {
                if ($strData != '')
                {
                  $strBuffer[0] .= $strData;
                  if (($unPosition = strpos($strBuffer[0], "\n")) !== false)
                  {
                    $bExit = true;
                    $response = json_decode(substr($strBuffer[0], 0, $unPosition), true);
                    $strBuffer[0] = substr($strBuffer[0], ($unPosition + 1), (strlen($strBuffer[0]) - ($unPosition + 1)));
                    if (isset($response['Status']) && $response['Status'] == 'okay')
                    {
                      $bResult = true;
                    }
                    else if (isset($response['Error']))
                    {
                      else if ($response['Error'] != '')
                      {
                        $this->setError($response['Error']);
                      }
                      else
                      {
                        $this->setError('Encountered an unknown error.');
                      }
                    }
                    else
                    {
                      $this->setError('Encountered an unknown error.');
                    }
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
                $strError = 'fread() Failed to read.';
                $this->setError($strError);
              }
            }
          }
          else if ($nReturn === false)
          {
            $bExit = true;
            $strError = 'stream_select() Failed to select.';
            $this->setError($strError);
          }
          unset($readfds);
          unset($writefds);
        }
        fclose($handle);
      }
      else
      {
        $this->setError('stream_socket_client('.$nErrorNo.') '.$strError);
      }
    }
    else
    {
      $this->setError('Please provide the Load Balancer server via the '.$this->m_strConf.' file.');
    }
    unset($servers);

    return $bResult;
  }
  // }}}
  // {{{ setCredentials()
  public function setCredentials($strUser, $strPassword, $strUserID = null)
  {
    $this->m_strUser = $strUser;
    $this->m_strPassword = $strPassword;
    $this->m_strUserID = $strUserID;
  }
  // }}}
  // {{{ setError()
  public function setError($strError)
  {
    $this->m_strError = $strError;
  }
  // }}}
  // {{{ setStreamContext()
  public function setStreamContext($context)
  {
    $this->m_streamContext = stream_context_create($context);
  }
  // }}}
}
?>
