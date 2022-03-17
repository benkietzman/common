<?php
// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2013-11-12
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
**************************************************************************/

class ServiceJunction
{
  // {{{ variables
  protected $m_bUseSecureJunction;
  protected $m_conf;
  protected $m_strApplication;
  protected $m_strConf;
  protected $m_streamContext;
  protected $m_strError;
  protected $m_strProgram;
  protected $m_ulModifyTime;
  // }}}
  // {{{ __construct()
  public function __construct($strConf = null)
  {
    $this->m_bUseSecureJunction = false;
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
  // {{{ batch()
  public function batch($request, &$response)
  {
    $bConnected = false;
    $bResult = false;
    $handle = null;
    $nErrorNo = null;
    $strError = null;
    $unAttempt = 0;

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
        if ($handle = stream_socket_client(($i == 0)?'tls://'.$strServer.':5863':$strServer.':5862', $nErrorNo, $strError, 10, STREAM_CLIENT_CONNECT, $this->m_streamContext))
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

    return $bResult;
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
      $req['reqProg'] = $this->m_strProgram;
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
      $req['reqProg'] = $this->m_strProgram;
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
      $req['reqProg'] = $this->m_strProgram;
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
      $req['reqProg'] = $this->m_strProgram;
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
      $req['reqProg'] = $this->m_strProgram;
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
      $req['reqProg'] = $this->m_strProgram;
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
      $req['reqProg'] = $this->m_strProgram;
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
    $in[] = $request;
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
    $in[] = $json_encode($request);
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
      $req['reqProg'] = $this->m_strProgram;
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
      $req['reqProg'] = $this->m_strProgram;
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
      $req['reqProg'] = $this->m_strProgram;
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
      $req['reqProg'] = $this->m_strProgram;
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
      $req['reqProg'] = $this->m_strProgram;
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
      $req['reqProg'] = $this->m_strProgram;
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
      $req['reqProg'] = $this->m_strProgram;
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
  public function page($strUserID, $strMessage)
  {
    $bResult = false;

    $request = array();
    if ($this->m_strApplication != '')
    {
      $request['reqApp'] = $this->m_strApplication;
    }
    if ($this->m_strProgram != '')
    {
      $req['reqProg'] = $this->m_strProgram;
    }
    $request['Service'] = 'pager';
    $request['User'] = $strUserID;
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
    $bConnected = false;
    $bResult = false;
    $handle = null;
    $nErrorNo = null;
    $strError = null;
    $unAttempt = 0;

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
        if ($handle = stream_socket_client(($i == 0)?'tls://'.$strServer.':5863':$strServer.':5862', $nErrorNo, $strError, 10, STREAM_CLIENT_CONNECT, $this->m_streamContext))
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
        if (trim($request[$i]) != 'end')
        {
          // check if request is a line of json or php array or object
          if(is_array($request[$i])){
            $bReturnArr = true;
            $strMessage .= json_encode($request[$i])."\n";
          } elseif(is_object($request[$i])){
            $bReturnObj = true;  
            $strMessage .= json_encode($request[$i])."\n";
          } else {
            // just process as normal if no types can be determined
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
          else
          {
            if($bReturnArr){
              $response[] = json_decode($strLine, true);
            } elseif($bReturnObj){
              $response[] = json_decode($strLine, false);
            } else {
              $response[] = $strLine;
            }
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

    return $bResult;
  }
  // }}}
  // {{{ setApplication()
  public function setApplication($strApplication)
  {
    $this->m_strApplication = $strApplication;
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
  // {{{ useSecureJunction()
  public function useSecureJunction($bUseSecureJunction = true)
  {
    $this->m_bUseSecureJunction = $bUseSecureJunction;
  }
  // }}}
  // {{{ setError()
  public function setError($strError)
  {
    $this->m_strError = $strError;
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
      $req['reqProg'] = $this->m_strProgram;
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
