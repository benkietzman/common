<?php
// vim600: fdm=marker
//////////////////////////////////////////////////////////////////////////
// MySql Query
// -------------------
// begin                : 09/19/2005
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
include_once("DatabaseQuery.php");
// }}}
//! MySqlQuery Class
/*!
* Provides functions for performing MySQL database queries.
*/
class MySqlQuery extends DatabaseQuery
{
  // {{{ variables
  private $m_strQuery;
  private $m_bFirst;
  private $m_bExecute;
  // }}}
  // {{{ __construct()
  public function __construct($bError = false)
  {
    parent::__construct($bError);
    $this->m_bFirst = true;
    $this->m_bExecute = true;
  }
  // }}}
  // {{{ __destruct()
  public function __destruct()
  {
    parent::__destruct();
  }
  // }}}
  // {{{ getQuery()
  public function getQuery()
  {
    return $this->m_strQuery;
  }
  // }}}
  // {{{ parse()
  public function parse($strQuery)
  {
    $this->m_strQuery = $strQuery;
  }
  // }}}
  // {{{ setPreFetch()
  public function setPreFetch($nPreFetch = 1000)
  {
  }
  // }}}
  // {{{ bind()
  public function bind($strBind, $strValue, $bUseQuotes = true)
  {
    $this->m_strQuery = str_replace($strBind, (($bUseQuotes)?'\'':'').$strValue.(($bUseQuotes)?'\'':''), $this->m_strQuery);
  }
  // }}}
  // {{{ execute()
  public function execute()
  {
    $backtrace = $this->m_error->setBacktrace();
    if (($this->m_query = $this->m_db->query($this->m_strQuery)) !== false)
    {
      $this->m_bExecute = false;
    }
    else
    { 
      $this->m_strError = $this->m_db->error;
    }

    return $this->m_query;
  }
  // }}}
  // {{{ numRows()
  public function numRows()
  {
    $backtrace = $this->m_error->setBacktrace();
    return $this->m_query->num_rows;
  }
  // }}}
  // {{{ fetch()
  public function fetch($strMode = "both")
  {
    $result = false;
    $backtrace = $this->m_error->setBacktrace();
    switch ($strMode)
    {
      case "assoc" : $nMode = MYSQLI_ASSOC; break;
      case "num"   : $nMode = MYSQLI_NUM; break;
      default      : $nMode = MYSQLI_BOTH; break;
    }
    if ($this->m_bFirst)
    {
      $this->m_bFirst = false;
      if ($this->m_bExecute)
      {
        $this->execute();
      }
    }
    if ($this->m_query !== false)
    {
      $result = $this->m_query->fetch_array($nMode);
    }
    return $result;
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
      $this->m_query->free();
    }
  }
  // }}}
}
?>
