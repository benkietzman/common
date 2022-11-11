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
    let s = common.store('centralMenu',
    {
      bindings:
      {
        application: new Observable
      },
      common: common,
      slideMenu: () =>
      {
        common.centralMenu.show = !common.centralMenu.show;
        common.render();
      }
    });
    // }}}
    // {{{ go()
    s.go = () =>
    {
      document.location.href = common.centralMenu.applications[s.bindings.application.value].website;
    };
    // }}}
    // {{{ main
    common.request('applications', {_script: common.centralScript}, (data) =>
    {
      let error = {};
      if (common.response(data, error))
      {
        let unIndex = 0;
        common.centralMenu.applications = [];
        for (let i = 0; i < data.Response.out.length; i++)
        {
          if (((data.Response.out[i].menu_id == 1 && common.isValid()) || data.Response.out[i].menu_id == 2) && (data.Response.out[i].retirement_date == null || data.Response.out[i].retirement_date == '0000-00-00 00:00:00'))
          {
            data.Response.out[i].i = unIndex;
            common.centralMenu.applications.push(data.Response.out[i]);
            if (data.Response.out[i].name == common.application)
            {
              s.bindings.application.value = unIndex;
            }
            unIndex++;
          }
        }
      }
      common.render(id, 'centralMenu', this);
    });
    // }}}
  },
  // }}}
  // {{{ template
  template: `
    <div style="position: relative; z-index: 1000;">
      <div id="central-slide-panel" class="bg-success" style="position: fixed; top: 120px; right: 0px;">
        <button id="central-slide-opener" class="btn btn-sm btn-success float-start" c-click="slideMenu()" style="font-size: 18px; font-weight: bold; margin: 0px 0px 0px -33px; border-radius: 10px 0px 00px 10px;">&#8803;</button>
        {{#common.centralMenu.show}}
        <div id="central-slide-content" style="padding: 10px;">
          <select class="form-control form-control-sm" c-change="go()" c-model="application">
            {{#common.centralMenu.applications}}
            <option value="{{i}}">{{name}}</option>
            {{/common.centralMenu.applications}}
          </select>
        </div>
        {{/common.centralMenu.show}}
      </div>
    </div>
  `
  // }}}
}
