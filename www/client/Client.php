<?php
// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ed Messman
// begin      : 2018-08-03
// copyright  : kietzman.org
///////////////////////////////////////////
// {{{ includes
require_once(dirname(__FILE__).'/../servicejunction/ServiceJunction.php');
require_once(dirname(__FILE__).'/../bridge/Bridge.php');
// }}}
// {{{ Client
class Client
{
  // {{{ variables
  private $strAppl   = '';
  private $strError  = '';
  private $strUser   = '';
  private $strPass   = '';
  private $strType   = '';
  private $SJ;
  private $BRIDGE;
  // }}}
  // {{{ construct()
  public function __construct ( $strType, $strAppl, $strUser, $strPass)
  {
    if ( isset( $strType) && isset( $strAppl) && isset( $strUser) && isset( $strPass))
    {
      $strType = strtolower( $strType);
      if ( $strType == 'bridge' || $strType == 'junction' )
      {
        $this->strType = $strType;
        $this->strUser = $strUser;
        $this->strPass = $strPass;
        $this->strAppl = $strAppl;
        if ( $strType == 'junction')
        {
          $this->SJ = new ServiceJunction;
          $this->SJ->setApplication ( $strAppl );
          if ($this->SJ->getError() != '')
          {
            $this->strError = $this->SJ->getError();
          }
        }
        else
        {
          $this->BRIDGE = new common\Bridge;
          if ($this->BRIDGE->getError() != '')
          {
            $this->strError = $this->BRIDGE->getError();
          }
        }
      }
      else
      {
        $this->strError = 'Please provide a valid Type:  bridge, junction.';
      }
    }
    else
    {
      $this->strError = 'Please provide Type, Appl, User, Pass.';
    }
  }
  // }}}
  // {{{ destruct
  public function __destruct()
  {
    // nothing yet
  }
  // }}}
  // {{{ getError()
  public function getError()
  {
    return $this->strError;
  }
  // }}}
  // {{{ getType()
  public function getType()
  {
    return $this->strType;
  }
  // }}}
  // {{{ request()
  public function request($req, $request, &$response)
  {
    $bResult = false;

    $response = array();
    if ( $this->strType == 'bridge' )
    {
      $subRequest = $req;
      if (!isset($req['Section']) && isset($req['Service']))
      {
        $req['Section'] = $req['Service'];
      }
      $subRequest['User']     = $this->strUser;
      $subRequest['Password'] = $this->strPass;
      $subRequest['reqApp']   = $this->strAppl;
      $subRequest['Request']  = $request;
      if ($this->BRIDGE->request($subRequest, $subResponse))
      {
        if (isset($subResponse['Status']) && $subResponse['Status'] == 'okay')
        {
          $bResult = true;
          if (isset($subResponse['Response']))
          {
            $response = $subResponse['Response'];
          }
        }
        else if (isset($subResponse['Error']) && isset($subResponse['Error']['Message']) && $subResponse['Error']['Message'] != '')
        {
          $this->strError = $subResponse['Error']['Message'];
        }
      }
      else
      {
        $this->strError = $this->BRIDGE->getError();
      }
    }
    else if ( $this->strType == 'junction' )
    {
      $in = array();
      if (!isset($req['Service']) && isset($req['Section']))
      {
        $req['Service'] = $req['Section'];
      }
      $req['User']     = $this->strUser;
      $req['Password'] = $this->strPass;
      $req['reqApp']   = $this->strAppl;
      $in[] = json_encode($req);
      if (is_array($request) && sizeof($request) > 0)
      {
        $in[] = json_encode($request);
      }
      $out = null;
      if ($this->SJ->request($in, $out))
      {
        $nSize = sizeof($out);
        if ($nSize >= 1)
        {
          $status = json_decode($out[0], true);
          if (isset($status['Status']) && $status['Status'] == 'okay')
          {
            $bResult = true;
            if ($nSize == 1)
            {
              $response = $status;
              if (isset($response['Password']))
              {
                unset($response['Password']);
              }
            }
            else if ($nSize == 2)
            {
              $response = json_decode($out[1], true);
            }
            else
            {
              for ($i = 1; $i < $nSize; $i++)
              {
                $response[] = json_decode($out[$i], true);
              }
            }
          }
          else if (isset($status['Error']) && $status['Error'] != '')
          {
            $this->strError = $status['Error'];
          }
          else
          {
            $this->strError = 'Encountered an unknown error.';
          }
          unset($status);
        }
        else
        {
          $this->strError = 'Failed to receive response.';
        }
      }
      else
      {
        $this->strError = $this->SJ->getError();
      }
      unset($in);
      unset($out);
    }
    else
    {
      $this->strError = 'Please provide a valid Type:  bridge, junction.';
    }

    return $bResult;
  }
  // }}}
  // {{{ setError()
  public function setError($strError)
  {
    $this->strError = $strError;
  }
  // }}}
  // {{{ setType()
  public function setType($strType)
  {
    $this->strType = $strType;
  }
  // }}}
}
// }}}
?>
