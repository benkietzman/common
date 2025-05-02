<?php
//////////////////////////////////////////////////////////////////////////
// Bridge Query
// -------------------
// begin      : 2018-08-08
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
//////////////////////////////////////////////////////////////////////////
// {{{ includes
include_once('DatabaseQuery.php');
include_once(dirname(__FILE__).'/../bridge/Bridge.php');
// }}}
//! BridgeQuery Class
/*!
* Provides functions for performing database queries through the Bridge.
*/
class BridgeQuery extends DatabaseQuery
{
  // {{{ variables
  private $m_bridge;
  private $m_strQuery;
  private $m_bFirst;
  private $m_bExecute;
  private $m_unRow;
  private $m_unRows;
  // }}}
  // {{{ __construct()
  public function __construct($bError = false)
  {
    parent::__construct($bError);
    $this->m_bFirst = true;
    $this->m_bExecute = true;
    $this->m_bridge = new common\Bridge;
    $this->m_query = false;
    $this->m_unRow = 0;
    $this->m_unRows = 0;
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
    $bQuery = ((strlen($this->m_strQuery) >= 7 && strtolower(substr($this->m_strQuery, 0, 7)) == 'select ')?true:false);
    $backtrace = $this->m_error->setBacktrace();
    $request = array();
    $request['Section'] = 'database';
    $request['Database'] = $this->m_db['Database'];
    $request['Password'] = $this->m_db['Password'];
    $request['User'] = $this->m_db['User'];
    $request['Request'] = array();
    $request['Request'][(($bQuery)?'Query':'Update')] = $this->m_strQuery;
    $response = null;
    if ($this->m_bridge->request($request, $response))
    {
      if (isset($response['Status']) && $response['Status'] == 'okay')
      {
        $this->m_bExecute = false;
        if ($bQuery)
        {
          if (isset($response['Response']) && is_array($response['Response']))
          {
            $this->m_query = $response['Response'];
            $this->m_unRows = sizeof($this->m_query);
          }
          else
          {
            $this->m_query = true;
          }
        }
        else
        {
          $this->m_query = true;
        }
      }
      else if (isset($response['Error']) && is_array($response['Error']) && isset($response['Error']['Message']) && $response['Error']['Message'] != '')
      {
        $this->m_strError = $response['Error']['Message'];
      }
      else
      {
        $this->m_strError = 'Encountered an unknown error.';
      }
    }
    else
    {
      $this->m_strError = $this->m_bridge->getError();
    }
    unset($request);
    unset($response);

    return $this->m_query;
  }
  // }}}
  // {{{ numRows()
  public function numRows()
  {
    $backtrace = $this->m_error->setBacktrace();
    return $this->m_unRows;
  }
  // }}}
  // {{{ fetch()
  public function fetch($strMode = "both")
  {
    $result = false;
    $backtrace = $this->m_error->setBacktrace();
    if ($this->m_bFirst)
    {
      $this->m_bFirst = false;
      if ($this->m_bExecute)
      {
        $this->execute();
      }
    }
    if ($this->m_query !== false && $this->m_unRow < $this->m_unRows)
    {
      $result = $this->m_query[$this->m_unRow++];
    }
    return $result;
  }
  // }}}
  // {{{ fetchAll()
  public function fetchAll($strMode = "both")
  {
    return $this->m_query;
  }
  // }}}
  // {{{ free()
  public function free()
  {
    if ($this->m_query)
    {
      unset($this->m_query);
    }
  }
  // }}}
}
?>
