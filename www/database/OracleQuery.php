<?php
// vim600: fdm=marker
//////////////////////////////////////////////////////////////////////////
// Oracle Query
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
//! OracleQuery Class
/*!
* Provides functions for performing Oracle database queries.
*/
class OracleQuery extends DatabaseQuery
{
  // {{{ variables
  private $m_bFirst;
  private $m_bPreFetch;
  private $m_nPreFetch;
  private $m_bExecute;
  // }}}
  // {{{ __construct()
  public function __construct($bError = false)
  {
    parent::__construct($bError);
    $this->m_bFirst = true;
    $this->m_bPreFetch = false;
    $this->m_bExecute = true;
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
    $this->m_query = oci_parse($this->m_db, $strQuery);
    if (($e = oci_error($this->m_query)) !== false)
    {
      $this->m_strError = $e['message'];
    }
  }
  // }}}
  // {{{ setPreFetch()
  public function setPreFetch($nPreFetch = 1000)
  {
    $this->m_bPreFetch = true;
    $this->m_nPreFetch = $nPreFetch;
  }
  // }}}
  // {{{ bind()
  public function bind($strBind, $strValue, $bUseQuotes = true)
  {
    $backtrace = $this->m_error->setBacktrace();
    oci_bind_by_name($this->m_query, $strBind, $strValue, -1);
  }
  // }}}
  // {{{ execute()
  public function execute()
  {
    $bResult = false;
    $backtrace = $this->m_error->setBacktrace();
    $this->m_bExecute = false;
    $bResult = oci_execute($this->m_query);
    if (($e = oci_error($this->m_query)) !== false)
    {
      $this->m_strError = $e['message'];
    }
    return $bResult;
  }
  // }}}
  // {{{ getColumns()
  public function getColumns()
  {
    $backtrace = $this->m_error->setBacktrace();
    $column = array(array());
    $this->execute();
    $nColumns = ocinumcols($this->m_query);
    for ($i = 0; $i < $nColumns; $i++)
    {
      $column[$i]['name'] = ocicolumnname($this->m_query, $i + 1);
      $column[$i]['type'] = ocicolumntype($this->m_query, $i + 1);
      $column[$i]['size'] = ocicolumnsize($this->m_query, $i + 1);
    }
    return $column;
  }
  // }}}
  // {{{ numRows()
  public function numRows()
  {
    $backtrace = $this->m_error->setBacktrace();
    return oci_num_rows($this->m_query);
  }
  // }}}
  // {{{ fetch()
  public function fetch($strMode = "both")
  {
    $backtrace = $this->m_error->setBacktrace();
    switch ($strMode)
    {
      case "assoc" : $nMode = OCI_ASSOC; break;
      case "num"   : $nMode = OCI_NUM; break;
      default      : $nMode = OCI_BOTH; break;
    }
    if ($this->m_bFirst)
    {
      $this->m_bFirst = false;
      if ($this->m_bPreFetch)
      {
        oci_set_prefetch($this->m_query, $this->m_nPreFetch);
      }
      if ($this->m_bExecute)
      {
        $this->execute();
      }
    }
    $row = oci_fetch_array($this->m_query, $nMode);
    if (($e = oci_error($this->m_query)) !== false)
    {
      $this->m_strError = $e['message'];
    }
    $result = array();
    if (is_array($row))
    {
      foreach ($row as $key => $value)
      {
        $result[((is_numeric($key))?$key:strtolower($key))] = $value;
      }
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
      oci_free_statement($this->m_query);
    }
    $this->m_bFirst = true;
  }
  // }}}
}
?>
