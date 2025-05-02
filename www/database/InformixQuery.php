<?php
//////////////////////////////////////////////////////////////////////////
// Informix Query
// -------------------
// begin      : 2005-09-19
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
//////////////////////////////////////////////////////////////////////////
// {{{ includes
include_once('DatabaseQuery.php');
// }}}
//! InformixQuery Class
/*!
* Provides functions for performing Informix database queries.
*/
class InformixQuery extends DatabaseQuery
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
    $this->m_query = ifx_query($strQuery, $this->m_db);

    return $this->m_query;
  }
  public function query($strQuery)
  {
    $this->parse($strQuery);
  }
  // }}}
  // {{{ fetch()
  public function fetch()
  {
    $backtrace = $this->m_error->setBacktrace();
    return ifx_fetch_row($this->m_query);
  }
  // }}}
  // {{{ fetchAll()
  public function fetchAll()
  {
    $nIndex = 0;
    $results = array();
    while (($getRow = $this->fetch()))
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
