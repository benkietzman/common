<?php
//////////////////////////////////////////////////////////////////////////
// Service Junction Query
// -------------------
// begin      : 2024-03-13
// copyright  : kietzman.org
// email      : ben@kietzman.org
//////////////////////////////////////////////////////////////////////////
// {{{ includes
include_once('DatabaseQuery.php');
include_once(dirname(__FILE__).'/../servicejunction/ServiceJunction.php');
// }}}
//! RadialQuery Class
/*!
* Provides functions for performing database queries through the Service Junction.
*/
class ServiceJunctionQuery extends DatabaseQuery
{
  // {{{ variables
  private $m_junction;
  private $m_bFirst;
  private $m_bExecute;
  private $m_strQuery;
  private $m_strService;
  private $m_unRow;
  private $m_unRows;
  // }}}
  // {{{ __construct()
  public function __construct($bError = false)
  {
    parent::__construct($bError);
    $this->m_bFirst = true;
    $this->m_bExecute = true;
    $this->m_junction = new ServiceJunction;
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
    $req = array();
    $request = array();
    $request['Service'] = $this->m_strService;
    if ($this->m_strService == 'mssql')
    {
      $request['Password'] = $this->m_db['Password'];
      $request['Server'] = $this->m_db['Server'];
      $request['User'] = $this->m_db['User'];
    }
    else if ($this->m_strService == 'mysql')
    {
      $request['Database'] = $this->m_db['Database'];
      $request['Password'] = $this->m_db['Password'];
      $request['Server'] = $this->m_db['Server'];
      $request['User'] = $this->m_db['User'];
    }
    else if ($this->m_strService == 'oracle')
    {
      $request['Password'] = $this->m_db['Password'];
      $request['Schema'] = $this->m_db['User'];
      $request['tnsName'] = $this->m_db['Database'];
    }
    $request[(($bQuery)?'Query':'Update')] = $this->m_strQuery;
    $req[] = json_encode($request);
    unset($request);
    $res = null;
    if ($this->m_junction->request($req, $res))
    {
      if (sizeof($res) > 0)
      {
        $status = json_decode($res[0], true);
        if (isset($status['Status']) && $status['Status'] == 'okay')
        {
          $this->m_bExecute = false;
          if ($bQuery)
          {
            if (sizeof($res) > 1)
            {
              $this->m_query = array();
              for ($i = 1; $i < sizeof($res); $i++)
              {
                $this->m_query[] = json_decode($res[$i], true);
              }
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
        else if (isset($status['Error']) && $status['Error'] != '')
        {
          $this->m_strError = $status['Error'];
        }
        else
        {
          $this->m_strError = 'Encountered an unknown error.';
        }
        unset($status);
      }
      else
      {
        $this->m_strError = 'Failed to receive a response.';
      }
    }
    else
    {
      $this->m_strError = $this->m_radial->getError();
    }
    unset($req);
    unset($res);

    return $this->m_query;
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
  // {{{ getQuery()
  public function getQuery()
  {
    return $this->m_strQuery;
  }
  // }}}
  // {{{ numRows()
  public function numRows()
  {
    $backtrace = $this->m_error->setBacktrace();
    return $this->m_unRows;
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
  // {{{ setService()
  public function setService($strService)
  {
    $this->m_strService = $strService;
  }
  // }}}
}
?>
