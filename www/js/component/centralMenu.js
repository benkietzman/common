// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-11-09
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
    c.attachEvent('commonWsReady_radial', (data) =>
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
              s.applications[response.Response[i].name] = response.Response[i];
              if (response.Response[i].name == c.application)
              {
                s.application.v = response.Response[i];
              }
            }
          }
        }
        c.render(id, this);
      });
    });
    // }}}
  },
  // }}}
  // {{{ template
  template: `
    <div style="position: relative; z-index: 1000;">
      <div id="central-slide-panel" class="bg-success" style="position: fixed; top: 120px; right: 0px;">
        <button id="central-slide-opener" class="btn btn-sm btn-success float-start" c-click="slideMenu()" style="width: 33px; height: 33px; font-size: 18px; font-weight: bold; margin: 0px 0px 0px -33px; border-radius: 10px 0px 0px 10px; vertical-align: top;">&#8803;</button>
        {{#if c.centralMenu.show}}
        <div id="central-slide-content" style="padding: 10px;">
          <select class="form-control form-control-sm" c-change="go()" c-model="application" c-json>
            {{#each applications}}
            <option value="{{json .}}">{{@key}}</option>
            {{/each}}
          </select>
        </div>
        {{/if}}
      </div>
    </div>
  `
  // }}}
}
