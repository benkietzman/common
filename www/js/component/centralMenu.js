///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-11-09
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
export default
{
  // {{{ controller()
  controller(id)
  {
    // {{{ prep work
    let c = common;
    let s = c.store('centralMenu',
    {
      application: new Observable,
      c: c,
      go: () =>
      {
        document.location.href = s.application.v.website;
      },
      slideMenu: () =>
      {
        c.centralMenu.show = !c.centralMenu.show;
        c.render(id, this);
      }
    });
    // }}}
    // {{{ main
    s.applications = () =>
    {
      let request = {Interface: 'central', 'Function': 'applications', Request: {menu: 1, retired: 0}};
      c.wsRequest(c.m_strAuthProtocol, request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.applications = [];
          for (let i = 0; i < response.Response.length; i++)
          {
            if ((response.Response[i].menu_id == 1 && c.isValid()) || response.Response[i].menu_id == 2)
            {
              s.applications.push(response.Response[i]);
              if (response.Response[i].name == c.application)
              {
                s.application.v = response.Response[i];
              }
            }
          }
          c.render(id, this);
        }
      });
    };
    c.attachEvent('commonWsReady_radial', (data) =>
    {
      s.applications();
    });
    if (c.m_ws['radial'].Connected)
    {
      s.applications();
    }
    // }}}
  },
  // }}}
  // {{{ template
  template: `
    <div style="position: relative; z-index: 999;">
      <div id="central-slide-panel" class="bg-success" style="position: fixed; top: 160px; right: 0px; border-style: solid; border-width: 1px; border-color: black; border-radius: 0px 0px 0px 10px;">
        <button id="central-slide-opener" class="btn btn-sm btn-success float-start" c-click="slideMenu()" style="width: 33px; height: 33px; font-size: 18px; font-weight: bold; margin: 0px 0px 0px -33px; border-style: solid; border-width: 1px; border-color: black; border-radius: 10px 0px 0px 10px; vertical-align: top;" title="applications"><i class="bi bi-app"></i></button>
        {{#if c.centralMenu.show}}
        <div id="central-slide-content" style="padding: 10px;">
          <select class="form-select form-select-sm" c-change="go()" c-model="application" c-json>
            {{#each applications}}
            <option value="{{json .}}">{{name}}</option>
            {{/each}}
          </select>
        </div>
        {{/if}}
      </div>
    </div>
  `
  // }}}
}
