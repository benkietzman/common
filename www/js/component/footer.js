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
    s.footer = () =>
    {
      let request = {Interface: ((c.isDefined(c.footer['interface']))?c.footer['interface']:'central'), 'Function': 'footer', Request: c.footer};
      c.wsRequest(c.m_strAuthProtocol, request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          c.footer = response.Response;
        }
        c.dispatchEvent('commonFooterReady', null);
        c.render(id, this);
      });
    };
    c.attachEvent('commonWsReady_radial', (data) =>
    {
      s.footer();
    });
    if (c.m_ws['radial'].Connected)
    {
      s.footer();
    }
  },
  // }}}
  // {{{ template
  template: `
    <br>
    <div class="container">
      <div class="row">
        <div class="col-md-4 text-muted credit" style="font-size:10px;">
          {{#if c.footer.year}}&copy; {{c.footer.year}}{{/if}}
          {{#if c.footer.website}}{{#if c.footer.company}}<a href="{{c.footer.website}}" target="_new">{{c.footer.company}}</a>{{/if}}{{/if}}
          {{#if c.footer.version}} (v{{c.footer.version}}){{/if}}
          {{#if c.footer.email}}<br><a href="mailto:{{c.footer.email}}?subject={{c.footer.subject}}">Contact Admin</a>{{/if}}
        </div>
        <div class="col-md-4 hidden-xs text-muted credit" style="font-size:9px;text-align:center;">
          {{#if c.footer.title}}<strong style="font-size:11px;">{{c.footer.title}}</strong><br>{{/if}}
          {{c.footer.message}}
        </div>
        <div class="col-md-4 text-muted credit" style="font-size:10px;text-align:right;">
          {{#if c.footer.engineer.first_name}}{{#if c.footer.engineer.last_name}}Engineered by
          {{#if c.footer.engineer.link}}
            <a href="{{c.footer.engineer.link}}" target="{{c.footer.engineer.target}}">{{c.footer.engineer.first_name}} {{c.footer.engineer.last_name}}</a>
          {{else}}
            {{c.footer.engineer.first_name}} {{c.footer.engineer.last_name}}
          {{/if}}
          {{/if}}{{/if}}
          {{#if c.footer.power.application}}<br>Powered by
          {{#if c.footer.power.link}}
            <a href="{{c.footer.power.link}}" target="{{c.footer.power.target}}">{{c.footer.power.application}}</a>
          {{else}}
            {{c.footer.power.application}}
          {{/if}}
          {{/if}}
        </div>
      </div>
    </div>
    <br>
  `
  // }}}
}
