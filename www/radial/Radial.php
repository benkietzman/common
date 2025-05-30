<?php
namespace common;
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-01-27
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
class Radial
{
  // {{{ variables
  protected $m_buffer;
  protected $m_conf;
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
    $this->m_buffer = [[], []];
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
    $this->disconnect();
    unset($this->m_buffer);
    unset($this->m_strBuffer);
  }
  // }}}
  // {{{ central
  // {{{ centralApplicationNotify()
  public function centralApplicationNotify($strApplication, $strMessage)
  {
    return centralNotify('application', $strApplication, $strMessage);
  }
  // }}}
  // {{{ centralGroupNotify()
  public function centralGroupNotify($strGroup, $strMessage)
  {
    return centralNotify('group', $strGroup, $strMessage);
  }
  // }}}
  // {{{ centralNotify()
  public function centralNotify($strType, $strName, $strMessage)
  {
    $bResult = false;

    $request = [];
    $request['Interface'] = 'central';
    $request['Function'] = $strType.'Notify';
    $request['Request'] = [];
    $request['Request'][(($strType == 'user')?'userid':'name')] = $strName;
    $request['Request']['notification'] = $strMessage;
    $response = null;
    if ($this->request($request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ centralServerNotify()
  public function centralServerNotify($strServer, $strMessage)
  {
    return centralNotify('server', $strServer, $strMessage);
  }
  // }}}
  // {{{ centralUserNotify()
  public function centralUserNotify($strUser, $strMessage)
  {
    return centralNotify('user', $strUser, $strMessage);
  }
  // }}}
  // }}}
  // {{{ connect()
  public function connect()
  {
    $bResult = false;

    if ($this->m_handle === false)
    {
      if ($this->m_strUser != '')
      {
        if ($this->m_strPassword != '')
        {
          $strToken = null;
          $req = ['Interface'=>'application', 'Function'=>'connect'];
          $res = null;
          if ($this->request($req, $res))
          {
            if (isset($res['Response']))
            {
              if (isset($res['Response']['Token']) && $res['Response']['Token'] != '')
              {
                $strToken = $res['Response']['Token'];
              }
              else
              {
                $this->setError('Failed to find the Token within the Response');
              }
            }
            else
            {
              $this->setError('Failed to find the Response.');
            }
          }
          unset($req);
          unset($res);
          if ($strToken != '')
          {
            $servers = [];
            $this->m_strBuffer[1] .= $strToken."\n";
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
            if (isset($this->m_conf['Bridge Server']) && $this->m_conf['Bridge Server'] != '')
            {
              $server = explode(',', $this->m_conf['Bridge Server']);
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
              $port = 7277;
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
                if ($handle = stream_socket_client('ssl://'.$strServer.':'.$port, $nErrorNo, $strError, 10, STREAM_CLIENT_CONNECT, $this->m_streamContext))
                {
                  $bConnected = true;
                }
              }
              if ($bConnected)
              {
                $bExit = false;
                $messages = [];
                $CTime = [];
                $this->m_handle = $handle;
                $CTime[0] = $CTime[1] = time();
                while (!$bExit && ($CTime[1] - $CTime[0]) < 5 && $this->getMessages($messages))
                {
                  if (sizeof($messages) > 0)
                  {
                    $bExit = true;
                    if (isset($messages[0]['Status']) && $messages[0]['Status'] == 'okay')
                    {
                      $bResult = true;
                    }
                    else if (isset($messages[0]['Error']) && $messages[0]['Error'] != '')
                    {
                      $this->setError($messages[0]['Error']);
                    }
                    else
                    {
                      $this->setError('Encountered an unkown error.');
                    }
                  }
                  else
                  {
                    $CTime[1] = time();
                  }
                }
                unset($messages);
              }
              else
              {
                $this->setError('stream_socket_client() '.$strError);
              }
            }
            else
            {
              $this->setError('Please provide the Load Balancer server via the '.$this->m_strConf.' file.');
            }
            unset($servers);
          }
        }
        else
        {
          $this->setError('Please provide the Passworrd.');
        }
      }
      else
      {
        $this->setError('Please provide the User.');
      }
    }
    else
    {
      $this->setError('Already connected.');
    }

    return $bResult;
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
      $bResult = true;
      if (isset($response['Response']) && is_array($response['Response']))
      {
        $result = $response['Response'];
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
      $bResult = true;
      if (isset($response['ID']))
      {
        $nID = $response['ID'];
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
  // {{{ disconnect()
  public function disconnect()
  {
    $bResult = false;

    if ($this->m_handle !== false)
    {
      fclose($this->m_handle);
      $this->m_handle = false;
      $bResult = true;
    }
    $this->m_strBuffer[1] = null;
    if ($this->m_strBuffers[1] != '' && $this->m_strLine != '')
    {
      $this->m_buffers[1][] = $this->m_strLine;
    }
    $this->m_strBuffer[1] = '';
    $this->m_strLine = '';

    return $bResult;
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
  // {{{ getMessages()
  public function getMessages(&$messages)
  {
    $bResult = false;

    if ($this->getMessagesInternal())
    {
      $bResult = true;
      while (sizeof($this->m_buffer[0]) > 0)
      {
        $messages[] = json_decode(array_shift($this->m_buffer[0]), true);
      }
    }

    return $bResult;
  }
  // }}}
  // {{{ getMessagesInternal()
  protected function getMessagesInternal()
  {
    $bResult = false;

    if ($this->m_handle !== false || $this->connect())
    {
      if (sizeof($this->m_buffer[0]) > 0)
      {
        $bResult = true;
      }
      else
      {
        $bClose = false;
        $bExit = false;
        while (!$bExit)
        {
          $readfds = [$this->m_handle];
          $writefds = [];
          if ($this->m_strBuffer[1] == '' && sizeof($this->m_buffer[1]) > 0)
          {
            $this->m_strBuffer[1] .= $this->m_buffer[1][0]."\n";
            $this->m_strLine = array_shift($this->m_buffer[1]);
          }
          if ($this->m_strBuffer[1] != '')
          {
            $writefds[] = $this->m_handle;
          }
          $errorfds = null;
          if (($nReturn = stream_select($readfds, $writefds, $errorfds, 0, 10000)) > 0)
          {
            if (in_array($this->m_handle, $readfds))
            {
              if (($strBuffer = fread($this->m_handle, 65536)) !== false)
              {
                $this->m_strBuffer[0] .= $strBuffer;
                while (($unPosition = strpos($this->m_strBuffer[0], "\n")) !== false)
                {
                  $bResult = true;
                  $this->m_buffer[0][] = substr($this->m_strBuffer[0], 0, $unPosition);
                  $this->m_strBuffer[0] = substr($this->m_strBuffer[0], ($unPosition + 1), (strlen($this->m_strBuffer[0]) - ($unPosition + 1)));
                }
              }
              else
              {
                $bClose = $bExit = true;
                $strError = 'fread() Failed to read.';
                $this->setError($strError);
              }
            }
            if (in_array($this->m_handle, $writefds))
            {
              if (($nReturn = fwrite($this->m_handle, $this->m_strBuffer[1])) !== false)
              {
                $this->m_strBuffer[1] = substr($this->m_strBuffer[1], $nReturn, (strlen($this->m_strBuffer[1]) - $nReturn));
              }
              else
              {
                $bClose = $bExit = true;
                $strError = 'fwrite() Failed to write.';
                $this->setError($strError);
              }
            }
          }
          else if ($nReturn === false)
          {
            $bClose = $bExit = true;
            $strError = 'stream_select() Failed to select.';
            $this->setError($strError);
          }
          else
          {
            $bExit = $bResult = true;
          }
          unset($readfds);
          unset($writefds);
        }
        if ($bClose)
        {
          $this->disconnect();
        }
      }
    }

    return $bResult;
  }
  // }}}
  // {{{ ircChat()
  public function ircChat($strTarget, $strMessage)
  {
    $bResult = false;

    $request = [];
    $request['Interface'] = 'irc';
    $request['Function'] = 'chat';
    $request['Target'] = $strTarget;
    $request['Message'] = $strMessage;
    $response = null;
    if ($this->request($request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ putMessages()
  public function putMessages(&$messages)
  {
    $bResult = true;

    while (sizeof($messages) > 0)
    {
      $this->m_buffer[1][] = json_encode(array_shift($messages));
    }

    return $bResult;
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
        if (($handle = stream_socket_client('ssl://'.$strServer.':'.$port, $nErrorNo, $strError, 10, STREAM_CLIENT_CONNECT, $this->m_streamContext)) !== false)
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
                    else if (isset($response['Error']) && $response['Error'] != '')
                    {
                      $this->setError($response['Error']);
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
  // {{{ terminal
  // {{{ terminalConnect()
  public function terminalConnect(&$t, $strServer = null, $strPort = null)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'connect';
    $request['Request'] = [];
    $request['Request']['Server'] = $strServer;
    $request['Request']['Port'] = $strPort;
    $response = null;
    if ($this->terminalRequest($t, $request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ terminalCtrl()
  public function terminalCtrl(&$t, $cKey, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'ctrl';
    $request['Request'] = [];
    $request['Request']['Data'] = $cKey;
    if ($bWait)
    {
      $request['Request']['Wait'] = true;
    }
    $response = null;
    if ($this->terminalRequest($t, $request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ terminalDisconnect()
  public function terminalDisconnect(&$t)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'disconnect';
    $response = null;
    if ($this->terminalRequest($t, $request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ terminalDown()
  public function terminalDown(&$t, $unCount = 1, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'down';
    $request['Request'] = [];
    $request['Request']['Count'] = $unCount;
    if ($bWait)
    {
      $request['Request']['Wait'] = true;
    }
    $response = null;
    if ($this->terminalRequest($t, $request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ terminalEnter()
  public function terminalEnter(&$t, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'enter';
    if ($bWait)
    {
      $request['Request'] = [];
      $request['Request']['Wait'] = true;
    }
    $response = null;
    if ($this->terminalRequest($t, $request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ terminalFind()
  public function terminalFind(&$t, $strData)
  {
    $unRow = null;
    $unCol = null;

    return $this->terminalFindPos($t, $strData, $unRow, $unCol);

  }
  // }}}
  // {{{ terminalFindPos()
  public function terminalFindPos($t, $strData, &$unRow, &$unCol)
  {
    $bResult = false;

    if (isset($t['Screen']) && is_array($t['Screen']))
    {
      $unSize = sizeof($t['Screen']);
      for ($i = 0; !$bResult && $i < $unSize; $i++)
      {
        if (($unPosition = strpos($t['Screen'][$i], $strData)) !== false)
        {
          $bResult = true;
          $unRow = $i;
          $unCol = $unPosition;
        }
      }
    }

    return $bResult;
  }
  // }}}
  // {{{ terminalFunction()
  public function terminalFunction(&$t, $nKey)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'function';
    $request['Request'] = [];
    $request['Request']['Data'] = $nKey;
    $response = null;
    if ($this->terminalRequest($t, $request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ terminalGetSocketTimeout()
  public function terminalGetSocketTimeout(&$t, &$nShort, &$nLong)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'getSocketTimeout';
    $request['Request'] = [];
    $response = null;
    if ($this->terminalRequest($t, $request, $response))
    {
      $bResult = true;
      if (isset($response['Response']) && is_array($response['Response']))
      {
        $nShort = $response['Response']['Short'];
        $nLong = $response['Response']['Long'];
      }
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ terminalKeypadEnter()
  public function terminalKeypadEnter(&$t, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'keypadEnter';
    if ($bWait)
    {
      $request['Request'] = [];
      $request['Request']['Wait'] = true;
    }
    $response = null;
    if ($this->terminalRequest($t, $request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ terminalHome()
  public function terminalHome(&$t, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'home';
    $request['Request'] = [];
    if ($bWait)
    {
      $request['Request']['Wait'] = true;
    }
    $response = null;
    if ($this->terminalRequest($t, $request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ terminalKey()
  public function terminalKey(&$t, $cKey, $unCount = 1, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'key';
    $request['Request'] = [];
    $request['Request']['Data'] = $cKey;
    $request['Request']['Count'] = $unCount;
    if ($bWait)
    {
      $request['Request']['Wait'] = true;
    }
    $response = null;
    if ($this->terminalRequest($t, $request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ terminalLeft()
  public function terminalLeft(&$t, $unCount = 1, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'left';
    $request['Request'] = [];
    $request['Request']['Count'] = $unCount;
    if ($bWait)
    {
      $request['Request']['Wait'] = true;
    }
    $response = null;
    if ($this->terminalRequest($t, $request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ terminalRequest()
  public function terminalRequest(&$t, $request, &$response)
  {
    $bResult = false;

    $request['Interface'] = 'terminal';
    if (!is_array($t))
    {
      $t = ['Session'=>null, 'Screen'=>[], 'Col'=>null, 'Cols'=>null, 'Row'=>null, 'Rows'=>null];
    }
    if (isset($t['Session']) && $t['Session'] != '')
    {
      if (!isset($request['Request']))
      {
        $request['Request'] = [];
      }
      $request['Request']['Session'] = $t['Session'];
    }
    if ($this->request($request, $response))
    {
      $bResult = true;
    }
    if (isset($response['Response']) && is_array($response['Response']))
    {
      if (isset($response['Response']['Session']) && $response['Response']['Session'] != '')
      {
        $t['Session'] = $response['Response']['Session'];
      }
      else
      {
        $t['Session'] = null;
      }
      if (isset($response['Response']['Screen']) && is_array($response['Response']['Screen']))
      {
        $t['Screen'] = $response['Response']['Screen'];
      }
      if (!is_array($t['Screen']))
      {
        $t['Screen'] = [];
      }
      if (isset($response['Response']['Col']) && $response['Response']['Col'] != '')
      {
        $t['Col'] = $response['Response']['Col'];
      }
      if (isset($response['Response']['Cols']) && $response['Response']['Cols'] != '')
      {
        $t['Cols'] = $response['Response']['Cols'];
      }
      if (isset($response['Response']['Row']) && $response['Response']['Row'] != '')
      {
        $t['Row'] = $response['Response']['Row'];
      }
      if (isset($response['Response']['Rows']) && $response['Response']['Rows'] != '')
      {
        $t['Rows'] = $response['Response']['Rows'];
      }
    }

    return $bResult;
  }
  // }}}
  // {{{ terminalRight()
  public function terminalRight(&$t, $unCount = 1, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'right';
    $request['Request'] = [];
    $request['Request']['Count'] = $unCount;
    if ($bWait)
    {
      $request['Request']['Wait'] = true;
    }
    $response = null;
    if ($this->terminalRequest($t, $request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ terminalSend()
  public function terminalSend(&$t, $strData, $unCount = 1, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'send';
    $request['Request'] = [];
    $request['Request']['Data'] = $strData;
    $request['Request']['Count'] = $unCount;
    $request['Request']['Wait'] = $bWait;
    $response = null;
    if ($this->terminalRequest($t, $request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ terminalSetSocketTimeout()
  public function terminalSetSocketTimeout(&$t, $nShort, $nLong)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'setSocketTimeout';
    $request['Request'] = [];
    $request['Request']['Short'] = $nShort;
    $request['Request']['Long'] = $nLong;
    $response = null;
    if ($this->terminalRequest($t, $request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ terminalShiftFunction()
  public function terminalShiftFunction(&$t, $nKey)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'shiftFunction';
    $request['Request'] = [];
    $request['Request']['Data'] = $nKey;
    $response = null;
    if ($this->terminalRequest($t, $request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ terminalTab()
  public function terminalTab(&$t, $unCount = 1, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'tab';
    $request['Request'] = [];
    $request['Request']['Count'] = $unCount;
    if ($bWait)
    {
      $request['Request']['Wait'] = true;
    }
    $response = null;
    if ($this->terminalRequest($t, $request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ terminalUp()
  public function terminalUp(&$t, $unCount = 1, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'up';
    $request['Request'] = [];
    $request['Request']['Count'] = $unCount;
    if ($bWait)
    {
      $request['Request']['Wait'] = true;
    }
    $response = null;
    if ($this->terminalRequest($t, $request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ terminalWait()
  public function terminalWait(&$t, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'wait';
    $request['Request'] = [];
    if ($bWait)
    {
      $request['Request']['Wait'] = true;
    }
    $response = null;
    if ($this->terminalRequest($t, $request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // }}}
}
?>
