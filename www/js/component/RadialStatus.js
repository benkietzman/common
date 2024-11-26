// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2024-11-26
// copyright  : AT&T
// email      : bk6471@att.com
///////////////////////////////////////////
export default
{
  // [[[ controller()
  controller(id, nav)
  {
    // [[[ prep work
    let a = app;
    let c = common;
    let s = c.scope('Status',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Status');
      },
      // ]]]
      a: a,
      c: c,
      bDeveloper: false,
      bDisabled: false,
      nodes: {}
    });
    // ]]]
    // [[[ action()
    s.action = (strAction, strNode) =>
    {
      if (s.bDeveloper)
      {
        s.bDisabled = true;
        s.u();
        let request = {Interface: a.m_strInterface, 'Function': 'action', Request: {Action: strAction, Node: strNode}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          s.bDisabled = false;
          if (c.wsResponse(response, error))
          {
            s.init();
          }
          else
          {
            s.message.v = error.message;
            s.u();
          }
        });
      }
      else
      {
        s.message.v = 'You are not authorized to perform this action.';
      }
    };
    // ]]]
    // [[[ init()
    s.init = () =>
    {
      s.bDeveloper = c.isGlobalAdmin();
      if (s.bDeveloper)
      {
        s.u();
      }
      else if (c.isValid())
      {
        let request = {Interface: 'central', 'Function': 'application', Request: {name: c.application}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            let request = {Interface: 'central', 'Function': 'isApplicationDeveloper', Request: {id: response.Response.id}};
            c.wsRequest('radial', request).then((response) =>
            {
              let error = {};
              if (c.wsResponse(response, error))
              {
                s.bDeveloper = true;
                s.u();
              }
            });
          }
          else
          {
            s.message.v = error.message;
          }
        });
      }
      s.info.v = 'Retrieving nodes...';
      let request = {Interface: 'link', 'Function': 'status'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
        if (c.wsResponse(response, error))
        {
          let strNode = response.Response.Node;
          let nodes = [strNode];
          for (let i = 0; i < response.Response.Links.length; i++)
          {
            nodes.push(response.Response.Links[i]);
          }
          nodes.sort();
          s.nodes = null;
          s.nodes = {};
          for (let i = 0; i < nodes.length; i++)
          {
            s.nodes[nodes[i]] = {};
            let request = {Interface: a.m_strInterface, 'Function': 'status', Request: {Node: nodes[i]}};
            if (nodes[i] != strNode)
            {
              request.Node = nodes[i];
            }
            c.wsRequest('radial', request).then((response) =>
            {
              let error = {};
              if (c.wsResponse(response, error))
              {
                s.nodes[response.Request.Node] = response.Response;
                s.u();
              }
              else
              {
                s.message.v = error.message;
              }
            });
          }
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ main
    c.setMenu('Status');
    s.u();
    if (a.ready())
    {
      s.init();
    }
    else
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
      s.init();
    });
    c.attachEvent('commonWsMessage_' + a.m_strApplication, (data) =>
    { 
      if (!s.bDisabled && data.detail && data.detail.Action && data.detail.Action == 'status' && data.detail.Source)
      {
        s.nodes[data.detail.Source] = data.detail;
        s.u();
      } 
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <div c-model="info" class="text-warning"></div>
  <div c-model="message" class="text-danger" style="font-weight:bold;"></div>
  <div class="row">
  {{#each nodes}}
  <div class="col-auto">
  <div class="card border border-primary-subtle" style="margin-top: 10px;">
    <div class="card-header {{#ifCond Master.Node "==" @key}}bg-primary fw-bold{{else}}bg-primary-subtle{{/ifCond}}">
      {{#if @root.bDeveloper}}
      {{#if PID}}
      <button class="btn btn-sm btn-danger bi bi-x-circle float-end" c-click="action('stop', '{{@key}}')" style="margin-left: 10px;" title="stop"{{#if @root.bDisabled}} disabled{{/if}}></button>
      {{/if}}
      {{#if PID}}
      <button class="btn btn-sm btn-warning bi bi-arrow-clockwise float-end" c-click="action('restart', '{{@key}}')" style="margin-left: 10px;" title="restart"{{#if @root.bDisabled}} disabled{{/if}}></button>
      {{/if}}
      {{^if PID}}
      <button class="btn btn-sm btn-success bi bi-power float-end" c-click="action('start', '{{@key}}')" title="start"{{#if @root.bDisabled}} disabled{{/if}}></button>
      {{/if}}
      {{/if}}
      {{@key}}
    </div>
    <div class="card-body bg-primary-subtle">
      <b>Status</b>
      <table class="table table-sm table-condensed table-striped">
        <tr><td style="background: inherit;">Process ID</td><td style="background: inherit; text-align: right;">{{PID}}</td></tr>
        <tr><td style="background: inherit;">Resident</td><td style="background: inherit; text-align: right;">{{#if Memory.Resident}}{{byteShort (multiply Memory.Resident 1024) 2}}{{/if}}</td></tr>
        <tr><td style="background: inherit;">Threads</td><td style="background: inherit; text-align: right;">{{#if Threads}}{{numberShort Threads 0}}{{/if}}</td></tr>
      </table>
      {{#if Throughput}}
      <b>Throughput</b>
      <table class="table table-sm table-condensed table-striped">
      {{#each Throughput}}
        <tr><td style="background: inherit;">{{@key}}</td><td style="background: inherit; text-align: right;">{{numberShort . 0}}</td></tr>
      {{/each}}
      </table>
      {{/if}}
    </div>
  </div>
  </div>
  {{/each}}
  </div>
  `
  // ]]]
}
