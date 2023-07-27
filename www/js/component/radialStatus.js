// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-01-20
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
export default
{
  // {{{ controller()
  controller()
  {
    let c = common;
    let last = 0;
    let s = c.store('radialStatus',
    {
      c: c,
      expire: () =>
      {
        let current = Date.now();
        if ((current - last) >= 5000)
        {
          s.stat = null;
          c.render('radialStatus', c.autoLoads['radialStatus']);
        }
      }
    });
    c.attachEvent('radialPurpose_status', (stat) =>
    {
      last = Date.now();
      if (stat.detail.Activity)
      {
        stat.detail.Activity = stat.detail.Activity.split(',').join(' --> ');
      }
      s.stat = stat.detail;
      c.render('radialStatus', c.autoLoads['radialStatus']);
    });
    setInterval(() => {s.expire()}, 250);
  },
  // }}}
  // {{{ template
  template: `
    {{#if stat}}
    <div style="background: black; border-radius: 10px; bottom: 4px; color: orange; display: inline-block; opacity: 0.8; left: 4px; padding: 8px 16px 12px 16px; position: fixed;">
      {{stat.radialInterface}}
      --&gt;
      {{stat.radialFunction}}
      {{#if stat.Function}}
        --&gt;
        {{stat.Function}}
      {{/if}}
      {{#if stat.Activity}}
        --&gt;
        {{stat.Activity}}
      {{/if}}
    </div>
    {{/if}}
  `
  // }}}
}
