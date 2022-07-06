<?php
namespace common;
// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2020-03-17
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
include_once('Bridge.php');
class Terminal extends Bridge
{
  // {{{ variables
  protected $m_screen;
  // }}}
  // {{{ __construct()
  public function __construct($strConf = null)
  {
    parent::__construct($strConf);
    $this->m_screen = [];
  }
  // }}}
  // {{{ __destruct()
  public function __destruct()
  {
    unset($this->m_screen);
    parent::__destruct();
  }
  // }}}
  // {{{ connect()
  public function connect($strSystem = null, $strSubSystem = null, $strServer = null, $strPort = null, $strUser = null, $strPassword = null, $bLogin = false)
  {
    $bResult = false;

    if ($this->m_handle !== false || parent::connect())
    {
      $request = [];
      $request['Section'] = 'terminal';
      $request['Function'] = 'connect';
      if ($this->m_strUser != '')
      {
        $request['User'] = $this->m_strUser;
      }
      if ($this->m_strPassword != '')
      {
        $request['Password'] = $this->m_strPassword;
      }
      if ($this->m_strUserID != '')
      {
        $request['UserID'] = $this->m_strUserID;
      }
      if ($strSystem != '')
      {
        $request['System'] = $strSystem;
      }
      $request['Request'] = [];
      $request['Request']['Screen'] = true;
      if ($strSubSystem != '')
      {
        $request['Request']['System'] = $strSubSystem;
      }
      if ($strServer != '')
      {
        $request['Request']['Server'] = $strServer;
      }
      if ($strPort != '')
      {
        $request['Request']['Port'] = $strPort;
      }
      if ($strUser != '')
      {
        $request['Request']['User'] = $strUser;
      }
      if ($strPassword != '')
      {
        $request['Request']['Password'] = $strPassword;
      }
      if ($bLogin)
      {
        $request['Request']['Login'] = true;
      }
      $response = null;
      if ($this->request($request, $response))
      {
        $bResult = true;
      }
      else
      {
        parent::disconnect();
      }
      unset($request);
      unset($response);
    }

    return $bResult;
  }
  // }}}
  // {{{ disconnect()
  public function disconnect()
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'disconnect';
    $request['Request'] = [];
    $response = null;
    if ($this->request($request, $response))
    {
      $bResult = true;
    }
    unset($request);
    unset($response);
    parent::disconnect();

