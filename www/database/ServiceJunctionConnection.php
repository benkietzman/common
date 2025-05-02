<?php
//////////////////////////////////////////////////////////////////////////
// Service Junction Connection
// -------------------
// begin      : 2024-03-13
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
//////////////////////////////////////////////////////////////////////////
// {{{ includes
include_once('DatabaseConnection.php');
include_once('ServiceJunctionQuery.php');
// }}}
//! ServiceJunctionConnection Class
/*!
* Provides functions for establishing database connections through the Service Junction.
*/
class ServiceJunctionConnection extends DatabaseConnection
{
  // {{{ variables
  private $m_bGood;
  protected $m_strService;
  // }}}
  // {{{ __construct()
  public function __construct($bError = false)
  {
    parent::__construct($bError);
    $this->m_bDatabaseSelected = false;
    $this->m_db = array();
    $this->m_strService = 'mysql';
  }
  // }}}
  // {{{ __destruct()
  public function __destruct()
  {
    parent::__destruct();
    unset($this->m_db);
  }
  // }}}
  // {{{ connect()
  public function connect($strUser, $strPassword, $strServer, $strDatabase = null)
  {
    $backtrace = $this->m_error->setBacktrace();
    $this->m_bGood = true;
    $this->m_strUser = $strUser;
    $this->m_strPassword = $strPassword;
    $this->m_strServer = $strServer;
    $this->m_db['Database'] = $strDatabase;
    $this->m_db['Password'] = $strPassword;
    $this->m_db['Server'] = $strServer;
    $this->m_db['User'] = $strUser;
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
    $query = new ServiceJunctionQuery($this->m_bError);
    $query->setService($this->m_strService);
    $query->setDatabase($this->m_db);
    $query->parse($strQuery);
    return $query;
  }
  public function query($strQuery)
  {
    return $this->parse($strQuery);
  }
  // }}}
  // {{{ selectDatabase()
  public function selectDatabase($strDatabase)
  {
    $this->m_db['Database'] = $strDatabase;
  }
  // }}}
  // {{{ setCharset()
  public function setCharset($strCharset)
  { 
  }
  // }}}
  // {{{ setService()
  public function setService($strService)
  {
    $this->m_strService = $strService;
  }
  // }}}
}
?>
