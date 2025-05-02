<?php
//////////////////////////////////////////////////////////////////////////
// Service Junction (mysql) Connection
// -------------------
// begin      : 2024-03-13
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
//////////////////////////////////////////////////////////////////////////
// {{{ includes
include_once('ServiceJunctionConnection.php');
include_once('ServiceJunctionQuery.php');
// }}}
//! ServiceJunctionMySqlConnection Class
/*!
* Provides functions for establishing database connections through the mysql service of the Service Junction.
*/
class ServiceJunctionMySqlConnection extends ServiceJunctionConnection
{
  // {{{ __construct()
  public function __construct($bError = false)
  {
    parent::__construct($bError);
    $this->setService('mysql');
  }
  // }}}
  // {{{ __destruct()
  public function __destruct()
  {
    parent::__destruct();
  }
  // }}}
  // {{{ connect()
  public function connect($strUser, $strPassword, $strServer, $strDatabase = null)
  {
    parent::connect($strUser, $strPassword, $strServer, $strDatabase);
  }
  // }}}
}
?>
