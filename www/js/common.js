// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2021-10-12
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////

class Common
{
  // {{{ constructor()
  constructor(strApplication)
  {
    this.m_strApplication = strApplication;
  }
  // }}}
  // {{{ getUserEmail()
  getUserEmail()
  {
    return this.m_auth.email;
  };
  // }}}
  // {{{ getUserFirstName()
  getUserFirstName()
  {
    return this.m_auth.first_name;
  };
  // }}}
  // {{{ getUserID()
  getUserID()
  {
    return this.m_auth.userid
  };
  // }}}
  // {{{ getUserLastName()
  getUserLastName()
  {
    return this.m_auth.last_name;
  };
  // }}}
  // {{{ isGlobalAdmin()
  isGlobalAdmin()
  {
    let bResult = false;

    if (this.m_auth && this.m_auth.admin)
    {
      bResult = true;
    }

    return bResult;
  }
  // }}}
  // {{{ isLocalAdmin()
  isLocalAdmin(strApplication)
  {
    let bResult = false;

    if (this.m_auth && (this.m_auth.admin || (strApplication != null && this.m_auth.apps && this.m_auth.apps[strApplication])))
    {
      bResult = true;
    }

    return bResult;
  }
  // }}}
  // {{{ isValid()
  isValid(strApplication)
  {
    let bResult = false;

    if (this.m_auth && (this.m_auth.admin || (strApplication != null && this.m_auth.apps && this.m_auth.apps[strApplication]) || (strApplication == null && this.m_auth.userid && this.m_auth.userid != null && this.m_auth.userid.length > 0)))
    {
      bResult = true;
    }

    return bResult;
  }
  // }}}
  // {{{ wsCreate()
  wsCreate(strName, strServer, strPort, bSecure, strProtocol)
  {
    let bResult = false;

    if (strApplication.length > 0 && strName.length > 0 && strPort.length > 0 && strProtocol.length > 0 && (bSecure == true || bSecure == false) && strServer.length > 0)
    {
    }

    return bResult;
  }
  // }}}
}
