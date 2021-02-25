<?php
// vim600: fdm=marker
//////////////////////////////////////////////////////////////////////////
// Postgre Query
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
include_once('DatabaseQuery.php');
// }}}
//! PostgreQuery Class
/*!
* Provides functions for performing Postgre database queries.
*/
class PostgreQuery extends DatabaseQuery
{
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
  // {{{ parse()
  public function parse($strQuery)
  {
    $backtrace = $this->m_error->setBacktrace();
    $this->m_query = pg_query($this->m_db, pg_escape_string($strQuery));

    return $this->m_query;
  }
  public function query($strQuery)
  {
    $this->parse($strQuery);
  }
  // }}}
  // {{{ fetch()
  public function fetch($strMode = "both")
  {
    $backtrace = $this->m_error->setBacktrace();
    switch ($strMode)
    {
      case "assoc" : $nMode = PGSQL_ASSOC; break;
      case "num"   : $nMode = PGSQL_NUM; break;
      default      : $nMode = PGSQL_BOTH; break;
    }
    return pg_fetch_array($this->m_query, NULL, $nMode);
  }
  // }}}
  // {{{ fetchAll()
  public function fetchAll($strMode = "both")
  {
    $nIndex = 0;
    $results = array();
    while (($getRow = $this->fetch($strMode)))
    {
      $results[$nIndex++] = $getRow;
    }
    return $results;
  }
  // }}}
  // {{{ free()
  public function free()
  {
    if ($this->m_query)
    {
      ifx_free_result($this->m_query);
    }
  }
  // }}}
}
?>
