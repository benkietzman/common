<?php
//////////////////////////////////////////////////////////////////////////
// Service Junction (mssql) Connection
// -------------------
// begin      : 2024-03-13
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
//////////////////////////////////////////////////////////////////////////
// {{{ includes
include_once('ServiceJunctionConnection.php');
include_once('ServiceJunctionQuery.php');
// }}}
//! ServiceJunctionMsSqlConnection Class
/*!
* Provides functions for establishing database connections through the mysql service of the Service Junction.
*/
class ServiceJunctionMsSqlConnection extends ServiceJunctionConnection
{
  // {{{ __construct()
  public function __construct($bError = false)
  {
    parent::__construct($bError);
    $this->setService('mssql');
  }
  // }}}
  // {{{ __destruct()
  public function __destruct()
  {
    parent::__destruct();
  }
  // }}}
  // {{{ connect()
  public function connect($strUser, $strPassword, $strServer, $strIgnore = null)
  {
    parent::__construct($strUser, $strPassword, $strServer, $strIgnore);
  }
  // }}}
}
?>
