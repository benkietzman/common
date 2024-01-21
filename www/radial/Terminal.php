<?php
namespace common;
// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2024-01-19
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
include_once('Radial.php');
class Terminal extends Radial
{
  // {{{ variables
  protected $m_screen;
  protected $m_strSession;
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
  public function connect($strServer = null, $strPort = null)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'connect';
    $request['Request'] = [];
    $request['Request']['Server'] = $strServer;
    $request['Request']['Port'] = $strPort;
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
  // {{{ disconnect()
  public function disconnect()
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'disconnect';
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
  // {{{ request()
  public function request($request, &$response)
  {
    $bResult = false;

    $request['Interface'] = 'terminal';
    if ($this->m_strSession != '')
    {
      if (!isset($request['Request']))
      {
        $request['Request'] = [];
      }
      $request['Request']['Session'] = $this->m_strSession;
    }
    if (parent::request($request, $response))
    {
      $bResult = true;
      if (isset($response['Response']) && is_array($response['Response']))
      {
        if (isset($response['Response']['Session']) && $response['Response']['Session'] != '')
        {
          $this->m_strSession = $response['Response']['Session'];
        }
        else
        {
          $this->m_strSession = null;
        }
        if (isset($response['Response']['Screen']) && is_array($response['Response']['Screen']))
        {
          $this->m_screen = $response['Response']['Screen'];
        }
      }
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
  public function send($strData, $unCount = 1, $bWait = true)
  {
    $bResult = false;

    $request = [];
    $request['Function'] = 'send';
    $request['Request'] = [];
    $request['Request']['Data'] = $strData;
    $request['Request']['Count'] = $unCount;
    $request['Request']['Wait'] = $bWait;
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
    $request['Function'] = 'ctrl';
    $request['Request'] = [];
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
    $request['Function'] = 'down';
    $request['Request'] = [];
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
  // {{{ sendEnter()
  public function sendEnter($bWait = true)
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
    $request['Function'] = 'function';
    $request['Request'] = [];
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
  // {{{ sendKeypadEnter()
  public function sendKeypadEnter($bWait = true)
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
    $request['Function'] = 'shiftFunction';
    $request['Request'] = [];
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
    $request['Function'] = 'home';
    $request['Request'] = [];
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
    $request['Function'] = 'key';
    $request['Request'] = [];
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
    $request['Function'] = 'left';
    $request['Request'] = [];
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
    $request['Function'] = 'right';
    $request['Request'] = [];
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
    $request['Function'] = 'tab';
    $request['Request'] = [];
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
    $request['Function'] = 'up';
    $request['Request'] = [];
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
