<?php
// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2016-02-13
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
// {{{ includes
include_once(dirname(__FILE__).'/auth/Secure.php');
include_once(dirname(__FILE__).'/database/CommonDatabase.php');
include_once(dirname(__FILE__).'/servicejunction/ServiceJunction.php');
// }}}
class Central extends Secure
{
  // {{{ variables
  protected $m_centraldb;
  protected $m_database;
  protected $m_junction;
  // }}}
  // {{{ __construct()
  public function __construct($strUser, $strPassword, $strServer, $strDatabase)
  {
    parent::__construct($strUser, $strPassword, $strServer, $strDatabase);
    $this->setApplication('Central');
    if ($strServer != '')
    {
      $this->m_database = new CommonDatabase('MySql');
      $this->m_centraldb = $this->m_database->connect($strUser, $strPassword, $strServer);
      $this->m_centraldb->selectDatabase($strDatabase);
    }
    else
    {
      $this->m_database = new CommonDatabase('Bridge');
      $this->m_centraldb = $this->m_database->connect($strUser, $strPassword, $strDatabase);
    }
    $this->m_junction = new ServiceJunction;
    $this->m_junction->setApplication('Central');
  }
  // }}}
  // {{{ __destruct()
  public function __destruct()
  {
    $this->m_database->disconnect($this->m_centraldb);
    parent::__destruct();
  }
  // }}}
  // {{{ accountType()
  public function accountType($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $getAccountType = $this->m_centraldb->parse('select id, type, description from account_type where id = '.$request['id']);
      if ($getAccountType->execute())
      {
        if (($getAccountTypeRow = $getAccountType->fetch('assoc')))
        {
          $bResult = true;
          $response = $getAccountTypeRow;
        }
      }
      else
      {
        $strError = $getAccountType->getError();
      }
      $this->m_centraldb->free($getAccountType);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ accountTypes()
  public function accountTypes($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    $getAccountType = $this->m_centraldb->parse('select id, type, description from account_type order by type');
    if ($getAccountType->execute())
    {
      $bResult = true;
      while (($getAccountTypeRow = $getAccountType->fetch('assoc')))
      {
        $response[] = $getAccountTypeRow;
      }
    }
    else
    {
      $strError = $getAccountType->getError();
    }
    $this->m_centraldb->free($getAccountType);

    return $bResult;
  }
  // }}}
  // {{{ application()
  public function application($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if ((isset($request['id']) && $request['id'] != '') || (isset($request['name']) && $request['name'] != ''))
    {
      $getApplication = $this->m_centraldb->parse('select id, name, date_format(creation_date, \'%Y-%m-%d\') creation_date, notify_priority_id, website, login_type_id, secure_port, auto_register, account_check, dependable, date_format(retirement_date, \'%Y-%m-%d\') retirement_date, menu_id, package_type_id, wiki, highlight, description from application where '.((isset($request['id']) && $request['id'] != '')?'id = '.$request['id']:'name = \''.addslashes($request['name']).'\''));
      if ($getApplication->execute())
      {
        if (($getApplicationRow = $getApplication->fetch('assoc')))
        {
          $bResult = true;
          $getApplicationRow['account_check'] = $this->noyes($getApplicationRow['account_check']);
          $getApplicationRow['auto_register'] = $this->noyes($getApplicationRow['auto_register']);
          $getApplicationRow['dependable'] = $this->noyes($getApplicationRow['dependable']);
          if ($getApplicationRow['login_type_id'] > 0)
          {
            $logintyperequest = array('id'=>$getApplicationRow['login_type_id']);
            $logintyperesponse = null;
            if ($this->loginType($logintyperequest, $logintyperesponse, $strError))
            {
              $getApplicationRow['login_type'] = $logintyperesponse;
            }
            unset($logintyperequest);
            unset($logintyperesponse);
          }
          if ($getApplicationRow['menu_id'] > 0)
          {
            $menuaccessrequest = array('id'=>$getApplicationRow['menu_id']);
            $menuaccessresponse = null;
            if ($this->menuAccess($menuaccessrequest, $menuaccessresponse, $strError))
            {
              $getApplicationRow['menu_access'] = $menuaccessresponse;
            }
            unset($menuaccessrequest);
            unset($menuaccessresponse);
          }
          if ($getApplicationRow['notify_priority_id'] > 0)
          {
            $notifypriorityrequest = array('id'=>$getApplicationRow['notify_priority_id']);
            $notifypriorityresponse = null;
            if ($this->notifyPriority($notifypriorityrequest, $notifypriorityresponse, $strError))
            {
              $getApplicationRow['notify_priority'] = $notifypriorityresponse;
            }
            unset($notifypriorityrequest);
            unset($notifypriorityresponse);
          }
          if ($getApplicationRow['package_type_id'] > 0)
          {
            $packagetyperequest = array('id'=>$getApplicationRow['package_type_id']);
            $packagetyperesponse = null;
            if ($this->packageType($packagetyperequest, $packagetyperesponse, $strError))
            {
              $getApplicationRow['package_type'] = $packagetyperesponse;
            }
            unset($packagetyperequest);
            unset($packagetyperesponse);
          }
          $getApplicationRow['secure_port'] = $this->noyes($getApplicationRow['secure_port']);
          $getApplicationRow['wiki'] = $this->noyes($getApplicationRow['wiki']);
          $response = $getApplicationRow;
        }
        else
        {
          $strError = 'Failed to query results.';
        }
      }
      else
      {
        $strError = $getApplication->getError();
      }
      $this->m_centraldb->free($getApplication);
    }
    else
    {
      $strError = 'Please provide the id or name.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationAccount()
  public function applicationAccount($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $strQuery = 'select id, application_id, user_id, encrypt, aes, password, ';
      if (isset($request['aes']) && $request['aes'] != '' && $request['aes'] !== 0)
      {
        $strQuery .= 'aes_decrypt(from_base64(password), sha2(\''.addslashes($request['aes']).'\', 512)) decrypted_password, ';
      }
      $strQuery .= 'type_id, description from application_account where id = '.$request['id'];
      $getAccount = $this->m_centraldb->parse($strQuery);
      if ($getAccount->execute())
      {
        if (($getAccountRow = $getAccount->fetch('assoc')))
        {
          $devrequest = array('id'=>$getAccountRow['application_id']);
          $devresponse = null;
          if ($this->isGlobalAdmin() || $this->isApplicationDeveloper($devrequest, $devresponse, $strError))
          {
            $bResult = true;
            if ($getAccountRow['encrypt'] == 1)
            {
              unset($getAccountRow['password']);
            }
            else if ($getAccountRow['aes'] == 1)
            {
              if (isset($getAccountRow['decrypted_password']) && $getAccountRow['decrypted_password'] != '')
              {
                $getAccountRow['password'] = $getAccountRow['decrypted_password'];
              }
              else
              {
                unset($getAccountRow['password']);
              }
            }
            if (isset($getAccountRow['decrypted_password']))
            {
              unset($getAccountRow['decrypted_password']);
            }
            $response = $getAccountRow;
          }
          else
          {
            $strError = 'You are not authorized to perform this action.';
          }
          unset($devrequest);
          unset($devresponse);
        }
        else
        {
          $strError = 'Failed to query results.';
        }
      }
      else
      {
        $strError = $getAccount->getError();
      }
      $this->m_centraldb->free($getAccount);
    }
    else
    {
      $strError = 'Please provide the application_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationAccountAdd()
  public function applicationAccountAdd($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['application_id']) && $request['application_id'] != '')
    {
      $devrequest = array('id'=>$request['application_id']);
      $devresponse = null;
      if ($this->isGlobalAdmin() || $this->isApplicationDeveloper($devrequest, $devresponse, $strError))
      {
        if (isset($request['user_id']) && $request['user_id'] != '')
        {
          if (isset($request['encrypt']) && is_array($request['encrypt']))
          {
            if (isset($request['type']) && is_array($request['type']))
            {
              $strQuery = 'insert into application_account (application_id, user_id, encrypt, aes, `password`, type_id, description) values ('.$request['application_id'].', \''.$request['user_id'].'\', '.$request['encrypt']['value'].', ';
              if ($request['encrypt']['value'] == 1)
              {
                $strQuery .= '0, concat(\'*\',upper(sha1(unhex(sha1(\''.addslashes($request['password']).'\')))))';
              }
              else if (isset($request['aes']) && $request['aes'] != '' && $request['aes'] !== 0)
              {
                $strQuery .= '1, to_base64(aes_encrypt(\''.addslashes($request['password']).'\', sha2(\''.addslashes($request['aes']).'\', 512)))';
              }
              else
              {
                $strQuery .= '0, \''.addslashes($request['password']).'\'';
              }
              $strQuery .= ', '.$request['type']['id'].', \''.addslashes($request['description']).'\')';
              $insertApplicationAccount = $this->m_centraldb->parse($strQuery);
              if ($insertApplicationAccount->execute())
              {
                $bResult = true;
                $this->syslog()->userAccountCreated('Created the user.', $request['user_id']);
              }
              else
              {
                $strError = $insertApplicationAccount->getError();
                $this->syslog()->userAccountCreated('Failed to create the user.', $request['user_id'], false);
              }
            }
            else
            {
              $strError = 'Please provide the type.';
            }
          }
          else
          {
            $strError = 'Please provide the encrypt.';
          }
        }
        else
        {
          $strError = 'Please provide the user_id.';
        }
      }
      else
      {
        $strError = 'You are not authorized to perform this action.';
      }
      unset($devrequest);
      unset($devresponse);
    }
    else
    {
      $strError = 'Please provide the application_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationAccountEdit()
  public function applicationAccountEdit($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $accountrequest = array('id'=>$request['id']);
      $accountresponse = null;
      if ($this->applicationAccount($accountrequest, $accountresponse, $strError))
      {
        if (isset($request['user_id']) && $request['user_id'] != '')
        {
          if (isset($request['encrypt']) && is_array($request['encrypt']))
          {
            if (isset($request['type']) && is_array($request['type']))
            {
              $strQuery = 'update application_account set user_id = \''.$request['user_id'].'\', encrypt = '.$request['encrypt']['value'];
              if ($request['encrypt']['value'] == 1)
              {
                if ($request['password'] != '')
                {
                  $strQuery .= ', aes = 0, `password` = concat(\'*\',upper(sha1(unhex(sha1(\''.addslashes($request['password']).'\')))))';
                }
              }
              else if (isset($request['aes']) && $request['aes'] != '' && $request['aes'] !== 0)
              {
                $strQuery .= ', aes = 1, `password` = to_base64(aes_encrypt(\''.addslashes($request['password']).'\', sha2(\''.addslashes($request['aes']).'\', 512)))';
              }
              else
              {
                $strQuery .= ', aes = 0, `password` = \''.addslashes($request['password']).'\'';
              }
              $strQuery .= ', type_id = '.$request['type']['id'].', description = \''.addslashes($request['description']).'\' where id = '.$request['id'];
              $updateApplicationAccount = $this->m_centraldb->parse($strQuery);
              if ($updateApplicationAccount->execute())
              {
                $bResult = true;
                $this->syslog()->userAccountModified('Modified the user.', $request['user_id']);
              }
              else
              {
                $strError = $updateApplicationAccount->getError();
                $this->syslog()->userAccountModified('Failed to modify the user.', $request['user_id'], false);
              }
            }
            else
            {
              $strError = 'Please provide the type.';
            }
          }
          else
          {
            $strError = 'Please provide the encrypt.';
          }
        }
        else
        {
          $strError = 'Please provide the user_id.';
        }
      }
      unset($accountrequest);
      unset($accountresponse);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationAccountRemove()
  public function applicationAccountRemove($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $accountrequest = array('id'=>$request['id']);
      $accountresponse = null;
      if ($this->applicationAccount($accountrequest, $accountresponse, $strError))
      {
        $deleteApplicationAccount = $this->m_centraldb->parse('delete from application_account where id = '.$request['id']);
        if ($deleteApplicationAccount->execute())
        {
          $bResult = true;
          $this->syslog()->userAccountDeleted('Deleted the user.');
        }
        else
        {
          $strError = $deleteApplicationAccount->getError();
          $this->syslog()->userAccountDeleted('Failed to delete the user.', null, false);
        }
      }
      unset($accountrequest);
      unset($accountresponse);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationAccountsByApplicationID()
  public function applicationAccountsByApplicationID($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['application_id']) && $request['application_id'] != '')
    {
      $devrequest = array('id'=>$request['application_id']);
      $devresponse = null;
      if ($this->isGlobalAdmin() || $this->isApplicationDeveloper($devrequest, $devresponse, $strError))
      {
        $strQuery = 'select id, application_id, user_id, encrypt, aes, password, ';
        if (isset($request['aes']) && $request['aes'] != '' && $request['aes'] !== 0)
        {
          $strQuery .= 'aes_decrypt(from_base64(password), sha2(\''.addslashes($request['aes']).'\', 512)) decrypted_password, ';
        }
        $strQuery .= 'type_id, description from application_account where application_id = '.$request['application_id'].' order by user_id';
        $getAccount = $this->m_centraldb->parse($strQuery);
        if ($getAccount->execute())
        {
          $bResult = true;
          while (($getAccountRow = $getAccount->fetch('assoc')))
          {
            if ($getAccountRow['encrypt'] == 1)
            {
              unset($getAccountRow['password']);
            }
            else if ($getAccountRow['aes'] == 1)
            {
              if (isset($getAccountRow['decrypted_password']) && $getAccountRow['decrypted_password'] != '')
              {
                $getAccountRow['password'] = $getAccountRow['decrypted_password'];
              }
              else
              {
                unset($getAccountRow['password']);
              }
            }
            if (isset($getAccountRow['decrypted_password']))
            {
              unset($getAccountRow['decrypted_password']);
            }
            $getAccountRow['encrypt'] = $this->noyes($getAccountRow['encrypt']);
            $typerequest = array('id'=>$getAccountRow['type_id']);
            $typeresponse = null;
            if ($this->accountType($typerequest, $typeresponse, $strError))
            {
              $getAccountRow['type'] = $typeresponse;
            }
            unset($typerequest);
            unset($typeresponse);
            $response[] = $getAccountRow;
          }
        }
        else
        {
          $strError = $getAccount->getError();
        }
        $this->m_centraldb->free($getAccount);
      }
      else
      {
        $strError = 'You are not authorized to perform this action.';
      }
      unset($devrequest);
      unset($devresponse);
    }
    else
    {
      $strError = 'Please provide the application_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationAdd()
  public function applicationAdd($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if ($this->isValid('Central'))
    {
      if (isset($request['name']) && $request['name'] != '')
      {
        $applicationrequest = array('name'=>$request['name']);
        $applicationresponse = null;
        if (!$this->application($applicationrequest, $applicationresponse, $strError) && $strError == 'Failed to query results.')
        {
          $strError = null;
          $insertApplication = $this->m_centraldb->parse('insert into application (name, creation_date) values (\''.addslashes($request['name']).'\', now())');
          if ($insertApplication->execute())
          {
            $applicationnewrequest = array('name'=>$request['name']);
            $applicationnewresponse = null;
            if ($this->application($applicationnewrequest, $applicationnewresponse, $strError))
            {
              $userrequest = array('userid'=>$this->getUserID());
              $userresponse = null;
              if ($this->user($userrequest, $userresponse, $strError))
              {
                $contacttyperequest = array('type'=>'Primary Developer');
                $contacttyperesponse = null;
                if ($this->contactType($contacttyperequest, $contacttyperesponse, $strError))
                {
                  $insertApplicationContact = $this->m_centraldb->parse('insert into application_contact (application_id, contact_id, type_id, admin, locked, notify) values ('.$applicationnewresponse['id'].', '.$userresponse['id'].', '.$contacttyperesponse['id'].', 1, 0, 1)');
                  if ($insertApplicationContact->execute())
                  {
                    $bResult = true;
                  }
                  else
                  {
                    $strError = $insertApplicationContact->getError();
                  }
                }
                unset($contacttyperequest);
                unset($contacttyperesponse);
              }
              unset($userrequest);
              unset($userresponse);
            }
            unset($applicationnewrequest);
            unset($applicationnewresponse);
          }
          else
          {
            $strError = $insertApplication->getError();
          }
        }
        unset($applicationrequest);
        unset($applicationresponse);
      }
      else
      {
        $strError = 'Please provide the name.';
      }
    }
    else
    {
      $strError = 'You are not authorized to perform this action.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationDepend()
  public function applicationDepend($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $getApplicationDepend = $this->m_centraldb->parse('select id, application_id, dependant_id from application_dependant where id = '.$request['id']);
      if ($getApplicationDepend->execute())
      {
        if (($getApplicationDependRow = $getApplicationDepend->fetch('assoc')))
        {
          $bResult = true;
          $response = $getApplicationDependRow;
        }
        else
        {
          $strError = 'Failed to find application depend.';
        }
      }
      else
      {
        $strError = $insertApplicationDepend->getError();
      }
      $this->m_centraldb->free($getApplicationDepend);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationDependAdd()
  public function applicationDependAdd($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['application_id']) && $request['application_id'] != '')
    {
      $devrequest = array('id'=>$request['application_id']);
      $devresponse = null;
      if ($this->isGlobalAdmin() || $this->isApplicationDeveloper($devrequest, $devresponse, $strError))
      {
        if (isset($request['dependant_id']) && $request['dependant_id'] != '')
        {
          $insertApplicationDepend = $this->m_centraldb->parse('insert into application_dependant (application_id, dependant_id) values ('.$request['application_id'].', '.$request['dependant_id'].')');
          if ($insertApplicationDepend->execute())
          {
            $bResult = true;
          }
          else
          {
            $strError = $insertApplicationDepend->getError();
          }
        }
        else
        {
          $strError = 'Please provide the dependant_id.';
        }
      }
      else
      {
        $strError = 'You are not authorized to perform this action.';
      }
      unset($devrequest);
      unset($devresponse);
    }
    else
    {
      $strError = 'Please provide the application_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationDependRemove()
  public function applicationDependRemove($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $dependrequest = array('id'=>$request['id']);
      $dependresponse = null;
      if ($this->applicationDepend($dependrequest, $dependresponse, $strError))
      {
        $devrequest = array('id'=>$dependresponse['application_id']);
        $devresponse = null;
        if ($this->isGlobalAdmin() || $this->isApplicationDeveloper($devrequest, $devresponse, $strError))
        {
          $deleteApplicationDepend = $this->m_centraldb->parse('delete from application_dependant where id = '.$request['id']);
          if ($deleteApplicationDepend->execute())
          {
            $bResult = true;
          }
          else
          {
            $strError = $deleteApplicationDepend->getError();
          }
        }
        else
        {
          $strError = 'You are not authorized to perform this action.';
        }
        unset($devrequest);
        unset($devresponse);
      }
      unset($dependrequest);
      unset($dependresponse);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationEdit()
  public function applicationEdit($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $devrequest = array('id'=>$request['id']);
      $devresponse = null;
      if ($this->isGlobalAdmin() || $this->isApplicationDeveloper($devrequest, $devresponse, $strError))
      {
        if (isset($request['name']) && $request['name'] != '')
        {
          $strQuery = 'update application set';
          $strQuery .= ' name = \''.addslashes($request['name']).'\'';
          $strQuery .= ', notify_priority_id = '.((isset($request['notify_priority']) && is_array($request['notify_priority']) && isset($request['notify_priority']['id']) && $request['notify_priority']['id'] != '')?$request['notify_priority']['id']:'null');
          $strQuery .= ', website = '.((isset($request['website']) && $request['website'] != '')?'\''.addslashes($request['website']).'\'':'null');
          $strQuery .= ', login_type_id = '.((isset($request['login_type']) && is_array($request['login_type']) && isset($request['login_type']['id']) && $request['login_type']['id'] != '')?$request['login_type']['id']:'null');
          if (isset($request['secure_port']) && is_array($request['secure_port']) && isset($request['secure_port']['value']) && $request['secure_port']['value'] != '')
          {
            $strQuery .= ', secure_port = '.$request['secure_port']['value'];
          }
          if (isset($request['auto_register']) && is_array($request['auto_register']) && isset($request['auto_register']['value']) && $request['auto_register']['value'] != '')
          {
            $strQuery .= ', auto_register = '.$request['auto_register']['value'];
          }
          if (isset($request['account_check']) && is_array($request['account_check']) && isset($request['account_check']['value']) && $request['account_check']['value'] != '')
          {
            $strQuery .= ', account_check = '.$request['account_check']['value'];
          }
          if (isset($request['dependable']) && is_array($request['dependable']) && isset($request['dependable']['value']) && $request['dependable']['value'] != '')
          {
            $strQuery .= ', dependable = '.$request['dependable']['value'];
          }
          $strQuery .= ', menu_id = '.((isset($request['menu_access']) && is_array($request['menu_access']) && isset($request['menu_access']['id']) && $request['menu_access']['id'] != '')?$request['menu_access']['id']:'null');
          $strQuery .= ', package_type_id = '.((isset($request['package_type']) && is_array($request['package_type']) && isset($request['package_type']['id']) && $request['package_type']['id'] != '')?$request['package_type']['id']:'null');
          if (isset($request['wiki']) && is_array($request['wiki']) && isset($request['wiki']['value']) && $request['wiki']['value'] != '')
          {
            $strQuery .= ', wiki = '.$request['wiki']['value'];
          }
          $strQuery .= ', highlight = '.((isset($request['highlight']) && $request['highlight'] != '')?'\''.addslashes($request['highlight']).'\'':'null');
          $strQuery .= ', description = '.((isset($request['description']) && $request['description'] != '')?'\''.addslashes($request['description']).'\'':'null');
          $strQuery .= ', retirement_date = '.((isset($request['retirement_date']) && $request['retirement_date'] != '')?'\''.addslashes($request['retirement_date']).'\'':'null');
          $strQuery .= ' where id = '.$request['id'];
          $updateApplication = $this->m_centraldb->parse($strQuery);
          if ($updateApplication->execute())
          {
            $bResult = true;
          }
          else
          {
            $strError = $updateApplication->getError();
          }
        }
        else
        {
          $strError = 'Please provide the name.';
        }
      }
      else
      {
        $strError = 'You are not authorized to perform this action.';
      }
      unset($devrequest);
      unset($devresponse);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationIssue()
  public function applicationIssue($request, &$response, &$strError)
  {
    $bComments = ((isset($request['comments']) && $request['comments'] == 1)?true:false);
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $getIssue = $this->m_centraldb->parse('select id, application_id, date_format(open_date, \'%Y-%m-%d\') open_date, date_format(due_date, \'%Y-%m-%d\') due_date, date_format(close_date, \'%Y-%m-%d\') close_date, hold, priority from application_issue where id = '.$request['id']);
      if ($getIssue->execute())
      {
        $bResult = true;
        if (($getIssueRow = $getIssue->fetch('assoc')))
        {
          if ($bComments)
          {
            $issueCommentsRequest = array('issue_id'=>$getIssueRow['id']);
            $issueCommentsResponse = null;
            if ($this->applicationIssueComments($issueCommentsRequest, $issueCommentsResponse, $strError))
            {
              $getIssueRow['comments'] = $issueCommentsResponse;
            }
            unset($issueCommentsRequest);
            unset($issueCommentsResponse);
          }
          $response = $getIssueRow;
        }
      }
      else
      {
        $strError = $getIssue->getError();
      }
      $this->m_centraldb->free($getIssue);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationIssueAdd()
  public function applicationIssueAdd($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if ($this->isValid())
    {
      if (isset($request['application_id']) && $request['application_id'] != '')
      {
        $strQuery = 'insert into application_issue (application_id, open_date';
        if (isset($request['due_date']) && $request['due_date'] != '')
        {
          $strQuery .= ', due_date';
        }
        if (isset($request['priority']) && $request['priority'] != '')
        {
          $strQuery .= ', priority';
        }
        $strQuery .= ') values ('.$request['application_id'].', now()';
        if (isset($request['due_date']) && $request['due_date'] != '')
        {
          $strQuery .= ', \''.$request['due_date'].'\'';
        }
        if (isset($request['priority']) && $request['priority'] != '')
        {
          $strQuery .= ', '.$request['priority'];
        }
        $strQuery .= ')';
        $insertIssue = $this->m_centraldb->parse($strQuery);
        if ($insertIssue->execute())
        {
          $issuesRequest = array('application_id'=>$request['application_id'], 'open'=>1);
          $issuesResponse = null;
          if ($this->applicationIssuesByApplicationID($issuesRequest, $issuesResponse, $strError))
          {
            $nSize = sizeof($issuesResponse);
            if ($nSize > 0)
            {
              $bResult = true;
              $response = $issuesResponse[$nSize - 1];
            }
          }
          unset($issuesRequest);
          unset($issuesResponse);
        }
        else
        {
          $strError = $insertIssue->getError();
        }
      }
      else
      {
        $strError = 'Please provide the application_id.';
      }
    }
    else
    {
      $strError = 'You are not authorized to perform this action.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationIssueClose()
  public function applicationIssueClose($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if ($this->isValid())
    {
      if (isset($request['id']) && $request['id'] != '')
      {
        $updateIssue = $this->m_centraldb->parse('update application_issue set close_date = now() where id = '.$request['id']);
        if ($updateIssue->execute())
        {
          $bResult = true;
        }
        else
        {
          $strError = $updateIssue->getError();
        }
      }
      else
      {
        $strError = 'Please provide the id.';
      }
    }
    else
    {
      $strError = 'You are not authorized to perform this action.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationIssueCommentAdd()
  public function applicationIssueCommentAdd($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if ($this->isValid())
    {
      $getPerson = $this->m_centraldb->parse('select id from person where userid = \''.$this->getUserID().'\'');
      if ($getPerson->execute())
      {
        if (($getPersonRow = $getPerson->fetch('assoc')))
        {
          if (isset($request['issue_id']) && $request['issue_id'] != '')
          {
            if (isset($request['comments']) && $request['comments'] != '')
            {
              $insertIssueComment = $this->m_centraldb->parse('insert issue_comment (issue_id, entry_date, user_id, comments) values ('.$request['issue_id'].', now(), '.$getPersonRow['id'].', \''.addslashes($request['comments']).'\')');
              if ($insertIssueComment->execute())
              {
                $bResult = true;
              }
              else
              {
                $strError = $insertIssueComment->getError();
              }
            }
            else
            {
              $strError = 'Please provide the comments.';
            }
          }
          else
          {
            $strError = 'Please provide the issue_id.';
          }
        }
        else
        {
          $strError = 'Failed to find person.';
        }
      }
      else
      {
        $strError = 'Failed to query results.';
      }
      $this->m_centraldb->free($getPerson);
    }
    else
    {
      $strError = 'You are not authorized to perform this action.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationIssueCommentEdit()
  public function applicationIssueCommentEdit($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if ($this->isValid())
    {
      if (isset($request['id']) && $request['id'] != '')
      {
        $getPerson = $this->m_centraldb->parse('select id from person where userid = \''.$this->getUserID().'\'');
        if ($getPerson->execute())
        {
          if (($getPersonRow = $getPerson->fetch('assoc')))
          {
            $getIssueComment = $this->m_centraldb->parse('select user_id from issue_comment where id = '.$request['id']);
            if ($getIssueComment->execute())
            {
              if (($getIssueCommentRow = $getIssueComment->fetch('assoc')))
              {
                if ($getPersonRow['id'] == $getIssueCommentRow['user_id'])
                {
                  if (isset($request['comments']) && $request['comments'] != '')
                  {
                    $strQuery = 'update issue_comment set ';
                    $strQuery .= 'comments = \''.addslashes($request['comments']).'\'';
                    $strQuery .= ' where id = '.$request['id'];
                    $updateIssue = $this->m_centraldb->parse($strQuery);
                    if ($updateIssue->execute())
                    {
                      $bResult = true;
                    }
                    else
                    {
                      $strError = $updateIssue->getError();
                    }
                  }
                  else
                  {
                    $strError = 'Please provide the comments.';
                  }
                }
                else
                {
                  $strError = 'You are not authorized to perform this action.';
                }
              }
              else
              {
                $strError = 'Failed to find issue comment.';
              }
            }
            else
            {
              $strError = 'Failed to query results.';
            }
            $this->m_centraldb->free($getIssueComment);
          }
          else
          {
            $strError = 'Failed to find person.';
          }
        }
        else
        {
          $strError = 'Failed to query results.';
        }
        $this->m_centraldb->free($getPerson);
      }
      else
      {
        $strError = 'Please provide the id.';
      }
    }
    else
    {
      $strError = 'You are not authorized to perform this action.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationIssueComments()
  public function applicationIssueComments($request, &$response, &$strError)
  {
    $bResult = false;
    $nLimit = ((isset($request['limit']) && is_numeric($request['limit']))?$request['limit']:null);
    $response = array();

    if (isset($request['issue_id']) && $request['issue_id'] != '')
    {
      $getIssueComment = $this->m_centraldb->parse('select a.id, date_format(a.entry_date, \'%Y-%m-%d %H:%i:%s\') entry_date, a.user_id, a.comments, b.last_name, b.first_name, b.userid, b.email from issue_comment a, person b where a.user_id = b.id and a.issue_id = '.$request['issue_id'].' order by entry_date, id'.(($nLimit != null)?' limit '.$nLimit:''));
      if ($getIssueComment->execute())
      {
        $bResult = true;
        while (($getIssueCommentRow = $getIssueComment->fetch('assoc')))
        {
          $response[] = $getIssueCommentRow;
        }
      }
      else
      {
        $strError = $getIssueComment->getError();
      }
      $this->m_centraldb->free($getIssueComment);
    }
    else
    {
      $strError = 'Please provide the issue_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationIssueEdit()
  public function applicationIssueEdit($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['application_id']) && $request['application_id'] != '')
    {
      $developerRequest = array('application_id'=>$request['application_id']);
      $developerResponse = null;
      if ($this->isGlobalAdmin() || $this->isApplicationDeveloper($developerRequest, $developerResponse, $strError))
      {
        if (isset($request['id']) && $request['id'] != '')
        {
          $strQuery = 'update application_issue set ';
          $strQuery .= 'application_id = '.((isset($request['transfer']) && is_array($request['transfer']) && isset($request['transfer']['id']) && $request['transfer']['id'] != '' && $request['transfer']['id'] != $request['application_id'])?$request['transfer']['id']:$request['application_id']);
          $strQuery .= ', close_date = '.(($request['close_date'] != '')?'\''.$request['close_date'].'\'':'null');
          $strQuery .= ', due_date = '.(($request['due_date'] != '')?'\''.$request['due_date'].'\'':'null');
          $strQuery .= ', priority = '.((isset($request['priority']) && $request['priority'] != '')?$request['priority']:'null');
          $strQuery .= ' where id = '.$request['id'];
          $updateIssue = $this->m_centraldb->parse($strQuery);
          if ($updateIssue->execute())
          {
            $bResult = true;
          }
          else
          {
            $strError = $updateIssue->getError();
          }
        }
        else
        {
          $strError = 'Please provide the id.';
        }
      }
      else
      {
        $strError = 'You are not authorized to perform this action.';
      }
      unset($developerRequest);
      unset($developerResponse);
    }
    else
    {
      $strError = 'Please provide the application_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationIssueEmail()
  public function applicationIssueEmail($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if ($this->isValid())
    {
      if (isset($request['action']) && $request['action'] != '')
      {
        if ($request['action'] == 'add' || $request['action'] == 'close' || $request['action'] == 'transfer' || $request['action'] == 'update')
        {
          $strServer = null;
          if (isset($request['server']) && $request['server'] != '')
          {
            $strServer = $request['server'];
          }
          else if (isset($_SERVER['SERVER_NAME']) && $_SERVER['SERVER_NAME'] != '')
          {
            $strServer = $_SERVER['SERVER_NAME'];
          }
          if ($strServer != '')
          {
            if (isset($request['id']) && $request['id'] != '')
            {
              $issueRequest = array('id'=>$request['id'], 'comments'=>1);
              $issueResponse = null;
              if ($this->applicationIssue($issueRequest, $issueResponse, $strError))
              {
                $applicationRequest = array('id'=>$issueResponse['application_id']);
                $applicationResponse = null;
                if ($this->application($applicationRequest, $applicationResponse, $strError))
                {
                  $contactsRequest = array('application_id'=>$applicationResponse['id'], 'Primary Developer'=>1, 'Backup Developer'=>1);
                  $contactsResponse = null;
                  if ($this->applicationUsersByApplicationID($contactsRequest, $contactsResponse, $strError))
                  {
                    $strApplication = null;
                    $toList = array();
                    $nSize = sizeof($contactsResponse);
                    for ($i = 0; $i < $nSize; $i++)
                    {
                      $toList[$contactsResponse[$i]['userid']] = $contactsResponse[$i]['email'];
                    }
                    if ($request['action'] == 'transfer' && isset($request['application_id']) && $request['application_id'] != '')
                    {
                      $subapplicationRequest = array('id'=>$request['application_id']);
                      $subapplicationResponse = null;
                      if ($this->application($subapplicationRequest, $subapplicationResponse, $strError))
                      {
                        $strApplication = $subapplicationResponse['name'];
                        $subcontactsRequest = array('application_id'=>$request['application_id'], 'Primary Developer'=>1, 'Backup Developer'=>1);
                        $subcontactsResponse = null;
                        if ($this->applicationUsersByApplicationID($subcontactsRequest, $subcontactsResponse, $strError))
                        {
                          $toList = array();
                          $nSize = sizeof($subcontactsResponse);
                          for ($i = 0; $i < $nSize; $i++)
                          {
                            $toList[$subcontactsResponse[$i]['userid']] = $subcontactsResponse[$i]['email'];
                          }
                        }
                        unset($subcontactsRequest);
                        unset($subcontactsResponse);
                      }
                      unset($subapplicationRequest);
                      unset($subapplicationResponse);
                    }
                    $nSize = sizeof($issueResponse['comments']);
                    for ($i = 0; $i < $nSize; $i++)
                    {
                      $toList[$issueResponse['comments'][$i]['userid']] = $issueResponse['comments'][$i]['email'];
                    }
                    $strName = $this->getUserName();
                    $message = array('html'=>null, 'text'=>null);
                    $message['html'] = '<html><body style="background:#f3f3f3;padding:10px;">';
                    if ($request['action'] == 'add')
                    {
                      $message['html'] .= '<a href="http://'.$strServer.'/central/#/Applications/Issues/'.$request['id'].'" style="text-decoration:none;">Issue #'.$request['id'].'</a> has been <b>created</b> by '.$strName.'.';
                      $message['text'] .= "Issue #".$request['id']." has been created by ".$strName.".\n\nYou can view this issue at:\nhttp://".$strServer."/central/#/Applications/Issues/".$request['id'];
                    }
                    else if ($request['action'] == 'close')
                    {
                      $message['html'] .= '<a href="http://'.$strServer.'/central/#/Applications/Issues/'.$request['id'].'" style="text-decoration:none;">Issue #'.$request['id'].'</a> has been <b>closed</b> by '.$strName.'.';
                      $message['text'] .= "Issue #".$request['id']." has been closed by ".$strName.".\n\nYou can view this issue at:\nhttp://".$strServer."/central/#/Applications/Issues/".$request['id'];
                    }
                    else if ($request['action'] == 'transfer')
                    {
                      $message['html'] .= '<a href="http://'.$strServer.'/central/#/Applications/Issues/'.$request['id'].'" style="text-decoration:none;">Issue #'.$request['id'].'</a> has been <b>transferred</b> from '.$strApplication.' to '.$applicationResponse['name'].' by '.$strName.'.';
                      $message['text'] .= "Issue #".$request['id']." has been transferred from ".$strApplication.' to '.$applicationResponse['name']." by ".$strName.".\n\nYou can view this issue at:\nhttp://".$strServer."/central/#/Applications/Issues/".$request['id'];
                    }
                    else if ($request['action'] == 'update')
                    {
                      $message['html'] .= '<a href="http://'.$strServer.'/central/#/Applications/Issues/'.$request['id'].'" style="text-decoration:none;">Issue #'.$request['id'].'</a> has been <b>updated</b> by '.$strName.'.';
                      $message['text'] .= "Issue #".$request['id']." has been updated by ".$strName.".\n\nYou can view this issue at:\nhttp://".$strServer."/central/#/Applications/Issues/".$request['id'];
                    }
                    if (($nSize = sizeof($issueResponse['comments'])) > 0)
                    {
                      $message['html'] .= '<div style="margin:10px 5px;border-style:solid;border-width:1px;border-color:#cccccc;border-radius:10px;background:white;box-shadow: 3px 3px 4px #888888;padding:10px;">';
                      $message['html'] .= '<small style="float:right;color:#999999;"><i>'.$issueResponse['comments'][($nSize-1)]['entry_date'].' by '.$issueResponse['comments'][($nSize-1)]['first_name'].' '.$issueResponse['comments'][($nSize-1)]['last_name'].'</i></small>';
                      $message['html'] .= '<pre style="white-space:pre-wrap;">'.str_replace("\n", '<br>', str_replace('>', '&gt;', str_replace('<', '&lt;', $issueResponse['comments'][($nSize-1)]['comments']))).'</pre>';
                      $message['html'] .= '</div>';
                    }
                    $message['html'] .= 'This message was sent by <a href="http://'.$strServer.'/central" style="text-decoration:none;">Central</a>.';
                    $message['html'] .= '</body></html>';
                    $strTo = array();
                    foreach ($toList as $key => $value)
                    {
                      $strTo[] = $value;
                    }
                    unset($toList);
                    if ($this->m_junction->email($this->getUserEmail(), $strTo, $applicationResponse['name'].' ['.$request['action'].']:  Issue #'.$request['id'], $message['text'], $message['html']))
                    {
                      $bResult = true;
                    }
                    else
                    {
                      $strError = 'Failed to send email.';
                    }
                    unset($message);
                  }
                  unset($contactsRequest);
                  unset($contactsResponse);
                }
                unset($applicationRequest);
                unset($applicationResponse);
              }
              unset($issueRequest);
              unset($issueResponse);
            }
            else
            {
              $strError = 'Please provide the id.';
            }
          }
          else
          {
            $strError = 'Please provide the server.';
          }
        }
        else
        {
          $strError = 'Please provide a valid action.';
        }
      }
      else
      {
        $strError = 'Please provide the action.';
      }
    }
    else
    {
      $strError = 'You are not authorized to perform this action.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationIssues()
  public function applicationIssues($request, &$response, &$strError)
  {
    $bResult = false;
    $bApplication = ((isset($request['application']) && $request['application'] == 1)?true:false);
    $bBackupDeveloper = ((isset($request['Backup Developer']) && $request['Backup Developer'] == 1)?true:false);
    $bComments = ((isset($request['comments']) && $request['comments'] == 1)?true:false);
    $bContact = ((isset($request['Contact']) && $request['Contact'] == 1)?true:false);
    $bOpen = ((isset($request['open']) && $request['open'] == 1)?true:false);
    $bOwner = ((isset($request['owner']) && $request['owner'] == 1)?true:false);
    $bPrimaryContact = ((isset($request['Primary Contact']) && $request['Primary Contact'] == 1)?true:false);
    $bPrimaryDeveloper = ((isset($request['Primary Developer']) && $request['Primary Developer'] == 1)?true:false);
    $strCloseDateEnd = ((isset($request['close_date_end']) && $request['close_date_end'] != '')?$request['close_date_end']:'');
    $strCloseDateStart = ((isset($request['close_date_start']) && $request['close_date_start'] != '')?$request['close_date_start']:'');
    $strDisplay = ((isset($request['display']) && $request['display'] != '')?$request['display']:'');
    $strOpenDateEnd = ((isset($request['open_date_end']) && $request['open_date_end'] != '')?$request['open_date_end']:'');
    $strOpenDateStart = ((isset($request['open_date_start']) && $request['open_date_start'] != '')?$request['open_date_start']:'');
    $response = array();

    $strQuery = 'select id, application_id, date_format(open_date, \'%Y-%m-%d\') open_date, date_format(due_date, \'%Y-%m-%d\') due_date, date_format(close_date, \'%Y-%m-%d\') close_date, hold, priority from application_issue';
    if ($bOpen || $strOpenDateStart != '' || $strOpenDateEnd != '' || $strCloseDateStart != '' || $strCloseDateEnd != '')
    {
      $bFirst = true;
      if ($bOpen && $strDisplay != 'all')
      {
        $strQuery .= (($bFirst)?' where':' and').' (close_date is null or date_format(close_date, \'%Y-%m-%d\') = \'0000-00-00\')';
        $bFirst = false;
      }
      if ($strOpenDateStart != '')
      {
        $strQuery .= (($bFirst)?' where':' and').' date_format(open_date, \'%Y-%m-%d\') >= \''.$strOpenDateStart.'\'';
        $bFirst = false;
      }
      if ($strOpenDateEnd != '')
      {
        $strQuery .= (($bFirst)?' where':' and').' date_format(open_date, \'%Y-%m-%d\') < \''.$strOpenDateEnd.'\'';
        $bFirst = false;
      }
      if ($strCloseDateStart != '')
      {
        $strQuery .= (($bFirst)?' where':' and').' date_format(close_date, \'%Y-%m-%d\') >= \''.$strCloseDateStart.'\'';
        $bFirst = false;
      }
      if ($strCloseDateEnd != '')
      {
        $strQuery .= (($bFirst)?' where':' and').' date_format(close_date, \'%Y-%m-%d\') <= \''.$strCloseDateEnd.'\'';
        $bFirst = false;
      }
      $strQuery .= ' order by (due_date is null or date_format(due_date, \'%Y-%m-%d\') = \'0000-00-00\'), due_date, priority desc, id';
    }
    $getIssue = $this->m_centraldb->parse($strQuery);
    if ($getIssue->execute())
    {
      $bResult = true;
      while (($getIssueRow = $getIssue->fetch('assoc')))
      {
        $bUse = (($bOwner)?false:true);
        if ($bApplication)
        {
          $applicationRequest = array('id'=>$getIssueRow['application_id']);
          $applicaitonResponse = null;
          if ($this->application($applicationRequest, $applicationResponse, $strError))
          {
            $getIssueRow['application'] = $applicationResponse;
            $contactsRequest = array('application_id'=>$getIssueRow['application_id']);
            if ($bPrimaryDeveloper)
            {
              $contactsRequest['Primary Developer'] = 1;
            }
            if ($bBackupDeveloper)
            {
              $contactsRequest['Backup Developer'] = 1;
            }
            if ($bPrimaryContact)
            {
              $contactsRequest['Primary Contact'] = 1;
            }
            if ($bContact)
            {
              $contactsRequest['Contact'] = 1;
            }
            $contactsResponse = null;
            if ($this->applicationUsersByApplicationID($contactsRequest, $contactsResponse, $strError))
            {
              $getIssueRow['application']['contacts'] = $contactsResponse;
              if ($bOwner)
              {
                for ($i = 0; !$bUse && $i < sizeof($getIssueRow['application']['contacts']); $i++)
                {
                  if ($getIssueRow['application']['contacts'][$i]['userid'] == $this->getUserID())
                  {
                    $bUse = true;
                  }
                }
              }
            }
            unset($contactsRequest);
            unset($contactsResponse);
          }
          unset($applicationRequest);
          unset($applicationResponse);
        }
        if ($bComments)
        {
          $issueCommentsRequest = array('issue_id'=>$getIssueRow['id']);
          $issueCommentsResponse = null;
          if ($this->applicationIssueComments($issueCommentsRequest, $issueCommentsResponse, $strError))
          {
            $getIssueRow['comments'] = $issueCommentsResponse;
            if ($bOwner)
            {
              for ($i = 0; !$bUse && $i < sizeof($getIssueRow['comments']); $i++)
              {
                if ($getIssueRow['comments'][$i]['userid'] == $this->getUserID())
                {
                  $bUse = true;
                }
              }
            }
          }
          unset($issueCommentsRequest);
          unset($issueCommentsResponse);
        }
        if ($bUse)
        {
          $response[] = $getIssueRow;
        }
      }
    }
    else
    {
      $strError = $getIssue->getError();
    }
    $this->m_centraldb->free($getIssue);

    return $bResult;
  }
  // }}}
  // {{{ applicationIssuesByApplicationID()
  public function applicationIssuesByApplicationID($request, &$response, &$strError)
  {
    $bResult = false;
    $bComments = ((isset($request['comments']) && $request['comments'] == 1)?true:false);
    $bOpen = ((isset($request['open']) && $request['open'] == 1)?true:false);
    $response = array();

    if (isset($request['application_id']) && $request['application_id'] != '')
    {
      $getIssue = $this->m_centraldb->parse('select id, date_format(open_date, \'%Y-%m-%d\') open_date, date_format(close_date, \'%Y-%m-%d\') close_date, date_format(due_date, \'%Y-%m-%d\') due_date, hold, priority from application_issue where application_id = '.$request['application_id'].(($bOpen)?' and (close_date is null or date_format(close_date, \'%Y-%m-%d\') = \'0000-00-00\')':'').' order by close_date, open_date, id');
      if ($getIssue->execute())
      {
        $bResult = true;
        while (($getIssueRow = $getIssue->fetch('assoc')))
        {
          if ($bComments)
          {
            $issueCommentsRequest = array('issue_id'=>$getIssueRow['id']);
            $issueCommentsResponse = null;
            if ($this->applicationIssueComments($issueCommentsRequest, $issueCommentsResponse, $strError))
            {
              $getIssueRow['comments'] = $issueCommentsResponse;
            }
            unset($issueCommentsRequest);
            unset($issueCommentsResponse);
          }
          $response[] = $getIssueRow;
        }
      }
      else
      {
        $strError = $getIssue->getError();
      }
      $this->m_centraldb->free($getIssue);
    }
    else
    {
      $strError = 'Please provide the application_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationNotify()
  public function applicationNotify($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      if (isset($request['notification']) && $request['notification'] != '')
      {
        $strServer = null;
        if (isset($request['server']) && $request['server'] != '')
        {
          $strServer = $request['server'];
        }
        else if (isset($_SERVER['SERVER_NAME']) && $_SERVER['SERVER_NAME'] != '')
        {
          $strServer = $_SERVER['SERVER_NAME'];
        }
        if ($strServer != '')
        {
          $developerRequest = array('id'=>$request['id']);
          $developerResponse = null;
          if ($this->isGlobalAdmin() || $this->isApplicationDeveloper($developerRequest, $developerResponse, $strError))
          {
            $applicationRequest = array('id'=>$request['id']);
            $applicationResponse = null;
            if ($this->application($applicationRequest, $applicationResponse, $strError))
            {
              $contactsRequest = array('application_id'=>$request['id'], 'Primary Developer'=>1, 'Backup Developer'=>1, 'Primary Contact'=>1, 'Contact'=>1);
              $contactsResponse = null;
              if ($this->applicationUsersByApplicationID($contactsRequest, $contactsResponse, $strError))
              {
                $bResult = true;
                $developer = array('primary'=>array(), 'backup'=>array());
                $nSize = sizeof($contactsResponse);
                for ($i = 0; $i < $nSize; $i++)
                {
                  if ($contactsResponse[$i]['notify']['value'] == 1)
                  {
                    if (!isset($response[$contactsResponse[$i]['userid']]))
                    {
                      $response[$contactsResponse[$i]['userid']] = array();
                    }
                    $response[$contactsResponse[$i]['userid']]['sent'] = false;
                    $response[$contactsResponse[$i]['userid']]['email'] = $contactsResponse[$i]['email'];
                    $response[$contactsResponse[$i]['userid']]['name'] = $contactsResponse[$i]['last_name'].', '.$contactsResponse[$i]['first_name'];
                  }
                  if ($contactsResponse[$i]['type']['type'] == 'Primary Developer')
                  {
                    if ($contactsResponse[$i]['notify']['value'] == 1)
                    {
                      $response[$contactsResponse[$i]['userid']]['primary'] = true;
                    }
                    $developer['primary'][$contactsResponse[$i]['userid']] = $contactsResponse[$i]['last_name'].', '.$contactsResponse[$i]['first_name'];
                  }
                  else if ($contactsResponse[$i]['type']['type'] == 'Backup Developer')
                  {
                    if ($contactsResponse[$i]['notify']['value'] == 1)
                    {
                      $response[$contactsResponse[$i]['userid']]['backup'] = true;
                    }
                    $developer['backup'][$contactsResponse[$i]['userid']] = $contactsResponse[$i]['last_name'].', '.$contactsResponse[$i]['first_name'];
                  }
                  else if ($contactsResponse[$i]['notify']['value'] == 1)
                  {
                    $response[$contactsResponse[$i]['userid']]['contact'] = true;
                  }
                }
                $dependentsRequest = array('application_id'=>$request['id'], 'contacts'=>1);
                $dependentsResponse = null;
                if ($this->dependentsByApplicationID($dependentsRequest, $dependentsResponse, $strError))
                {
                  $nSize = sizeof($dependentsResponse['dependents']);
                  for ($i = 0; $i < $nSize; $i++)
                  {
                    if ($dependentsResponse['dependents'][$i]['retirement_date'] == '')
                    {
                      $nSubSize = sizeof($dependentsResponse['dependents'][$i]['contacts']);
                      for ($j = 0; $j < $nSubSize; $j++)
                      {
                        if (($dependentsResponse['dependents'][$i]['contacts'][$j]['type'] == 'Primary Developer' || $dependentsResponse['dependents'][$i]['contacts'][$j]['type'] == 'Backup Developer') && $dependentsResponse['dependents'][$i]['contacts'][$j]['notify']['value'] == 1)
                        {
                          if (!isset($response[$dependentsResponse['dependents'][$i]['contacts'][$j]['userid']]))
                          {
                            $response[$dependentsResponse['dependents'][$i]['contacts'][$j]['userid']] = array();
                          }
                          $response[$dependentsResponse['dependents'][$i]['contacts'][$j]['userid']]['sent'] = false;
                          $response[$dependentsResponse['dependents'][$i]['contacts'][$j]['userid']]['email'] = $dependentsResponse['dependents'][$i]['contacts'][$j]['email'];
                          $response[$dependentsResponse['dependents'][$i]['contacts'][$j]['userid']]['name'] = $dependentsResponse['dependents'][$i]['contacts'][$j]['last_name'].', '.$dependentsResponse['dependents'][$i]['contacts'][$j]['first_name'];
                          if (!isset($response[$dependentsResponse['dependents'][$i]['contacts'][$j]['userid']]['depend']))
                          {
                              $response[$dependentsResponse['dependents'][$i]['contacts'][$j]['userid']]['depend'] = array();
                            }
                          $response[$dependentsResponse['dependents'][$i]['contacts'][$j]['userid']]['depend'][] = $dependentsResponse['dependents'][$i]['name'];
                        }
                      }
                    }
                  }
                }
                unset($dependentsRequest);
                unset($dependentsResponse);
                $strSubject = 'Application Notification:  '.$applicationResponse['name'];
                foreach ($response as $key => $value)
                {
                  if ($key != '' && $value['email'] != '')
                  {
                    $strMessage  = '<html><body>';
                    $strMessage .= '<div style="font-family: arial, helvetica, sans-serif; font-size: 12px;">';
                    $strMessage .= '<b><h3>Application Notification:  <a href="http://'.$strServer.'/central/#/Applications/'.$applicationResponse['id'].'">'.$applicationResponse['name'].'</a></b></h3>';
                    $strMessage .= str_replace("\n", '<br>', $request['notification']);
                    $strMessage .= '<br><br>';
                    $strMessage .= '<b>You are receiving this application notification for the following reason(s):</b>';
                    $strMessage .= '<br><br>';
                    $strMessage .= '<ul>';
                    if (isset($value['primary']))
                    {
                      $strMessage .= '<li>You are a Primary Developer for this application.</li>';
                    }
                    else if (isset($value['backup']))
                    {
                      $strMessage .= '<li>You are a Backup Developer for this application.</li>';
                    }
                    else if (isset($value['contact']))
                    {
                      $strMessage .= '<li>You are a Contact for this application.</li>';
                    }
                    if (isset($value['depend']))
                    {
                      $strMessage .= '<li>';
                      $strMessage .= 'You are a developer for the following application(s) which depend on the Service Junction:';
                      $strMessage .= '<ul>';
                      $nSize = sizeof($value['depend']);
                      for ($i = 0; $i < $nSize; $i++)
                      {
                        $strMessage .= '<li>'.$value['depend'][$i].'</li>';
                      }
                      $strMessage .= '</ul>';
                      $strMessage .= '</li>';
                    }
                    $strMessage .= '</ul>';
                    if (sizeof($developer['primary']) > 0)
                    {
                      $strMessage .= '<br><br>';
                      $strMessage .= '<b>Primary Developer(s):</b><br>';
                      $bFirst = true;
                      foreach ($developer['primary'] as $subkey => $subvalue)
                      {
                        if ($bFirst)
                        {
                          $bFirst = false;
                        }
                        else
                        {
                          $strMessage .= ', ';
                        }
                        $strMessage .= '<a href="http://'.$strServer.'/central/#/Users/?userid='.rawurlencode($subkey).'">'.$subvalue.'</a>';
                      }
                    }
                    if (sizeof($developer['backup']) > 0)
                    {
                      $strMessage .= '<br><br>';
                      $strMessage .= '<b>Backup Developer(s):</b><br>';
                      $bFirst = true;
                      foreach ($developer['backup'] as $subkey => $subvalue)
                      {
                        if ($bFirst)
                        {
                          $bFirst = false;
                        }
                        else
                        {
                          $strMessage .= ', ';
                        }
                        $strMessage .= '<a href="http://'.$strServer.'/central/#/Users/?userid='.rawurlencode($subkey).'">'.$subvalue.'</a>';
                      }
                    }
                    $strMessage .= '<br><br>';
                    $strMessage .= 'If you have any questions or concerns, please contact your application contacts.';
                    $strMessage .= '</div>';
                    $strMessage .= '</body></html>';
                    if ($this->m_junction->email($this->getUserEmail(), $value['email'], $strSubject, null, $strMessage))
                    {
                      $response[$key]['sent'] = 1;
                    }
                    else
                    {
                      $response[$key]['sent'] = 0;
                    }
                  }
                }
                unset($developer);
              }
              unset($contactsRequest);
              unset($contactsResponse);
            }
            unset($applicationRequest);
            unset($applicationResponse);
          }
          else
          {
            $strError = 'You are not authorized to perform this action.';
          }
          unset($developerRequest);
          unset($developerResponse);
        }
        else
        {
          $strError = 'Please provide the server.';
        }
      }
      else
      {
        $strError = 'Please provide the notification.';
      }
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationRemove()
  public function applicationRemove($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $devrequest = array('id'=>$request['id']);
      $devresponse = null;
      if ($this->isGlobalAdmin() || $this->isApplicationDeveloper($devrequest, $devresponse, $strError))
      {
        $deleteApplication = $this->m_centraldb->parse('delete from application where id = '.$request['id']);
        if ($deleteApplication->execute())
        {
          $bResult = true;
        }
        else
        {
          $strError = $deleteApplication->getError();
        }
      }
      else
      {
        $strError = 'You are not authorized to perform this action.';
      }
      unset($devrequest);
      unset($devresponse);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applications()
  public function applications($request, &$response, &$strError)
  {
    $bResult = false;
    $bContacts = ((isset($request['contacts']) && $request['contacts'] == 1)?true:false);
    $bDependable = ((isset($request['dependable']) && $request['dependable'] == 1)?true:false);
    $bServers = ((isset($request['servers']) && $request['servers'] == 1)?true:false);
    $cLetter = ((isset($request['letter']) && $request['letter'] != '')?$request['letter']:null);
    $response = array();

    $strQuery = 'select id, highlight, menu_id, name, package_type_id, date_format(retirement_date, \'%Y-%m-%d %H:%i:%s\') retirement_date, website from application where 1';
    if ($bDependable)
    {
      $strQuery .= ' and dependable = 1';
    }
    if ($cLetter != '')
    {
      $strQuery .= ' and';
      if ($request['letter'] == '#')
      {
        $strQuery .= ' name regexp \'^[ -@[-`{-~]\'';
      }
      else
      {
        $strQuery .= ' upper(name) like \''.$request['letter'].'%\'';
      }
    }
    $strQuery .= ' order by name';
    if (isset($request['page']) && $request['page'] != '' && is_numeric($request['page']))
    {
      $nNumPerPage = ((isset($request['numPerPage']) && $request['numPerPage'] != '' && is_numeric($request['numPerPage']))?$request['numPerPage']:25);
      $nOffset = $request['page'] * $nNumPerPage;
      $strQuery .= ' limit '.$nNumPerPage.' offset '.$nOffset;
    }
    $getApplication = $this->m_centraldb->parse($strQuery);
    if ($getApplication->execute())
    {
      $bResult = true;
      while (($getApplicationRow = $getApplication->fetch('assoc')))
      {
        if ($bContacts)
        {
          $contactsRequest = array('application_id'=>$getApplicationRow['id'], 'Primary Developer'=>1, 'Backup Developer'=>1, 'Primary Contact'=>1);
          $contactsResponse = null;
          if ($this->applicationUsersByApplicationID($contactsRequest, $contactsResponse, $strError))
          {
            $getApplicationRow['contacts'] = $contactsResponse;
          }
          unset($contactsRequest);
          unset($contactsResponse);
        }
        if ($bServers)
        {
          $serversRequest = array('application_id'=>$getApplicationRow['id']);
          $serversResponse = null;
          if ($this->serversByApplicationID($serversRequest, $serversResponse, $strError))
          {
            $getApplicationRow['servers'] = $serversResponse;
          }
          unset($serversRequest);
          unset($serversResponse);
        }
        $response[] = $getApplicationRow;
      }
    }
    else
    {
      $strError = $getApplication->getError();
    }
    $this->m_centraldb->free($getApplication);

    return $bResult;
  }
  // }}}
  // {{{ applicationsByServerID()
  public function applicationsByServerID($request, &$response, &$strError)
  {
    $bResult = false;
    $bRetired = ((isset($request['retired']) && $request['retired'] == 1)?true:false);
    $response = array();

    if (isset($request['server_id']) && $request['server_id'] != '')
    {
      $strQuery = 'select a.id, b.id application_id, b.name from application_server a, application b where a.application_id = b.id and a.server_id = '.$request['server_id'];
      if (!$bRetired)
      {
        $strQuery .= ' and (b.retirement_date is null or date_format(b.retirement_date, \'%Y-%m-%d %H:%i:%s\') = \'0000-00-00 00:00:00\')';
      }
      $strQuery .= ' order by b.name';
      $getApplication = $this->m_centraldb->parse($strQuery);
      if ($getApplication->execute())
      {
        $bResult = true;
        while (($getApplicationRow = $getApplication->fetch('assoc')))
        {
          $response[] = $getApplicationRow;
        }
      }
      else
      {
        $strError = $getApplication->getError();
      }
      $this->m_centraldb->free($getApplication);
    }
    else
    {
      $strError = 'Please provide the server_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationsByUserID()
  public function applicationsByUserID($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['contact_id']) && $request['contact_id'] != '')
    {
      $getApplication = $this->m_centraldb->parse('select distinct a.id, b.id application_id, b.name, c.type from application_contact a, application b, contact_type c where a.application_id = b.id and a.type_id = c.id and a.contact_id = '.$request['contact_id'].' order by b.name');
      if ($getApplication->execute())
      {
        $bResult = true;
        while (($getApplicationRow = $getApplication->fetch('assoc')))
        {
          $response[] = $getApplicationRow;
        }
      }
      else
      {
        $strError = $getApplication->getError();
      }
      $this->m_centraldb->free($getApplication);
    }
    else
    {
      $strError = 'Please provide the contact_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationServer()
  public function applicationServer($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $getApplicationServer = $this->m_centraldb->parse('select id, application_id, server_id from application_server where id = '.$request['id']);
      if ($getApplicationServer->execute())
      {
        if (($getApplicationServerRow = $getApplicationServer->fetch('assoc')))
        {
          $bResult = true;
          $response = $getApplicationServerRow;
        }
        else
        {
          $strError = 'Failed to find application server.';
        }
      }
      else
      {
        $strError = $insertApplicationServer->getError();
      }
      $this->m_centraldb->free($getApplicationServer);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationServerAdd()
  public function applicationServerAdd($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['application_id']) && $request['application_id'] != '')
    {
      $devrequest = array('id'=>$request['application_id']);
      $devresponse = null;
      if ($this->isGlobalAdmin() || $this->isApplicationDeveloper($devrequest, $devresponse, $strError))
      {
        if (isset($request['server_id']) && $request['server_id'] != '')
        {
          $insertApplicationServer = $this->m_centraldb->parse('insert into application_server (application_id, server_id) values ('.$request['application_id'].', '.$request['server_id'].')');
          if ($insertApplicationServer->execute())
          {
            $bResult = true;
          }
          else
          {
            $strError = $insertApplicationServer->getError();
          }
        }
        else
        {
          $strError = 'Please provide the server_id.';
        }
      }
      else
      {
        $strError = 'You are not authorized to perform this action.';
      }
      unset($devrequest);
      unset($devresponse);
    }
    else
    {
      $strError = 'Please provide the application_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationServerDetail()
  public function applicationServerDetail($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $getApplicationServerDetail = $this->m_centraldb->parse('select id, application_server_id, version, daemon, owner, script, delay, min_processes max_processes, min_image, max_image, min_resident, max_resident from application_server_detail where id = '.$request['id']);
      if ($getApplicationServerDetail->execute())
      {
        if (($getApplicationServerDetailRow = $getApplicationServerDetail->fetch('assoc')))
        {
          $bResult = true;
          $response = $getApplicationServerDetailRow;
        }
        else
        {
          $strError = 'Failed to find application server detail.';
        }
      }
      else
      {
        $strError = $insertApplicationServerDetail->getError();
      }
      $this->m_centraldb->free($getApplicationServerDetail);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationServerDetailAdd()
  public function applicationServerDetailAdd($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['application_server_id']) && $request['application_server_id'] != '')
    {
      $asrequest = array('id'=>$request['application_server_id']);
      $asresponse = null;
      if ($this->isGlobalAdmin() || $this->applicationServer($asrequest, $asresponse, $strError))
      {
        $devrequest = array('id'=>$asresponse['id']);
        $devresponse = null;
        if ($this->isGlobalAdmin() || $this->isApplicationDeveloper($devrequest, $devresponse, $strError))
        {
          $insertApplicationServerDetail = $this->m_centraldb->parse('insert into application_server_detail (application_server_id, version, daemon, owner, script, delay, min_processes, max_processes, min_image, max_image, min_resident, max_resident) values ('.$request['application_server_id'].', '.((isset($request['version']) && $request['version'] != '')?'\''.addslashes($request['version']).'\'':'null').', '.((isset($request['daemon']) && $request['daemon'] != '')?'\''.addslashes($request['daemon']).'\'':'null').', '.((isset($request['owner']) && $request['owner'] != '')?'\''.addslashes($request['owner']).'\'':'null').', '.((isset($request['script']) && $request['script'] != '')?'\''.addslashes($request['script']).'\'':'null').', '.((isset($request['delay']) && $request['delay'] != '')?$request['delay']:0).', '.((isset($request['min_processes']) && $request['min_processes'] != '')?$request['min_processes']:0).', '.((isset($request['max_processes']) && $request['max_processes'] != '')?$request['max_processes']:0).', '.((isset($request['min_image']) && $request['min_image'] != '')?$request['min_image']:0).', '.((isset($request['max_image']) && $request['max_image'] != '')?$request['max_image']:0).', '.((isset($request['min_resident']) && $request['min_resident'] != '')?$request['min_resident']:0).', '.((isset($request['min_resident']) && $request['max_resident'] != '')?$request['max_resident']:0).')');
          if ($insertApplicationServerDetail->execute())
          {
            $bResult = true;
          }
          else
          {
            $strError = $insertApplicationServerDetail->getError();
          }
        }
        else
        {
          $strError = 'You are not authorized to perform this action.';
        }
        unset($devrequest);
        unset($devresponse);
      }
      unset($asrequest);
      unset($asresponse);
    }
    else
    {
      $strError = 'Please provide the application_server_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationServerDetailEdit()
  public function applicationServerDetailEdit($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $asdrequest = array('id'=>$request['id']);
      $asresponse = null;
      if ($this->isGlobalAdmin() || $this->applicationServerDetail($asdrequest, $asdresponse, $strError))
      {
        $asrequest = array('id'=>$asdresponse['application_server_id']);
        $asresponse = null;
        if ($this->isGlobalAdmin() || $this->applicationServer($asrequest, $asresponse, $strError))
        {
          $devrequest = array('id'=>$asresponse['id']);
          $devresponse = null;
          if ($this->isGlobalAdmin() || $this->isApplicationDeveloper($devrequest, $devresponse, $strError))
          {
            $updateApplicationServerDetail = $this->m_centraldb->parse('update application_server_detail set version = '.((isset($request['version']) && $request['version'] != '')?'\''.addslashes($request['version']).'\'':'null').', daemon = '.((isset($request['daemon']) && $request['daemon'] != '')?'\''.addslashes($request['daemon']).'\'':'null').', owner = '.((isset($request['owner']) && $request['owner'] != '')?'\''.addslashes($request['owner']).'\'':'null').', script = '.((isset($request['script']) && $request['script'] != '')?'\''.addslashes($request['script']).'\'':'null').', delay = '.((isset($request['delay']) && $request['delay'] != '')?$request['delay']:'null').', min_processes = '.((isset($request['min_processes']) && $request['min_processes'] != '')?$request['min_processes']:0).', max_processes = '.((isset($request['max_processes']) && $request['max_processes'] != '')?$request['max_processes']:0).', min_image = '.((isset($request['min_image']) && $request['min_image'] != '')?$request['min_image']:0).', max_image = '.((isset($request['max_image']) && $request['max_image'] != '')?$request['max_image']:0).', min_resident = '.((isset($request['min_resident']) && $request['min_resident'] != '')?$request['min_resident']:0).', max_resident = '.((isset($request['max_resident']) && $request['max_resident'] != '')?$request['max_resident']:0).' where id = '.$request['id']);
            if ($updateApplicationServerDetail->execute())
            {
              $bResult = true;
            }
            else
            {
              $strError = $updateApplicationServerDetail->getError();
            }
          }
          else
          {
            $strError = 'You are not authorized to perform this action.';
          }
          unset($devrequest);
          unset($devresponse);
        }
        unset($asrequest);
        unset($asresponse);
      }
      unset($asdrequest);
      unset($asdresponse);
    }
    else
    {
      $strError = 'Please provide the application_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationServerDetailRemove()
  public function applicationServerDetailRemove($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $asdrequest = array('id'=>$request['id']);
      $asresponse = null;
      if ($this->isGlobalAdmin() || $this->applicationServerDetail($asdrequest, $asdresponse, $strError))
      {
        $asrequest = array('id'=>$asdresponse['application_server_id']);
        $asresponse = null;
        if ($this->isGlobalAdmin() || $this->applicationServer($asrequest, $asresponse, $strError))
        {
          $devrequest = array('id'=>$asresponse['id']);
          $devresponse = null;
          if ($this->isGlobalAdmin() || $this->isApplicationDeveloper($devrequest, $devresponse, $strError))
          {
            $deleteApplicationServerDetail = $this->m_centraldb->parse('delete from application_server_detail where id = '.$request['id']);
            if ($deleteApplicationServerDetail->execute())
            {
              $bResult = true;
            }
            else
            {
              $strError = $deleteApplicationServerDetail->getError();
            }
          }
          else
          {
            $strError = 'You are not authorized to perform this action.';
          }
          unset($devrequest);
          unset($devresponse);
        }
        unset($asrequest);
        unset($asresponse);
      }
      unset($asdrequest);
      unset($asdresponse);
    }
    else
    {
      $strError = 'Please provide the application_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationServerDetails()
  public function applicationServerDetails($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['application_server_id']) && $request['application_server_id'] != '')
    {
      $getApplicationServerDetail = $this->m_centraldb->parse('select id, application_server_id, version, daemon, owner, script, delay, min_processes, max_processes, min_image, max_image, min_resident, max_resident from application_server_detail where application_server_id = '.$request['application_server_id']);
      if ($getApplicationServerDetail->execute())
      {
        $bResult = true;
        while (($getApplicationServerDetailRow = $getApplicationServerDetail->fetch('assoc')))
        {
          $response[] = $getApplicationServerDetailRow;
        }
      }
      else
      {
        $strError = $insertApplicationServerDetail->getError();
      }
      $this->m_centraldb->free($getApplicationServerDetail);
    }
    else
    {
      $strError = 'Please provide the application_server_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationServerRemove()
  public function applicationServerRemove($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $serverrequest = array('id'=>$request['id']);
      $serverresponse = null;
      if ($this->applicationServer($serverrequest, $serverresponse, $strError))
      {
        $devrequest = array('id'=>$serverresponse['application_id']);
        $devresponse = null;
        if ($this->isGlobalAdmin() || $this->isApplicationDeveloper($devrequest, $devresponse, $strError))
        {
          $deleteApplicationServer = $this->m_centraldb->parse('delete from application_server where id = '.$request['id']);
          if ($deleteApplicationServer->execute())
          {
            $bResult = true;
          }
          else
          {
            $strError = $deleteApplicationServer->getError();
          }
        }
        else
        {
          $strError = 'You are not authorized to perform this action.';
        }
        unset($devrequest);
        unset($devresponse);
      }
      unset($serverrequest);
      unset($serverresponse);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationUser()
  public function applicationUser($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $getContact = $this->m_centraldb->parse('select a.id, a.application_id, a.admin, a.locked, a.notify, a.description, b.type, c.last_name, c.first_name, c.userid, c.email from application_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and a.id = '.$request['id']);
      if ($getContact->execute())
      {
        if (($getContactRow = $getContact->fetch('assoc')))
        {
          $bResult = true;
          $getContactRow['admin'] = $this->noyes($getContactRow['admin']);
          $getContactRow['locked'] = $this->noyes($getContactRow['locked']);
          $getContactRow['notify'] = $this->noyes($getContactRow['notify']);
          $response = $getContactRow;
        }
        else
        {
          $strError = 'Failed to query results.';
        }
      }
      else
      {
        $strError = $getContact->getError();
      }
      $this->m_centraldb->free($getContact);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationUserAdd()
  public function applicationUserAdd($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['application_id']) && $request['application_id'] != '')
    {
      $applicationrequest = array('id'=>$request['application_id']);
      $applicationresponse = null;
      if ($this->application($applicationrequest, $applicationresponse, $strError))
      {
        $devrequest = array('id'=>$request['application_id']);
        $devresponse = null;
        if ($this->isLocalAdmin($applicationresponse['name']) || $this->isApplicationDeveloper($devrequest, $devresponse, $strError))
        {
          if (isset($request['userid']) && $request['userid'] != '')
          {
            $bReady = false;
            $userrequest = array('userid'=>$request['userid'], 'application_id'=>$request['application_id']);
            $userresponse = null;
            if ($this->user($userrequest, $userresponse, $strError))
            {
              $bReady = true;
            }
            else if ($strError == 'Failed to query results.')
            {
              unset($userresponse);
              if ($this->userAdd($userrequest, $userresponse, $strError))
              {
                unset($userresponse);
                if ($this->user($userrequest, $userresponse, $strError))
                {
                  $bReady = true;
                }
              }
            }
            if ($bReady)
            {
              if (isset($request['type']) && is_array($request['type']) && isset($request['type']['type']) && $request['type']['type'] != '')
              {
                $contacttyperequest = array('type'=>$request['type']['type']);
                $contacttyperesponse = null;
                if ($this->contactType($contacttyperequest, $contacttyperesponse, $strError))
                {
                  if (isset($request['admin']) && is_array($request['admin']))
                  {
                    if (isset($request['locked']) && is_array($request['locked']))
                    {
                      if (isset($request['notify']) && is_array($request['notify']))
                      {
                        if (!isset($request['description']))
                        {
                          $request['description'] = null;
                        }
                        $insertApplicationContact = $this->m_centraldb->parse('insert into application_contact (application_id, contact_id, type_id, admin, locked, notify, description) values ('.$request['application_id'].', '.$userresponse['id'].', '.$contacttyperesponse['id'].', '.$request['admin']['value'].', '.$request['locked']['value'].', '.$request['notify']['value'].', \''.addslashes($request['description']).'\')');
                        if ($insertApplicationContact->execute())
                        {
                          $bResult = true;
                          $this->syslog()->userAccountCreated('Created the user.', $request['userid']);
                        }
                        else
                        {
                          $strError = $insertApplicationContact->getError();
                          $this->syslog()->userAccountCreated('Failed to create the user.', $request['userid'], false);
                        }
                      }
                      else
                      {
                        $strError = 'Please provide the notify.';
                      }
                    }
                    else
                    {
                      $strError = 'Please provide the locked.';
                    }
                  }
                  else
                  {
                    $strError = 'Please provide the admin.';
                  }
                }
                unset($contacttyperequest);
                unset($contacttyperesponse);
              }
              else
              {
                $strError = 'Please provide the type.';
              }
            }
            unset($userrequest);
            unset($userresponse);
          }
          else
          {
            $strError = 'Please provide the userid.';
          }
        }
        else
        {
          $strError = 'You are not authorized to perform this action.';
        }
        unset($devrequest);
        unset($devresponse);
      }
      unset($applicationrequest);
      unset($applicationresponse);
    }
    else
    {
      $strError = 'Please provide the application_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationUserEdit()
  public function applicationUserEdit($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $userrequest = array('id'=>$request['id']);
      $userresponse = null;
      if ($this->applicationUser($userrequest, $userresponse, $strError))
      {
        $applicationrequest = array('id'=>$userresponse['application_id']);
        $applicationresponse = null;
        if ($this->application($applicationrequest, $applicationresponse, $strError))
        {
          $devrequest = array('id'=>$userresponse['application_id']);
          $devresponse = null;
          if ($this->isLocalAdmin($applicationresponse['name']) || $this->isApplicationDeveloper($devrequest, $devresponse, $strError))
          {
            if (isset($request['userid']) && $request['userid'] != '')
            {
              $bReady = false;
              $userrequest = array('userid'=>$request['userid'], 'application_id'=>$userresponse['application_id']);
              $userresponse = null;
              if ($this->user($userrequest, $userresponse, $strError))
              {
                $bReady = true;
              }
              else if ($strError == 'Failed to query results.')
              {
                unset($userresponse);
                if ($this->userAdd($userrequest, $userresponse, $strError))
                {
                  unset($userresponse);
                  if ($this->user($userrequest, $userresponse, $strError))
                  {
                    $bReady = true;
                  }
                }
              }
              if ($bReady)
              {
                if (isset($request['type']) && is_array($request['type']) && isset($request['type']['type']) && $request['type']['type'] != '')
                {
                  $contacttyperequest = array('type'=>$request['type']['type']);
                  $contacttyperesponse = null;
                  if ($this->contactType($contacttyperequest, $contacttyperesponse, $strError))
                  {
                    if (isset($request['admin']) && is_array($request['admin']))
                    {
                      if (isset($request['locked']) && is_array($request['locked']))
                      {
                        if (isset($request['notify']) && is_array($request['notify']))
                        {
                          $updateApplicationContact = $this->m_centraldb->parse('update application_contact set contact_id = '.$userresponse['id'].', type_id = '.$contacttyperesponse['id'].', admin = '.$request['admin']['value'].', locked = '.$request['locked']['value'].', notify = '.$request['notify']['value'].', description = \''.addslashes($request['description']).'\' where id = '.$request['id']);
                          if ($updateApplicationContact->execute())
                          {
                            $bResult = true;
                            $this->syslog()->userAccountModified('Modified the user.', $request['userid']);
                          }
                          else
                          {
                            $strError = $updateApplicationContact->getError();
                            $this->syslog()->userAccountModified('Failed to modify the user.', $request['userid'], false);
                          }
                        }
                        else
                        {
                          $strError = 'Please provide the notify.';
                        }
                      }
                      else
                      {
                        $strError = 'Please provide the locked.';
                      }
                    }
                    else
                    {
                      $strError = 'Please provide the admin.';
                    }
                  }
                  unset($contacttyperequest);
                  unset($contacttyperesponse);
                }
                else
                {
                  $strError = 'Please provide the type.';
                }
              }
            }
            else
            {
              $strError = 'Please provide the userid.';
            }
          }
          else
          {
            $strError = 'You are not authorized to perform this action.';
          }
          unset($devrequest);
          unset($devresponse);
        }
        unset($applicationrequest);
        unset($applicationresponse);
      }
      unset($userrequest);
      unset($userresponse);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationUserRemove()
  public function applicationUserRemove($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $userrequest = array('id'=>$request['id']);
      $userresponse = null;
      if ($this->applicationUser($userrequest, $userresponse, $strError))
      {
        $applicationrequest = array('id'=>$userresponse['application_id']);
        $applicationresponse = null;
        if ($this->application($applicationrequest, $applicationresponse, $strError))
        {
          $devrequest = array('id'=>$userresponse['application_id']);
          $devresponse = null;
          if ($this->isLocalAdmin($applicationresponse['name']) || $this->isApplicationDeveloper($devrequest, $devresponse, $strError))
          {
            $deleteApplicationContact = $this->m_centraldb->parse('delete from application_contact where id = '.$request['id']);
            if ($deleteApplicationContact->execute())
            {
              $bResult = true;
              $this->syslog()->userAccountDeleted('Deleted the user.');
            }
            else
            {
              $strError = $deleteApplicationContact->getError();
              $this->syslog()->userAccountDeleted('Deleted the user.', null, false);
            }
          }
          else
          {
            $strError = 'You are not authorized to perform this action.';
          }
          unset($devrequest);
          unset($devresponse);
        }
        unset($applicationrequest);
        unset($applicationresponse);
      }
      unset($userrequest);
      unset($userresponse);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ applicationUsersByApplicationID()
  public function applicationUsersByApplicationID($request, &$response, &$strError)
  {
    $bResult = false;
    $bPrimaryDeveloper = ((isset($request['Primary Developer']) && $request['Primary Developer'] == 1)?true:false);
    $bBackupDeveloper = ((isset($request['Backup Developer']) && $request['Backup Developer'] == 1)?true:false);
    $bPrimaryContact = ((isset($request['Primary Contact']) && $request['Primary Contact'] == 1)?true:false);
    $bContact = ((isset($request['Contact']) && $request['Contact'] == 1)?true:false);
    $response = array();

    if (isset($request['application_id']) && $request['application_id'] != '')
    {
      $strQuery = 'select a.id, a.application_id, a.admin, a.locked, a.notify, a.description, a.type_id, c.id user_id, c.last_name, c.first_name, c.userid, c.email from application_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and a.application_id = '.$request['application_id'];
      if ($bPrimaryDeveloper || $bBackupDeveloper || $bPrimaryContact || $bContact)
      {
        $bFirst = true;
        $strQuery .= ' and b.type in (';
        if ($bPrimaryDeveloper)
        {
          $strQuery .= ((!$bFirst)?', ':'').'\'Primary Developer\'';
          $bFirst = false;
        }
        if ($bBackupDeveloper)
        {
          $strQuery .= ((!$bFirst)?', ':'').'\'Backup Developer\'';
          $bFirst = false;
        }
        if ($bPrimaryContact)
        {
          $strQuery .= ((!$bFirst)?', ':'').'\'Primary Contact\'';
          $bFirst = false;
        }
        if ($bContact)
        {
          $strQuery .= ((!$bFirst)?', ':'').'\'Contact\'';
          $bFirst = false;
        }
        $strQuery .= ')';
      }
      $strQuery .= ' order by c.last_name, c.first_name, c.userid';
      if (isset($request['page']) && $request['page'] != '' && is_numeric($request['page']))
      {
        $nNumPerPage = ((isset($request['numPerPage']) && $request['numPerPage'] != '' && is_numeric($request['numPerPage']))?$request['numPerPage']:25);
        $nOffset = $request['page'] * $nNumPerPage;
        $strQuery .= ' limit '.$nNumPerPage.' offset '.$nOffset;
      }
      $getContact = $this->m_centraldb->parse($strQuery);
      if ($getContact->execute())
      {
        $bResult = true;
        while (($getContactRow = $getContact->fetch('assoc')))
        {
          $getContactRow['admin'] = $this->noyes($getContactRow['admin']);
          $getContactRow['locked'] = $this->noyes($getContactRow['locked']);
          $getContactRow['notify'] = $this->noyes($getContactRow['notify']);
          $contactrequest = array('id'=>$getContactRow['type_id']);
          $contactresponse = null;
          if ($this->contactType($contactrequest, $contactresponse, $strError))
          {
            $getContactRow['type'] = $contactresponse;
          }
          unset($contactrequest);
          unset($contactresponse);
          $response[] = $getContactRow;
        }
      }
      else
      {
        $strError = $getContact->getError();
      }
      $this->m_centraldb->free($getContact);
    }
    else
    {
      $strError = 'Please provide the application_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ contactType()
  public function contactType($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if ((isset($request['id']) && $request['id'] != '') || (isset($request['type']) && $request['type'] != ''))
    {
      $getContactType = $this->m_centraldb->parse('select id, type from contact_type where'.((isset($request['id']) && $request['id'] != '')?' id = '.$request['id']:((isset($request['type']) && $request['type'] != '')?' type = \''.$request['type'].'\'':'')));
      if ($getContactType->execute())
      {
        $bResult = true;
        if (($getContactTypeRow = $getContactType->fetch('assoc')))
        {
          $response = $getContactTypeRow;
        }
      }
      else
      {
        $strError = $getContactType->getError();
      }
      $this->m_centraldb->free($getContactType);
    }
    else
    {
      $strError = 'Please provide the id or type.';
    }

    return $bResult;
  }
  // }}}
  // {{{ dependentsByApplicationID()
  public function dependentsByApplicationID($request, &$response, &$strError)
  {
    $bResult = false;
    $bContacts = ((isset($request['contacts']) && $request['contacts'] == 1)?true:false);
    $bServers = ((isset($request['servers']) && $request['servers'] == 1)?true:false);
    $response = array('depends'=>array(), 'dependents'=>array());

    if (isset($request['application_id']) && $request['application_id'] != '')
    {
      $getDepend = $this->m_centraldb->parse('select a.id, b.id application_id, b.name from application_dependant a, application b where a.dependant_id = b.id and a.application_id = '.$request['application_id'].' order by b.name');
      if ($getDepend->execute())
      {
        while (($getDependRow = $getDepend->fetch('assoc')))
        {
          if ($bContacts)
          {
            $contactsRequest = array('application_id'=>$getDependRow['application_id'], 'Primary Developer'=>1, 'Backup Developer'=>1, 'Primary Contact'=>1);
            $contactsResponse = null;
            if ($this->applicationUsersByApplicationID($contactsRequest, $contactsResponse, $strError))
            {
              $getDependRow['contacts'] = $contactsResponse;
            }
            unset($contactsRequest);
            unset($contactsResponse);
          }
          if ($bServers)
          {
            $serversRequest = array('application_id'=>$getDependRow['application_id']);
            $serversResponse = null;
            if ($this->serversByApplicationID($serversRequest, $serversResponse, $strError))
            {
              $getDependRow['servers'] = $serversResponse;
            }
            unset($serversRequest);
            unset($serversResponse);
          }
          $response['depends'][] = $getDependRow;
        }
        $getDependent = $this->m_centraldb->parse('select a.id, b.id application_id, b.name from application_dependant a, application b where a.application_id = b.id and a.dependant_id = '.$request['application_id'].' order by b.name');
        if ($getDependent->execute())
        {
          $bResult = true;
          while (($getDependentRow = $getDependent->fetch('assoc')))
          {
            if ($bContacts)
            {
              $contactsRequest = array('application_id'=>$getDependentRow['application_id'], 'Primary Developer'=>1, 'Backup Developer'=>1, 'Primary Contact'=>1);
              $contactsResponse = null;
              if ($this->applicationUsersByApplicationID($contactsRequest, $contactsResponse, $strError))
              {
                $getDependentRow['contacts'] = $contactsResponse;
              }
              unset($contactsRequest);
              unset($contactsResponse);
            }
            if ($bServers)
            {
              $serversRequest = array('application_id'=>$getDependentRow['application_id']);
              $serversResponse = null;
              if ($this->serversByApplicationID($serversRequest, $serversResponse, $strError))
              {
                $getDependentRow['servers'] = $serversResponse;
              }
              unset($serversRequest);
              unset($serversResponse);
            }
            $response['dependents'][] = $getDependentRow;
          }
        }
        else
        {
          $strError = $getDependent->getError();
        }
        $this->m_centraldb->free($getDependent);
      }
      else
      {
        $strError = $getDepend->getError();
      }
      $this->m_centraldb->free($getDepend);
    }
    else
    {
      $strError = 'Please provide the application_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ isApplicationDeveloper()
  public function isApplicationDeveloper($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if ((isset($request['id']) && $request['id'] != ''))
    {
      if ($this->isValid())
      {
        $getApplicationDeveloper = $this->m_centraldb->parse('select a.id from application_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and a.application_id = '.$request['id'].' and b.type in (\'Primary Developer\', \'Backup Developer\') and c.userid = \''.$this->getUserID().'\'');
        if ($getApplicationDeveloper->execute())
        {
          if ($getApplicationDeveloper->fetch('assoc'))
          {
            $bResult = true;
            $this->syslog()->logon('Authorized the user as an application developer', $this->getUserID());
          }
          else
          {
            $strError = 'You are not a developer for this application.';
            $this->syslog()->logon('Failed to authorize the user as an application developer', $this->getUserID(), false);
          }
        }
        else
        {
          $strError = $getApplicationDeveloper->getError();
        }
        $this->m_centraldb->free($getApplicationDeveloper);
      }
      else
      {
        $strError = 'You are not authorized to run this request.';
      }
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ isServerAdmin()
  public function isServerAdmin($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if ((isset($request['id']) && $request['id'] != ''))
    {
      if ($this->isValid())
      {
        $getServerAdmin = $this->m_centraldb->parse('select a.id from server_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and a.server_id = '.$request['id'].' and b.type in (\'Primary Admin\', \'Backup Admin\') and c.userid = \''.$this->getUserID().'\'');
        if ($getServerAdmin->execute())
        {
          if ($getServerAdmin->fetch('assoc'))
          {
            $bResult = true;
            $this->syslog()->logon('Authorized the user as a server administrator', $this->getUserID());
          }
          else
          {
            $strError = 'You are not a admin for this server.';
            $this->syslog()->logon('Failed to authorize the user as a server administrator', $this->getUserID(), false);
          }
        }
        else
        {
          $strError = $getServerAdmin->getError();
        }
        $this->m_centraldb->free($getServerAdmin);
      }
      else
      {
        $strError = 'You are not authorized to run this request.';
      }
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ loginType()
  public function loginType($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $getLoginType = $this->m_centraldb->parse('select id, type from login_type where id = '.$request['id']);
      if ($getLoginType->execute())
      {
        if (($getLoginTypeRow = $getLoginType->fetch('assoc')))
        {
          $bResult = true;
          $response = $getLoginTypeRow;
        }
      }
      else
      {
        $strError = $getLoginType->getError();
      }
      $this->m_centraldb->free($getLoginType);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ loginTypes()
  public function loginTypes($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    $getLoginType = $this->m_centraldb->parse('select id, type from login_type order by type');
    if ($getLoginType->execute())
    {
      $bResult = true;
      while (($getLoginTypeRow = $getLoginType->fetch('assoc')))
      {
        $response[] = $getLoginTypeRow;
      }
    }
    else
    {
      $strError = $getLoginType->getError();
    }
    $this->m_centraldb->free($getLoginType);

    return $bResult;
  }
  // }}}
  // {{{ menuAccess()
  public function menuAccess($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $getMenuAccess = $this->m_centraldb->parse('select id, type from menu_access where id = '.$request['id']);
      if ($getMenuAccess->execute())
      {
        if (($getMenuAccessRow = $getMenuAccess->fetch('assoc')))
        {
          $bResult = true;
          $response = $getMenuAccessRow;
        }
      }
      else
      {
        $strError = $getMenuAccess->getError();
      }
      $this->m_centraldb->free($getMenuAccess);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ menuAccesses()
  public function menuAccesses($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    $getMenuAccess = $this->m_centraldb->parse('select id, type from menu_access order by id');
    if ($getMenuAccess->execute())
    {
      $bResult = true;
      while (($getMenuAccessRow = $getMenuAccess->fetch('assoc')))
      {
        $response[] = $getMenuAccessRow;
      }
    }
    else
    {
      $strError = $getMenuAccess->getError();
    }
    $this->m_centraldb->free($getMenuAccess);

    return $bResult;
  }
  // }}}
  // {{{ notifyPriorities()
  public function notifyPriorities($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    $getNotifyPriority = $this->m_centraldb->parse('select id, priority from notify_priority order by id');
    if ($getNotifyPriority->execute())
    {
      $bResult = true;
      while (($getNotifyPriorityRow = $getNotifyPriority->fetch('assoc')))
      {
        $response[] = $getNotifyPriorityRow;
      }
    }
    else
    {
      $strError = $getNotifyPriority->getError();
    }
    $this->m_centraldb->free($getNotifyPriority);

    return $bResult;
  }
  // }}}
  // {{{ notifyPriority()
  public function notifyPriority($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $getNotifyPriority = $this->m_centraldb->parse('select id, priority from notify_priority where id = '.$request['id']);
      if ($getNotifyPriority->execute())
      {
        if (($getNotifyPriorityRow = $getNotifyPriority->fetch('assoc')))
        {
          $bResult = true;
          $response = $getNotifyPriorityRow;
        }
      }
      else
      {
        $strError = $getNotifyPriority->getError();
      }
      $this->m_centraldb->free($getNotifyPriority);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ packageType()
  public function packageType($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $getPackageType = $this->m_centraldb->parse('select id, type from package_type where id = '.$request['id']);
      if ($getPackageType->execute())
      {
        if (($getPackageTypeRow = $getPackageType->fetch('assoc')))
        {
          $bResult = true;
          $response = $getPackageTypeRow;
        }
      }
      else
      {
        $strError = $getPackageType->getError();
      }
      $this->m_centraldb->free($getPackageType);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ packageTypes()
  public function packageTypes($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    $getPackageType = $this->m_centraldb->parse('select id, type from package_type order by id');
    if ($getPackageType->execute())
    {
      $bResult = true;
      while (($getPackageTypeRow = $getPackageType->fetch('assoc')))
      {
        $response[] = $getPackageTypeRow;
      }
    }
    else
    {
      $strError = $getPackageType->getError();
    }
    $this->m_centraldb->free($getPackageType);

    return $bResult;
  }
  // }}}
  // {{{ noyes()
  public function noyes($nValue)
  {
    $yesno = array(array('name'=>'No', 'value'=>0), array('name'=>'Yes', 'value'=>1));

    return $yesno[$nValue];
  }
  // }}}
  // {{{ server()
  public function server($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if ((isset($request['id']) && $request['id'] != '') || (isset($request['name']) && $request['name'] != ''))
    {
      $getServer = $this->m_centraldb->parse('select id, name, description, processes, cpu_usage, main_memory, swap_memory, disk_size from server where '.((isset($request['id']) && $request['id'] != '')?'id = '.$request['id']:'name = \''.addslashes($request['name']).'\''));
      if ($getServer->execute())
      {
        if (($getServerRow = $getServer->fetch('assoc')))
        {
          $bResult = true;
          $response = $getServerRow;
        }
        else
        {
          $strError = 'Failed to query results.';
        }
      }
      else
      {
        $strError = $getServer->getError();
      }
      $this->m_centraldb->free($getServer);
    }
    else
    {
      $strError = 'Please provide the id or name.';
    }

    return $bResult;
  }
  // }}}
  // {{{ serverAdd()
  public function serverAdd($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if ($this->isValid('Central'))
    {
      if (isset($request['name']) && $request['name'] != '')
      {
        $serverrequest = array('name'=>$request['name']);
        $serverresponse = null;
        if (!$this->server($serverrequest, $serverresponse, $strError) && $strError == 'Failed to query results.')
        {
          $strError = null;
          $insertServer = $this->m_centraldb->parse('insert into server (name) values (\''.addslashes($request['name']).'\')');
          if ($insertServer->execute())
          {
            $bResult = true;
          }
          else
          {
            $strError = $insertServer->getError();
          }
        }
        unset($serverrequest);
        unset($serverresponse);
      }
      else
      {
        $strError = 'Please provide the name.';
      }
    }
    else
    {
      $strError = 'You are not authorized to perform this action.';
    }

    return $bResult;
  }
  // }}}
  // {{{ serverEdit()
  public function serverEdit($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $adminrequest = array('id'=>$request['id']);
      $adminresponse = null;
      if ($this->isGlobalAdmin() || $this->isServerAdmin($adminrequest, $adminresponse, $strError))
      {
        if (isset($request['name']) && $request['name'] != '')
        {
          $strQuery = 'update server set';
          $strQuery .= ' name = \''.addslashes($request['name']).'\'';
          $strQuery .= ', description = '.((isset($request['description']) && $request['description'] != '')?'\''.addslashes($request['description']).'\'':'null');
          $strQuery .= ', processes = '.((isset($request['processes']) && is_numeric($request['processes']))?$request['processes']:0);
          $strQuery .= ', cpu_usage = '.((isset($request['cpu_usage']) && is_numeric($request['cpu_usage']))?$request['cpu_usage']:0);
          $strQuery .= ', main_memory = '.((isset($request['main_memory']) && is_numeric($request['main_memory']))?$request['main_memory']:0);
          $strQuery .= ', swap_memory = '.((isset($request['swap_memory']) && is_numeric($request['swap_memory']))?$request['swap_memory']:0);
          $strQuery .= ', disk_size = '.((isset($request['disk_size']) && is_numeric($request['disk_size']))?$request['disk_size']:0);
          $strQuery .= ' where id = '.$request['id'];
          $updateServer = $this->m_centraldb->parse($strQuery);
          if ($updateServer->execute())
          {
            $bResult = true;
          }
          else
          {
            $strError = $updateServer->getError();
          }
        }
        else
        {
          $strError = 'Please provide the name.';
        }
      }
      else
      {
        $strError = 'You are not authorized to perform this action.';
      }
      unset($adminrequest);
      unset($adminresponse);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ serverRemove()
  public function serverRemove($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $adminrequest = array('id'=>$request['id']);
      $adminresponse = null;
      if ($this->isGlobalAdmin() || $this->isServerAdmin($adminrequest, $adminresponse, $strError))
      {
        $deleteServer = $this->m_centraldb->parse('delete from server where id = '.$request['id']);
        if ($deleteServer->execute())
        {
          $bResult = true;
        }
        else
        {
          $strError = $deleteServer->getError();
        }
      }
      else
      {
        $strError = 'You are not authorized to perform this action.';
      }
      unset($adminrequest);
      unset($adminresponse);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ servers()
  public function servers($request, &$response, &$strError)
  {
    $bResult = false;
    $bContacts = ((isset($request['contacts']) && $request['contacts'] == 1)?true:false);
    $cLetter = ((isset($request['letter']) && $request['letter'] != '')?$request['letter']:null);
    $response = array();

    $strQuery = 'select id, name from server';
    if ($cLetter != '')
    {
      $strQuery .= ' where';
      if ($request['letter'] == '#')
      {
        $strQuery .= ' name regexp \'^[ -@[-`{-~]\'';
      }
      else
      {
        $strQuery .= ' upper(name) like \''.$request['letter'].'%\'';
      }
    }
    $strQuery .= ' order by name';
    if (isset($request['page']) && $request['page'] != '' && is_numeric($request['page']))
    {
      $nNumPerPage = ((isset($request['numPerPage']) && $request['numPerPage'] != '' && is_numeric($request['numPerPage']))?$request['numPerPage']:25);
      $nOffset = $request['page'] * $nNumPerPage;
      $strQuery .= ' limit '.$nNumPerPage.' offset '.$nOffset;
    }
    $getServer = $this->m_centraldb->parse($strQuery);
    if ($getServer->execute())
    {
      $bResult = true;
      while (($getServerRow = $getServer->fetch('assoc')))
      {
        if ($bContacts)
        {
          $contactsRequest = array('server_id'=>$getServerRow['id'], 'Primary Admin'=>1, 'Backup Admin'=>1, 'Primary Contact'=>1);
          $contactsResponse = null;
          if ($this->serverUsersByServerID($contactsRequest, $contactsResponse, $strError))
          {
            $getServerRow['contacts'] = $contactsResponse;
          }
          unset($contactsRequest);
          unset($contactsResponse);
        }
        $response[] = $getServerRow;
      }
    }
    else
    {
      $strError = $getServer->getError();
    }
    $this->m_centraldb->free($getServer);

    return $bResult;
  }
  // }}}
  // {{{ serversByApplicationID()
  public function serversByApplicationID($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['application_id']) && $request['application_id'] != '')
    {
      $getServer = $this->m_centraldb->parse('select a.id, b.id server_id, b.name from application_server a, server b where a.server_id = b.id and a.application_id = '.$request['application_id'].' order by b.name');
      if ($getServer->execute())
      {
        $bResult = true;
        while (($getServerRow = $getServer->fetch('assoc')))
        {
          $response[] = $getServerRow;
        }
      }
      else
      {
        $strError = $getServer->getError();
      }
      $this->m_centraldb->free($getServer);
    }
    else
    {
      $strError = 'Please provide the application_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ serversByUserID()
  public function serversByUserID($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['contact_id']) && $request['contact_id'] != '')
    {
      $getServer = $this->m_centraldb->parse('select distinct a.id, b.id server_id, b.name, c.type from server_contact a, server b, contact_type c where a.server_id = b.id and a.type_id = c.id and a.contact_id = '.$request['contact_id'].' order by b.name');
      if ($getServer->execute())
      {
        $bResult = true;
        while (($getServerRow = $getServer->fetch('assoc')))
        {
          $response[] = $getServerRow;
        }
      }
      else
      {
        $strError = $getServer->getError();
      }
      $this->m_centraldb->free($getServer);
    }
    else
    {
      $strError = 'Please provide the contact_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ serverDetailsByApplicationID()
  public function serverDetailsByApplicationID($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['application_id']) && $request['application_id'] != '')
    {
      $getApplicationServerDetail = $this->m_centraldb->parse('select a.id application_server_id, b.id server_id, b.name, c.id application_server_detail_id, c.daemon from application_server a, server b, application_server_detail c where a.server_id = b.id and a.id = c.application_server_id and a.application_id = '.$request['application_id'].' order by b.name, c.daemon');
      if ($getApplicationServerDetail->execute())
      {
        $bResult = true;
        while (($getApplicationServerDetailRow = $getApplicationServerDetail->fetch('assoc')))
        {
          $response[] = $getApplicationServerDetailRow;
        }
      }
      else
      {
        $strError = $getApplicationServerDetail->getError();
      }
      $this->m_centraldb->free($getApplicationServerDetail);
    }
    else
    {
      $strError = 'Please provide the application_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ serverNotify()
  public function serverNotify($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      if (isset($request['notification']) && $request['notification'] != '')
      {
        $strServer = null;
        if (isset($request['server']) && $request['server'] != '')
        {
          $strServer = $request['server'];
        }
        else if (isset($_SERVER['SERVER_NAME']) && $_SERVER['SERVER_NAME'] != '')
        {
          $strServer = $_SERVER['SERVER_NAME'];
        }
        if ($strServer != '')
        {
          $adminRequest = array('id'=>$request['id']);
          $adminResponse = null;
          if ($this->isGlobalAdmin() || $this->isServerAdmin($adminRequest, $adminResponse, $strError))
          {
            $serverRequest = array('id'=>$request['id']);
            $serverResponse = null;
            if ($this->server($serverRequest, $serverResponse, $strError))
            {
              $contactsRequest = array('server_id'=>$request['id'], 'Primary Admin'=>1, 'Backup Admin'=>1, 'Primary Contact'=>1, 'Contact'=>1);
              $contactsResponse = null;
              if ($this->serverUsersByServerID($contactsRequest, $contactsResponse, $strError))
              {
                $bResult = true;
                $admin = array('primary'=>array(), 'backup'=>array());
                $nSize = sizeof($contactsResponse);
                for ($i = 0; $i < $nSize; $i++)
                {
                  if ($contactsResponse[$i]['notify']['value'] == 1)
                  {
                    if (!isset($response[$contactsResponse[$i]['userid']]))
                    {
                      $response[$contactsResponse[$i]['userid']] = array();
                    }
                    $response[$contactsResponse[$i]['userid']]['sent'] = false;
                    $response[$contactsResponse[$i]['userid']]['email'] = $contactsResponse[$i]['email'];
                    $response[$contactsResponse[$i]['userid']]['name'] = $contactsResponse[$i]['last_name'].', '.$contactsResponse[$i]['first_name'];
                  }
                  if ($contactsResponse[$i]['type']['type'] == 'Primary Admin')
                  {
                    if ($contactsResponse[$i]['notify']['value'] == 1)
                    {
                      $response[$contactsResponse[$i]['userid']]['primary'] = true;
                    }
                    $admin['primary'][$contactsResponse[$i]['userid']] = $contactsResponse[$i]['last_name'].', '.$contactsResponse[$i]['first_name'];
                  }
                  else if ($contactsResponse[$i]['type']['type'] == 'Backup Admin')
                  {
                    if ($contactsResponse[$i]['notify']['value'] == 1)
                    {
                      $response[$contactsResponse[$i]['userid']]['backup'] = true;
                    }
                    $admin['backup'][$contactsResponse[$i]['userid']] = $contactsResponse[$i]['last_name'].', '.$contactsResponse[$i]['first_name'];
                  }
                  else if ($contactsResponse[$i]['notify']['value'] == 1)
                  {
                    $response[$contactsResponse[$i]['userid']]['contact'] = true;
                  }
                }
                $strSubject = 'Server Notification:  '.$serverResponse['name'];
                foreach ($response as $key => $value)
                {
                  if ($key != '' && $value['email'] != '')
                  {
                    $strMessage  = '<html><body>';
                    $strMessage .= '<div style="font-family: arial, helvetica, sans-serif; font-size: 12px;">';
                    $strMessage .= '<b><h3>Server Notification:  <a href="http://'.$strServer.'/central/#/Servers/'.$serverResponse['id'].'">'.$serverResponse['name'].'</a></b></h3>';
                    $strMessage .= str_replace("\n", '<br>', $request['notification']);
                    $strMessage .= '<br><br>';
                    $strMessage .= '<b>You are receiving this server notification for the following reason(s):</b>';
                    $strMessage .= '<br><br>';
                    $strMessage .= '<ul>';
                    if (isset($value['primary']))
                    {
                      $strMessage .= '<li>You are a Primary Admin for this server.</li>';
                    }
                    else if (isset($value['backup']))
                    {
                      $strMessage .= '<li>You are a Backup Admin for this server.</li>';
                    }
                    else if (isset($value['contact']))
                    {
                      $strMessage .= '<li>You are a Contact for this server.</li>';
                    }
                    $strMessage .= '</ul>';
                    if (sizeof($admin['primary']) > 0)
                    {
                      $strMessage .= '<br><br>';
                      $strMessage .= '<b>Primary Admin(s):</b><br>';
                      $bFirst = true;
                      foreach ($admin['primary'] as $subkey => $subvalue)
                      {
                        if ($bFirst)
                        {
                          $bFirst = false;
                        }
                        else
                        {
                          $strMessage .= ', ';
                        }
                        $strMessage .= '<a href="http://'.$strServer.'/central/#/Users/?userid='.rawurlencode($subkey).'">'.$subvalue.'</a>';
                      }
                    }
                    if (sizeof($admin['backup']) > 0)
                    {
                      $strMessage .= '<br><br>';
                      $strMessage .= '<b>Backup Admin(s):</b><br>';
                      $bFirst = true;
                      foreach ($admin['backup'] as $subkey => $subvalue)
                      {
                        if ($bFirst)
                        {
                          $bFirst = false;
                        }
                        else
                        {
                          $strMessage .= ', ';
                        }
                        $strMessage .= '<a href="http://'.$strServer.'/central/#/Users/?userid='.rawurlencode($subkey).'">'.$subvalue.'</a>';
                      }
                    }
                    $strMessage .= '<br><br>';
                    $strMessage .= 'If you have any questions or concerns, please contact your server contacts.';
                    $strMessage .= '</div>';
                    $strMessage .= '</body></html>';
                    if ($this->m_junction->email($this->getUserEmail(), $value['email'], $strSubject, null, $strMessage))
                    {
                      $response[$key]['sent'] = 1;
                    }
                    else
                    {
                      $response[$key]['sent'] = 0;
                    }
                  }
                }
                $applicationsRequest = array('server_id'=>$request['id']);
                $applicationsResponse = null;
                if ($this->applicationsByServerID($applicationsRequest, $applicationsResponse, $strError))
                {
                  $subResponse = array();
                  $nSize = sizeof($applicationsResponse);
                  for ($i = 0; $i < $nSize; $i++)
                  {
                    $applicationRequest = array('id'=>$applicationsResponse[$i]['application_id']);
                    $applicationResponse = null;
                    if ($this->application($applicationRequest, $applicationResponse, $strError))
                    {
                      if ($applicationResponse['retirement_date'] == '')
                      {
                        $contactsRequest = array('application_id'=>$applicationsResponse[$i]['application_id'], 'Primary Developer'=>1, 'Backup Developer'=>1, 'Primary Contact'=>1, 'Contact'=>1);
                        $contactsResponse = null;
                        if ($this->applicationUsersByApplicationID($contactsRequest, $contactsResponse, $strError))
                        {
                          $bResult = true;
                          $nSubSize = sizeof($contactsResponse);
                          for ($j = 0; $j < $nSubSize; $j++)
                          {
                            if ($contactsResponse[$j]['notify']['value'] == 1)
                            {
                              if (!isset($subResponse[$contactsResponse[$j]['userid']]))
                              {
                                $subResponse[$contactsResponse[$j]['userid']] = array();
                              }
                              if (!isset($subResponse[$contactsResponse[$j]['userid']][$applicationsResponse[$i]['application_id']]))
                              {
                                $subResponse[$contactsResponse[$j]['userid']][$applicationsResponse[$i]['application_id']] = array();
                                $subResponse[$contactsResponse[$j]['userid']][$applicationsResponse[$i]['application_id']]['application'] = $applicationResponse['name'];
                              }
                              $subResponse[$contactsResponse[$j]['userid']][$applicationsResponse[$i]['application_id']]['sent'] = false;
                              $subResponse[$contactsResponse[$j]['userid']][$applicationsResponse[$i]['application_id']]['email'] = $contactsResponse[$j]['email'];
                              $subResponse[$contactsResponse[$j]['userid']][$applicationsResponse[$i]['application_id']]['name'] = $contactsResponse[$j]['last_name'].', '.$contactsResponse[$j]['first_name'];
                              if ($contactsResponse[$j]['type']['type'] == 'Primary Developer')
                              {
                                $subResponse[$contactsResponse[$j]['userid']][$applicationsResponse[$i]['application_id']]['primary'] = true;
                              }
                              else if ($contactsResponse[$j]['type']['type'] == 'Backup Developer')
                              {
                                $subResponse[$contactsResponse[$j]['userid']][$applicationsResponse[$i]['application_id']]['backup'] = true;
                              }
                            }
                          }
                        }
                        unset($contactsRequest);
                        unset($contactsResponse);
                      }
                    }
                    unset($applicationRequest);
                    unset($applicationResponse);
                  }
                  $strSubject = 'Server Notification:  '.$serverResponse['name'];
                  foreach ($subResponse as $key => $value)
                  {
                    if ($key != '')
                    {
                      $strEmail = null;
                      $strMessage  = '<html><body>';
                      $strMessage .= '<div style="font-family: arial, helvetica, sans-serif; font-size: 12px;">';
                      $strMessage .= '<b><h3>Server Notification:  <a href="http://'.$strServer.'/central/#/Servers/'.$serverResponse['id'].'">'.$serverResponse['name'].'</a></b></h3>';
                      $strMessage .= str_replace("\n", '<br>', $request['notification']);
                      $strMessage .= '<br><br>';
                      $strMessage .= '<b>You are associated with the following applications that depend upon this server:</b>';
                      $strMessage .= '<br><br>';
                      $strMessage .= '<ul>';
                      foreach ($value as $subkey => $subvalue)
                      {
                        if ($subvalue['email'] != '')
                        {
                          $strEmail = $subvalue['email'];
                          $strMessage .= '<li><a href="http://'.$strServer.'/central/#/Applications/'.$subkey.'">'.$subvalue['application'].'</a>';
                          if (isset($subvalue['primary']))
                          {
                            $strMessage .= ':  You are a Primary Developer for this application.';
                          }
                          else if (isset($subvalue['backup']))
                          {
                            $strMessage .= ':  You are a Backup Developer for this application.';
                          }
                          $strMessage .= '</li>';
                        }
                      }
                      $strMessage .= '</ul>';
                      if (sizeof($admin['primary']) > 0)
                      {
                        $strMessage .= '<br><br>';
                        $strMessage .= '<b>Primary Server Admin(s):</b><br>';
                        $bFirst = true;
                        foreach ($admin['primary'] as $subkey => $subvalue)
                        {
                          if ($bFirst)
                          {
                            $bFirst = false;
                          }
                          else
                          {
                            $strMessage .= ', ';
                          }
                          $strMessage .= '<a href="http://'.$strServer.'/central/#/Users/?userid='.rawurlencode($subkey).'">'.$subvalue.'</a>';
                        }
                      }
                      if (sizeof($admin['backup']) > 0)
                      {
                        $strMessage .= '<br><br>';
                        $strMessage .= '<b>Backup Server Admin(s):</b><br>';
                        $bFirst = true;
                        foreach ($admin['backup'] as $subkey => $subvalue)
                        {
                          if ($bFirst)
                          {
                            $bFirst = false;
                          }
                          else
                          {
                            $strMessage .= ', ';
                          }
                          $strMessage .= '<a href="http://'.$strServer.'/central/#/Users/?userid='.rawurlencode($subkey).'">'.$subvalue.'</a>';
                        }
                      }
                      $strMessage .= '<br><br>';
                      $strMessage .= 'If you have any questions or concerns, please contact primary/backup server admins listed above.';
                      $strMessage .= '</div>';
                      $strMessage .= '</body></html>';
                      if ($strEmail != '' && $this->m_junction->email($this->getUserEmail(), $strEmail, $strSubject, null, $strMessage))
                      {
                        $subResponse[$key][$subkey]['sent'] = 1;
                      }
                      else
                      {
                        $subResponse[$key][$subkey]['sent'] = 0;
                      }
                    }
                  }
                  unset($subResponse);
                }
                unset($applicationsRequest);
                unset($applicationsResponse);
                unset($admin);
              }
              unset($contactsRequest);
              unset($contactsResponse);
            }
            unset($serverRequest);
            unset($serverResponse);
          }
          else
          {
            $strError = 'You are not authorized to perform this action.';
          }
          unset($adminRequest);
          unset($adminResponse);
        }
        else
        {
          $strError = 'Please provide the server.';
        }
      }
      else
      {
        $strError = 'Please provide the notification.';
      }
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ serverUser()
  public function serverUser($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $getContact = $this->m_centraldb->parse('select a.id, a.server_id, a.notify, b.type, c.last_name, c.first_name, c.userid, c.email from server_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and a.id = '.$request['id']);
      if ($getContact->execute())
      {
        if (($getContactRow = $getContact->fetch('assoc')))
        {
          $bResult = true;
          $getContactRow['notify'] = $this->noyes($getContactRow['notify']);
          $response = $getContactRow;
        }
        else
        {
          $strError = 'Failed to query results.';
        }
      }
      else
      {
        $strError = $getContact->getError();
      }
      $this->m_centraldb->free($getContact);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ serverUserAdd()
  public function serverUserAdd($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['server_id']) && $request['server_id'] != '')
    {
      $devrequest = array('id'=>$request['server_id']);
      $devresponse = null;
      if ($this->isGlobalAdmin() || $this->isServerAdmin($devrequest, $devresponse, $strError))
      {
        if (isset($request['userid']) && $request['userid'] != '')
        {
          $bReady = false;
          $userrequest = array('userid'=>$request['userid'], 'server_id'=>$request['server_id']);
          $userresponse = null;
          if ($this->user($userrequest, $userresponse, $strError))
          {
            $bReady = true;
          }
          else if ($strError == 'Failed to query results.')
          {
            unset($userresponse);
            if ($this->userAdd($userrequest, $userresponse, $strError))
            {
              unset($userresponse);
              if ($this->user($userrequest, $userresponse, $strError))
              {
                $bReady = true;
              }
            }
          }
          if ($bReady)
          {
            if (isset($request['type']) && is_array($request['type']) && isset($request['type']['type']) && $request['type']['type'] != '')
            {
              $contacttyperequest = array('type'=>$request['type']['type']);
              $contacttyperesponse = null;
              if ($this->contactType($contacttyperequest, $contacttyperesponse, $strError))
              {
                if (isset($request['notify']) && is_array($request['notify']))
                {
                  $insertServerContact = $this->m_centraldb->parse('insert into server_contact (server_id, contact_id, type_id, notify) values ('.$request['server_id'].', '.$userresponse['id'].', '.$contacttyperesponse['id'].', '.$request['notify']['value'].')');
                  if ($insertServerContact->execute())
                  {
                    $bResult = true;
                    $this->syslog()->userAccountCreated('Created the user.', $request['userid']);
                  }
                  else
                  {
                    $strError = $insertServerContact->getError();
                    $this->syslog()->userAccountCreated('Failed to create the user.', $request['userid'], false);
                  }
                }
                else
                {
                  $strError = 'Please provide the notify.';
                }
              }
              unset($contacttyperequest);
              unset($contacttyperesponse);
            }
            else
            {
              $strError = 'Please provide the type.';
            }
          }
          unset($userrequest);
          unset($userresponse);
        }
        else
        {
          $strError = 'Please provide the userid.';
        }
      }
      else
      {
        $strError = 'You are not authorized to perform this action.';
      }
      unset($devrequest);
      unset($devresponse);
    }
    else
    {
      $strError = 'Please provide the server_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ serverUserEdit()
  public function serverUserEdit($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $userrequest = array('id'=>$request['id']);
      $userresponse = null;
      if ($this->serverUser($userrequest, $userresponse, $strError))
      {
        $devrequest = array('id'=>$userresponse['server_id']);
        $devresponse = null;
        if ($this->isGlobalAdmin() || $this->isServerAdmin($devrequest, $devresponse, $strError))
        {
          if (isset($request['userid']) && $request['userid'] != '')
          {
            $bReady = false;
            $userrequest = array('userid'=>$request['userid'], 'server_id'=>$userresponse['server_id']);
            $userresponse = null;
            if ($this->user($userrequest, $userresponse, $strError))
            {
              $bReady = true;
            }
            else if ($strError == 'Failed to query results.')
            {
              unset($userresponse);
              if ($this->userAdd($userrequest, $userresponse, $strError))
              {
                unset($userresponse);
                if ($this->user($userrequest, $userresponse, $strError))
                {
                  $bReady = true;
                }
              }
            }
            if ($bReady)
            {
              if (isset($request['type']) && is_array($request['type']) && isset($request['type']['type']) && $request['type']['type'] != '')
              {
                $contacttyperequest = array('type'=>$request['type']['type']);
                $contacttyperesponse = null;
                if ($this->contactType($contacttyperequest, $contacttyperesponse, $strError))
                {
                  if (isset($request['notify']) && is_array($request['notify']))
                  {
                    $updateServerContact = $this->m_centraldb->parse('update server_contact set contact_id = '.$userresponse['id'].', type_id = '.$contacttyperesponse['id'].', notify = '.$request['notify']['value'].' where id = '.$request['id']);
                    if ($updateServerContact->execute())
                    {
                      $bResult = true;
                      $this->syslog()->userAccountModified('Modified the user.', $request['userid']);
                    }
                    else
                    {
                      $strError = $updateServerContact->getError();
                      $this->syslog()->userAccountModified('Failed to modify the user.', $request['userid'], false);
                    }
                  }
                  else
                  {
                    $strError = 'Please provide the notify.';
                  }
                }
                unset($contacttyperequest);
                unset($contacttyperesponse);
              }
              else
              {
                $strError = 'Please provide the type.';
              }
            }
          }
          else
          {
            $strError = 'Please provide the userid.';
          }
        }
        else
        {
          $strError = 'You are not authorized to perform this action.';
        }
        unset($devrequest);
        unset($devresponse);
      }
      unset($userrequest);
      unset($userresponse);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ serverUserRemove()
  public function serverUserRemove($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $userrequest = array('id'=>$request['id']);
      $userresponse = null;
      if ($this->serverUser($userrequest, $userresponse, $strError))
      {
        $devrequest = array('id'=>$userresponse['server_id']);
        $devresponse = null;
        if ($this->isGlobalAdmin() || $this->isServerAdmin($devrequest, $devresponse, $strError))
        {
          $deleteServerContact = $this->m_centraldb->parse('delete from server_contact where id = '.$request['id']);
          if ($deleteServerContact->execute())
          {
            $bResult = true;
            $this->syslog()->userAccountDeleted('Deleted the user.');
          }
          else
          {
            $strError = $deleteServerContact->getError();
            $this->syslog()->userAccountDeleted('Failed to delete the user.', null, false);
          }
        }
        else
        {
          $strError = 'You are not authorized to perform this action.';
        }
        unset($devrequest);
        unset($devresponse);
      }
      unset($userrequest);
      unset($userresponse);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ serverUsersByServerID()
  public function serverUsersByServerID($request, &$response, &$strError)
  {
    $bResult = false;
    $bPrimaryAdmin = ((isset($request['Primary Admin']) && $request['Primary Admin'] == 1)?true:false);
    $bBackupAdmin = ((isset($request['Backup Admin']) && $request['Backup Admin'] == 1)?true:false);
    $bPrimaryContact = ((isset($request['Primary Contact']) && $request['Primary Contact'] == 1)?true:false);
    $bContact = ((isset($request['Contact']) && $request['Contact'] == 1)?true:false);
    $response = array();

    if (isset($request['server_id']) && $request['server_id'] != '')
    {
      $strQuery = 'select a.id, a.server_id, a.notify, a.type_id, c.id user_id, c.last_name, c.first_name, c.userid, c.email from server_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and a.server_id = '.$request['server_id'];
      if ($bPrimaryAdmin || $bBackupAdmin || $bPrimaryContact || $bContact)
      {
        $bFirst = true;
        $strQuery .= ' and b.type in (';
        if ($bPrimaryAdmin)
        {
          $strQuery .= ((!$bFirst)?', ':'').'\'Primary Admin\'';
          $bFirst = false;
        }
        if ($bBackupAdmin)
        {
          $strQuery .= ((!$bFirst)?', ':'').'\'Backup Admin\'';
          $bFirst = false;
        }
        if ($bPrimaryContact)
        {
          $strQuery .= ((!$bFirst)?', ':'').'\'Primary Contact\'';
          $bFirst = false;
        }
        if ($bContact)
        {
          $strQuery .= ((!$bFirst)?', ':'').'\'Contact\'';
          $bFirst = false;
        }
        $strQuery .= ')';
      }
      $strQuery .= ' order by c.last_name, c.first_name, c.userid';
      if (isset($request['page']) && $request['page'] != '' && is_numeric($request['page']))
      {
        $nNumPerPage = ((isset($request['numPerPage']) && $request['numPerPage'] != '' && is_numeric($request['numPerPage']))?$request['numPerPage']:25);
        $nOffset = $request['page'] * $nNumPerPage;
        $strQuery .= ' limit '.$nNumPerPage.' offset '.$nOffset;
      }
      $getContact = $this->m_centraldb->parse($strQuery);
      if ($getContact->execute())
      {
        $bResult = true;
        while (($getContactRow = $getContact->fetch('assoc')))
        {
          $getContactRow['notify'] = $this->noyes($getContactRow['notify']);
          $contactrequest = array('id'=>$getContactRow['type_id']);
          $contactresponse = null;
          if ($this->contactType($contactrequest, $contactresponse, $strError))
          {
            $getContactRow['type'] = $contactresponse;
          }
          unset($contactrequest);
          unset($contactresponse);
          $response[] = $getContactRow;
        }
      }
      else
      {
        $strError = $getContact->getError();
      }
      $this->m_centraldb->free($getContact);
    }
    else
    {
      $strError = 'Please provide the server_id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ user()
  public function user($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if ((isset($request['id']) && $request['id'] != '') || (isset($request['userid']) && $request['userid'] != ''))
    {
      $getUser = $this->m_centraldb->parse('select id, last_name, first_name, userid, email, pager, active, admin, locked from person where '.((isset($request['id']) && $request['id'] != '')?'id = '.$request['id']:'userid = \''.$request['userid'].'\''));
      if ($getUser->execute())
      {
        if (($getUserRow = $getUser->fetch('assoc')))
        {
          $bResult = true;
          $getUserRow['active'] = $this->noyes($getUserRow['active']);
          $getUserRow['admin'] = $this->noyes($getUserRow['admin']);
          $getUserRow['locked'] = $this->noyes($getUserRow['locked']);
          $response = $getUserRow;
        }
        else
        {
          $strError = 'Failed to query results.';
        }
      }
      else
      {
        $strError = $getUser->getError();
      }
      $this->m_centraldb->free($getUser);
    }
    else
    {
      $strError = 'Please provide the id or userid.';
    }

    return $bResult;
  }
  // }}}
  // {{{ userAdd()
  public function userAdd($request, &$response, &$strError)
  {
    $bApplication = false;
    $bCentral = false;
    $bResult = false;
    $bServer = false;
    $response = array();

    if ($this->isValid('Central'))
    {
      $bCentral = true;
    }
    else if (isset($request['application_id']) && $request['application_id'] != '')
    {
      $applicationrequest = array('id'=>$request['application_id']);
      $applicationresponse = null;
      if ($this->application($applicationrequest, $applicationresponse, $strError))
      {
        $devrequest = array('id'=>$request['application_id']);
        $devresponse = null;
        if ($this->isLocalAdmin($applicationresponse['name']) || $this->isApplicationDeveloper($devrequest, $devresponse, $strError))
        {
          $bApplication = true;
        }
        unset($devrequest);
        unset($devresponse);
      }
      unset($applicationrequest);
      unset($applicationresponse);
    }
    else if (isset($request['server_id']) && $request['server_id'] != '')
    {
      $serverrequest = array('id'=>$request['application_id']);
      $serverresponse = null;
      if ($this->application($serverrequest, $serverresponse, $strError))
      {
        $adminrequest = array('id'=>$request['server_id']);
        $adminresponse = null;
        if ($this->isLocalAdmin($applicationresponse['name']) || $this->isServerAdmin($adminrequest, $adminresponse, $strError))
        {
          $bServer = true;
        }
        unset($adminrequest);
        unset($adminresponse);
      }
      unset($serverrequest);
      unset($serverresponse);
    }
    if ($bCentral || $bApplication || $bServer)
    {
      if (isset($request['userid']) && $request['userid'] != '')
      {
        $userrequest = array('userid'=>$request['userid']);
        $userresponse = null;
        if (!$this->user($userrequest, $userresponse, $strError) && $strError == 'Failed to query results.')
        {
          $insertUser = $this->m_centraldb->parse('insert into person (userid, active, admin, locked) values (\''.$request['userid'].'\', 1, 0, 0)');
          if ($insertUser->execute())
          {
            $bResult = true;
            $this->syslog()->userAccountCreated('Created the user.', $request['userid']);
          }
          else
          {
            $strError = $insertUser->getError();
            $this->syslog()->userAccountCreated('Failed to create the user.', $request['userid'], false);
          }
        }
        unset($userrequest);
        unset($userresponse);
      }
      else
      {
        $strError = 'Please provide the userid.';
      }
    }
    else
    {
      $strError = 'You are not authorized to perform this action.';
    }

    return $bResult;
  }
  // }}}
  // {{{ userEdit()
  public function userEdit($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $userrequest = array('id'=>$request['id']);
      $userresponse = null;
      if ($this->isGlobalAdmin() || ($this->user($userrequest, $userresponse, $strError) && $this->getUserID() == $userresponse['userid']))
      {
        if (isset($request['userid']) && $request['userid'] != '')
        {
          $strQuery = 'update person set';
          $strQuery .= ' userid = \''.addslashes($request['userid']).'\'';
          $strQuery .= ', first_name = '.((isset($request['first_name']) && $request['first_name'] != '')?'\''.addslashes($request['first_name']).'\'':'null');
          $strQuery .= ', last_name = '.((isset($request['last_name']) && $request['last_name'] != '')?'\''.addslashes($request['last_name']).'\'':'null');
          $strQuery .= ', email = '.((isset($request['email']) && $request['email'] != '')?'\''.addslashes($request['email']).'\'':'null');
          $strQuery .= ', pager = '.((isset($request['pager']) && $request['pager'] != '')?'\''.addslashes($request['pager']).'\'':'null');
          $strQuery .= ', active = '.((isset($request['active']) && is_array($request['active']) && isset($request['active']['value']) && is_numeric($request['active']['value']))?$request['active']['value']:'null');
          $strQuery .= ', admin = '.((isset($request['admin']) && is_array($request['admin']) && isset($request['admin']['value']) && is_numeric($request['admin']['value']))?$request['admin']['value']:'null');
          $strQuery .= ', locked = '.((isset($request['locked']) && is_array($request['locked']) && isset($request['locked']['value']) && is_numeric($request['locked']['value']))?$request['locked']['value']:'null');
          if (isset($request['password']) && $request['password'] != '')
          {
            $strQuery .= ', `password` = concat(\'*\',upper(sha1(unhex(sha1(\''.$request['password'].'\')))))';
          }
          $strQuery .= ' where id = '.$request['id'];
          $updateUser = $this->m_centraldb->parse($strQuery);
          if ($updateUser->execute())
          {
            $bResult = true;
            $this->syslog()->userAccountModified('Modified the user.', $request['userid']);
          }
          else
          {
            $strError = $updateUser->getError();
            $this->syslog()->userAccountModified('Failed to modify the user.', $request['userid'], false);
          }
        }
        else
        {
          $strError = 'Please provide the userid.';
        }
      }
      else
      {
        $strError = 'You are not authorized to perform this action.';
      }
      unset($userrequest);
      unset($userresponse);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ userRemove()
  public function userRemove($request, &$response, &$strError)
  {
    $bResult = false;
    $response = array();

    if (isset($request['id']) && $request['id'] != '')
    {
      $userrequest = array('id'=>$request['id']);
      $userresponse = null;
      if ($this->isGlobalAdmin() || ($this->user($userrequest, $userresponse, $strError) && $this->getUserID() == $userresponse['userid']))
      {
        $deleteUser = $this->m_centraldb->parse('delete from person where id = '.$request['id']);
        if ($deleteUser->execute())
        {
          $bResult = true;
          $this->syslog()->userAccountDeleted('Deleted the user.');
        }
        else
        {
          $strError = $deleteUser->getError();
          $this->syslog()->userAccountDeleted('Failed to delete the user.', null, false);
        }
      }
      else
      {
        $strError = 'You are not authorized to perform this action.';
      }
      unset($userrequest);
      unset($userresponse);
    }
    else
    {
      $strError = 'Please provide the id.';
    }

    return $bResult;
  }
  // }}}
  // {{{ users()
  public function users($request, &$response, &$strError)
  {
    $bResult = false;
    $cLetter = ((isset($request['letter']) && $request['letter'] != '')?$request['letter']:null);
    $response = array();

    $strQuery = 'select id, last_name, first_name, userid, email, pager, active, admin, locked from person';
    if ($cLetter != '')
    {
      $strQuery .= ' where ';
      if ($request['letter'] == '#')
      {
        $strQuery .= ' last_name regexp \'^[ -@[-`{-~]\'';
      }
      else
      {
        $strQuery .= ' upper(last_name) like \''.$request['letter'].'%\'';
      }
    }
    $strQuery .= ' order by last_name, first_name, userid';
    if (isset($request['page']) && $request['page'] != '' && is_numeric($request['page']))
    {
      $nNumPerPage = ((isset($request['numPerPage']) && $request['numPerPage'] != '' && is_numeric($request['numPerPage']))?$request['numPerPage']:25);
      $nOffset = $request['page'] * $nNumPerPage;
      $strQuery .= ' limit '.$nNumPerPage.' offset '.$nOffset;
    }
    $getUser = $this->m_centraldb->parse($strQuery);
    if ($getUser->execute())
    {
      $bResult = true;
      while (($getUserRow = $getUser->fetch('assoc')))
      {
        $response[] = $getUserRow;
      }
    }
    else
    {
      $strError = $getUser->getError();
    }
    $this->m_centraldb->free($getUser);

    return $bResult;
  }
  // }}}
}
?>
