// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2015-08-03
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////

var controllers = {};
// {{{ messages()
controllers.messages = function ($scope, common)
{
  $scope.common = common;
  $scope.close = function (nIndex)
  {
    var messages = [];
    for (var i = 0; i < common.m_messages.length; i++)
    {
      if (i != nIndex)
      {
        messages.push(common.m_messages[i]);
        messages[messages.length - 1].Index = (messages.length - 1);
      }
    }
    common.m_messages = null;
    common.m_messages = messages;
  }
};
// }}}
// {{{ Login()
controllers.Login = function ($cookies, $http, $location, $scope, $window, common)
{
  // {{{ variables
  $scope.controller = 'Login';
  $scope.info = 'Processing login...';
  $scope.showForm = false;
  // }}}
  // {{{ processLogin
  $scope.processLogin = function (login)
  {
    $scope.info = 'Processing login...';
    if (!angular.isDefined(login) || login == null)
    {
      login = {};
    }
    if (common.m_bJwt)
    {
      var request = {Interface: 'radial', Section: 'secure', 'Function': 'getSecurityModule', Request: {}};
      common.wsRequest(common.m_strAuthProtocol, request).then(function (response)
      {
        if (common.m_strLoginType == null)
        {
          common.m_strLoginType = 'password';
          if (angular.isDefined(response.Response.Module) && response.Response.Module != '')
          {
            common.m_strLoginType = response.Response.Module;
          }
        }
        $http.get('/include/common_addons/auth/modules.json').then(function(response)
        {
          var data = null;
          if (angular.isDefined(response.data[common.m_strLoginType]) && angular.isDefined(response.data[common.m_strLoginType]['cookie']) && angular.isDefined($cookies.get(response.data[common.m_strLoginType]['cookie'])))
          {
            data = $cookies.get(response.data[common.m_strLoginType]['cookie']);
          }
          $scope.processLoginCont(login, data);
        }, function ()
        {
          $scope.processLoginCont(login, null);
        });
      });
    }
    else
    {
      login.Application = common.getApplication();
      if (common.m_strLoginType != '')
      {
        login.LoginType = common.m_strLoginType;
      }
      common.request('authProcessLogin', login, function (result)
      {
        var error = {};
        if (common.response(result, error))
        {
          common.m_auth = result.data.Response.out;
          common.m_bHaveAuth = true;
          if (angular.isDefined(common.m_auth.login_title))
          {
            if (!angular.isDefined($scope.login) || $scope.login == null)
            {
              $scope.login = {};
            }
            $scope.login.title = common.m_auth.login_title;
            if ($scope.login.title.length <= 30 || $scope.login.title.substr($scope.login.title.length - 30, 30) != ' (please wait for redirect...)')
            {
              $scope.showForm = true;
            }
          }
          if (common.isValid(null))
          {
            $scope.$root.$broadcast('resetMenu', null);
            document.location.href = common.m_strRedirectPath;
          }
          else
          {
            var login = {};
            login.Application = common.getApplication();
            if (common.m_strLoginType != '')
            {
              login.LoginType = common.m_strLoginType;
            }
            login.Redirect = common.getRedirectPath();
            common.request('authLogin', login, function (result)
            {
              var error = {};
              $scope.info = null;
              if (common.response(result, error))
              {
                if (angular.isDefined(result.data.Response.out.Status) && result.data.Response.out.Status == 'okay')
                {
                  if (angular.isDefined(result.data.Response.out.Redirect) && result.data.Response.out.Redirect.length > 0)
                  {
                    $scope.$root.$broadcast('resetMenu', null);
                    document.location.href = result.data.Response.out.Redirect;
                  }
                }
                else if (angular.isDefined(result.data.Response.out.Error) && result.data.Response.out.Error.length > 0)
                {
                  $scope.message = result.data.Response.out.Error;
                }
                else
                {
                  $scope.message = 'Encountered an unknown error.';
                }
              }
              else
              {
                $scope.message = error.message;
              }
            });
          }
        }
        else
        {
          $scope.message = error.message;
        }
      });
    }
  }
  // }}}
  // {{{ processLoginCont
  $scope.processLoginCont = function (login, data)
  {
    var request = null;
    request = {Interface: 'radial', Section: 'secure', 'Function': 'process', Request: login};
    request.Request.Type = common.m_strLoginType;
    if (angular.isDefined($cookies.get('sl_commonUniqueID')))
    {
      request.Request.UniqueID = $cookies.get('sl_commonUniqueID');
    }
    if (data)
    {
      request.Request.Data = data;
    }
    common.wsRequest(common.m_strAuthProtocol, request).then(function (response)
    {
      var error = {};
      if (common.wsResponse(response, error))
      {
        if (angular.isDefined(response.Error) && angular.isDefined(response.Error.Message) && response.Error.Message.length > 0 && response.Error.Message.search('Please provide the User.') == -1)
        {
          $scope.message = response.Error.Message;
        }
        if (angular.isDefined(response.Response.auth))
        {
          common.m_auth = response.Response.auth;
          common.m_bHaveAuth = true;
          if (angular.isDefined(response.Response.jwt))
          {
            if (common.m_sessionStorage)
            {
              common.m_sessionStorage.sl_wsJwt = response.Response.jwt;
            }
            else if (response.Response.jwt.length > 4096)
            {
              common.m_wsJwt = response.Response.jwt;
            }
            else
            {
              $cookies.put('sl_commonWsJwt', response.Response.jwt);
            }
          }
          if (angular.isDefined(common.m_auth.login_title))
          {
            if (!angular.isDefined($scope.login) || $scope.login == null)
            {
              $scope.login = {};
            }
            $scope.login.title = common.m_auth.login_title;
            if ($scope.login.title.length <= 30 || $scope.login.title.substr($scope.login.title.length - 30, 30) != ' (please wait for redirect...)')
            {
              $scope.showForm = true;
            }
          }
          if (common.isValid(null))
          {
            $scope.$root.$broadcast('resetMenu', null);
            document.location.href = common.m_strRedirectPath;
          }
          else
          {
            var request = {Interface: 'secure', Section: 'secure', 'Function': 'login'};
            request.Request = {Type: common.m_strLoginType, Return: document.location.href};
            common.wsRequest(common.m_strAuthProtocol, request).then(function (response)
            {
              var error = {};
              $scope.info = null;
              if (common.wsResponse(response, error))
              {
                if (angular.isDefined(response.Response.UniqueID) && response.Response.UniqueID.length > 0)
                {
                  $cookies.put('sl_commonUniqueID', response.Response.UniqueID);
                }
                if (angular.isDefined(response.Response.Redirect) && response.Response.Redirect.length > 0)
                {
                  $scope.$root.$broadcast('resetMenu', null);
                  document.location.href = response.Response.Redirect;
                }
                else
                {
                  $('#login_userid').focus();
                }
              }
              else
              {
                $scope.message = error.message;
              }
            });
          }
        }
        if (angular.isDefined(error.message))
        {
          $scope.message = error.message;
        }
      }
      else
      {
        $scope.message = error.message;
      }
    });
  }
  // }}}
  // {{{ submit()
  $scope.submit = function ()
  {
    $scope.processLogin($scope.login);
  };
  // }}}
  // {{{ main
  common.setMenu($scope.controller);
  $('#login_userid').focus();
  $scope.processLogin(null);
  // }}}
};
// }}}
// {{{ Logout()
controllers.Logout = function ($cookies, $location, $scope, common)
{
  $scope.info = 'Processing logout...';
  if (common.isValid(null))
  {
    if (common.m_bJwt)
    {
      var request = null;
      request = {Interface: 'secure', Section: 'secure', 'Function': 'getSecurityModule', Request: {}};
      common.wsRequest(common.m_strAuthProtocol, request).then(function (response)
      {
        if (common.m_strLoginType == null)
        {
          common.m_strLoginType = 'password';
          if (angular.isDefined(response.Response.Module) && response.Response.Module != '')
          {
            common.m_strLoginType = response.Response.Module;
          }
        }
        var request = {Interface: 'secure', Section: 'secure', 'Function': 'logout'};
        request.Request = {Type: common.m_strLoginType, Return: common.getRedirectPath()};
        common.wsRequest(common.m_strAuthProtocol, request).then(function (response)
        {
          var error = {};
          $scope.info = null;
          if (common.wsResponse(response, error))
          {
            if (angular.isDefined(response.Response.Redirect) && response.Response.Redirect.length > 0)
            {
              common.m_bHaveAuth = false;
              common.m_auth = null;
              common.m_auth = {admin: false, apps: {}};
              if (common.m_sessionStorage && common.m_sessionStorage.sl_wsJwt)
              {
                common.m_sessionStorage.sl_wsJwt = null;
              }
              else if (common.m_wsJwt)
              {
                common.m_wsJwt = null;
              }
              else
              {
                $cookies.put('sl_commonWsJwt', '');
              }
              if (angular.isDefined($cookies.get('sl_commonUniqueID')))
              {
                $cookies.put('sl_commonUniqueID', '');
              }
              $scope.$root.$broadcast('resetMenu', null);
              document.location.href = response.Response.Redirect;
            }
          }
          else
          {
            $scope.message = error.message;
          }
        });
      });
    }
    else
    {
      var logout = {};
      logout.Application = common.getApplication();
      if (common.m_strLoginType != '')
      {
        logout.LoginType = common.m_strLoginType;
      }
      logout.Redirect = common.getRedirectPath();
      common.request('authLogout', logout, function (result)
      {
        var error = {};
        $scope.info = null;
        if (common.response(result, error))
        {
          if (angular.isDefined(result.data.Response.out.Status) && result.data.Response.out.Status == 'okay')
          {
            if (angular.isDefined(result.data.Response.out.Redirect) && result.data.Response.out.Redirect.length > 0)
            {
              $scope.$root.$broadcast('resetMenu', null);
              document.location.href = result.data.Response.out.Redirect;
            }
            else
            {
              $scope.message = 'Redirect not found.';
            }
          }
          else if (angular.isDefined(result.data.Response.out.Error) && result.data.Response.out.Error.length > 0)
          {
            $scope.message = result.data.Response.out.Error;
          }
          else
          {
            $scope.message = 'Encountered an unknown error.';
          }
        }
        else
        {
          $scope.message = error.message;
        }
      });
    }
  }
  else
  {
    common.m_bHaveAuth = false;
    common.m_auth = null;
    common.m_auth = {admin: false, apps: {}};
    if (common.m_bJwt)
    {
      if (common.m_sessionStorage && common.m_sessionStorage.sl_wsJwt)
      {
        common.m_sessionStorage.sl_wsJwt = null;
      }
      else if (common.m_wsJwt)
      {
        common.m_wsJwt = null;
      }
      else
      {
        $cookies.put('sl_commonWsJwt', '');
      }
      if (angular.isDefined($cookies.get('sl_commonUniqueID')))
      {
        $cookies.put('sl_commonUniqueID', '');
      }
    }
    $scope.$root.$broadcast('resetMenu', null);
    document.location.href = common.m_strRedirectPath;
  }
};
// }}}
// {{{ bridgeStatus()
controllers.bridgeStatus = function ($interval, $scope, common)
{
  $scope.last = 0;

  $scope.expire = function()
  {
    var current = Date.now();
    if ((current - $scope.last) >= 5000)
    {
      $scope.stat = null;
    }
  }
  $scope.$on('bridgePurpose_status', function (event, stat)
  {
    $scope.last = Date.now();
    if (stat.Activity)
    {
      stat.Activity = stat.Activity.split(',').join(' --> ');
    }
    $scope.stat = stat;
  });
  $interval(function()
  {
    $scope.expire();
  }, 250);
};
// }}}
// {{{ radialStatus()
controllers.radialStatus = function ($interval, $scope, common)
{
  $scope.last = 0;

  $scope.expire = function()
  {
    var current = Date.now();
    if ((current - $scope.last) >= 5000)
    {
      $scope.stat = null;
    }
  }
  $scope.$on('radialPurpose_status', function (event, stat)
  {
    $scope.last = Date.now();
    if (stat.Activity)
    {
      stat.Activity = stat.Activity.split(',').join(' --> ');
    }
    $scope.stat = stat;
  });
  $interval(function()
  {
    $scope.expire();
  }, 250);
};
// }}}
app.controller(controllers);
