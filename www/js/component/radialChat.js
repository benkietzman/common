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
      s.history.push({Message: s.message.v, User: c.getUserID()});
      let request = {Interface: 'live', 'Function': 'message', Request: {User: s.user, Message: {Action: 'chat', Message: s.message.v, User: c.getUserID()}} };
      s.message.v = null;
      c.wsRequest(c.m_strAuthProtocol, request).then((response) => {});
      s.u();
      document.getElementById('history').scrollTop = document.getElementById('history').scrollHeight;
      document.getElementById('message').focus();
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
      s.u();
    };
    // }}}
    // {{{ init()
    s.init = () =>
    {
      c.attachEvent('commonWsMessage_'+c.application, (data) =>
      {
        if (c.isDefined(data.detail) && c.isDefined(data.detail.Action) && data.detail.Action == 'chat' && c.isDefined(data.detail.Message) && c.isDefined(data.detail.User))
        {
          s.history.push(data.detail);
          while (s.history.length > 50)
          {
            s.history.shift();
          }
          s.u();
          document.getElementById('message').focus();
          document.getElementById('history').scrollTop = document.getElementById('history').scrollHeight;
        }
      });
      let request = {Interface: 'link', 'Function': 'status'};
      c.wsRequest(c.m_strAuthProtocol, request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error) && c.isDefined(response.Response))
        {
          let nodes = [];
          if (c.isDefined(response.Response.Node))
          {
            nodes.push(response.Response.Node);
          }
          if (c.isDefined(response.Response.Links))
          {
            for (let i = 0; i < response.Response.Links.length; i++)
            {
              nodes.push(response.Response.Links[i]);
            }
          }
          for (let i = 0; i < nodes.length; i++)
          {
            let request = {Interface: 'live', 'Function': 'list', Node: nodes[i], Request: {i: i}};
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
                      if (s.users[i] == data.User)
                      {
                        bFound = true;
                      }
                    }
                    if (!bFound)
                    {
                      s.users.push(data.User);
                    }
                  }
                }
                s.users.sort();
                if (s.users.length > 0)
                {
                  s.user = s.users[0];
                }
                s.u();
              }
            });
          }
        }
      });
    };
    // }}}
    // {{{ main
    c.attachEvent('commonWsReady_radial', (data) =>
    {
      s.init();
    });
    if (c.m_ws['radial'].Connected)
    {
      s.init();
    }
    // }}}
  },
  // }}}
  // {{{ template
  template: `
    <div style="position: relative; z-index: 1000;">
      <div id="radial-slide-panel" class="bg-info" style="position: fixed; top: 160px; right: 0px;">
        <button id="radial-slide-opener" class="btn btn-sm btn-info float-start" c-click="slide()" style="width: 33px; height: 33px; font-size: 18px; font-weight: bold; margin: 0px 0px 0px -33px; border-radius: 10px 0px 0px 10px; vertical-align: top;"><i class="bi bi-chat-fill"></i></button>
        {{#if menu}}
        <div id="radial-slide-content" style="padding: 10px;">
          <select class="form-control form-control-sm" c-model="user" style="margin-bottom: 10px;">
            {{#each users}}
            <option value="{{.}}">{{.}}</option>
            {{/each}}
          </select>
          <div class="card card-body card-inverse table-responsive" id="history" style="max-height: 200px; max-width: 400px;">
            <table class="table table-condensed table-striped">
              {{#each history}}
              <tr>
                <td>{{User}}</td>
                <td>{{Message}}</td>
              </tr>
              {{/each}}
            </table>
          </div>
          <input type="text" class="form-control form-control-sm" id="message" c-model="message" c-keyup="enter()" style="margin-top: 10px;">
        </div>
        {{/if}}
      </div>
    </div>
  `
  // }}}
}
