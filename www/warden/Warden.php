<?php
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2021-04-15
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
/*! \file Warden.php
* \brief Warden Class
*/
class Warden
{
  // {{{ variables
  protected $m_strApplication;
  protected $m_strError;
  protected $m_strUnix;
  // }}}
  // {{{ __construct()
  public function __construct($strApplication, $strUnix)
  {
    $this->m_strUnix = $strUnix;
    if ($strApplication != '')
    {
      $this->m_strApplication = $strApplication;
    }
    else
    {
      $this->m_strError = 'Please provide the Application.';
    }
  }
  // }}}
  // {{{ __destruct()
  public function __destruct()
  {
  }
  // }}}
  // {{{ getError()
  public function getError()
  {
    return $this->m_strError;
  }
  // }}}
  // {{{ authn()
  public function authn(&$data)
  {
    return $this->request('authn', $data);
  }
  // }}}
  // {{{ authz()
  public function authz(&$data)
  {
    return $this->request('authz', $data);
  }
  // }}}
  // {{{ bridge
  // {{{ bridge()
  public function bridge(&$data)
  {
    return $this->request('bridge', $data);
  }
  // }}}
  // {{{ bridgeAuthn()
  public function bridgeAuthn($strUser, $strPassword)
  {
    $bResult = false;

    $data = [];
    $bResult = $this->bridgeAuthz($strUser, $strPassword, $data);
    unset($data);

    return $bResult;
  }
  // }}}
  // {{{ bridgeAuthz()
  public function bridgeAuthz($strUser, $strPassword, &$data)
  {
    $data['User'] = $strUser;
    $data['Password'] = $strPassword;

    return $this->bridge($data);
  }
  // }}}
  // }}}
  // {{{ central
  // {{{ central()
  public function central(&$data)
  {
    return $this->request('central', $data);
  }
  // }}}
  // {{{ centralUser()
  public function centralUser($strUser, &$data)
  {
    $data['User'] = $strUser;

    return $this->central($data);
  }
  // }}}
  // }}}
  // {{{ password
  // {{{ password()
  public function password(&$data)
  {
    return $this->request('password', $data);
  }
  // }}}
  // {{{ passwordApplication()
  public function passwordApplication($strApplication, $strUser, $strPassword, $strType)
  {
    $bResult = false;

    $data = [];
    $data['Application'] = $strApplication;
    $data['User'] = $strUser;
    $data['Password'] = $strPassword;
    if ($strType != '')
    {
      $data['Type'] = $strType;
    }
    $bResult = $this->password($data);
    unset($data);

    return $bResult;
  }
  // }}}
  // {{{ passwordUser()
  public function passwordUser($strUser, $strPassword)
  {
    $bResult = false;

    $data = [];
    $data['User'] = $strUser;
    $data['Password'] = $strPassword;
    $bResult = $this->password($data);
    unset($data);

    return $bResult;
  }
  // }}}
  // }}}
  // {{{ request
  // {{{ request()
  public function request($strModule, &$data)
  {
    $bResult = false;

    $request = $data;
    $data = [];
    $request['Module'] = $strModule;
    $response = null;
    if ($this->requestFull($request, $response))
    {
      $bResult = true;
      if (is_array($response) && isset($response['Data']))
      {
        $data = $response['Data'];
      }
    }
    unset($request);
    unset($response);

    return $bResult;
  }
  // }}}
  // {{{ requestFull()
  public function requestFull($request, &$response)
  {
    $bResult = false;
    $handle = null;
    $nErrorNo = null;
    $strError = null;

    $response = [];
    if (($handle = fsockopen('unix://'.$this->m_strUnix, -1, $nErrorNo, $strError)) !== false)
    {
      $bExit = false;
      fwrite($handle, json_encode($request)."\n");
      while (!$bExit && !feof($handle))
      {
        if (($strLine = fgets($handle)) !== false)
        {
          $bExit = true;
          $response = json_decode($strLine, true);
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
      if (!$bResult && $this->getError() == '')
      {
        $this->setError('Failed to receive Warden response.');
      }
      fclose($handle);
    }
    else if ($strError != '')
    {
      $this->setError('fsockopen('.$nErrorNo.') '.$strError);
    }
    else
    {
      $this->setError('Could not connect Warden socket.');
    }

    return $bResult;
  }
  // }}}
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
  // {{{ vault
  // {{{ vault()
  public function vault($strFunction, $keys, &$data)
  {
    $bResult = false;
    $subData = $data;

    $data = [];
    if ($this->m_strApplication != '')
    {
      $data['Function'] = $strFunction;
      if (!is_array($keys))
      {
        $keys = [];
      }
      array_unshift($keys, $this->m_strApplication);
      $data['Keys'] = $keys;
      if ($subData != null)
      {
        $data['Data'] = $subData;
      }
      $bResult = $this->request('vault', $data);
    }
    else
    {
      $this->setError('Please provide the Application.');
    }
    unset($subData);

    return $bResult;
  }
  // }}}
  // {{{ vaultAdd()
  public function vaultAdd($keys, &$data)
  {
    return $this->vault("add", $keys, $data);
  }
  // }}}
  // {{{ vaultRemove()
  public function vaultRemove($keys)
  {
    return $this->vault("remove", $keys);
  }
  // }}}
  // {{{ vaultRetrieve()
  public function vaultRetrieve($keys, &$data)
  {
    return $this->vault("retrieve", $keys, $data);
  }
  // }}}
  // {{{ vaultRetrieveKeys()
  public function vaultRetrieveKeys($keys, &$data)
  {
    return $this->vault("retrieveKeys", $keys, $data);
  }
  // }}}
  // {{{ vaultUpdate()
  public function vaultUpdate($keys, &$data)
  {
    return $this->vault("update", $keys, $data);
  }
  // }}}
  // }}}
  // {{{ windows
  // {{{ windows()
  public function windows(&$data)
  {
    return $this->request('windows', $data);
  }
  // }}}
  // {{{ windowsUser()
  public function windowsUser($strUser, $strPassword)
  {
    $bResult = false;

    $data = [];
    $data['User'] = $strUser;
    $data['Password'] = $strPassword;
    $bResult = $this->windows($data);
    unset($data);

    return $bResult;
  }
  // }}}
  // }}}
}
?>
