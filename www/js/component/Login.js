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
    });
    // }}}
    // {{{ getLoginTypes()
    s.getLoginTypes = () =>
    {
      let request = {Interface: 'central', Section: 'central', 'Function': 'loginTypes'};
      c.wsRequest(c.m_strAuthProtocol, request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error) && c.isDefined(response.Response))
        {
          s.loginTypes = response.Response;
          c.update('Login');
        }
      });
    };
    // }}}
    // {{{ passkey()
    s.passkey = () =>
    {
      let request = {Interface: 'secure', Section: 'secure', 'Function': 'passkeyAssertion'};
      c.wsRequest(c.m_strAuthProtocol, request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error) && c.isDefined(response.Response))
        {
          if (c.isDefined(response.Response.publicKey) && c.isDefined(response.Response.publicKey.challenge))
          {
            response.Response.publicKey.challenge = c.base64ToBuffer(response.Response.publicKey.challenge);
          }
          navigator.credentials.get(response.Response)
          .then((publicKeyCredential) =>
          {
            s.processLogin
            ({
              authenticatorData: c.bufferToBase64(publicKeyCredential.response.authenticatorData),
              clientDataJSON: c.bufferToBase64(publicKeyCredential.response.clientDataJSON),
              id: publicKeyCredential.id,
              password: null,
              rawId: c.bufferToBase64(publicKeyCredential.rawId),
              signature: c.bufferToBase64(publicKeyCredential.response.signature),
              type: publicKeyCredential.type
            });
          });
        }
      });
    };
    // }}}
    // {{{ processLogin()
    s.processLogin = (publicKeyCredential) =>
    {
      c.processLogin(publicKeyCredential);
    };
    // }}}
    // {{{ processLoginKey()
    s.processLoginKey = () =>
    {
      if (window.event.keyCode == 13)
      {
        s.processLogin();
      }
    };
    // }}}
    // {{{ switchLoginType()
    s.switchLoginType = (strLoginType) =>
    {
      if (!c.isDefined(strLoginType))
      {
        strLoginType = c.simplify(s.loginType);
      }
      for (let i = 0; i < s.loginTypes.length; i++)
      {
        if (s.loginTypes[i].type == strLoginType)
        {
          let loginType = c.simplify(s.loginTypes[i]);
          c.m_bLoginRemote = loginType.remote.value;
          c.m_strLoginTitle = loginType.title;
        }
      }
      c.m_strLoginType = strLoginType.charAt(0).toLowerCase() + strLoginType.slice(1);
      c.unsetRerouteTimeout();
      c.login.message = false;
      s.processLogin();
    };
    // }}}
    // {{{ main
    c.setMenu('Login', null);
    c.attachEvent('commonWsReady_' + c.application, (data) =>
    {
      s.getLoginTypes();
      s.passkey();
      s.processLogin();
    });
    s.getLoginTypes();
    s.passkey();
    s.processLogin();
    // }}}
  },
  // }}}
  // {{{ template
  template: `
    <div class="row justify-content-md-center" style="margin-top: 100px;">
      <div class="col-md-auto">
        <div class="card border border-primary-subtle">
          <div class="card-header bg-primary fs-5 fw-bold">
            <select class="form-select float-end" c-change="switchLoginType()" c-model="loginType" style="background: inherit; margin-left: 10px; width: 20px;">{{#each loginTypes}}<option value="{{type}}">{{type}}</option>{{/each}}</select>
            <i class="bi bi-box-arrow-in-right"></i> {{c.m_strLoginTitle}}
          </div>
          {{#ifCond c.m_strLoginType "==" 'auto'}}
          <div class="card-body bg-primary-subtle">
            <ul class="list-group">
              {{#each ../loginTypes}}
              {{#ifCond type "!=" 'Auto'}}
              <button class="btn btn-primary list-group-item" type="button" c-click="switchLoginType('{{../type}}')" style="margin: 10px;">{{../type}}</button>
              {{/ifCond}}
              {{/each}}
            </ul>
          </div>
          {{else}}
          {{#if ../c.login.showForm}}
          <div class="card-body bg-primary-subtle">
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
          </div>
          {{/if}}
          {{/ifCond}}
          <div class="card-footer">
            {{#if c.login.message}}
            <b class="fs-6 text-danger">{{c.login.message}}</b>
            {{/if}}
            {{#if c.login.info}}
            <span class="fs-6 text-warning">{{c.login.info}}</span>
            {{/if}}
            <span class="fs-6 text-warning" c-model="c.login.rerouteMessage"></span>
            {{#if c.login.reroutePath}}
            <a href="{{c.login.reroutePath}}">proceed</a>
            {{/if}}
            {{#if c.login.showForm}}
            <button class="btn btn-primary float-end" c-click="processLogin()">Login</button>
            {{/if}}
          </div>
        </div>
      </div>
    </div>
    <div class="row">
      <div class="col-md-3">
      </div>
      <div class="col-md-6">
        <div class="card card-body card-inverse" style="margin-top: 20px;">
          It is recommended to register a passkey.  You can do that once authenticated by clicking the green applications tab on the right, select Central, click the Profile menu, locate the Security Profile section, and click the Passkeys button.  Registering a passkey enhances your online security and simplifies the login process by eliminating the need to remember complex passwords.  It allows you to authenticate using biometric methods or a PIN, making it faster and more intuitive.
        </div>
      </div>
      <div class="col-md-3">
      </div>
    </div>
  `
  // }}}
}
