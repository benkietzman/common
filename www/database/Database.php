<?php
//////////////////////////////////////////////////////////////////////////
// Database
// -------------------
// begin      : 2005-09-19
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
//////////////////////////////////////////////////////////////////////////
// {{{ includes
include_once('BridgeConnection.php');
include_once('DatabaseConnection.php');
include_once('InformixConnection.php');
include_once('MySqlConnection.php');
include_once('OracleConnection.php');
include_once('PdoConnection.php');
include_once('PostgreConnection.php');
include_once('RadialConnection.php');
include_once('ServiceJunctionConnection.php');
include_once('ServiceJunctionMsSqlConnection.php');
include_once('ServiceJunctionMySqlConnection.php');
include_once('ServiceJunctionOracleConnection.php');
// }}}
//! Database Class
/*!
* Provides functions for accessing different databases in a uniform manner.
*/
class Database
{
  // {{{ variables
  private $m_strDatabase;
  private $m_strSchema;
  private $m_strPassword;
  private $m_strTnsName;
  private $m_bError;
  private $m_bPersistant;
  private $m_bPreFetch;
  private $m_nPreFetch;
  // }}}
  // {{{ __construct()
  public function __construct($strDatabase = 'MySql', $bError = false)
  {
    $this->m_strDatabase = $strDatabase;
    $this->m_bError = $bError;
    $this->m_bPersistant = false;
    $this->m_bPreFetch = false;
  }
  // }}}
  // {{{ __destruct()
  public function __destruct()
  {
  }
  // }}}
  // {{{ setLogin()
  public function setLogin($strSchema, $strPassword, $strTnsName, $bPersistant = false)
  {
    $this->m_strSchema = $strSchema;
    $this->m_strPassword = $strPassword;
    $this->m_strTnsName = $strTnsName;
    $this->m_bPersistant = $bPersistant;
  }
  // }}}
  // {{{ connect()
  public function connect($strSchema = NULL, $strPassword = NULL, $strTnsName = NULL, $bPersistant = NULL)
  {
    eval("\$connection = new ".$this->m_strDatabase."Connection(".$this->m_bError.");");
    if ($strSchema == NULL)
    {
      $strSchema = $this->m_strSchema;
    }
    if ($strPassword == NULL)
    {
      $strPassword = $this->m_strPassword;
    }
    if ($strTnsName == NULL)
    {
      $strTnsName = $this->m_strTnsName;
    }
    if ($bPersistant == NULL)
    {
      $bPersistant = $this->m_bPersistant;
    }
    $connection->connect($strSchema, $strPassword, $strTnsName, $bPersistant);
    if ($this->m_bPreFetch)
    {
      $connection->setPreFetch($this->m_nPreFetch);
    }
    return $connection;
  }
  // }}}
  // {{{ setPreFetch()
  public function setPreFetch($nPreFetch = 1000)
  {
    $this->m_bPreFetch = true;
    $this->m_nPreFetch = $nPreFetch;
  }
  // }}}
  // {{{ disconnect()
  public function disconnect(&$connection)
  {
    if ($connection)
    {
      $connection->disconnect();
      unset($connection);
    }
  }
  // }}}
}
?>
