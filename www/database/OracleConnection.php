<?php
// vim600: fdm=marker
//////////////////////////////////////////////////////////////////////////
// Oracle Connection
// -------------------
// begin                : 2005-09-19
// copyright            : kietzman.org
// email                : ben@kietzman.org
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//////////////////////////////////////////////////////////////////////////

// {{{ includes
include_once('DatabaseConnection.php');
include_once('OracleQuery.php');
// }}}
//! OracleConnection Class
/*!
* Provides functions for establishing Oracle database connections.
*/
class OracleConnection extends DatabaseConnection
{
  // {{{ variables
  private $m_bPreFetch;
  private $m_nPreFetch;
  private $m_bGood;
  // }}}
  // {{{ __construct()
  public function __construct($bError = false)
  {
    parent::__construct($bError);
    $this->m_bPreFetch = false;
  }
  // }}}
  // {{{ __destruct()
  public function __destruct()
  {
    parent::__destruct();
  }
  // }}}
  // {{{ setPreFetch()
  public function setPreFetch($nPreFetch = 1000)
  {
    $this->m_bPreFetch = true;
    $this->m_nPreFetch = $nPreFetch;
  }
  // }}}
  // {{{ connect()
  public function connect($strSchema, $strPassword, $strTnsName, $bPersistant = false)
  {
    $backtrace = $this->m_error->setBacktrace();
    $this->m_strUser = $strSchema;
    $this->m_strPassword = $strPassword;
    $this->m_strDatabase = $strSchema;
    $this->m_strServer = $strTnsName;
    $this->m_db = ($bPersistant)?oci_pconnect($strSchema, $strPassword, $strTnsName):oci_connect($strSchema, $strPassword, $strTnsName);
    if (($e = oci_error()) !== false)
    {
      $this->m_strError = $e['message'];
    }
    $this->m_bGood = ($this->m_db !== false)?true:false;
  }
  // }}}
  // {{{ disconnect()
  public function disconnect()
  {
    if ($this->m_db)
    {
      oci_close($this->m_db);
    }
    $this->m_bGood = false;
  }
  // }}}
  // {{{ good()
  public function good()
  {
    return $this->m_bGood;
  }
  // }}}
  // {{{ parse()
  public function parse($strQuery)
  {
    $query = new OracleQuery($this->m_bError);
    $query->setDatabase($this->m_db);
    $query->parse($strQuery);
    if ($this->m_bPreFetch)
    {
      $query->setPreFetch($this->m_nPreFetch);
    }
    return $query;
  }
  // }}}
}
?>
