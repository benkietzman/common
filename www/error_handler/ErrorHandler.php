<?php
// vim600: fdm=marker
//////////////////////////////////////////////////////////////////////////
// Error Handler
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

class ErrorHandler
{
  // {{{ variables
  private $m_bEnabled;
  private $m_bNoticesEnabled;
  private $m_bWarningsEnabled;
  private $m_nErrorIndex;
  private $m_errorList;
  // }}}
  // {{{ __construct()
  public function __construct()
  {
    $this->m_bEnabled = false;
    $this->m_bNoticesEnabled = true;
    $this->m_bWarningsEnabled = true;
    $this->m_nErrorIndex = array();
    $this->m_nErrorIndex['warning'] = 0;
    $this->m_nErrorIndex['notice'] = 0;
    $this->m_nErrorIndex['unknown'] = 0;
    $this->m_errorList = array();
  }
  // }}}
  // {{{ __destruct()
  public function __destruct()
  {
    if ($this->m_bEnabled)
    {
      restore_error_handler();
    }
    unset($this->m_errorList);
  }
  // }}}
  // {{{ enable()
  public function enable()
  {
    $this->m_bEnabled = true;
    error_reporting(E_ALL);
    set_error_handler(array($this, "handler"));
  }
  // }}}
  // {{{ disable()
  public function disable()
  {
    $this->m_bEnabled = false;
    error_reporting(0);
    set_error_handler(array($this, "handler"));
  }
  // }}}
  // {{{ enableNotices()
  public function enableNotices()
  {
    $this->m_bNoticesEnabled = true;
  }
  // }}}
  // {{{ disableNotices()
  public function disableNotices()
  {
    $this->m_bNoticesEnabled = false;
  }
  // }}}
  // {{{ enableWarnings()
  public function enableWarnings()
  {
    $this->m_bWarningsEnabled = true;
  }
  // }}}
  // {{{ disableWarnings()
  public function disableWarnings()
  {
    $this->m_bWarningsEnabled = false;
  }
  // }}}
  // {{{ messagesExist()
  public function messagesExist($strType = "all")
  {
    $bFound = false;

    if ($this->m_bEnabled)
    {
      foreach ($this->m_errorList as $key => $value)
      {
        if (($strType == "all" || $strType == $key) && sizeof($value) > 0)
        {
          $bFound = true;
        }
      }
    }

    return $bFound;
  }
  // }}}
  // {{{ showMessages()
  public function showMessages($strType = "all")
  {
    if ($this->m_bEnabled && sizeof($this->m_errorList) > 0)
    {
      echo '<div style="border-style:solid;border-width:1px;border-color:black;background:white;color:black;">';
      foreach ($this->m_errorList as $key => $value)
      {
        if ($strType == "all" || $strType == $key)
        {
          if (sizeof($value))
          {
            echo '<p>';
            echo '<center>';
            echo '<b>'.strtoupper($key).'</b><br>';
            for ($i = 0; $i < sizeof($value); $i++)
            {
              echo $value[$i][0];
            }
            echo '</center>';
            echo '</p>';
          }
        }
      }
      echo '</div>';
    }
  }
  // }}}
  // {{{ setBacktrace()
  public function setBacktrace()
  {
    $storage = debug_backtrace();
    $backtrace  = '<table border="1">';
    for ($i = 0; $i < sizeof($storage); $i++)
    {
      $backtrace .= '<tr><td>Line: '.(isset($storage[$i]['line'])?$storage[$i]['line']:'').'</td><td>'.(isset($storage[$i]['file'])?$storage[$i]['file']:'').'</td><td>'.(isset($storage[$i]['class'])?$storage[$i]['class']:'').(isset($storage[$i]['type'])?$storage[$i]['type']:'').(isset($storage[$i]['function'])?$storage[$i]['function']:'').'(';
      $bFirst = true;
      if (isset($storage[$i]['args']))
      {
        for ($j = 0; $j < sizeof($storage[$i]['args']); $j++)
        {
          if ($bFirst)
          {
            $bFirst = false;
          }
          else
          {
            $backtrace .= ', ';
          }
          if (!is_object($storage[$i]['args'][$j]))
          {
            //$backtrace .= $storage[$i]['args'][$j];
          }
          else
          {
            $backtrace .= '[object found]';
          }
        }
      }
      $backtrace .= ')';
      $backtrace .= '</td></tr>';
    }
    $backtrace .= '</table>';
    unset($storage);
    unset($i);
    unset($j);
    unset($bFirst);
    return $backtrace;
  }
  // }}}
  // {{{ handler()
  public function handler($errno, $errstr, $errfile, $errline)
  {
    if ($errno == E_ERROR || $errno == E_CORE_ERROR || $errno == E_COMPILE_ERROR || $errno == E_USER_ERROR)
    {
      echo "<br><br>";
      echo "<hr>";
      if ($this->messagesExist())
      {
        $this->showMessages();
      }
      echo "<b>ERROR:</b> [".$errno."] ".$errstr." in <b>".$errfile."</b> on line <b>".$errline."</b><br>";
      exit(1);
    }
    else
    {
      $bStore = true;
      switch ($errno)
      {
        case E_WARNING         :
        case E_COMPILE_WARNING :
        case E_USER_WARNING    : $bStore = ($this->m_bWarningsEnabled)?true:false; $strType = "warning"; break;
        case E_NOTICE          :
        case E_USER_NOTICE     : $bStore = ($this->m_bNoticesEnabled)?true:false; $strType = "notice"; break;
        default                : $strType = "unknown"; break;
      }
      if ($bStore)
      {
        if (!isset($this->m_errorList[$strType]))
        {
          $this->m_errorList[$strType] = array();
        }
        $this->m_errorList[$strType][$this->m_nErrorIndex[$strType]++] = $errstr.' in <b>'.$errfile.'</b> on line <b>'.$errline.'</b>';
      }
    }
  }
  // }}}
}
?>
