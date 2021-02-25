<?php
// vim600: fdm=marker
//////////////////////////////////////////////////////////////////////////
// Pdo Query
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
//! PdoQuery Class
/*!
* Provides functions for performing MySQL database queries.
*/
class PdoQuery extends DatabaseQuery
{
  // {{{ variables
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
    return $this->m_query;
  }
  // }}}
  // {{{ parse()
  public function parse($strQuery)
  {
    $backtrace = $this->m_error->setBacktrace();
    try{
      $this->m_query = $this->m_db->prepare($strQuery);
    } catch(PDOException $e){
      $this->m_strErro = $e->getMessage();
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
  public function bind($strBind, $strValue, $type)
  {
    if (is_null($type)) {
      switch (true) {
        case is_int($strValue):
          $type = PDO::PARAM_INT;
          break;
        case is_bool($strValue):
          $type = PDO::PARAM_BOOL;
          break;
        case is_null($strValue):
          $type = PDO::PARAM_NULL;
          break;
        default:
          $type = PDO::PARAM_STR;
      }
    }
    $this->m_query->bindValue($strBind, $strValue, $type);
  }
  // }}}
  // {{{ execute()
  public function execute()
  {
    $backtrace = $this->m_error->setBacktrace();
    try {
      $this->m_query->execute();
      $this->m_bExecute = false;
    } catch(PDOException $e){
      $this->m_strError = $e->getMessage();
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
      case "assoc" : $nMode = PDO::FETCH_ASSOC; break;
      case "num"   : $nMode = PDO::FETCH_NUM; break;
      default      : $nMode = PDO::FETCH_BOTH; break;
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
      $result = $this->m_query->fetch($nMode);
    }
    return $result;
  }
  // }}}
  // {{{ fetchAll()
  public function fetchAll($strMode = "both")
  {
    switch ($strMode)
    {
      case "assoc" : $nMode = PDO::FETCH_ASSOC; break;
      case "num"   : $nMode = PDO::FETCH_NUM; break;
      default      : $nMode = PDO::FETCH_BOTH; break;
    }
    return $this->fetchAll($nMode);
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
