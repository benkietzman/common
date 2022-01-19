// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2013-11-08
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////

var factories = {};
factories.junction = function ($http)
{
  var factory = {};
  // {{{ batch()
  factory.batch = function (request, bSecure, callback)
  {
    $http.post('/include/common/servicejunction/angularjs/serviceJunctionWorker.php', {'Function': 'batch', Transport: ((bSecure)?'secure':'standard'), Request: request}).then(callback);
  };
  // }}}
  // {{{ request()
  factory.request = function (request, bSecure, callback)
  {
    $http.post('/include/common/servicejunction/angularjs/serviceJunctionWorker.php', {'Function': 'request', Transport: ((bSecure)?'secure':'standard'), Request: request}).then(callback);
  };
  // }}}
  // {{{ response()
  factory.response = function (response, error)
  {
    var bResult = false;

    if (response.status == 200)
    {
      if (response.data.Status && response.data.Status == 'okay')
      {
        if (response.data['Function'] == 'batch')
        {
          bResult = true;
        }
        else if (response.data['Function'] == 'request')
        {
          if (response.data.Response.length > 0)
          {
            if (response.data.Response[0].Status && response.data.Response[0].Status == 'okay')
            {
              bResult = true;
            }
            else if (response.data.Response[0].Error && response.data.Response[0].Error.length > 0)
            {
              error.message = response.data.Response[0].Error;
            }
            else
            {
              error.message = 'Failed to receive Service Junction Error message.';
            }
          }
          else
          {
            error.message = 'Failed to receive Service Junction response.';
          }
        }
        else
        {
          error.message = 'Invalid Service Junction factory Function.';
        }
      }
      else if (error != null)
      {
        if (response.data.Error && response.data.Error.length > 0)
        {
          error.message = response.data.Error;
        }
        else
        {
          error.message = 'Encountered an unknown error.';
        }
      }
    }
    else if (error != null)
    {
      error.message = response.status;
    }

    return bResult;
  };
  // }}}
  return factory;
}
app.factory(factories);
