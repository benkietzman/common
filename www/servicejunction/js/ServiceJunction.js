// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-01-19
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////

class ServiceJunction
{
  // {{{ batch()
  batch(request, bSecure, callback)
  {
    fetch('/common/servicejunction/angularjs/serviceJunctionWorker.php', {method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({'Function': 'batch', Transport: ((bSecure)?'secure':'standard'), Request: request})}).then(response => response.json()).then(callback);
  }
  // }}}
  // {{{ request()
  request(request, bSecure, callback)
  {
    fetch('/common/servicejunction/angularjs/serviceJunctionWorker.php', {method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({'Function': 'request', Transport: ((bSecure)?'secure':'standard'), Request: request})}).then(response => response.json()).then(callback);
  }
  // }}}
  // {{{ response()
  response(response, error)
  {
    let bResult = false;

    if (this.isDefined(response.Status) && response.Status == 'okay')
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

    return bResult;
  }
  // }}}
}
