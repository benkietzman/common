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
      menu: false,
      users: [];
    });
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
                    s.users.push(data.User);
                  }
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
      <div id="radial-slide-panel" class="bg-success" style="position: fixed; top: 140px; right: 0px;">
        <button id="radial-slide-opener" class="btn btn-sm btn-warning float-start" c-click="slide()" style="width: 33px; height: 33px; font-size: 18px; font-weight: bold; margin: 0px 0px 0px -33px; border-radius: 10px 0px 0px 10px; vertical-align: top;">&#8803;</button>
        {{#if menu}}
        <div id="radial-slide-content" style="padding: 10px;">
          <select class="form-control form-control-sm" c-model="user">
            {{#each users}}
            <option value="{{.}}">{{.}}</option>
            {{/each}}
          </select>
        </div>
        {{/if}}
      </div>
    </div>
  `
  // }}}
}
