<?php
//////////////////////////////////////////////////////////////////////////
// Database Connection
// -------------------
// begin      : 2005-09-19
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
//////////////////////////////////////////////////////////////////////////
// {{{ includes
include_once(dirname(__FILE__).'/../error_handler/ErrorHandler.php');
include_once('DatabaseQuery.php');
// }}}
//! DatabaseConnection Class
/*!
* Provides functions for establishing database connections.
*/
class DatabaseConnection
{
  // {{{ variables
  protected $m_error;
  protected $m_bError;
  protected $m_db;
  protected $m_strError;
  protected $m_strUser;
  protected $m_strPassword;
  protected $m_strServer;
  protected $m_strDatabase;
  // }}}
  // {{{ __construct()
  public function __construct($bError = false)
  {
    $this->m_error = new ErrorHandler();
    $this->m_bError = $bError;
    if ($this->m_bError)
    {
      $this->m_error->enable();
    }
  }
  // }}}
  // {{{ __destruct()
  public function __destruct()
  {
  }
  // }}}
  // {{{ error()
  public function error()
  {
    return $this->m_error;
  }
  // }}}
  // {{{ errorExist()
  public function errorExist()
  {
    return !empty($this->m_strError);
  } 
  // }}}
  // {{{ getDatabase()
  public function getDatabase()
  {
    return $this->m_strDatabase;
  }
  // }}}
  // {{{ getDatabaseHandle()
  public function getDatabaseHandle()
  {
    return $this->m_db;
  }
  // }}}
  // {{{ getError()
  public function getError()
  {
    return $this->m_strError;
  }
  // }}}
  // {{{ getPassword()
  public function getPassword()
  {
    return $this->m_strPassword;
  }
  // }}}
  // {{{ getServer()
  public function getServer()
  {
    return $this->m_strServer;
  }
  // }}}
  // {{{ getUser()
  public function getUser()
  {
    return $this->m_strUser;
  }
  // }}}
  // {{{ free()
  public function free(&$query)
  {
    if ($query)
    {
      $query->free();
      unset($query);
    }
  }
  // }}}
}
?>
