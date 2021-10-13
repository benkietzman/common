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
      else if (this.isDefined($cookies.get('sl_commonWsJwt')) && $cookies.get('sl_commonWsJwt'))
      {
        wsJwt = $cookies.get('sl_commonWsJwt');
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
            $rootScope.$root.$broadcast('commonAuthReady', null);
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
        $rootScope.$root.$broadcast('commonAuthReady', null);
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
          $rootScope.$root.$broadcast('commonAuthReady', null);
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
  // {{{ resetMenu()
  resetMenu(menu)
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
  // {{{ setMenu()
  setMenu(strMenu, strSubMenu)
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
            $cookies.put('sl_commonWsJwt', '');
          }
          delete response.Error;
          delete response.wsJwt;
          delete response.wsRequestID;
          delete response.Response;
          delete response.Status;
          $rootScope.$root.$broadcast('commonAuthReady', null);
          $rootScope.$root.$broadcast('resetMenu', null);
        }
        else if (!this.m_bJwt && strProtocol == 'bridge' && this.m_ws[strName].SetBridgeCredentials && this.m_ws[strName].SetBridgeCredentials.Script && this.m_ws[strName].SetBridgeCredentials.Script.length > 0 && this.m_ws[strName].SetBridgeCredentials['Function'] && this.m_ws[strName].SetBridgeCredentials['Function'].length > 0 && response.Status && response.Status == 'error' && response.Error && response.Error.Type && response.Error.Type == 'bridge' && response.Error.SubType && ((response.Error.SubType == 'request' && response.Error.Message && response.Error.Message == 'Your access has been denied.') || (response.Error.SubType == 'requestSocket' && response.Error.Message && response.Error.Message == 'Please provide the User.')))
        {
          if (this.isDefined($cookies.get('PHPSESSID')))
          {
            if (!this.m_bSetBridgeCredentials)
            {
              this.m_bSetBridgeCredentials = true;
              $http.post(this.m_ws[strName].SetBridgeCredentials['Script'], {'Function': this.m_ws[strName].SetBridgeCredentials['Function']}).then((response) => {this.m_bSetBridgeCredentials = false;}, (response) => {this.m_bSetBridgeCredentials = false});
            }
            delete response.Error;
            delete response.wsRequestID;
            delete response.Response;
            delete response.Status;
            response.wsSessionID = $cookies.get('PHPSESSID');
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
            $rootScope.$root.$broadcast(response.wsController, response);
          }
          else
          {
            $rootScope.$root.$broadcast('commonWsMessage_' + this.application, response);
            $rootScope.$root.$broadcast('commonWsMessage_' + this.application + '_' + strName, response);
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
          $rootScope.$root.$broadcast('bridgePurpose_status', response);
        }
      };
    }

    return bResult;
  }
  // }}}
  // {{{ wsRequest()
  wsRequest(strName, request)
  {
    let deferred = $q.defer();
    
    if (this.isDefined(this.m_ws[strName]))
    {
      let unHandle = this.wsUnique(strName);
      this.wsSend(strName, unHandle, request);
      let unbind = $rootScope.$on(unHandle, (event, response) =>
      { 
        deferred.resolve(response);
        unbind();
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
        else if (this.isDefined($cookies.get('sl_commonWsJwt')) && $cookies.get('sl_commonWsJwt'))
        {
          this.m_bSentJwt = true;
          request.wsJwt = $cookies.get('sl_commonWsJwt');
        }
      }
      if (this.isDefined(this.m_wsSessionID))
      {
        if (!this.isDefined($cookies.get('PHPSESSID')))
        {
          $cookies.put('PHPSESSID', this.m_wsSessionID);
        }
      }
      else if (this.isDefined($cookies.get('PHPSESSID')))
      {
        this.m_wsSessionID = $cookies.get('PHPSESSID');
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
