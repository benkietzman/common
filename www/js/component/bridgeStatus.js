// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-11-14
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
    let s = c.store('bridgeStatus',
    {
      c: c,
      expire: () =>
      {
        let current = Date.now();
        if ((current - last) >= 5000)
        {
          s.stat = null;
          c.render('bridgeStatus', c.autoLoads['bridgeStatus']);
        }
      }
    });
    c.attachEvent('bridgePurpose_status', (stat) =>
    {
      last = Date.now();
      if (stat.detail.Activity)
      {
        stat.detail.Activity = stat.detail.Activity.split(',').join(' --> ');
      }
      s.stat = stat.detail;
      c.render('bridgeStatus', c.autoLoads['bridgeStatus']);
    });
    setInterval(() => {s.expire()}, 250);
  },
  // }}}
  // {{{ template
  template: `
    {{#if stat}}
    <div style="background: black; border-radius: 10px; bottom: 4px; color: orange; display: inline-block; opacity: 0.8; left: 4px; padding: 8px 16px 12px 16px; position: fixed;">
      {{stat.bridgeFunction}}
      --&gt;
      {{stat.bridgeSection}}
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
