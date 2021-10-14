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
  constructor(options)
  {
    this.bridgeStatus = {stat: false};
    this.centralMenu = {};
    this.footer = {engineer: false};
    this.menu = {left: [], right: []};
    this.submenu = {left: [], right: []};
    if (this.isDefined(options.application))
    {
      this.application = options.application;
    }
    if (this.isDefined(options.script))
    {
      this.script = options.script;
    }
    var _this = this;
    this.request('applications', null, (data) =>
    {
      let error = {};
      if (_this.response(data, error))
      {
        var bFound = false;
        _this.centralMenu.applications = [];
        for (var i = 0; i < data.Response.out.length; i++)
        {
          if (((data.Response.out[i].menu_id == 1 && _this.isValid('')) || data.Response.out[i].menu_id == 2) && (data.Response.out[i].retirement_date == null || data.Response.out[i].retirement_date == '0000-00-00 00:00:00'))
          {
            _this.centralMenu.applications.push(data.Response.out[i]);
          }
        }
        for (var i = 0; !bFound && i < _this.centralMenu.applications.length; i++)
        {
          if (_this.centralMenu.applications[i].name == _this.application)
          {
            bFound = true;
            _this.centralMenu.application = _this.centralMenu.applications[i];
          }
        }
      }
    });
    if (this.isDefined(options.footer))
    {
      this.footer = {...this.footer, ...options.footer};
      var _this = this;
      this.request('footer', this.footer, function (response)
      {
        let error = {};
        if (_this.response(response, error))
        {
          _this.footer = response.Response.out;
        }
      });
    }
  }
  // }}}
  // {{{ attachEvent()
  attachEvent(strHandle, callback)
  {
    if (document.addEventListener)
    {
      document.addEventListener(strHandle, callback, false);
    }
    else
    {
      document.attachEvent(strHandle, callback);
    }
  }
  // }}}
  // {{{ auth()
  auth()
  {
    if (this.m_bJwt)
    {
      let wsJwt = null;
      if (this.m_wsJwt)
      {
        wsJwt = this.m_wsJwt;
      }
      else if (this.m_sessionStorage && this.m_sessionStorage.sl_wsJwt)
      {
        wsJwt = this.m_sessionStorage.sl_wsJwt;
      }
      else
      {
        wsJwt = this.getCookie('sl_commonWsJwt');
      }
      if (wsJwt)
      {
        let request = {Section: 'secure', 'Function': 'auth', wsJwt: wsJwt, Request: {}};
        this.wsRequest('bridge', request).then((response) =>
        {
          let error = {};
          if (this.wsResponse(response, error))
          {
            this.m_auth = response.Response;
            this.m_bHaveAuth = true;
            if (this.isValid())
            {
              this.m_menu.right[this.m_menu.right.length] = {value: 'Logout as ' + this.getUserFirstName(), href: '/Logout', icon: 'log-out', active: null};
              if (!this.m_wsRequestID && !this.m_bConnecting)
              {
                this.m_bConnecting = true;
                let request = {Section: 'bridge', 'Function': 'connect'};
                request.Request = {};
                this.wsRequest('bridge', request).then((response) =>
                {
                  let error = {};
                  if (this.wsResponse(response, error))
                  {
                    this.m_wsRequestID = response.wsRequestID;
                    this.m_bConnecting = false;
                  }
                });
              }
            }
            else
            {
              this.m_menu.right[this.m_menu.right.length] = {value: 'Login', href: '/Login', icon: 'user', active: null};
              if (this.m_wsRequestID)
              {
                this.m_bConnecting = true;
                let request = {Section: 'bridge', 'Function': 'disconnect', wsRequestID: this.m_wsRequestID};
                request.Request = {};
                this.wsRequest('bridge', request).then((response) =>
                {
                  this.m_wsRequestID = null;
                  this.m_bConnecting = false;
                });
              }
            }
          }
        });
      }
      else
      {
        if (this.isValid())
        {
          this.m_menu.right[this.m_menu.right.length] = {value: 'Logout as ' + this.getUserFirstName(), href: '/Logout', icon: 'log-out', active: null};
        }
        else
        {
          this.m_menu.right[this.m_menu.right.length] = {value: 'Login', href: '/Login', icon: 'user', active: null};
        }
      }
    }
    else
    {
      var _this = this;
      this.request('auth', null, (response) =>
      {
        let error = {};
        if (_this.response(response, error))
        {
          _this.m_auth = response.Response.out;
          _this.m_bHaveAuth = true;
          if (_this.isValid())
          {
            _this.m_menu.right[_this.m_menu.right.length] = {value: 'Logout as ' + _this.getUserFirstName(), href: '/Logout', icon: 'log-out', active: null};
          }
          else
          {
            _this.m_menu.right[_this.m_menu.right.length] = {value: 'Login', href: '/Login', icon: 'user', active: null};
          }
        }
      });
    }
  }
  // }}}
  // {{{ dispatchEvent()
  dispatchEvent(strHandle, data)
  {
    let eventHandle = new CustomEvent(strHandle, data);
    document.dispatchEvent(eventHandle);
  }
  // }}}
  // {{{ getCookie()
  getCookie(strName)
  {
    let bFound = false;
    let strValue = '';
    let strSubName = strtName+'=';
    let strCookies = decodeURIComponent(document.cookie);
    let cookies = strCookies.split(';');

    for (let i = 0; !bFound && i < cookies.length; i++)
    {
      let strCookie = cookies[i];
      while (strCookie.charAt(0) == ' ')
      {
        strCookie = strCookie.substring(1);
      }
      if (strCookie.indexOf(strSubName) == 0)
      {
        bFound = true;
        strValue = strCookie.substring(strSubName.length, strCookie.length);
      }
    }

    return strValue;
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
    return this.m_auth.userid;
  };
  // }}}
  // {{{ getUserLastName()
  getUserLastName()
  {
    return this.m_auth.last_name;
  };
  // }}}
  // {{{ isCookie()
  isCookie(strName)
  {
    let bFound = false;
    let strSubName = strtName+'=';
    let strCookies = decodeURIComponent(document.cookie);
    let cookies = strCookies.split(';');

    for (let i = 0; !bFound && i < cookies.length; i++)
    {
      let strCookie = cookies[i];
      while (strCookie.charAt(0) == ' ')
      {
        strCookie = strCookie.substring(1);
      }
      if (strCookie.indexOf(strSubName) == 0)
      {
        bFound = true;
      }
    }

    return bFound;
  }
  // }}}
  // {{{ isDefined()
  isDefined(variable)
  {
    let bResult = false;

    if (typeof variable !== 'undefined')
    {
      bResult = true;
    }

    return bResult;
  }
  // }}}
  // {{{ isGlobalAdmin()
  isGlobalAdmin()
  {
    let bResult = false;

    if (this.isDefined(this.m_auth) && this.m_auth.admin)
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

    if (this.isDefined(this.m_auth) && ((this.isDefined(this.m_auth.admin) && this.m_auth.admin) || (strApplication != null && this.isDefined(this.m_auth.apps) && this.isDefined(this.m_auth.apps[strApplication]) && this.m_auth.apps[strApplication])))
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

    if (this.isDefined(this.m_auth) && ((this.isDefined(this.m_auth.admin) && this.m_auth.admin) || (strApplication != null && this.isDefined(this.m_auth.apps) && this.isDefined(this.m_auth.apps[strApplication])) || (strApplication == null && this.isDefined(this.m_auth.userid) && this.m_auth.userid != null && this.m_auth.userid.length > 0)))
    {
      bResult = true;
    }

    return bResult;
  }
  // }}}
  // {{{ loginProcess()
  loginProcess(login)
  {
    if (!this.isDefined(login) || login == null)
    {
      login = {};
    }
    if (this.m_bJwt)
    {
      var _this = this;
      fetch('/include/common_addons/auth/modules.json',
      {
        method: 'GET',
        headers:
        {
          'Content-Type': 'application/json'
        }
      })
      .then(response => response.json())
      .then((response) =>
      {
        let request = null;
        request = {Section: 'secure', 'Function': 'process', Request: login};
        request.Request.Type = this.m_strLoginType;
        if (_this.isDefined(response.data[_this.m_strLoginType]) && _this.isDefined(response.data[_this.m_strLoginType]['cookie']) && _this.isCookie(response.data[_this.m_strLoginType]['cookie']))
        {
          request.Request.Data = _this.getCookie(response.data[_this.m_strLoginType]['cookie']);
        }
        this.wsRequest('bridge', request).then(function (response)
        {
          var error = {};
          if (this.wsResponse(response, error))
          {
            if (this.isDefined(response.Error) && this.isDefined(response.Error.Message) && response.Error.Message.length > 0)
            {
              this.login.message = response.Error.Message;
            }
            if (this.isDefined(response.Response.auth))
            {
              this.m_auth = response.Response.auth;
              this.m_bHaveAuth = true;
              if (this.isDefined(response.Response.jwt))
              {
                if (this.m_sessionStorage)
                {
                  this.m_sessionStorage.sl_wsJwt = response.Response.jwt;
                }
                else if (response.Response.jwt.length > 4096)
                {
                  this.m_wsJwt = response.Response.jwt;
                }
                else
                {
                  this.setCookie('sl_commonWsJwt', response.Response.jwt);
                }
              }
              if (this.isDefined(this.m_auth.login_title))
              {
                if (!this.isDefined(this.login.login) || this.login.login == null)
                {
                  this.login.login = {};
                }
                this.login.login.title = this.m_auth.login_title;
                if (this.login.login.title.length <= 30 || this.login.login.title.substr(this.login.login.title.length - 30, 30) != ' (please wait for redirect...)')
                {
                  this.login.showForm = true;
                }
              }
              if (this.isValid(null))
              {
                document.location.href = this.m_strRedirectPath;
              }
              else
              {
                var request = {Section: 'secure', 'Function': 'login'};
                request.Request = {Type: this.m_strLoginType, Return: document.location.href};
                this.wsRequest('bridge', request).then(function (response)
                {
                  var error = {};
                  this.login.info = null;
                  if (this.wsResponse(response, error))
                  {
                    if (this.isDefined(response.Response.Redirect) && response.Response.Redirect.length > 0)
                    {
                      document.location.href = response.Response.Redirect;
                    }
                  }
                  else
                  {
                    this.login.message = error.message;
                  }
                });
              }
            }
            if (this.isDefined(error.message))
            {
              this.login.message = error.message;
            }
          }
          else
          {
            this.login.message = error.message;
          }
        });
      });
    }
    else
    {
      login.Application = this.application;
      if (this.m_strLoginType != '')
      {
        login.LoginType = this.m_strLoginType;
      }
      this.request('authProcessLogin', login, function (result)
      {
        var error = {};
        if (this.response(result, error))
        {
          this.m_auth = result.data.Response.out;
          this.m_bHaveAuth = true;
          if (this.isDefined(this.m_auth.login_title))
          {
            if (!this.isDefined(this.login.login) || this.login.login == null)
            {
              this.login.login = {};
            }
            this.login.login.title = this.m_auth.login_title;
            if (this.login.login.title.length <= 30 || this.login.login.title.substr(this.login.login.title.length - 30, 30) != ' (please wait for redirect...)')
            {
              this.login.showForm = true;
            }
          }
          if (this.isValid(null))
          {
            document.location.href = this.m_strRedirectPath;
          }
          else
          {
            var login = {};
            login.Application = this.application;
            if (this.m_strLoginType != '')
            {
              login.LoginType = this.m_strLoginType;
            }
            login.Redirect = this.getRedirectPath();
            this.request('authLogin', login, function (result)
            {
              var error = {};
              this.login.info = null;
              if (this.response(result, error))
              {
                if (this.isDefined(result.data.Response.out.Status) && result.data.Response.out.Status == 'okay')
                {
                  if (this.isDefined(result.data.Response.out.Redirect) && result.data.Response.out.Redirect.length > 0)
                  {
                    document.location.href = result.data.Response.out.Redirect;
                  }
                }
                else if (this.isDefined(result.data.Response.out.Error) && result.data.Response.out.Error.length > 0)
                {
                  this.login.message = result.data.Response.out.Error;
                }
                else
                {
                  this.login.message = 'Encountered an unknown error.';
                }
              }
              else
              {
                this.login.message = error.message;
              }
            });
          }
        }
        else
        {
          this.login.message = error.message;
        }
      });
    }
  }
  // }}}
  // {{{ menuReset()
  menuReset(menu)
  {
    this.menu = menu;
    if (this.isValid(''))
    { 
      this.menu.right[this.menu.right.length] = {value: 'Logout as ' + this.getUserFirstName(), href: '/Logout', icon: 'log-out', active: null};
    }
    else
    { 
      this.menu.right[this.menu.right.length] = {value: 'Login', href: '/Login', icon: 'user', active: null};
    }
  }
  // }}}
  // {{{ menuSet()
  menuSet(strMenu, strSubMenu)
  {
    let nActiveLeft = -1;
    this.strPrevMenu = this.strMenu;
    this.strPrevSubMenu = this.strSubMenu;
    this.strMenu = strMenu;
    this.strSubMenu = strSubMenu;
    for (let i = 0; i < this.menu.left.length; i++)
    {
      if (this.menu.left[i].value == strMenu)
      {
        nActiveLeft = i;
        this.menu.left[i].active = 'active';
        this.activeMenu = strMenu;
      }
      else
      {
        this.menu.left[i].active = null;
      }
    }
    let nActiveRight = -1;
    for (let i = 0; i < this.menu.right.length; i++)
    {
      if (this.menu.right[i].value == strMenu)
      {
        nActiveRight = i;
        this.menu.right[i].active = 'active';
        this.activeMenu = strMenu;
      }
      else
      {
        this.menu.right[i].active = null;
      }
    }
    if (nActiveLeft >= 0 && this.menu.left[nActiveLeft].submenu)
    {
      this.submenu = this.menu.left[nActiveLeft].submenu;
      for (var i = 0; i < this.submenu.left.length; i++)
      {
        if (this.submenu.left[i].value == strSubMenu)
        {
          this.submenu.left[i].active = 'active';
        }
        else
        {
          this.submenu.left[i].active = null;
        }
      }
      for (let i = 0; i < this.submenu.right.length; i++)
      {
        if (this.submenu.right[i].value == strSubMenu)
        {
          this.submenu.right[i].active = 'active';
        }
        else
        {
          this.submenu.right[i].active = null;
        }
      }
    }
    else if (nActiveRight >= 0 && this.menu.right[nActiveRight].submenu)
    {
      this.submenu = this.menu.right[nActiveRight].submenu;
      for (let i = 0; i < this.submenu.left.length; i++)
      {
        if (this.submenu.left[i].value == strSubMenu)
        {
          this.submenu.left[i].active = 'active';
        }
        else
        {
          this.submenu.left[i].active = null;
        }
      }
      for (let i = 0; i < this.submenu.right.length; i++)
      {
        if (this.submenu.right[i].value == strSubMenu)
        {
          this.submenu.right[i].active = 'active';
        }
        else
        {
          this.submenu.right[i].active = null;
        }
      }
    }
    else
    {
      this.submenu = false;
    }
  }
  // }}}
  // {{{ request()
  request(strFunction, request, callback)
  {
    if (this.isDefined(this.script))
    {
      let data = {'Function': strFunction};
      if (request != null)
      {
        data.Arguments = request;
      }
      fetch(this.script,
      {
        method: 'POST',
        headers:
        {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(data)
      })
      .then(response => response.json())
      .then(callback);
    }
  }  
  // }}}
  // {{{ response()
  response(response, error)
  {   
    let bResult = false;
      
    if (this.isDefined(response.Status) && response.Status == 'okay')
    {
      bResult = true;
    }
    else if (this.isDefined(response.Error))
    {
      error.message = response.Error;
    }
    else
    {
      error.message = 'Encountered an unknown error.';
    }

    return bResult;
  }
  // }}}
  // {{{ setCookie()
  setCookie(strName, strValue)
  {
    const d = new Date();
    d.setTime(d.getTime()+(365*24*60*60*1000));
    let strExpires = 'expires'+d.toUTCString();
    document.cookie = strName+'='+strValue+';'+strExpires+';path=/';
  }
  // }}}
  // {{{ wsCreate()
  wsCreate(strName, strServer, strPort, bSecure, strProtocol)
  {
    let bResult = false;

    if (this.application.length > 0 && strName.length > 0 && strPort.length > 0 && strProtocol.length > 0 && (bSecure == true || bSecure == false) && strServer.length > 0)
    {
      let date = new Date();
      if (!this.isDefined(this.m_ws[strName]))
      { 
        this.m_ws[strName] = {};
      }
      this.m_ws[strName].Application = this.application;
      this.m_ws[strName].Attempts = 1;
      this.m_ws[strName].Connected = false;
      this.m_ws[strName].Port = strPort;
      this.m_ws[strName].Protocol = strProtocol;
      this.m_ws[strName].Secure = bSecure;
      this.m_ws[strName].Server = strServer;
      this.m_ws[strName].Unique = 0;
      let socket = new WebSocket(((bSecure)?'wss':'ws')+'://'+strServer+':'+strPort+'/'+strProtocol);
      socket.onopen = (e) =>
      {
        this.m_ws[strName].Attempts = 1;
        if (this.m_ws[strName].modalInstance != null)
        {
          this.m_ws[strName].modalInstance.close();
          this.m_ws[strName].modalInstance = null;
        }
        this.m_ws[strName].Connected = true;
        this.auth(null);
      };
      socket.onclose = (e) =>
      {
        let maxInterval = (Math.pow(2, this.m_wsAttempts) - 1) * 1000;
        console.log(ev);
        this.m_bConnecting = false;
        this.m_bSentJwt = false;
        this.m_wsRequestID = null;
        if (this.m_ws[strName].Connected)
        {
          this.m_ws[strName].Connected = false;
        }
        if (maxInterval > 30 * 1000)
        {
          maxInterval = 30 * 1000;
        }
        setTimeout(() =>
        {
          this.m_ws[strName].Attempts++;
          this.wsCreate(this.application, strName, strServer, strPort, bSecure, strProtocol);
        }, (Math.random() * maxInterval));
      };
      socket.onerror = (e) =>
      {
        console.log(ev);
      };
      socket.onmessage = (e) =>
      {
        let response = JSON.parse(event.data);
        if (this.m_bJwt && strProtocol == 'bridge' && response.Status == 'error' && response.Error && response.Error.Type && response.Error.Type == 'bridge' && response.Error.SubType && response.Error.SubType == 'request' && response.Error.Message && response.Error.Message == 'Failed: exp')
        {
          response.Error.Message = 'Session expired.  Please login again.';
          this.m_auth = null;
          this.m_auth = {admin: false, apps: {}};
          this.m_bSentJwt = false;
          if (this.m_sessionStorage && this.m_sessionStorage.sl_wsJwt)
          {
            this.m_sessionStorage.sl_wsJwt = null;
          }
          else if (this.m_wsJwt)
          {
            this.m_wsJwt = null;
          }
          else
          {
            this.setCookie('sl_commonWsJwt', '');
          }
          delete response.Error;
          delete response.wsJwt;
          delete response.wsRequestID;
          delete response.Response;
          delete response.Status;
        }
        else if (!this.m_bJwt && strProtocol == 'bridge' && this.m_ws[strName].SetBridgeCredentials && this.m_ws[strName].SetBridgeCredentials.Script && this.m_ws[strName].SetBridgeCredentials.Script.length > 0 && this.m_ws[strName].SetBridgeCredentials['Function'] && this.m_ws[strName].SetBridgeCredentials['Function'].length > 0 && response.Status && response.Status == 'error' && response.Error && response.Error.Type && response.Error.Type == 'bridge' && response.Error.SubType && ((response.Error.SubType == 'request' && response.Error.Message && response.Error.Message == 'Your access has been denied.') || (response.Error.SubType == 'requestSocket' && response.Error.Message && response.Error.Message == 'Please provide the User.')))
        {
          if (this.isCookie('PHPSESSID'))
          {
            if (!this.m_bSetBridgeCredentials)
            {
              this.m_bSetBridgeCredentials = true;
              fetch(this.m_ws[strName].SetBridgeCredentials['Script'],
              {
                method: 'POST',
                headers:
                {
                  'Content-Type': 'application/json'
                },
                body: JSON.stringify({'Function': this.m_ws[strName].SetBridgeCredentials['Function']})
              })
              .then(response => response.json())
              .then((response) =>
              {
                this.m_bSetBridgeCredentials = false;
              });
            }
            delete response.Error;
            delete response.wsRequestID;
            delete response.Response;
            delete response.Status;
            response.wsSessionID = this.getCookie('PHPSESSID');
            this.m_ws[strName].ws.send(JSON.stringify(response));
          }
          else if (this.isValid())
          {
            alert('Your session has expired.  Please login again.');
            document.location.href = '#/Logout';
          }
        }
        else
        {
          if (this.isDefined(response.wsController))
          {
          }
          else
          {
          }
          if (response.wsRequestID && response.wsRequestID == this.m_wsRequestID && response.Action)
          {
            if (response.Action == 'alert')
            {
              if (response.Message)
              {
                alert(response.Message);
              }
            }
            else if (response.Action == 'audio')
            {
              if ('Audio' in window)
              {
                if (response.Media)
                {
                  let audio = new Audio(response.Media);
                  audio.play();
                }
              }
            }
            else if (response.Action == 'location')
            {
              if (response.Location)
              {
                document.location.href = response.Location;
              }
            }
            else if (response.Action == 'message')
            {
              this.m_messages.push(response);
              this.m_messages[this.m_messages.length - 1].Index = (this.m_messages.length - 1);
            }
            else if (response.Action == 'notification')
            {
              if ('Notification' in window)
              {
                if (response.Title && response.Body)
                {
                  Notification.requestPermission((permission) =>
                  {
                    let nDuration = 5000;
                    if (response.Duration)
                    {
                      nDuration = response.Duration * 1000;
                    }
                    let notification = new Notification(response.Title, {body: response.Body, icon: null, dir: 'auto'});
                    setTimeout(() =>
                    {
                      notification.close();
                    }, nDuration);
                  });
                }
              }
            }
          }
        }
        if (response.bridgePurpose && response.bridgePurpose == 'status')
        {
        }
      };
    }

    return bResult;
  }
  // }}}
  // {{{ wsRequest()
  wsRequest(strName, request)
  {
    let deferred = {};
    let promise = new Promise((resolve, reject) =>
    {
      deferred.resolve = resolve;
      deferred.reject = reject;
    });
    deferred.promise = promise;
    if (this.isDefined(this.m_ws[strName]))
    {
      let strHandle = strName+'_ws_'+this.wsUnique(strName);
      this.wsSend(strName, strHandle, request);
      this.attachEvent(strHandle, (data) =>
      { 
        deferred.resolve(data);
      });
    }   
          
    return deferred.promise;
  }
  // }}}
  // {{{ wsResponse()
  wsResponse(response, error)
  {
    let bResult = false;
          
    if (response.Status && response.Status == 'okay')
    { 
      bResult = true;
    }
    else if (error != null) 
    {   
      if (response.Error)
      { 
        if (response.Error != 'Please provide the wsJwt.')
        {
          error.message = response.Error;
        }
      }
      else
      {
        error.message = 'Encountered an unknown error.';
      }
    }

    return bResult;
  }
  // }}}
  // {{{ wsSend()
  wsSend(strName, strController, request)
  {
    if (this.isDefined(this.m_ws[strName]))
    {
      request.reqApp = this.application;
      request.wsController = strController;
      if (this.m_bJwt && (this.m_bJwtInclusion || !this.m_bSentJwt))
      {
        if (this.m_wsJwt)
        {
          this.m_bSentJwt = true;
          request.wsJwt = this.m_wsJwt;
        }
        else if (this.m_sessionStorage && this.m_sessionStorage.sl_wsJwt)
        {
          this.m_bSentJwt = true;
          request.wsJwt = this.m_sessionStorage.sl_wsJwt;
        }
        else if (this.getCookie('sl_commonWsJwt'))
        {
          this.m_bSentJwt = true;
          request.wsJwt = this.getCookie('sl_commonWsJwt');
        }
      }
      if (this.isDefined(this.m_wsSessionID))
      {
        if (!this.isCookie('PHPSESSID'))
        {
          this.setCookie('PHPSESSID', this.m_wsSessionID);
        }
      }
      else if (this.isCookie('PHPSESSID'))
      {
        this.m_wsSessionID = this.getCookie('PHPSESSID');
      }
      request.wsSessionID = this.m_wsSessionID;
      this.m_ws[strName].ws.send(JSON.stringify(request));
    }
  }
  // }}}
  // {{{ wsSetBridgeCredentials()
  wsSetBridgeCredentials(strName, strScript, strFunction)
  {
    if (!this.isDefined(this.m_ws[strName]))
    {
      this.m_ws[strName] = {};
    }
    this.m_ws[strName].SetBridgeCredentials = {Script: strScript, 'Function': strFunction};
  }
  // }}}
  // {{{ wsUnique()
  wsUnique(strName)
  {
    let nUnique = 0;

    if (this.isDefined(this.m_ws[strName]))
    {
      nUnique = this.m_ws[strName].Unique++;
    }

    return nUnique;
  }
  // }}}
}
