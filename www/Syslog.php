<?php
// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2020-06-04
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
class Syslog
{
  // {{{ variables
  protected $m_nFacility;
  protected $m_nOption;
  protected $m_strApplication;
  protected $m_strProgram;
  protected $m_strUser;
  // }}}
  // {{{ __construct()
  public function __construct($strApplication = null, $strProgram = null, $nOption = LOG_PID, $nFacility = LOG_USER)
  {
    $this->m_nFacility = $nFacility;
    $this->m_nOption = $nOption;
    $this->m_strApplication = $strApplication;
    $this->m_strProgram = $strProgram;
    $this->m_strUser = get_current_user();
    openlog($this->m_strProgram, $this->m_nOption, $this->m_nFacility);
  }
  // }}}
  // {{{ __destruct()
  public function __destruct()
  {
    closelog();
  }
  // }}}
  // {{{ commandEnded()
  public function commandEnded($strCommandName, $strEventDetails = null, $nPriority = LOG_INFO)
  {
    $message = array();
    $message['Event Type'] = 'Command Ended';
    $message['Command Name'] = $strCommandName;
    $message['Login ID'] = $this->m_strUser;
    $this->log($message, $strEventDetails, $nPriority);
    unset($message);
  }
  // }}}
  // {{{ commandLaunched()
  public function commandLaunched($strCommandName, $strEventDetails = null, $nPriority = LOG_INFO)
  {
    $message = array();
    $message['Event Type'] = 'Command Launched';
    $message['Command Name'] = $strCommandName;
    $message['Login ID'] = $this->m_strUser;
    $this->log($message, $strEventDetails, $nPriority);
    unset($message);
  }
  // }}}
  // {{{ connectionStarted()
  public function connectionStarted($strEventDetails = null, $fdSocket = false, $bResult = true, $nPriority = LOG_INFO)
  {
    $message = array();
    $message['Event Type'] = 'Connection Started';
    $message['Action'] = (($bResult)?'Allowed':'Blocked');
    if ($fdSocket !== false)
    {
      $strAddress = null;
      if (socket_getpeername($fdSocket, $strAddress) !== false)
      {
        $message['Source IP'] = $strAddress;
      }
    }
    $this->log($message, $strEventDetails, $nPriority);
    unset($message);
  }
  // }}}
  // {{{ connectionStopped()
  public function connectionStopped($strEventDetails = null, $fdSocket = false, $nPriority = LOG_INFO)
  {
    $message = array();
    $message['Event Type'] = 'Connection Stopped';
    $message['Action'] = 'Teardown';
    if ($fdSocket !== false)
    {
      $strAddress = null;
      if (socket_getpeername($fdSocket, $strAddress) !== false)
      {
        $message['Source IP'] = $strAddress;
      }
    }
    $this->log($message, $strEventDetails, $nPriority);
    unset($message);
  }
  // }}}
  // {{{ jsonEncode()
  public function jsonEncode($val)
	{
    if (is_string($val))
    {
      return '"'.addslashes($val).'"';
    }
    if (is_numeric($val))
    {
      return $val;
    }
    if ($val === null)
    {
      return 'null';
    }
    if ($val === true)
    {
      return 'true';
    }
    if ($val === false)
    {
      return 'false';
    }
    $assoc = false;
    $i = 0;
    foreach ($val as $k => $v)
    {
      if ($k !== $i++)
      {
        $assoc = true;
        break;
      }
    }
    $res = array();
    foreach ($val as $k => $v)
    {
      $v = $this->jsonEncode($v);
      if ($assoc)
      {
        $k = '"'.addslashes($k).'"';
        $v = $k.':'.$v;
      }
      $res[] = $v;
    }
    $res = implode(',', $res);

    return (($assoc)?'{'.$res.'}':'['.$res.']');
	}
  // }}}
  // {{{ groupCreated()
  public function groupCreated($strEventDetails = null, $strLoginID = null, $bResult = true, $strProtocol = null, $strService = null,  $nPriority = LOG_INFO)
  {
    $message = array();
    $message['Event Type'] = 'Group Created';
    $message['Login ID'] = (($strLoginID != '')?$strLoginID:'N/A');
    $message['Result'] = (($bResult)?'Success':'Failure');
    $message['Protocol'] = (($strProtocol != '')?$strProcotol:'N/A');
    $message['Service'] = (($strService != '')?$strService:'N/A');
    $this->log($message, $strEventDetails, $nPriority);
    unset($message);
  }
  // }}}
  // {{{ groupDeleted()
  public function groupDeleted($strEventDetails = null, $strLoginID = null, $bResult = true, $strProtocol = null, $strService = null,  $nPriority = LOG_INFO)
  {
    $message = array();
    $message['Event Type'] = 'Group Deleted';
    $message['Login ID'] = (($strLoginID != '')?$strLoginID:'N/A');
    $message['Result'] = (($bResult)?'Success':'Failure');
    $message['Protocol'] = (($strProtocol != '')?$strProcotol:'N/A');
    $message['Service'] = (($strService != '')?$strService:'N/A');
    $this->log($message, $strEventDetails, $nPriority);
    unset($message);
  }
  // }}}
  // {{{ groupModified()
  public function groupModified($strEventDetails = null, $strLoginID = null, $bResult = true, $strProtocol = null, $strService = null,  $nPriority = LOG_INFO)
  {
    $message = array();
    $message['Event Type'] = 'Group Modified';
    $message['Login ID'] = (($strLoginID != '')?$strLoginID:'N/A');
    $message['Result'] = (($bResult)?'Success':'Failure');
    $message['Protocol'] = (($strProtocol != '')?$strProcotol:'N/A');
    $message['Service'] = (($strService != '')?$strService:'N/A');
    $this->log($message, $strEventDetails, $nPriority);
    unset($message);
  }
  // }}}
  // {{{ log()
  public function log($message, $strEventDetails = null, $nPriority = LOG_INFO)
  {
    if (!is_array($message))
    {
      $message = array();
    }
    if ($this->m_strApplication != '')
    {
      $message['Application'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $message['Program'] = $this->m_strProgram;
    }
    if ($strEventDetails != '')
    {
      $message['Event Details'] = $strEventDetails;
    }
    $item = explode(' ', microtime());
    $nTime = $item[1];
    $nMillisecond = round($item[0] * 1000);
    unset($item);
    $message['DateTime'] = sprintf('%s.%03d UTC', gmdate('Y-m-d H:i:s', $nTime), $nMillisecond);
    $message['TimeStamp'] = $nTime;
    if(function_exists('json_encode')){
		  syslog($nPriority, json_encode($message));
    } else {
      syslog($nPriority, $this->jsonEncode($message));
	  }
  }
  // }}}
  // {{{ logoff()
  public function logoff($strEventDetails = null, $strLoginID = null, $bResult = true, $strProtocol = null, $strService = null,  $nPriority = LOG_INFO)
  {
    $message = array();
    $message['Event Type'] = 'Logoff';
    $message['Login ID'] = (($strLoginID != '')?$strLoginID:'N/A');
    $message['Result'] = (($bResult)?'Success':'Failure');
    $message['Protocol'] = (($strProtocol != '')?$strProcotol:'N/A');
    $message['Service'] = (($strService != '')?$strService:'N/A');
    $this->log($message, $strEventDetails, $nPriority);
    unset($message);
  }
  // }}}
  // {{{ logon()
  public function logon($strEventDetails = null, $strLoginID = null, $bResult = true, $strProtocol = null, $strService = null,  $nPriority = LOG_INFO)
  {
    $message = array();
    $message['Event Type'] = 'Logon';
    $message['Login ID'] = (($strLoginID != '')?$strLoginID:'N/A');
    $message['Result'] = (($bResult)?'Success':'Failure');
    $message['Protocol'] = (($strProtocol != '')?$strProcotol:'N/A');
    $message['Service'] = (($strService != '')?$strService:'N/A');
    $this->log($message, $strEventDetails, $nPriority);
    unset($message);
  }
  // }}}
  // {{{ privilegeLevelChanged()
  public function privilegeLevelChanged($strEventDetails = null, $strLoginID = null, $bResult = true, $strProtocol = null, $strService = null,  $nPriority = LOG_INFO)
  {
    $message = array();
    $message['Event Type'] = 'Privilege Level Changed';
    $message['Login ID'] = (($strLoginID != '')?$strLoginID:'N/A');
    $message['Result'] = (($bResult)?'Success':'Failure');
    $message['Protocol'] = (($strProtocol != '')?$strProcotol:'N/A');
    $message['Service'] = (($strService != '')?$strService:'N/A');
    $this->log($message, $strEventDetails, $nPriority);
    unset($message);
  }
  // }}}
  // {{{ userAccountCreated()
  public function userAccountCreated($strEventDetails = null, $strLoginID = null, $bResult = true, $strProtocol = null, $strService = null,  $nPriority = LOG_INFO)
  {
    $message = array();
    $message['Event Type'] = 'User Account Created';
    $message['Login ID'] = (($strLoginID != '')?$strLoginID:'N/A');
    $message['Result'] = (($bResult)?'Success':'Failure');
    $message['Protocol'] = (($strProtocol != '')?$strProcotol:'N/A');
    $message['Service'] = (($strService != '')?$strService:'N/A');
    $this->log($message, $strEventDetails, $nPriority);
    unset($message);
  }
  // }}}
  // {{{ userAccountDeleted()
  public function userAccountDeleted($strEventDetails = null, $strLoginID = null, $bResult = true, $strProtocol = null, $strService = null,  $nPriority = LOG_INFO)
  {
    $message = array();
    $message['Event Type'] = 'User Account Deleted';
    $message['Login ID'] = (($strLoginID != '')?$strLoginID:'N/A');
    $message['Result'] = (($bResult)?'Success':'Failure');
    $message['Protocol'] = (($strProtocol != '')?$strProcotol:'N/A');
    $message['Service'] = (($strService != '')?$strService:'N/A');
    $this->log($message, $strEventDetails, $nPriority);
    unset($message);
  }
  // }}}
  // {{{ userAccountModified()
  public function userAccountModified($strEventDetails = null, $strLoginID = null, $bResult = true, $strProtocol = null, $strService = null,  $nPriority = LOG_INFO)
  {
    $message = array();
    $message['Event Type'] = 'User Account Modified';
    $message['Login ID'] = (($strLoginID != '')?$strLoginID:'N/A');
    $message['Result'] = (($bResult)?'Success':'Failure');
    $message['Protocol'] = (($strProtocol != '')?$strProcotol:'N/A');
    $message['Service'] = (($strService != '')?$strService:'N/A');
    $this->log($message, $strEventDetails, $nPriority);
    unset($message);
  }
  // }}}
}
?>
