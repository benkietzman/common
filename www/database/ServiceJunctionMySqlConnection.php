<?php
// vim600: fdm=marker
//////////////////////////////////////////////////////////////////////////
// Service Junction (mysql) Connection
// -------------------
// begin                : 2024-03-13
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
