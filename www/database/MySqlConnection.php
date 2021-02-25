<?php
// vim600: fdm=marker
//////////////////////////////////////////////////////////////////////////
// MySql Connection
// -------------------
// begin                : 09/19/2005
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
include_once("DatabaseConnection.php");
include_once("MySqlQuery.php");
// }}}
//! MySqlConnection Class
/*!
* Provides functions for establishing MySQL database connections.
*/
class MySqlConnection extends DatabaseConnection
{
  // {{{ variables
  private $m_bDatabaseSelected;
  private $m_bGood;
  // }}}
  // {{{ __construct()
  public function __construct($bError = false)
  {
    parent::__construct($bError);
    $this->m_bDatabaseSelected = false;
  }
  // }}}
  // {{{ __destruct()
  public function __destruct()
  {
    parent::__destruct();
  }
  // }}}
  // {{{ connect()
  public function connect($strUser, $strPassword, $strServer, $bPersistant = false)
  {
    $backtrace = $this->m_error->setBacktrace();
    $this->m_strUser = $strUser;
    $this->m_strPassword = $strPassword;
    $this->m_strServer = $strServer;
    $this->m_db = new mysqli((($bPersistant)?'p:':'').$strServer, $strUser, $strPassword);
    if ($this->m_db->connect_error != '')
    {
      $this->m_strError = $this->m_db->connect_error;
    }
    $this->m_bGood = ($this->m_db !== false)?true:false;
  }
  // }}}
  // {{{ disconnect()
  public function disconnect()
  {
    $backtrace = $this->m_error->setBacktrace();
    if ($this->m_db)
    {
      $this->m_db->close();
    }
    $this->m_bGood = false;
  }
  // }}}
  // {{{ getCharset()
  public function getCharset()
  {
    return $this->m_db->character_set_name();  
  }
  // }}}
  // {{{ good()
  public function good()
  {
    return $this->m_bGood;
  }
  // }}}
  // {{{ setCharset()
  public function setCharset($strCharset)
  { 
    if ($this->m_db->set_charset($strCharset))
    { 
      return $this->m_db->character_set_name();
    }
    else
    { 
      $this->m_strError = $this->m_db->error;
    }
  }
  // }}}
  // {{{ selectDatabase()
  public function selectDatabase($strDatabase)
  {
    $backtrace = $this->m_error->setBacktrace();
    $this->m_strDatabase = $strDatabase;
    if ($this->m_db->select_db($strDatabase))
    {
      $this->m_bDatabaseSelected = true;
    }
    else
    {
      $this->m_strError = $this->m_db->error;
    }
  }
  // }}}
  // {{{ parse()
  public function parse($strQuery)
  {
    if (!$this->m_bDatabaseSelected)
    {
      $this->selectDatabase($this->m_strUser);
    }
    $query = new MySqlQuery($this->m_bError);
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
