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
          c.processLogin();
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
    {{#if c.login.showForm}}
    <div class="row">
      <div class="col-md-4">
      </div>
      <div class="col-md-4">
        <div class="card">
          <div class="card-header bg-info fs-4 text-white">
            <i class="bi bi-box-arrow-in-right"></i> {{c.login.login.title}}
          </div>
          <div class="card-body">
            <table class="table">
            <tbody>
            <tr><th class="fs-5">User</th><td><input class="form-control" type="text" c-model="c.login.login.userid" maxlength="20" c-keyup="processLoginKey()" autofocus></td</tr>
            <tr><th class="fs-5">Password</th><td><input class="form-control" type="password" c-model="c.login.login.password" maxlength="64" c-keyup="processLoginKey()"></td></tr>
            </tbody>
            </table>
          </div>
          <div class="card-footer">
            {{#if c.login.message}}
            <b class="fs-5 text-danger">{{c.login.message}}</b>
            {{/if}}
            {{#if c.login.info}}
            <span class="fs-5 text-warning">{{c.login.info}}</span>
            {{/if}}
            <button class="btn btn-success float-end" c-click="c.processLogin()">Login</button>
          </div>
        </div>
      </div>
      <div class="col-md-4">
      </div>
    </div>
    {{else}}
    {{#if c.login.message}}
    <div style="color:red;font-weight:bold;"><br>{{c.login.message}}</div>
    {{/if}}
    {{#if c.login.info}}
    <div style="color:orange;"><br>{{c.login.info}}</div>
    {{/if}}
    {{/if}}
  `
  // }}}
}
