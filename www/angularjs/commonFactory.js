// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2015-06-30
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////

var factories = {};
factories.common = function ($cookies, $http, $location, $q, $rootScope, $uibModal, $websocket, $window)
{
  var factory = {};
  // {{{ variables
  factory.m_activeMenu;
  factory.m_auth = {admin: false, apps: {}};
  factory.m_bConnecting = false;
  factory.m_bHaveAuth = false;
  factory.m_bJwt = false;
  factory.m_bJwtInclusion = false;
  factory.m_bSecureLogin = false;
  factory.m_bSentJwt = false;
  factory.m_bSetBridgeCredentials = false;
  factory.m_menu = {left: [], right: []};
  factory.m_messages = [];
  factory.m_sessionStorage = null;
  factory.m_store = {};
  factory.m_strApplication = null;
  factory.m_strLoginType = 'password';
  factory.m_strMenu = null;
  factory.m_strRedirectPath = null;
  factory.m_strRequestPath = null;
  factory.m_strPrevMenu = 'Home';
  factory.m_strPrevSubMenu = null;
  factory.m_strSubMenu = null;
  factory.m_submenu = null;
  factory.m_ws = {};
  factory.m_wsJwt = null;
  factory.m_wsRequestID = null;
  // }}}
  // {{{ auth()
  factory.auth = function ($scope)
  {
    if (this.m_bJwt)
    {
      var wsJwt = null;
      if (this.m_wsJwt)
      {
        wsJwt = this.m_wsJwt;
      }
      else if (this.m_sessionStorage && this.m_sessionStorage.sl_wsJwt)
      {
        wsJwt = this.m_sessionStorage.sl_wsJwt;
      }
      else if (angular.isDefined($cookies.get('sl_commonWsJwt')) && $cookies.get('sl_commonWsJwt'))
      {
        wsJwt = $cookies.get('sl_commonWsJwt');
      }
      if (wsJwt)
      {
        var request = {Section: 'secure', 'Function': 'auth', wsJwt: wsJwt, Request: {}};
        this.wsRequest('bridge', request).then(function (response)
        {
          var error = {};
          if (factory.wsResponse(response, error))
          {
            factory.m_auth = response.Response;
            factory.m_bHaveAuth = true;
            $rootScope.$root.$broadcast('commonAuthReady', null);
            if (factory.isValid())
            {
              if ($scope)
              {
                factory.m_menu.right[factory.m_menu.right.length] = {value: 'Logout as ' + factory.getUserFirstName(), href: '/Logout', icon: 'log-out', active: null};
              }
              if (!factory.m_wsRequestID && !factory.m_bConnecting)
              {
                factory.m_bConnecting = true;
                var request = {Section: 'bridge', 'Function': 'connect'};
                request.Request = {};
                factory.wsRequest('bridge', request).then(function (response)
                {
                  var error = {};
                  if (factory.wsResponse(response, error))
                  {
                    factory.m_wsRequestID = response.wsRequestID;
                    factory.m_bConnecting = false;
                  }
                });
              }
            }
            else
            {
              if ($scope)
              {
                factory.m_menu.right[factory.m_menu.right.length] = {value: 'Login', href: '/Login', icon: 'user', active: null};
              }
              if (factory.m_wsRequestID)
              {
                factory.m_bConnecting = true;
                var request = {Section: 'bridge', 'Function': 'disconnect', wsRequestID: factory.m_wsRequestID};
                request.Request = {};
                factory.wsRequest('bridge', request).then(function (response)
                {
                  factory.m_wsRequestID = null;
                  factory.m_bConnecting = false;
                });
              }
            }
          }
          else if ($scope)
          {
            $scope.message = error.message;
          }
        });
      }
      else
      {
        $rootScope.$root.$broadcast('commonAuthReady', null);
        if ($scope)
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
    }
    else
    {
      var data = this;
      this.request('auth', null, (function (data, $scope)
      {
        return function (result)
        {
          var error = {};
          if (data.response(result, error))
          {
            data.m_auth = result.data.Response.out;
            data.m_bHaveAuth = true;
            $rootScope.$root.$broadcast('commonAuthReady', null);
            if ($scope)
            {
              if (data.isValid())
              {
                data.m_menu.right[data.m_menu.right.length] = {value: 'Logout as ' + data.getUserFirstName(), href: '/Logout', icon: 'log-out', active: null};
              }
              else
              {
                data.m_menu.right[data.m_menu.right.length] = {value: 'Login', href: '/Login', icon: 'user', active: null};
              }
            }
          }
          else if ($scope)
          {
            $scope.message = error.message;
            if (angular.isDefined($scope.store))
            {
              $scope.store.message = error.message;
            }
          }
        };
      })(data, $scope));
    }
  };
  // }}}
  // {{{ clear()
  factory.clear = function (controller)
  {
    if (angular.isDefined(this.m_store[controller]))
    {
      this.m_store[controller] = null;
      this.m_store[controller] = {'in': {}, links: null, links_message: null, message: null, out: null, processing: null};
    }

    return this.m_store[controller];
  };
  // }}}
  // {{{ clearMenu()
  factory.clearMenu = function (controller)
  {
    this.m_menu = null;
    this.m_menu = {left: [], right: []};
  };
  // }}}
  // {{{ enableJwt()
  factory.enableJwt = function (bEnable)
  {
    this.m_bJwt = bEnable;
  };
  // }}}
  // {{{ enableJwtInclusion()
  factory.enableJwtInclusion = function (bEnable)
  {
    this.m_bJwtInclusion = bEnable;
  };
  // }}}
  // {{{ getApplication()
  factory.getApplication = function ()
  {
    return this.m_strApplication;
  };
  // }}}
  // {{{ getRedirectPath()
  factory.getRedirectPath = function ()
  {
    return this.m_strRedirectPath;
  }
  // }}}
  // {{{ getSecureLogin()
  factory.getSecureLogin = function ()
  {
    return this.m_bSecureLogin;
  };
  // }}}
  // {{{ getUserEmail()
  factory.getUserEmail = function ()
  {
    return this.m_auth.email;
  };
  // }}}
  // {{{ getUserFirstName()
  factory.getUserFirstName = function ()
  {
    return this.m_auth.first_name;
  };
  // }}}
  // {{{ getUserID()
  factory.getUserID = function ()
  {
    return this.m_auth.userid
  };
  // }}}
  // {{{ getUserLastName()
  factory.getUserLastName = function ()
  {
    return this.m_auth.last_name;
  };
  // }}}
  // {{{ isGlobalAdmin()
  factory.isGlobalAdmin = function ()
  {
    var bResult = false;

    if (angular.isDefined(this.m_auth.admin) && this.m_auth.admin)
    {
      bResult = true;
    }

    return bResult;
  };
  // }}}
  // {{{ isLocalAdmin()
  factory.isLocalAdmin = function (strApplication)
  {
    var bResult = false;

    if (angular.isDefined(this.m_auth) && ((angular.isDefined(this.m_auth.admin) && this.m_auth.admin) || (strApplication != null && angular.isDefined(this.m_auth.apps) && angular.isDefined(this.m_auth.apps[strApplication]) && this.m_auth.apps[strApplication])))
    {
      bResult = true;
    }

    return bResult;
  };
  // }}}
  // {{{ isValid()
  factory.isValid = function (strApplication)
  {
    var bResult = false;

    if (angular.isDefined(this.m_auth) && ((angular.isDefined(this.m_auth.admin) && this.m_auth.admin) || (strApplication != null && angular.isDefined(this.m_auth.apps) && angular.isDefined(this.m_auth.apps[strApplication])) || (strApplication == null && angular.isDefined(this.m_auth.userid) && this.m_auth.userid != null && this.m_auth.userid.length > 0)))
    {
      bResult = true;
    }

    return bResult;
  };
  // }}}
  // {{{ request()
  factory.request = function (strFunction, request, callback)
  {
    var data = {'Function': strFunction};
    if (request != null)
    {
      data.Arguments = request;
    }
    $http.post(this.m_strRequestPath, data).then(callback);
  };
  // }}}
  // {{{ response()
  factory.response = function (response, error)
  {
    var bResult = false;

    if (response.status == 200)
    {
      if (angular.isDefined(response.data.Status) && response.data.Status == 'okay')
      {
        bResult = true;
      }
      else if (angular.isDefined(response.data.Error))
      {
        error.message = response.data.Error;
      }
      else
      {
        error.message = 'Encountered an unknown error.';
      }
    }
    else
    {
      error.message = response.status;
    }

    return bResult;
  };
  // }}}
  // {{{ retrieve()
  factory.retrieve = function (controller)
  {
    if (!angular.isDefined(this.m_store[controller]))
    {
      this.m_store[controller] = null;
      this.m_store[controller] = {'in': {}, links: null, links_message: null, message: null, out: null, processing: null};
    }

    return this.m_store[controller];
  };
  // }}}
  // {{{ setApplication()
  factory.setApplication = function (strApplication)
  {
    this.m_strApplication = strApplication;
    $http.post('/include/common/statistic.php', {'Application': strApplication}).then(function (response, error) {});
  };
  // }}}
  // {{{ setMenu()
  factory.setMenu = function (strMenu, strSubMenu)
  {
    var nActiveLeft = -1;
    this.m_strPrevMenu = this.m_strMenu;
    this.m_strPrevSubMenu = this.m_strSubMenu;
    this.m_strMenu = strMenu;
    this.m_strSubMenu = strSubMenu;
    for (var i = 0; i < this.m_menu.left.length; i++)
    {
      if (this.m_menu.left[i].value == strMenu)
      {
        nActiveLeft = i;
        this.m_menu.left[i].active = 'active';
        this.m_activeMenu = strMenu;
      }
      else
      {
        this.m_menu.left[i].active = null;
      }
    }
    var nActiveRight = -1;
    for (var i = 0; i < this.m_menu.right.length; i++)
    {
      if (this.m_menu.right[i].value == strMenu)
      {
        nActiveRight = i;
        this.m_menu.right[i].active = 'active';
        this.m_activeMenu = strMenu;
      }
      else
      {
        this.m_menu.right[i].active = null;
      }
    }
    if (nActiveLeft >= 0 && this.m_menu.left[nActiveLeft].submenu)
    {
      this.m_submenu = this.m_menu.left[nActiveLeft].submenu;
      for (var i = 0; i < this.m_submenu.left.length; i++)
      {
        if (this.m_submenu.left[i].value == strSubMenu)
        {
          this.m_submenu.left[i].active = 'active';
        }
        else
        {
          this.m_submenu.left[i].active = null;
        }
      }
      for (var i = 0; i < this.m_submenu.right.length; i++)
      {
        if (this.m_submenu.right[i].value == strSubMenu)
        {
          this.m_submenu.right[i].active = 'active';
        }
        else
        {
          this.m_submenu.right[i].active = null;
        }
      }
    }
    else if (nActiveRight >= 0 && this.m_menu.right[nActiveRight].submenu)
    {
      this.m_submenu = this.m_menu.right[nActiveRight].submenu;
      for (var i = 0; i < this.m_submenu.left.length; i++)
      {
        if (this.m_submenu.left[i].value == strSubMenu)
        {
          this.m_submenu.left[i].active = 'active';
        }
        else
        {
          this.m_submenu.left[i].active = null;
        }
      }
      for (var i = 0; i < this.m_submenu.right.length; i++)
      {
        if (this.m_submenu.right[i].value == strSubMenu)
        {
          this.m_submenu.right[i].active = 'active';
        }
        else
        {
          this.m_submenu.right[i].active = null;
        }
      }
    }
    else
    {
      this.m_submenu = null;
    }
  }
  // }}}
  // {{{ setRedirectPath()
  factory.setRedirectPath = function (strPath)
  {
    this.m_strRedirectPath = strPath;
  }
  // }}}
  // {{{ setRequestPath()
  factory.setRequestPath = function (strPath)
  {
    this.m_strRequestPath = strPath;
  }
  // }}}
  // {{{ setSecureLogin()
  factory.setSecureLogin = function (bSecureLogin)
  {
    this.m_bSecureLogin = bSecureLogin;
    if (bSecureLogin && $location.protocol() !== 'https')
    {
      $window.location.href = $location.absUrl().replace('http', 'https');
    }
  };
  // }}}
  // {{{ setSecurity()
  factory.setSecurity = function (strLoginType)
  {
    this.m_strLoginType = strLoginType;
  };
  // }}}
  // {{{ setSessionStorage()
  factory.setSessionStorage = function (sessionStorage)
  {
    this.m_sessionStorage = sessionStorage;
  };
  // }}}
  // {{{ store()
  factory.store = function (controller, data)
  {
    this.m_store[controller] = data;
  };
  // }}}
  // {{{ trackLocation()
  factory.trackLocation = function (nIndex)
  {
    var loc = $location.path().split('/');
    var strMenu = '';
    if (!angular.isDefined(nIndex))
    {
      nIndex = 1;
    }
    if (loc.length >= nIndex + 1)
    {
      strMenu = loc[nIndex];
    }
    if (strMenu == 'Login' || (strMenu.length >= 6 && strMenu.substr(0, 6) == 'Logout'))
    {
      $cookies.put('sl_commonLoginRedirectTrigger', 1);
    }
    else
    {
      if (angular.isDefined($cookies.get('sl_commonLoginRedirectTrigger')))
      {
        var strRedirect = ((angular.isDefined($cookies.get('sl_commonLoginRedirect')))?$cookies.get('sl_commonLoginRedirect'):this.m_strRedirectPath);
        $cookies.remove('sl_commonLoginRedirectTrigger');
        document.location.href = strRedirect;
      }
      else
      {
        this.setRedirectPath($location.absUrl());
        $cookies.put('sl_commonLoginRedirect', $location.absUrl());
      }
    }
  };
  // }}}
  // {{{ wsCreate()
  factory.wsCreate = function (strApplication, strName, strServer, strPort, bSecure, strProtocol)
  {
    var bResult = false;

    if (strApplication.length > 0 && strName.length > 0 && strPort.length > 0 && strProtocol.length > 0 && (bSecure == true || bSecure == false) && strServer.length > 0)
    {
      var date = new Date();
      if (!angular.isDefined(this.m_ws[strName]))
      {
        this.m_ws[strName] = {};
      }
      this.m_ws[strName].Application = strApplication;
      this.m_ws[strName].Attempts = 1;
      this.m_ws[strName].Connected = false;
      if (!angular.isDefined(this.m_ws[strName].modalInstance))
      {
        this.m_ws[strName].modalInstance = null;
      }
      this.m_ws[strName].Port = strPort;
      this.m_ws[strName].Protocol = strProtocol;
      this.m_ws[strName].Secure = bSecure;
      this.m_ws[strName].Server = strServer;
      this.m_ws[strName].Unique = 0;
      this.m_ws[strName].ws = $websocket(((bSecure)?'wss':'ws') + '://' + strServer + ':' + strPort + '/', strProtocol);
      this.m_ws[strName].ws.onOpen(function ()
      {
        $rootScope.$root.$broadcast('commonWsReady_' + strApplication, null);
        $rootScope.$root.$broadcast('commonWsReady_' + strApplication + '_' + strName, null);
        factory.m_ws[strName].Attempts = 1;
        if (factory.m_ws[strName].modalInstance != null)
        {
          factory.m_ws[strName].modalInstance.close();
          factory.m_ws[strName].modalInstance = null;
        }
        factory.m_ws[strName].Connected = true;
        factory.auth(null);
      });
      this.m_ws[strName].ws.onClose(function (ev)
      {
        var maxInterval = (Math.pow(2, factory.m_wsAttempts) - 1) * 1000;
        console.log(ev);
        factory.m_bConnecting = false;
        factory.m_bSentJwt = false;
        factory.m_wsRequestID = null;
        $rootScope.$root.$broadcast('commonWsClose_' + strApplication, ev);
        $rootScope.$root.$broadcast('commonWsClose_' + strApplication + '_' + strName, ev);
        if (factory.m_ws[strName].Connected)
        {
          factory.m_ws[strName].Connected = false;
          //factory.m_ws[strName].modalInstance = $uibModal.open(
          //{
          //  templateUrl: '/include/common/angularjs/websocket.html',
          //  backdrop: true,
          //  windowClass: 'modal',
          //  controller: function ($scope, $uibModalInstance)
          //  {
          //    $scope.Application = factory.m_ws[strName].Application;
          //  }
          //});
        }
        if (maxInterval > 30 * 1000)
        {
          maxInterval = 30 * 1000;
        }
        setTimeout(function ()
        {
          factory.m_ws[strName].Attempts++;
          factory.wsCreate(strApplication, strName, strServer, strPort, bSecure, strProtocol);
        }, (Math.random() * maxInterval));
      });
      this.m_ws[strName].ws.onError(function (ev)
      {
        console.log(ev);
        $rootScope.$root.$broadcast('commonWsError_' + strApplication, ev);
        $rootScope.$root.$broadcast('commonWsError_' + strApplication + '_' + strName, ev);
      });
      this.m_ws[strName].ws.onMessage(function (event)
      {
        var response = JSON.parse(event.data);
        if (factory.m_bJwt && strProtocol == 'bridge' && response.Status == 'error' && response.Error && response.Error.Type && response.Error.Type == 'bridge' && response.Error.SubType && response.Error.SubType == 'request' && response.Error.Message && response.Error.Message == 'Failed: exp')
        {
          response.Error.Message = 'Session expired.  Please login again.';
          factory.m_auth = null;
          factory.m_auth = {admin: false, apps: {}};
          factory.m_bSentJwt = false;
          if (factory.m_sessionStorage && factory.m_sessionStorage.sl_wsJwt)
          {
            factory.m_sessionStorage.sl_wsJwt = null;
          }
          else if (factory.m_wsJwt)
          {
            factory.m_wsJwt = null;
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
          //factory.m_ws[strName].ws.send(JSON.stringify(response));
        }
        else if (!factory.m_bJwt && strProtocol == 'bridge' && factory.m_ws[strName].SetBridgeCredentials && factory.m_ws[strName].SetBridgeCredentials.Script && factory.m_ws[strName].SetBridgeCredentials.Script.length > 0 && factory.m_ws[strName].SetBridgeCredentials['Function'] && factory.m_ws[strName].SetBridgeCredentials['Function'].length > 0 && response.Status && response.Status == 'error' && response.Error && response.Error.Type && response.Error.Type == 'bridge' && response.Error.SubType && ((response.Error.SubType == 'request' && response.Error.Message && response.Error.Message == 'Your access has been denied.') || (response.Error.SubType == 'requestSocket' && response.Error.Message && response.Error.Message == 'Please provide the User.')))
        {
          if (angular.isDefined($cookies.get('PHPSESSID')))
          {
            if (!factory.m_bSetBridgeCredentials)
            {
              factory.m_bSetBridgeCredentials = true;
              $http.post(factory.m_ws[strName].SetBridgeCredentials['Script'], {'Function': factory.m_ws[strName].SetBridgeCredentials['Function']}).then(function (response) {factory.m_bSetBridgeCredentials = false;}, function (response) {factory.m_bSetBridgeCredentials = false});
            }
            delete response.Error;
            delete response.wsRequestID;
            delete response.Response;
            delete response.Status;
            response.wsSessionID = $cookies.get('PHPSESSID');
            factory.m_ws[strName].ws.send(JSON.stringify(response));
          }
          else if (factory.isValid())
          {
            alert('Your session has expired.  Please login again.');
            document.location.href = '#/Logout';
          }
        }
        else
        {
          if (angular.isDefined(response.wsController))
          {
            $rootScope.$root.$broadcast(response.wsController, response);
          }
          else
          {
            $rootScope.$root.$broadcast('commonWsMessage_' + strApplication, response);
            $rootScope.$root.$broadcast('commonWsMessage_' + strApplication + '_' + strName, response);
          }
          if (response.wsRequestID && response.wsRequestID == factory.m_wsRequestID && response.Action)
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
                  var audio = new Audio(response.Media);
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
              var date = null;
              if (angular.isDefined(response.Time) && response.Time != '')
              {
                date = new Date(response.Time * 1000);
              }
              else
              {
                date = new Date();
              }
              response.Time = date.getFullYear() + '-' + (date.getMonth() + 1) + '-' + date.getDay() + ' ' + date.getHours() + ':' + date.getMinutes() + ':' + date.getSeconds();
              factory.m_messages.push(response);
              factory.m_messages[factory.m_messages.length - 1].Index = (factory.m_messages.length - 1);
            }
            else if (response.Action == 'notification')
            {
              if ('Notification' in window)
              {
                if (response.Title && response.Body)
                {
                  Notification.requestPermission(function (permission)
                  {
                    var nDuration = 5000;
                    if (response.Duration)
                    {
                      nDuration = response.Duration * 1000;
                    }
                    var notification = new Notification(response.Title, {body: response.Body, icon: null, dir: 'auto'});
                    setTimeout(function()
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
      });
    }

    return bResult;
  };
  // }}}
  // {{{ wsRequest()
  factory.wsRequest = function (strName, request)
  {
    var deferred = $q.defer();

    if (angular.isDefined(this.m_ws[strName]))
    {
      var data = this;
      var unHandle = this.wsUnique(strName);
      this.wsSend(strName, unHandle, request);
      var unbind = $rootScope.$on(unHandle, function (event, response)
      {
        deferred.resolve(response);
        unbind();
      });
    }

    return deferred.promise;
  }
  // }}}
  // {{{ wsResponse()
  factory.wsResponse = function (response, error)
  {
    var bResult = false;

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
  };
  // }}}
  // {{{ wsSend()
  factory.wsSend = function (strName, strController, request)
  {
    if (angular.isDefined(this.m_ws[strName]))
    {
      request.reqApp = this.m_strApplication;
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
        else if (angular.isDefined($cookies.get('sl_commonWsJwt')) && $cookies.get('sl_commonWsJwt'))
        {
          this.m_bSentJwt = true;
          request.wsJwt = $cookies.get('sl_commonWsJwt');
        }
      }
      if (angular.isDefined(factory.m_wsSessionID))
      {
        if (!angular.isDefined($cookies.get('PHPSESSID')))
        {
          $cookies.put('PHPSESSID', factory.m_wsSessionID);
        }
      }
      else if (angular.isDefined($cookies.get('PHPSESSID')))
      {
        factory.m_wsSessionID = $cookies.get('PHPSESSID');
      }
      request.wsSessionID = factory.m_wsSessionID;
      this.m_ws[strName].ws.send(JSON.stringify(request));
    }
  }
  // }}}
  // {{{ wsSetBridgeCredentials()
  factory.wsSetBridgeCredentials = function (strName, strScript, strFunction)
  {
    if (!angular.isDefined(this.m_ws[strName]))
    {
      this.m_ws[strName] = {};
    }
    this.m_ws[strName].SetBridgeCredentials = {Script: strScript, 'Function': strFunction};
  };
  // }}}
  // {{{ wsUnique()
  factory.wsUnique = function (strName)
  {
    var nUnique = 0;

    if (angular.isDefined(this.m_ws[strName]))
    {
      nUnique = this.m_ws[strName].Unique++;
    }

    return nUnique;
  }
  // }}}
  // {{{ main
  setInterval(function ()
  {
    factory.wsRequest('bridge', {Section: 'ping', Request: {}}).then(function (response) {});
  }, 120000);
  // }}}
  return factory;
}
app.factory(factories);
