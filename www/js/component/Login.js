///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-11-11
// copyright  : Ben Kietzman
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
      loginTypes: [],
      processLoginKey: () =>
      {
        if (window.event.keyCode == 13)
        {
          c.processLogin();
        }
      }
    });
    // }}}
    // {{{ getLoginTypes()
    s.getLoginTypes = () =>
    {
      let request = {Interface: 'central', Section: 'central', 'Function': 'loginTypes'};
      c.wsRequest(c.m_strAuthProtocol, request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          if (c.isDefined(response.Response))
          {
            s.loginTypes = response.Response;
            c.update('Login');
          }
        }
      });
    };
    // }}}
    // {{{ switchLoginType()
    s.switchLoginType = () =>
    {
      let loginType = c.simplify(s.loginType);
      c.unsetRedirectTimeout();
      if (c.isDefined(loginType) && c.isDefined(loginType.type) && loginType.type.length > 1)
      {
        c.processLogin(loginType.type.charAt(0).toLowerCase() + loginType.type.slice(1));
      }
    };
    // }}}
    // {{{ main
    c.setMenu('Login', null);
    c.attachEvent('commonWsReady_' + c.application, (data) =>
    {
      s.getLoginTypes();
      c.processLogin();
    });
    s.getLoginTypes();
    c.processLogin();
    // }}}
  },
  // }}}
  // {{{ template
  template: `
    <div class="row justify-content-md-center" style="margin-top: 100px;">
      <div class="col-md-auto">
        <div class="card border border-primary-subtle">
          <div class="card-header bg-primary fs-5 fw-bold">
            <select class="form-select float-end" c-change="switchLoginType()" c-model="loginType" style="width: 20px;" c-json>{{#each loginTypes}}<option value="{{json .}}">{{type}}</option>{{/each}}</select>
            <i class="bi bi-box-arrow-in-right"></i> {{c.login.login.title}}
          </div>
          <div class="card-body bg-primary-subtle">
            {{#if c.login.showForm}}
            <div class="row">
              <div class="col fs-5">
                User
              </div>
              <div class="col">
                <input class="form-control" type="text" c-model="c.login.login.userid" maxlength="20" c-keyup="processLoginKey()" autofocus>
              </div>
            </div>
            <div class="row" style="margin-top: 10px;">
              <div class="col fs-5">
                Password
              </div>
              <div class="col">
                <input class="form-control" type="password" c-model="c.login.login.password" maxlength="64" c-keyup="processLoginKey()">
              </div>
            </div>
            {{/if}}
          </div>
          <div class="card-footer">
            {{#if c.login.message}}
            <b class="fs-6 text-danger">{{c.login.message}}</b>
            {{/if}}
            {{#if c.login.info}}
            <span class="fs-6 text-warning">{{c.login.info}}</span>
            {{/if}}
            {{#if c.login.showForm}}
            <button class="btn btn-primary float-end" c-click="c.processLogin()">Login</button>
            {{/if}}
          </div>
        </div>
      </div>
    </div>
  `
  // }}}
}
