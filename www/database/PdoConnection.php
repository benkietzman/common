<?php
// vim600: fdm=marker
//////////////////////////////////////////////////////////////////////////
// Pdo Connection
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
include_once("PdoQuery.php");
// }}}
//! MySqlConnection Class
/*!
* Provides functions for establishing MySQL database connections.
*/
class PdoConnection extends DatabaseConnection
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
  public function connect($strUser, $strPassword, $strServer, $bPersistant = false)
  {
    $backtrace = $this->m_error->setBacktrace();
    $this->m_strUser = $strUser;
    $this->m_strPassword = $strPassword;
    $this->m_strServer = $strServer;
    try {
      $this->m_db = new PDO($strServer, $strUser, $strPassword, array(PDO::ATTR_PERSISTENT => $bPersistant));
      if(strpos($strServer,'sqlite:')!==false){
        $this->m_db->exec("pragma synchronous = off;");
      }
    } catch (PDOException $e) {
      $this->m_strError = $e->getMessage();
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
      $this->m_db = null;
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
    $query = new PdoQuery($this->m_bError);
    $query->setDatabase($this->m_db);
    $query->parse($strQuery);
    return $query;
  }
  // }}}
  // {{{ query()
  public function query($strQuery)
  {
    return $this->parse($strQuery);
  }
  // }}}
}
?>
