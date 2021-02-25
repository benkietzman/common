<?php
// vim600: fdm=marker
//////////////////////////////////////////////////////////////////////////
// Postgre Connection
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
include_once('PostgreQuery.php');
// }}}
//! PostgreConnection Class
/*!
* Provides functions for establishing Postgre database connections.
*/
class PostgreConnection extends DatabaseConnection
{
  // {{{ variables
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
    $this->m_db = ($bPersistant)?pg_pconnect("host=".$strServer." port=5432 dbname=".$strDatabase." user=".$strUser." password=".$strPassword):pg_connect("host=".$strServer." port=5432 dbname=".$strDatabase." user=".$strUser." password=".$strPassword);
    $this->m_bGood = ($this->m_db !== false)?true:false;
  }
  // }}}
  // {{{ disconnect()
  public function disconnect()
  {
    if ($this->m_db)
    {
      pg_close($this->m_db);
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
    $query = new PostgreQuery($this->m_bError);
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