    return $bResult;
  }
  // }}}
  // {{{ find()
  public function find($strData)
  {
    $bResult = false;
    $unRow = null;
    $unCol = null;

    if ($this->findPos($strData, $unRow, $unCol))
    {
      $bResult = true;
    }

    return $bResult;
  }
  // }}}
  // {{{ findPos()
  public function findPos($strData, &$unRow, &$unCol)
  {
    $bResult = false;

    $unSize = sizeof($this->m_screen);
    for ($i = 0; !$bResult && $i < $unSize; $i++)
    {
      if (($unPosition = strpos($this->m_screen[$i], $strData)) !== false)
      {
        $bResult = true;
        $unRow = $i;
        $unCol = $unPosition;
      }
    }

    return $bResult;
  }
  // }}}
  // {{{ getSocketTimeout()
  public function getSocketTimeout(&$nShort, &$nLong)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'getSocketTimeout';
    $request['Request'] = [];
    $response = null;
    if ($this->request($request, $response))
    {
      $bResult = true;
      $nShort = $response['Response']['Short'];
      $nLong = $response['Response']['Long'];
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ request()
  public function request($request, &$response)
  {
    $bResult = false;

    if ($this->m_handle !== false)
    {
      $bClose = $bExit = false;
      $strBuffer = array(null, json_encode($request)."\n");
      while (!$bExit)
      {
        $readfds = array($this->m_handle);
        $writefds = array();
        if ($strBuffer[1] != '')
        {
          $writefds[] = $this->m_handle;
        }
        $errorfds = null;
        if (($nReturn = stream_select($readfds, $writefds, $errorfds, 0, 250000)) > 0)
        {
          if (in_array($this->m_handle, $writefds))
          {
            if (($nReturn = fwrite($this->m_handle, $strBuffer[1])) !== false)
            {
              $strBuffer[1] = substr($strBuffer[1], $nReturn, (strlen($strBuffer[1]) - $nReturn));
            }
            else
            {
              $bClose = $bExit = true;
              $this->setError('fwrite() Failed to write.');
            }
          }
          if (in_array($this->m_handle, $readfds))
          {
            if (($strData = fread($this->m_handle, 65536)) !== false)
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
                    if (is_array($response['Error']))
                    {
                      if (isset($response['Error']['Message']) && $response['Error']['Message'] != '')
                      {
                        $this->setError($response['Error']['Message']);
                      }
                      else
                      {
                        $this->setError('Encountered an unknown error.');
                      }
                    }
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
                  if (isset($response['Response']) && is_array($response['Response']) && isset($response['Response']['Screen']) && is_array($response['Response']['Screen']))
                  {
                    $this->m_screen = $response['Response']['Screen'];
                  }
                }
              }
              else
              {
                $bClose = $bExit = true;
              }
            }
            else
            {
              $bClose = $bExit = true;
              $this->setError('fread() Failed to read.');
            }
          }
        }
        else if ($nReturn === false)
        {
          $bClose = $bExit = true;
          $this->setError('stream_select() Failed to select.');
        }
        unset($read);
        unset($write);
      }
      if ($bClose)
      {
        parent::disconnect();
      }
    }
    else
    {
      $this->setError('Please establish the connection.');
    }

    return $bResult;
  }
  // }}}
  // {{{ screen()
  public function screen()
  {
    return $this->m_screen;
  }
  // }}}
  // {{{ send()
  public function send($strData, $unCount = 1)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'send';
    $request['Request'] = [];
    $request['Request']['Screen'] = true;
    $request['Request']['Data'] = $strData;
    $request['Request']['Count'] = $unCount;
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
  // {{{ sendCtrl()
  public function sendCtrl($cKey, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'sendCtrl';
    $request['Request'] = [];
    $request['Request']['Screen'] = true;
    $request['Request']['Data'] = $cKey;
    if ($bWait)
    {
      $request['Request']['Wait'] = true;
    }
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
  // {{{ sendDown()
  public function sendDown($unCount = 1, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'sendDown';
    $request['Request'] = [];
    $request['Request']['Screen'] = true;
    $request['Request']['Count'] = $unCount;
    if ($bWait)
    {
      $request['Request']['Wait'] = true;
    }
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
  // {{{ sendFunction()
  public function sendFunction($nKey)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'sendFunction';
    $request['Request'] = [];
    $request['Request']['Screen'] = true;
    $request['Request']['Data'] = $nKey;
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
  // {{{ sendShiftFunction()
  public function sendShiftFunction($nKey)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'sendShiftFunction';
    $request['Request'] = [];
    $request['Request']['Screen'] = true;
    $request['Request']['Data'] = $nKey;
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
  // {{{ sendHome()
  public function sendHome($bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'sendHome';
    $request['Request'] = [];
    $request['Request']['Screen'] = true;
    if ($bWait)
    {
      $request['Request']['Wait'] = true;
    }
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
  // {{{ sendKey()
  public function sendKey($cKey, $unCount = 1, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'sendKey';
    $request['Request'] = [];
    $request['Request']['Screen'] = true;
    $request['Request']['Data'] = $cKey;
    $request['Request']['Count'] = $unCount;
    if ($bWait)
    {
      $request['Request']['Wait'] = true;
    }
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
  // {{{ sendLeft()
  public function sendLeft($unCount = 1, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'sendLeft';
    $request['Request'] = [];
    $request['Request']['Screen'] = true;
    $request['Request']['Count'] = $unCount;
    if ($bWait)
    {
      $request['Request']['Wait'] = true;
    }
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
  // {{{ sendRight()
  public function sendRight($unCount = 1, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'sendRight';
    $request['Request'] = [];
    $request['Request']['Screen'] = true;
    $request['Request']['Count'] = $unCount;
    if ($bWait)
    {
      $request['Request']['Wait'] = true;
    }
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
  // {{{ sendTab()
  public function sendTab($unCount = 1, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'sendTab';
    $request['Request'] = [];
    $request['Request']['Screen'] = true;
    $request['Request']['Count'] = $unCount;
    if ($bWait)
    {
      $request['Request']['Wait'] = true;
    }
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
  // {{{ sendUp()
  public function sendUp($unCount = 1, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'sendUp';
    $request['Request'] = [];
    $request['Request']['Screen'] = true;
    $request['Request']['Count'] = $unCount;
    if ($bWait)
    {
      $request['Request']['Wait'] = true;
    }
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
  // {{{ sendWait()
  public function sendWait($strData, $unCount = 1)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'sendWait';
    $request['Request'] = [];
    $request['Request']['Screen'] = true;
    $request['Request']['Data'] = $strData;
    $request['Request']['Count'] = $unCount;
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
  // {{{ setSocketTimeout()
  public function setSocketTimeout($nShort, $nLong)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'setSocketTimeout';
    $request['Request'] = [];
    $request['Request']['Short'] = $nShort;
    $request['Request']['Long'] = $nLong;
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
  // {{{ wait()
  public function wait($bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'wait';
    $request['Request'] = [];
    $request['Request']['Screen'] = true;
    if ($bWait)
    {
      $request['Request']['Wait'] = true;
    }
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
}
?>
