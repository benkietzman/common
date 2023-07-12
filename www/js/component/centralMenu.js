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
        document.location.href = c.centralMenu.applications[s.application.value].website;
      },
      slideMenu: () =>
      {
        c.centralMenu.show = !c.centralMenu.show;
        c.render(id, this);
      }
    });
    // }}}
    // {{{ main
    if (!c.isDefined(c.centralMenu.applications))
    {
      c.attachEvent('commonWsReady', (data) =>
      {
        let request = {Interface: 'central', 'Function': 'applications', Request: {}};
        c.wsRequest(c.m_strAuthProtocol, request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            let unIndex = 0;
            c.centralMenu.applications = [];
            for (let i = 0; i < response.Response.length; i++)
            {
              if (((response.Response[i].menu_id == 1 && c.isValid()) || response.Response[i].menu_id == 2) && (response.Response[i].retirement_date == null || response.Response[i].retirement_date == '0000-00-00 00:00:00'))
              {
                c.centralMenu.applications.push(response.Response[i]);
                if (response.Response[i].name == c.application)
                {
                  s.application.value = unIndex;
                }
                unIndex++;
              }
            }
          }
          c.render(id, this);
        });
      });
    }
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
          <select class="form-control form-control-sm" c-change="go()" c-model="application">
            {{#each c.centralMenu.applications}}
            <option value="{{@key}}">{{name}}</option>
            {{/each}}
          </select>
        </div>
        {{/if}}
      </div>
    </div>
  `
  // }}}
}
