// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2024-01-26
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
export default
{
  // {{{ controller()
  controller(id)
  {
    // {{{ prep work
    let c = common;
    let s = c.store('radialChat',
    {
      // {{{ u()
      u: () =>
      {
        c.render(id, this);
      },
      // }}}
      c: c,
      notify: false,
      history: [],
      menu: false,
      message: null,
      user: null,
      users: []
    });
    // }}}
    // {{{ chat()
    s.chat = () =>
    {
      if (c.isValid() && s.user.v)
      {
        s.history.push({Message: s.message.v, User: c.getUserID(), FirstName: c.getUserFirstName(), LastName: c.getUserLastName()});
        let request = {Interface: 'live', 'Function': 'message', Request: {User: s.user.v, Message: {Action: 'chat', Message: s.message.v, User: c.getUserID(), FirstName: c.getUserFirstName(), LastName: c.getUserLastName()}} };
        s.message.v = null;
        c.wsRequest(c.m_strAuthProtocol, request).then((response) => {});
        s.u();
        document.getElementById('history').scrollTop = document.getElementById('history').scrollHeight;
        document.getElementById('message').focus();
      }
    };
    // }}}
    // {{{ enter()
    s.enter = () =>
    {
      if (window.event.keyCode == 13)
      {
        s.chat();
      }
    };
    // }}}
    // {{{ slide()
    s.slide = () =>
    {
      s.menu = !s.menu;
      if (s.menu)
      {
        s.notify = false;
      }
      s.u();
      if (s.menu)
      {
        document.getElementById('message').focus();
        document.getElementById('history').scrollTop = document.getElementById('history').scrollHeight;
      }
    };
    // }}}
    // {{{ init()
    s.init = () =>
    {
      s.u();
      c.attachEvent('commonWsMessage_'+c.application, (data) =>
      {
        if (c.isDefined(data.detail) && c.isDefined(data.detail.Action) && data.detail.Action == 'chat' && c.isDefined(data.detail.Message) && c.isDefined(data.detail.User))
        {
          if (!s.menu)
          {
            s.notify = true;
          }
          s.history.push(data.detail);
          while (s.history.length > 50)
          {
            s.history.shift();
          }
          if (!s.user.v || s.user.v != data.detail.User)
          {
            s.user.v = data.detail.User;
          }
          s.u();
          document.getElementById('message').focus();
          document.getElementById('history').scrollTop = document.getElementById('history').scrollHeight;
        }
      });
      let request = {Interface: 'live', 'Function': 'list', Request: {Scope: 'all'}};
      c.wsRequest(c.m_strAuthProtocol, request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error) && c.isDefined(response.Response))
        {
          for (let [strSession, data] of Object.entries(response.Response))
          {
            if (c.isDefined(data.User))
            {
              let bFound = false;
              for (let i = 0; !bFound && i < s.users.length; i++)
              {
                if (s.users[i].User == data.User)
                {
                  bFound = true;
                }
              }
              if (!bFound)
              {
                let user = {User: data.User};
                if (c.isDefined(data.FirstName))
                {
                  user.FirstName = data.FirstName;
                }
                if (c.isDefined(data.LastName))
                {
                  user.LastName = data.LastName;
                }
                s.users.push(user);
              }
            }
          }
          s.u();
          if (s.users.length > 0)
          {
            s.user.v = s.users[0].User;
          }
        }
      });
    };
    // }}}
    // {{{ main
    c.attachEvent('commonAuthReady', (data) =>
    {
      s.init();
    });
    // }}}
  },
  // }}}
  // {{{ template
  template: `
    {{#isValid}}
    <div style="position: relative; z-index: 1000;">
      <div id="radial-slide-panel" class="bg-{{#if @root.notify}}warning{{else}}info{{/if}}" style="position: fixed; top: 180px; right: 0px;">
        <button id="radial-slide-opener" class="btn btn-sm btn-{{#if @root.notify}}warning{{else}}info{{/if}} float-start" c-click="slide()" style="width: 33px; height: 33px; font-size: 18px; font-weight: bold; margin: 0px 0px 0px -33px; border-radius: 10px 0px 0px 10px; vertical-align: top;"><i class="bi bi-chat-fill"></i></button>
        {{#if @root.menu}}
        <div id="radial-slide-content" style="padding: 10px;">
          <select class="form-control form-control-sm" c-model="user" style="margin-bottom: 10px;">
            {{#each @root.users}}
            <option value="{{User}}">{{LastName}}, {{FirstName}} ({{User}})</option>
            {{/each}}
          </select>
          <div class="card card-body card-inverse table-responsive" id="history" style="max-height: 200px; max-width: 400px;">
            <table class="table table-condensed table-striped">
              {{#each @root.history}}
              <tr>
                <td>{{LastName}}, {{FirstName}} ({{User}})</td>
                <td>{{Message}}</td>
              </tr>
              {{/each}}
            </table>
          </div>
          <input type="text" class="form-control form-control-sm" id="message" c-model="message" c-keyup="enter()" placeholder="Type message and hit enter..." style="margin-top: 10px;">
        </div>
        {{/if}}
      </div>
    </div>
    {{/isValid}}
  `
  // }}}
}
