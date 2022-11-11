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
          {{#c.footer.email}}{{#c.footer.subject}}<a href="mailto:{{c.footer.email}}?subject={{c.footer.subject}}">Contact Admin</a>{{/c.footer.subject}}{{/c.footer.email}}
          {{#c.footer.email}}{{#c.footer.subject}}{{#c.footer.version}}---{{/c.footer.version}}{{/c.footer.subject}}{{/c.footer.email}}
          {{c.footer.version}}
        </div>
        <div class="col-md-4 hidden-xs text-muted credit" style="font-size:9px;text-align:center;">
          {{#c.footer.title}}<strong style="font-size:11px;">{{c.footer.title}}</strong><br>{{/c.footer.title}}
          {{c.footer.message}}
        </div>
        <div class="col-md-4 text-muted credit" style="font-size:10px;text-align:right;">
          {{#c.footer.year}}&copy; {{c.footer.year}}{{/c.footer.year}}
          {{#c.footer.website}}{{#c.footer.company}}<a href="{{c.footer.website}}" target="_new">{{c.footer.company}}</a>{{/c.footer.company}}{{/c.footer.website}}
          {{#c.footer.engineer.link}}{{#c.footer.engineer.target}}{{#c.footer.engineer.first_name}}{{#c.footer.engineer.last_name}}<br>Engineered by <a href="{{c.footer.engineer.link}}" target="{{c.footer.engineer.target}}">{{c.footer.engineer.first_name}} {{c.footer.engineer.last_name}}</a>{{/c.footer.engineer.last_name}}{{/c.footer.engineer.first_name}}{{/c.footer.engineer.target}}{{/c.footer.engineer.link}}
        </div>
      </div>
    </div>
    <br>
  `
  // }}}
}
