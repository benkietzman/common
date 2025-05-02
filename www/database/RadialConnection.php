<?php
//////////////////////////////////////////////////////////////////////////
// Radial Connection
// -------------------
// begin      : 2023-02-18
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
//////////////////////////////////////////////////////////////////////////
// {{{ includes
include_once('DatabaseConnection.php');
include_once('RadialQuery.php');
// }}}
//! RadialConnection Class
/*!
* Provides functions for establishing database connections through Radial.
*/
class RadialConnection extends DatabaseConnection
{
  // {{{ variables
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
  public function connect($strUser, $strPassword, $strDatabase)
  {
    $backtrace = $this->m_error->setBacktrace();
    $this->m_bGood = true;
    $this->m_strUser = $strUser;
    $this->m_strPassword = $strPassword;
    $this->m_strServer = $strDatabase;
    $this->m_db = array('User'=>$strUser, 'Password'=>$strPassword, 'Database'=>$strDatabase);
  }
  // }}}
  // {{{ disconnect()
  public function disconnect()
  {
    $backtrace = $this->m_error->setBacktrace();
    unset($this->m_db);
    $this->m_bGood = false;
  }
  // }}}
  // {{{ getCharset()
  public function getCharset()
  {
    return;
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
    $query = new RadialQuery($this->m_bError);
    $query->setDatabase($this->m_db);
    $query->parse($strQuery);
    return $query;
  }
  public function query($strQuery)
  {
    return $this->parse($strQuery);
  }
  // }}}
  // {{{ setCharset()
  public function setCharset($strCharset)
  { 
  }
  // }}}
}
?>
