<?php
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2013-11-12
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
class ServiceJunction
{
  // {{{ variables
  protected $m_bSemaphore;
  protected $m_bSemNonBlocking;
  protected $m_bUseSecureJunction;
  protected $m_conf;
  protected $m_nSemTimeout;
  protected $m_sem;
  protected $m_strApplication;
  protected $m_strConf;
  protected $m_streamContext;
  protected $m_strError;
  protected $m_strProgram;
  protected $m_strTimeout;
  protected $m_ulModifyTime;
  // }}}
  // {{{ __construct()
  public function __construct($strConf = null)
  {
    $this->m_bSemaphore = false;
    $this->m_bSemNonBlocking = false;
    $this->m_bUseSecureJunction = false;
    $this->m_nSemTimeout = 60;
    $this->m_ulModifyTime = 0;
    $this->m_strConf = '/etc/central.conf';
    if ($strConf != '')
    {
      $this->m_strConf = $strConf;
    }
    $context = array();
    $context['ssl'] = array();
    $context['ssl']['verify_peer'] = false;
    $this->m_streamContext = stream_context_create($context);
    unset($context);
  }
  // }}}
  // {{{ __destruct()
  public function __destruct()
  {
  }
  // }}}
  // {{{ aes()
  public function aes($strSecret, &$strDecrypted, &$strEncrypted)
  {
    $bDecrypt = true;
    $bResult = false;

    $request = array();
    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $request['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'aes';
    if ($strDecrypted != '')
    {
      $bDecrypt = false;
      $request['Function'] = 'encrypt';
    }
    else
    {
      $request['Function'] = 'decrypt';
    }
    $in = array();
    $in[] = json_encode($request);
    unset($request);
    $request = array();
    $request['Secret'] = $strSecret;
    $request['Payload'] = (($bDecrypt)?base64_encode($strEncrypted):$strDecrypted);
    $in[] = json_encode($request);
    unset($request);
    $out = null;
    if ($this->request($in, $out))
    {
      $nSize = sizeof($out);
      if ($nSize >= 1)
      {
        $status = json_decode($out[0], true);
        if (isset($status['Status']) && $status['Status'] == 'okay')
        {
          if ($nSize == 2)
          {
            $data = json_decode($out[1], true);
            if (isset($data['Payload']))
            {
              $bResult = true;
              if ($bDecrypt)
              {
                $strDecrypted = $data['Payload'];
              }
              else
              {
                $strEncrypted = base64_decode($data['Payload']);
              }
            }
            else
            {
              $this->setError('Failed to find Payload within data.');
            }
            unset($data);
          }
          else
          {
            $this->setError('Failed to receive data.');
          }
        }
        else if (isset($status['Error']) && $status['Error'] != '')
        {
          $this->setError($status['Error']);
        }
        else
        {
          $this->setError('Encountered an unknown error.');
        }
        unset($status);
      }
      else
      {
        $this->setError('Failed to receive response.');
      }
    }
    unset($in);
    unset($out);

    return $bResult;
  }
  // }}}
  // {{{ batch()
  public function batch($request, &$response)
  {
    $bResult = false;
    $strError = null;

    if ($this->semAcquire())
    {
      $bConnected = false;
      $handle = null;
      $nErrorNo = null;
      $response = array();
      $this->readConf();
      $junctionServer = explode(',', $this->m_conf['Load Balancer']);
      $unSize = sizeof($junctionServer);
      for ($i = (($this->m_bUseSecureJunction)?0:1); !$bConnected && $i < 2; $i++)
      {
        $unAttempt = 0;
        $unPick = rand(0, ($unSize - 1));
        while (!$bConnected && $unAttempt++ < $unSize)
        {
          if ($unPick == $unSize)
          {
            $unPick = 0;
          }
          $strServer = $junctionServer[$unPick];
          if ($handle = stream_socket_client(($i == 0)?'ssl://'.$strServer.':5863':$strServer.':5862', $nErrorNo, $strError, 10, STREAM_CLIENT_CONNECT, $this->m_streamContext))
          {
            $bConnected = true;
          }
        }
      }
      unset($junctionServer);
      if ($bConnected)
      {
        $bResult = true;
        foreach ($request as $key => $value)
        {
          $strMessage = null;
          $nSize = sizeof($value);
          for ($i = 0; $i < $nSize; $i++)
          {
            $strMessage .= $value[$i]."\n";
          }
          $strMessage .= 'end'."\n";
          fwrite($handle, $strMessage);
        }
        $nRequests = sizeof($request);
        unset($request);
        $response = array();
        for ($i = 0; $i < $nRequests; $i++)
        {
          $data = array();
          $bFoundEnd = false;
          while (!$bFoundEnd && !feof($handle))
          {
            if (($strLine = trim(fgets($handle))) != '' && $strLine != '""')
            {
              if ($strLine == 'end')
              {
                $bFoundEnd = true;
              }
              else
              {
                $data[] = $strLine;
              }
            }
          }
          if ($bFoundEnd && sizeof($data) >= 1)
          {
            $stat = json_decode($data[0], true);
            if (isset($stat['UniqueID']))
            {
              $response[$stat['UniqueID']] = $data;
            }
          }
          unset($data);
        }
        fclose($handle);
      }
      else if ($strError != '')
      {
        $this->setError('stream_socket_client('.$nErrorNo.') '.$strError);
      }
      else
      {
        $this->setError('Could not connect Service Junction socket.');
      }
      $this->semRelease();
    }
    else
    {
      $this->setError('Could not acquire the semaphore within '.$this->m_nSemTimeout.' seconds.');
    }

    return $bResult;
  }
  // }}}
  // {{{ curl()
  public function curl($strURL, $strType, $auth, $get, $post, $put, $strProxy, &$strCookies, &$strHeader, &$strContent, $strUserAgent = '', $bMobile = false, $bFailOnError = true, $strCustomRequest = '')
  {
    $bResult = false;
    $in = array();
    $out = null;
    $request = array();

    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $request['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'curl';
    $unIndex = 0;
    $in[$unIndex++] = json_encode($request);
    unset($request);
    $request = array();
    $request['URL'] = $strURL;
    $request['Display'] = 'Content,Cookies,Header';
    if ($strCookies != '')
    {
      $request['Cookies'] = $strCookies;
      $strCookies = '';
    }
    if ($auth != null)
    {
      $request['Auth'] = $auth;
    }
    if ($strContent != '')
    {
      $request['Content'] = $strContent;
    }
    $request['FailOnError'] = (($bFailOnError)?'yes':'no');
    if ($strCustomRequest != '')
    {
      $request['CustomRequest'] = $strCustomRequest;
    }
    if ($get != null)
    {
      $request['Get'] = $get;
    }
    if ($strHeader != '')
    {
      $request['Header'] = $strHeader;
    }
    if ($strUserAgent != '')
    {
      $request['UserAgent'] = $strUserAgent;
    }
    $request['Mobile'] = (($bMobile)?'yes':'no');
    if ($post != null)
    {
      $request['Post'] = $post;
    }
    if ($put != null)
    {
      $request['Put'] = $put;
    }
    if ($strProxy != '')
    {
      $request['Proxy'] = $strProxy;
    }
    if ($strType != '')
    {
      $request['Type'] = $strType;
    }
    $in[$unIndex++] = json_encode($request);
    unset($request);
    if ($this->request($in, $out))
    {
      $status = json_decode($out[0], true);
      if (sizeof($out) == 2)
      {
        $result = json_decode($out[1], true);
        if (isset($result['Cookies']))
        {
          $strCookies = $result['Cookies'];
        }
        if (isset($result['Header']))
        {
          $strHeader = $result['Header'];
        }
        if (isset($result['Content']))
        {
          $strContent = $result['Content'];
        }
        unset($result);
      }
      if (isset($status['Status']) && $status['Status'] == 'okay')
      {
        $bResult = true;
      }
      else if (isset($status['Error']) && $status['Error'] != '')
      {
        $this->setError($status['Error']);
      }
      else
      {
        $this->setError('Encountered an unknown error.');
      }
      unset($status);
    }
    unset($in);
    unset($out);

    return $bResult;
  }
  // }}}
  // {{{ disableSemaphore()
  public function disableSemaphore()
  {
    $this->m_bSemaphore = false;
  }
  // }}}
  // {{{ email()
  public function email($strFrom, $strTo, $strSubject = null, $strText = null, $strHTML = null, $strFile = null)
  {
    $bResult = false;

    $request = array();
    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $request['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'email';
    $request['From'] = $strFrom;
    $request['To'] = $strTo;
    if ($strSubject != '')
    {
      $request['Subject'] = $strSubject;
    }
    if ($strText != '')
    {
      $request['Text'] = $strText;
    }
    if ($strHTML != '')
    {
      $request['HTML'] = $strHTML;
    }
    $wrapper = array();
    $unIndex = 0;
    $wrapper[$unIndex++] = json_encode($request);
    if ($strFile != null && !is_array($strFile))
    {
      $strFile = array($strFile);
    }
    if (is_array($strFile))
    {
      $nSize = sizeof($strFile);
      for ($i = 0; $i < $nSize; $i++)
      {
        if (file_exists($strFile[$i]))
        {
          $data = array();
          $data['Name'] = $strFile[$i];
          if (($nPosition = strrpos($strFile[$i], '/')) !== false)
          {
            $data['Name'] = substr($strFile[$i], $nPosition + 1, strlen($strFile[$i]) - ($nPosition + 1));
          }
          $data['Data'] = base64_encode(file_get_contents($strFile[$i]));
          $data['Encode'] = 'base64';
          $wrapper[$unIndex++] = json_encode($data);
        }
      }
    }
    $response = null;
    if ($this->request($wrapper, $response))
    {
      if (sizeof($response) == 1)
      {
        $status = json_decode($response[0], true);
        if (isset($status['Status']) && $status['Status'] == 'okay')
        {
          $bResult = true;
        }
        if (isset($status['Error']) && $status['Error'] != '')
        {
          $this->setError($status['Error']);
        }
        unset($status);
      }
    }
    unset($wrapper);
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ enableSemaphore()
  public function enableSemaphore($strName, $nMaxAquire = 100, $bNonBlocking = false, $nTimeout = 60)
  {
    $this->m_sem = sem_get($this->semKey($strName), $nMaxAquire);
    $this->m_bSemNonBlocking = $bNonBlocking;
    $this->m_nSemTimeout = $nTimeout;
    $this->m_bSemaphore = true;
  }
  // }}}
  // {{{ finance()
  public function finance($strFunction, $strSymbol, $strCred, &$result)
  {
    $bResult = false;

    $request = array();
    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $request['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'finance';
    $request['Function'] = $strFunction;
    if ($strSymbol != '')
    {
      $request['Symbol'] = $strSymbol;
    }
    if ($strCred != '' && ($handle = fopen($strCred, 'r')) !== false)
    {
      while (!feof($handle))
      {
        if (($strLine = trim(fgets($handle))) != '')
        {
          $cred = json_decode($strLine, true);
          if ($cred['Name'] == $strFunction)
          {
            foreach ($cred as $key => $value)
            {
              if ($key != 'Name')
              {
                $request[$key] = $value;
              }
            }
          }
          unset($cred);
        }
      }
      fclose($handle);
    }
    $response = null;
    if ($this->request(array(json_encode($request)), $response))
    {
      if (sizeof($response) == 2)
      {
        $status = json_decode($response[0], true);
        $result = json_decode($response[1], true);
        if (isset($status['Status']) && $status['Status'] == 'okay')
        {
          $bResult = true;
        }
        if (isset($status['Error']) && $status['Error'] != '')
        {
          $this->setError($status['Error']);
        }
        unset($status);
      }
    }
    unset($response);
    unset($request);

    return $bResult;
  }
  // }}}
  // {{{ fusionQuery()
  public function fusionQuery($strUser, $strPassword, &$strAuth, $strQuery, &$result)
  {
    $bResult = false;

    $request = array();
    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $request['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'google';
    $request['Application'] = 'fusion';
    if ($strAuth != '')
    {
      $request['Auth'] = $strAuth;
    }
    else
    {
      $request['User'] = $strUser;
      $request['Password'] = $strPassword;
    }
    $request['Query'] = $strQuery;
    $response = null;
    if ($this->request(array(json_encode($request)), $response))
    {
      if (($unSize = sizeof($response)) >= 1)
      {
        $status = json_decode($response[0], true);
        if (isset($status['Status']) && $status['Status'] == 'okay')
        {
          $bResult = true;
          if (isset($status['Auth']) && $status['Auth'] != '')
          {
            $strAuth = $status['Auth'];
          }
          $result = array();
          for ($i = 1; $i < $unSize; $i++)
          {
            $result[$i-1] = json_decode($response[$i], true);
          }
        }
        else if (isset($status['Error']) && $status['Error'] != '')
        {
          $this->setError($status['Error']);
        }
        unset($status);
      }
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ fusionUpdate()
  public function fusionUpdate($strUser, $strPassword, &$strAuth, $strUpdate)
  {
    $bResult = false;

    $request = array();
    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $request['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'google';
    $request['Application'] = 'fusion';
    if ($strAuth != '')
    {
      $request['Auth'] = $strAuth;
    }
    else
    {
      $request['User'] = $strUser;
      $request['Password'] = $strPassword;
    }
    $request['Update'] = $strUpdate;
    $response = null;
    if ($this->request(array(json_encode($request)), $response))
    {
      if (sizeof($response) == 1)
      {
        $status = json_decode($response[0], true);
        if (isset($status['Status']) && $status['Status'] == 'okay')
        {
          $bResult = true;
          if (isset($status['Auth']) && $status['Auth'] != '')
          {
            $strAuth = $status['Auth'];
          }
        }
        else if (isset($status['Error']) && $status['Error'] != '')
        {
          $this->setError($status['Error']);
        }
        unset($status);
      }
    }
    unset($request);
    unset($response);

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
  // {{{ getError()
  public function getErrorStr()
  {
    return $this->m_strError;
  }
  // }}}
  // {{{ historical()
  public function historical($strFunction, $strSymbol, $strStart, $strEnd, $strFrequency, &$result)
  {
    $bResult = false;

    $request = array();
    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $request['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'historical';
    $request['Function'] = $strFunction;
    $request['Symbol'] = $strSymbol;
    $request['Start'] = $strStart;
    $request['End'] = $strEnd;
    $request['Frequency'] = $strFrequency;
    $response = null;
    if ($this->request(array(json_encode($request)), $response))
    {
      if (sizeof($response) == 2)
      {
        $status = json_decode($response[0], true);
        $result = json_decode($response[1], true);
        if (isset($status['Status']) && $status['Status'] == 'okay')
        {
          $bResult = true;
        }
        if (isset($status['Error']) && $status['Error'] != '')
        {
          $this->setError($status['Error']);
        }
        unset($status);
      }
    }
    unset($response);
    unset($request);

    return $bResult;
  }
  // }}}
  // {{{ ircBot()
  public function ircBot($strRoom, $strMessage)
  {
    $bResult = false;

    $request = array();
    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $request['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'ircBot';
    $request['Room'] = $strRoom;
    $request['Message'] = $strMessage;
    $response = null;
    if ($this->request(array(json_encode($request)), $response))
    {
      if (sizeof($response) == 1)
      {
        $status = json_decode($response[0], true);
        if (isset($status['Status']) && $status['Status'] == 'okay')
        {
          $bResult = true;
        }
        if (isset($status['Error']) && $status['Error'] != '')
        {
          $this->setError($status['Error']);
        }
        unset($status);
      }
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ jwt()
  public function jwt($strSigner, $strSecret, &$strPayload, &$payload)
  {
    $bDecode = false;
    $bResult = false;

    $request = array();
    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $request['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'jwt';
    if ($strPayload != '')
    {
      $bDecode = true;
      $request['Function'] = 'decode';
    }
    else
    {
      $request['Function'] = 'encode';
    }
    $in = array();
    $in[] = json_encode($request);
    unset($request);
    $request = array();
    $request['Signer'] = $strSigner;
    if ($strSecret != '')
    {
      $request['Secret'] = $strSecret;
    }
    if ($bDecode)
    {
      $request['Payload'] = $strPayload;
    }
    else
    {
      $request['Payload'] = $payload;
    }
    $in[] = json_encode($request);
    unset($request);
    $out = null;
    if ($this->request($in, $out))
    {
      $nSize = sizeof($out);
      if ($nSize >= 1)
      {
        $status = json_decode($out[0], true);
        if (isset($status['Status']) && $status['Status'] == 'okay')
        {
          if ($nSize == 2)
          {
            $data = json_decode($out[1], true);
            if (isset($data['Payload']))
            {
              if ($bDecode)
              {
                $bResult = true;
                $payload = $data['Payload'];
              }
              else if ($data['Payload'] != '')
              {
                $bResult = true;
                $strPayload = $data['Payload'];
              }
              else
              {
                $this->setError('Payload was empty within data.');
              }
            }
            else
            {
              $this->setError('Failed to find Payload within data.');
            }
            unset($data);
          }
          else
          {
            $this->setError('Failed to receive data.');
          }
        }
        else if (isset($status['Error']) && $status['Error'] != '')
        {
          $this->setError($status['Error']);
        }
        else
        {
          $this->setError('Encountered an unknown error.');
        }
        unset($status);
      }
      else
      {
        $this->setError('Failed to receive response.');
      }
    }
    unset($in);
    unset($out);

    return $bResult;
  }
  // }}}
  // {{{ location()
  public function location($strState, $strCity, $strAddress, &$result)
  {
    $bResult = false;

    $request = array();
    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $request['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'location';
    $request['State'] = $strState;
    $request['City'] = $strCity;
    $request['Address'] = $strAddress;
    $response = null;
    if ($this->request(array(json_encode($request)), $response))
    {
      if (sizeof($response) == 2)
      {
        $status = json_decode($response[0], true);
        $result = json_decode($response[1], true);
        if (isset($status['Status']) && $status['Status'] == 'okay')
        {
          $bResult = true;
        }
        if (isset($status['Error']) && $status['Error'] != '')
        {
          $this->setError($status['Error']);
        }
        unset($status);
      }
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ logger()
  public function logger($strApplication, $strUser, $strPassword, $strFunction, $strMessage, $label, &$result)
  {
    $bResult = false;

    $request = array();
    $request['Application'] = $strApplication;
    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $request['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'logger';
    $request['User'] = $strUser;
    $request['Password'] = $strPassword;
    $request['Function'] = $strFunction;
    $request['Message'] = $strMessage;
    $request['Label'] = $label;
    $response = null;
    if ($this->request(array(json_encode($request)), $response))
    {
      if (sizeof($response) >= 1)
      {
        $status = json_decode($response[0], true);
        if (isset($status['Status']) && $status['Status'] == 'okay')
        {
          $bResult = true;
          for ($i = 1; $i < sizeof($response); $i++)
          {
            $result[] = json_decode($response[$i], true);
          }
        }
        else if (isset($status['Error']) && $status['Error'] != '')
        {
          $this->setError($status['Error']);
        }
        else
        {
          $this->setError('Encountered an unkonwn error.');
        }
        unset($status);
      }
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ mssqlQuery()
  public function mssqlQuery($strUser, $strPassword, $strServer, $strQuery, &$result)
  {
    $bResult = false;

    $request = array();
    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $request['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'mssql';
    $request['User'] = $strUser;
    $request['Password'] = $strPassword;
    $request['Server'] = $strServer;
    $request['Query'] = $strQuery;
    $response = null;
    if ($this->request(array(json_encode($request)), $response))
    {
      if (($unSize = sizeof($response)) >= 1)
      {
        $status = json_decode($response[0], true);
        if (isset($status['Status']) && $status['Status'] == 'okay')
        {
          $bResult = true;
          $result = array();
          for ($i = 1; $i < $unSize; $i++)
          {
            $result[$i-1] = json_decode($response[$i], true);
          }
        }
        else if (isset($status['Error']) && $status['Error'] != '')
        {
          $this->setError($status['Error']);
        }
        unset($status);
      }
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ mssqlUpdate()
  public function mssqlUpdate($strUser, $strPassword, $strServer, $strUpdate)
  {
    $bResult = false;

    $request = array();
    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $request['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'mssql';
    $request['User'] = $strUser;
    $request['Password'] = $strPassword;
    $request['Server'] = $strServer;
    $request['Update'] = $strUpdate;
    $response = null;
    if ($this->request(array(json_encode($request)), $response))
    {
      if (sizeof($response) == 1)
      {
        $status = json_decode($response[0], true);
        if (isset($status['Status']) && $status['Status'] == 'okay')
        {
          $bResult = true;
        }
        else if (isset($status['Error']) && $status['Error'] != '')
        {
          $this->setError($status['Error']);
        }
        unset($status);
      }
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ mysqlQuery()
  public function mysqlQuery($strUser, $strPassword, $strServer, $strDatabase, $strQuery, &$result)
  {
    $bResult = false;

    $request = array();
    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $request['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'mysql';
    $request['User'] = $strUser;
    $request['Password'] = $strPassword;
    $request['Server'] = $strServer;
    $request['Database'] = $strDatabase;
    $request['Query'] = $strQuery;
    $response = null;
    if ($this->request(array(json_encode($request)), $response))
    {
      if (($unSize = sizeof($response)) >= 1)
      {
        $status = json_decode($response[0], true);
        if (isset($status['Status']) && $status['Status'] == 'okay')
        {
          $bResult = true;
          $result = array();
          for ($i = 1; $i < $unSize; $i++)
          {
            $result[$i-1] = json_decode($response[$i], true);
          }
        }
        else if (isset($status['Error']) && $status['Error'] != '')
        {
          $this->setError($status['Error']);
        }
        unset($status);
      }
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ mysqlUpdateWithID()
  public function mysqlUpdateWithID($strUser, $strPassword, $strServer, $strDatabase, $strUpdate, &$nID)
  {
    $bResult = false;

    $request = array();
    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $request['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'mysql';
    $request['User'] = $strUser;
    $request['Password'] = $strPassword;
    $request['Server'] = $strServer;
    $request['Database'] = $strDatabase;
    $request['Update'] = $strUpdate;
    $response = null;
    if ($this->request(array(json_encode($request)), $response))
    {
      if (sizeof($response) == 1)
      {
        $status = json_decode($response[0], true);
        if (isset($status['Status']) && $status['Status'] == 'okay')
        {
          $bResult = true;
          if (isset($status['ID']) && $status['ID'] != '')
          {
            $nID = $status['ID'];
          }
        }
        else if (isset($status['Error']) && $status['Error'] != '')
        {
          $this->setError($status['Error']);
        }
        unset($status);
      }
    }
    unset($request);
    unset($response);

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
  // {{{ oracleQuery()
  public function oracleQuery($strSchema, $strPassword, $strTnsName, $strQuery, &$result)
  {
    $bResult = false;

    $request = array();
    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $request['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'oracle';
    $request['Schema'] = $strSchema;
    $request['Password'] = $strPassword;
    $request['tnsName'] = $strTnsName;
    $request['Query'] = $strQuery;
    $response = null;
    if ($this->request(array(json_encode($request)), $response))
    {
      if (($unSize = sizeof($response)) >= 1)
      {
        $status = json_decode($response[0], true);
        if (isset($status['Status']) && $status['Status'] == 'okay')
        {
          $bResult = true;
          $result = array();
          for ($i = 1; $i < $unSize; $i++)
          {
            $result[$i-1] = json_decode($response[$i], true);
          }
        }
        else if (isset($status['Error']) && $status['Error'] != '')
        {
          $this->setError($status['Error']);
        }
        unset($status);
      }
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ oracleUpdate()
  public function oracleUpdate($strSchema, $strPassword, $strTnsName, $strUpdate)
  {
    $bResult = false;

    $request = array();
    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $request['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'oracle';
    $request['Schema'] = $strSchema;
    $request['Password'] = $strPassword;
    $request['tnsName'] = $strTnsName;
    $request['Update'] = $strUpdate;
    $response = null;
    if ($this->request(array(json_encode($request)), $response))
    {
      if (sizeof($response) == 1)
      {
        $status = json_decode($response[0], true);
        if (isset($status['Status']) && $status['Status'] == 'okay')
        {
          $bResult = true;
        }
        else if (isset($status['Error']) && $status['Error'] != '')
        {
          $this->setError($status['Error']);
        }
        unset($status);
      }
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ page()
  public function page($strUser, $strMessage, $bGroup = false)
  {
    $bResult = false;

    $request = array();
    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $request['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'pager';
    if ($bGroup)
    {
      $request['Group'] = $strUser;
    }
    else
    {
      $request['User'] = $strUser;
    }
    $request['Message'] = $strMessage;
    $response = null;
    if ($this->request(array(json_encode($request)), $response))
    {
      if (sizeof($response) == 1)
      {
        $status = json_decode($response[0], true);
        if (isset($status['Status']) && $status['Status'] == 'okay')
        {
          $bResult = true;
        }
        if (isset($status['Error']) && $status['Error'] != '')
        {
          $this->setError($status['Error']);
        }
        unset($status);
      }
    }
    unset($request);
    unset($response);

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
    $strError = null;

    if ($this->semAcquire())
    {
      $bConnected = false;
      $handle = null;
      $nErrorNo = null;
      $response = array();
      $this->readConf();
      $junctionServer = explode(',', $this->m_conf['Load Balancer']);
      $unSize = sizeof($junctionServer);
      for ($i = (($this->m_bUseSecureJunction)?0:1); !$bConnected && $i < 2; $i++)
      {
        $unAttempt = 0;
        $unPick = rand(0, ($unSize - 1));
        while (!$bConnected && $unAttempt++ < $unSize)
        {
          if ($unPick == $unSize)
          {
            $unPick = 0;
          }
          $strServer = $junctionServer[$unPick];
          if ($handle = stream_socket_client(($i == 0)?'ssl://'.$strServer.':5863':$strServer.':5862', $nErrorNo, $strError, 10, STREAM_CLIENT_CONNECT, $this->m_streamContext))
          {
            $bConnected = true;
          }
        }
      }
      unset($junctionServer);
      if ($bConnected)
      {
        $nSize = sizeof($request);
        $strMessage = null;
        $bReturnArr = false;
        $bReturnObj = false;
        for ($i = 0; $i < $nSize; $i++)
        {
          if (is_array($request[$i]) || is_object($request[$i]) || trim($request[$i]) != 'end')
          {
            if (is_array($request[$i]))
            {
              $bReturnArr = true;
              if ($i == 0 && $this->m_strTimeout != '')
              {
                $request[$i]['Timeout'] = $this->m_strTimeout;
              }
              $strMessage .= json_encode($request[$i])."\n";
            }
            else if (is_object($request[$i]))
            {
              $bReturnObj = true;  
              if ($i == 0 && $this->m_strTimeout != '')
              {
                $request[$i]['Timeout'] = $this->m_strTimeout;
              }
              $strMessage .= json_encode($request[$i])."\n";
            }
            else if ($i == 0 && $this->m_strTimeout != '')
            {
              $req = json_decode($request[$i], true);
              $req['Timeout'] = $this->m_strTimeout;
              $strMessage .= json_encode($req)."\n";
              unset($req);
            }
            else
            {
              $strMessage .= $request[$i]."\n";
            }
          }
        }
        $strMessage .= 'end'."\n";
        fwrite($handle, $strMessage);
        while (!$bResult && !feof($handle))
        {
          if (($strLine = trim(fgets($handle))) != '' && $strLine != '""')
          {
            if ($strLine == 'end')
            {
              $bResult = true;
            }
            else if ($bReturnArr || $bReturnObj)
            {
              $response[] = json_decode($strLine, $bReturnArr);
            }
            else
            {
              $response[] = $strLine;
            }
          }
        }
        if (!$bResult)
        {
          $this->setError('Failed to receive end of Service Junction response.');
        }
        else if (sizeof($response) == 0)
        {
          $bResult = false;
          $this->setError('Failed to receive any data from the underlying service within the Service Junction.');
        }
        fclose($handle);
      }
      else if ($strError != '')
      {
        $this->setError('stream_socket_client('.$nErrorNo.') '.$strError);
      }
      else
      {
        $this->setError('Could not connect Service Junction socket.');
      }
      $this->semRelease();
    }
    else
    {
      $this->setError('Could not acquire the semaphore within '.$this->m_nSemTimeout.' seconds.');
    }

    return $bResult;
  }
  // }}}
  // {{{ semAcquire()
  public function semAcquire()
  {
    $bResult = true;

    if ($this->m_bSemaphore)
    {
      $bResult = false;
      $unAttempt = 0;
      while (!$bResult && $unAttempt < ($this->m_nSemTimeout * 4))
      {
        if (!($bResult = sem_acquire($this->m_sem, $this->m_bSemNonBlocking)))
        {
//echo $unAttempt."\n";
          usleep(250000);
        }
        $unAttempt++;
      }
    }

    return $bResult;
  }
  // }}}
  // {{{ semKey()
  public function semKey($strName)
  {
    $strKey = md5($strName);
    $nKey = 0;

    for ($i = 0; $i < 32; $i++)
    {
      $nKey += ord($strKey[$i]) * $i;
    }

    return $nKey;
  }
  // }}}
  // {{{ semRelease()
  public function semRelease()
  {
    $bResult = true;

    if ($this->m_bSemaphore)
    {
      $bResult = sem_release($this->m_sem);
    }

    return $bResult;
  }
  // }}}
  // {{{ setApplication()
  public function setApplication($strApplication)
  {
    $this->m_strApplication = $strApplication;
  }
  // }}}
  // {{{ setError()
  public function setError($strError)
  {
    $this->m_strError = $strError;
  }
  // }}}
  // {{{ setProgram()
  public function setProgram($strProgram)
  {
    $this->m_strProgram = $strProgram;
  }
  // }}}
  // {{{ setStreamContext()
  public function setStreamContext($context)
  {
    $this->m_streamContext = stream_context_create($context);
  }
  // }}}
  // {{{ setTimeout()
  public function setTimeout($strTimeout)
  {
    $this->m_strTimeout = $strTimeout;
  }
  // }}}
  // {{{ useSecureJunction()
  public function useSecureJunction($bUseSecureJunction = true)
  {
    $this->m_bUseSecureJunction = $bUseSecureJunction;
  }
  // }}}
  // {{{ weather()
  public function weather($strCountry, $strCity, &$result)
  {
    $bResult = false;

    $request = array();
    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $request['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'weather';
    $request['Country'] = $strCountry;
    $request['City'] = $strCity;
    $response = null;
    if ($this->request(array(json_encode($request)), $response))
    {
      if (sizeof($response) == 2)
      {
        $status = json_decode($response[0], true);
        $result = json_decode($response[1], true);
        if (isset($status['Status']) && $status['Status'] == 'okay' && isset($result['Status']) && $result['Status'] == 'Success')
        {
          $bResult = true;
        }
        if (isset($status['Error']) && $status['Error'] != '')
        {
          $this->setError($status['Error']);
        }
        unset($status);
      }
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
}
?>
