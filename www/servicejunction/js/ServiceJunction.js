///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-01-19
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////

class ServiceJunction
{
  // {{{ batch()
  batch(request, bSecure, callback)
  {
    fetch('/include/common/servicejunction/angularjs/serviceJunctionWorker.php', {method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({'Function': 'batch', Transport: ((bSecure)?'secure':'standard'), Request: request})}).then(response => response.json()).then(callback);
  }
  // }}}
  // {{{ request()
  request(request, bSecure, callback)
  {
    fetch('/include/common/servicejunction/angularjs/serviceJunctionWorker.php', {method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({'Function': 'request', Transport: ((bSecure)?'secure':'standard'), Request: request})}).then(response => response.json()).then(callback);
  }
  // }}}
  // {{{ response()
  response(response, error)
  {
    let bResult = false;

    if (response.Status && response.Status == 'okay')
    {
      if (response['Function'] == 'batch')
      {
        bResult = true;
      }
      else if (response['Function'] == 'request')
      {
        if (response.Response.length > 0)
        {
          if (response.Response[0].Status && response.Response[0].Status == 'okay')
          {
            bResult = true;
          }
          else if (response.Response[0].Error && response.Response[0].Error.length > 0)
          {
            error.message = response.Response[0].Error;
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
      if (response.Error && response.Error.length > 0)
      {
        error.message = response.Error;
      }
      else
      {
        error.message = 'Encountered an unknown error.';
      }
    }

    return bResult;
  }
  // }}}
}
