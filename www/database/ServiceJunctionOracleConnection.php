<?php
//////////////////////////////////////////////////////////////////////////
// Service Junction (oracle) Connection
// -------------------
// begin      : 2024-03-13
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
//////////////////////////////////////////////////////////////////////////
// {{{ includes
include_once('ServiceJunctionConnection.php');
include_once('ServiceJunctionQuery.php');
// }}}
//! ServiceJunctionOracleConnection Class
/*!
* Provides functions for establishing database connections through the mysql service of the Service Junction.
*/
class ServiceJunctionOracleConnection extends ServiceJunctionConnection
{
  // {{{ __construct()
  public function __construct($bError = false)
  {
    parent::__construct($bError);
    $this->setService('oracle');
  }
  // }}}
  // {{{ __destruct()
  public function __destruct()
  {
    parent::__destruct();
  }
  // }}}
  // {{{ connect()
  public function connect($strSchema, $strPassword, $strTnsName, $strIgnore = null)
  {
    parent::__construct($strSchema, $strPassword, null, $strTnsName);
  }
  // }}}
}
?>
