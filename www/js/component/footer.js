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
    let c = common;
    let s = c.store('footer',
    {
      c: c
    });
    c.footer._script = c.centralScript;
    c.request('footer', c.footer, (response) =>
    {
      let error = {};
      c.dispatchEvent('commonFooterReady', null);
      if (c.response(response, error))
      {
        c.footer = response.Response.out;
      }
      c.render(id, this);
    });
  },
  // }}}
  // {{{ template
  template: `
    <br>
    <div class="container">
      <div class="row">
        <div class="col-md-4 text-muted credit">
          <a href="mailto:{{c.footer.email}}?subject={{c.footer.subject}}">Contact Admin</a>
          ---
          {{c.footer.version}}
        </div>
        <div class="col-md-4 hidden-xs text-muted credit" style="font-size:9px;text-align:center;">
          <strong style="font-size:11px;">{{c.footer.title}}</strong>
          <br>
          {{c.footer.message}}
        </div>
        <div class="col-md-4 text-muted credit" style="font-size:10px;text-align:right;">
          &copy; {{c.footer.year}} <a href="{{c.footer.website}}" target="_new">{{c.footer.company}}</a>
          <br>Engineered by <a href="{{c.footer.engineer.link}}" target="{{c.footer.engineer.target}}">{{c.footer.engineer.first_name}} {{c.footer.engineer.last_name}}</a>
        </div>
      </div>
    </div>
    <br>
  `
  // }}}
}
