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
    let s = c.store('Logout',
    {
      c: c
    });
    // }}}
    // {{{ main
    c.setMenu('Logout as ' + c.getUserFirstName(), null);
    c.processLogout();
    // }}}
  },
  // }}}
  // {{{ template
  template: `
    <br><br><br>
    {{#if c.logout.message}}
    <div style="color:red;font-weight:bold;"><br><br>{{c.logout.message}}<br><br></div>
    {{/if}}
    {{#if c.logout.info}}
    <div style="color:orange;"><br><br>{{c.logout.info}}<br><br></div>
    {{/if}}
  `
  // }}}
}
