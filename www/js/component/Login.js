// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-11-11
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
export default
{
  // {{{ controller()
  controller(id, nav)
  {
    // {{{ prep work
    let c = common;
    let s = c.store('Login',
    {
      c: c,
      processLoginKey: () =>
      {
        if (window.event.keyCode == 13)
        {
          c.processLogin()
        }
      }
    });
    // }}}
    // {{{ main
    c.setMenu('Login', null);
    c.attachEvent('commonWsReady_' + c.application, (data) =>
    {
      c.processLogin();
    });
    c.processLogin();
    // }}}
  },
  // }}}
  // {{{ template
  template: `
    <br><br><br>
    {{#if c.login.message}}
    <div style="color:red;font-weight:bold;"><br><br>{{c.login.message}}<br><br></div>
    {{/if}}
    {{#if c.login.info}}
    <div style="color:orange;"><br><br>{{c.login.info}}<br><br></div>
    {{/if}}
    {{#if c.login.showForm}}
    <div class="row">
      <div class="col-md-3"></div>
      <div class="col-md-6">
        <div class="card">
          <div class="card-header">
            <h3 class="page-header">{{c.login.login.title}}</h3>
          </div>
          <div class="card-body table-responsive">
            <table class="table">
              <tr>
                <td><div class="input-group"><span class="input-group-text">User</span><input class="form-control" type="text" c-model="c.login.login.userid" maxlength="20" c-keyup="processLoginKey()" autofocus></div></td>
                <td><div class="input-group"><span class="input-group-text">Password</span><input class="form-control" type="password" c-model="c.login.login.password" maxlength="64" c-keyup="processLoginKey()"></div></td>
                <td><button class="btn btn-primary float-end" c-click="c.processLogin()">Login</button></td>
              </tr>
            </table>
          </div>
        </div>
      </div>
      <div class="col-md-3"></div>
    </div>
    {{/if}}
  `
  // }}}
}
