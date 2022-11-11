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
    let s = common.getStore('footer');
    s.common = common;
    common.footer._script = common.centralScript;
    common.request('footer', common.footer, (response) =>
    {
      let error = {};
      common.dispatchEvent('commonFooterReady', null);
      if (common.response(response, error))
      {
        common.footer = response.Response.out;
      }
      common.render(id, 'footer', this);
    });
  },
  // }}}
  // {{{ template
  template: `
    <br>
    <div class="container">
      <div class="row">
        <div class="col-md-4 text-muted credit">
          <a href="mailto:{{common.footer.email}}?subject={{common.footer.subject}}">Contact Admin</a>
          ---
          {{common.footer.version}}
        </div>
        <div class="col-md-4 hidden-xs text-muted credit" style="font-size:9px;text-align:center;">
          <strong style="font-size:11px;">{{common.footer.title}}</strong>
          <br>
          {{common.footer.message}}
        </div>
        <div class="col-md-4 text-muted credit" style="font-size:10px;text-align:right;">
          &copy; {{common.footer.year}} <a href="{{common.footer.website}}" target="_new">{{common.footer.company}}</a>
          <br>Engineered by <a href="{{common.footer.engineer.link}}" target="{{common.footer.engineer.target}}">{{common.footer.engineer.first_name}} {{common.footer.engineer.last_name}}</a>
        </div>
      </div>
    </div>
    <br>
  `
  // }}}
}
