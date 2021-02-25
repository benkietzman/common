<?php
// vim600: fdm=marker
//////////////////////////////////////////////////////////////////////////
// Informix Connection
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
include_once('InformixQuery.php');
// }}}
//! InformixConnection Class
/*!
* Provides functions for establishing Informix database connections.
*/
class InformixConnection extends DatabaseConnection
{
  // {{{ variable
  private $m_bGood;
  // }}}
  // {{{ __construct()
  public function __construct($bError = false)
  {
    parent::__construct($bError);
  }
  // }}}
  // {{{ __destruct()
  public function __destruct()
  {
    parent::__destruct();
  }
  // }}}
  // {{{ connect()
  public function connect($strUser, $strPassword, $strDatabase, $strServer, $bPersistant = false)
  {
    $backtrace = $this->m_error->setBacktrace();
    $this->m_strUser = $strUser;
    $this->m_strPassword = $strPassword;
    $this->m_strDatabase = $strDatabase;
    $this->m_strServer = $strServer;
    $this->m_db = ($bPersistant)?ifx_pconnect($strDatabase."@".$strServer, $strUser, $strPassword):ifx_connect($strDatabase."@".$strServer, $strUser, $strPassword);
    $this->m_bGood = ($this->m_db !== false)?true:false;
  }
  // }}}
  // {{{ disconnect()
  public function disconnect()
  {
    if ($this->m_db)
    {
      ifx_close($this->m_db);
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
    $query = new InformixQuery($this->m_bError);
    $query->setDatabase($this->m_db);
    $query->parse($strQuery);
    return $query;
  }
  public function query($strQuery)
  {
    return $this->parse($strQuery);
  }
  // }}}
}
?>
