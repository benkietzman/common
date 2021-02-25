<?php
// vim600: fdm=marker
//////////////////////////////////////////////////////////////////////////
// Database Query
// -------------------
// begin                : 2005-09-19
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
include_once(dirname(__FILE__).'/../error_handler/ErrorHandler.php');
// }}}
//! DatabaseQuery Class
/*!
* Provides functions for performing database queries.
*/
class DatabaseQuery
{
  // {{{ variables
  protected $m_error;
  protected $m_bError;
  protected $m_db;
  protected $m_query;
  protected $m_strError;
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
  // {{{ getError()
  public function getError()
  {
    return $this->m_strError;
  }
  // }}}
  // {{{ getQueryHandle()
  public function getQueryHandle()
  {
    return $this->m_query;
  }
  // }}}
  // {{{ setDatabase()
  public function setDatabase($db)
  {
    $this->m_db = $db;
  }
  // }}}
}
?>
