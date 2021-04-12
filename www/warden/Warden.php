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
  public function __construct($strApplication, $strUnix, $strConf = null)
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
  // {{{ getError()
  public function getError()
  {
    return $this->m_strError;
  }
  // }}}
  // {{{ request()
  public function request($request, &$response)
  {
    $bResult = false;
    $handle = null;
    $nErrorNo = null;
    $strError = null;

    $response = array();
    if (($handle = fsockopen('unix://'.$this->m_strUnix, -1, $nErrorNo, $strError)) !== false)
    {
      fwrite($handle, json_encode($request)."\n");
      while (!$bResult && !feof($handle))
      {
        if (($strLine = trim(fgets($handle))) != '' && $strLine != '""')
        {
          $bResult = true;
          $response = json_decode($strLine, true);
        }
      }
      if (!$bResult)
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

    if ($this->m_strApplication != '')
    {
      $request = [];
      $request['Module'] = 'vault';
      $request['Function'] = $strFunction;
      array_unshift($keys, $this->m_strApplication);
      $request['Keys'] = $keys;
      if ($data != null)
      {
        $request['Data'] = $data;
      }
      $response = null;
      if ($this->request($request, $response))
      {
        $bResult = true;
        if (is_array($response) && isset($response['Data']))
        {
          unset($data);
          $data = $response['Data'];
        }
      }
      unset($request);
      unset($response);
    }
    else
    {
      $this->setError('Please provide the Application.');
    }

    return $bResult;
  }
  // }}}
  // {{{ vaultAdd()
  public function vaultAdd($keys, &$data)
  {
    return $this->vault("add", $keys, $data);
  }
  // }}}
  // {{{ vaultDelete()
  public function vaultDelete($keys)
  {
    return $this->vault("delete", $keys);
  }
  // }}}
  // {{{ vaultRetrieve()
  public function vaultRetrieve($keys, &$data)
  {
    return $this->vault("ad", $keys, $data);
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
}
?>
