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
  controller()
  {
    let c = common;
    let s = c.store('messages',
    {
      c: c,
      close: (nIndex) =>
      {
        let messages = [];
        for (let i = 0; i < c.m_messages.length; i++)
        {
          if (i != nIndex)
          {
            messages.push(c.m_messages[i]);
            messages[messages.length - 1].Index = (messages.length - 1);
          }
        }
        c.m_messages = null;
        c.m_messages = messages;
        c.dispatchEvent('commonMessagesReady', null);
        c.render('messages', c.autoLoads['messages']);
      }
    });
  },
  // }}}
  // {{{ template
  template: `
    {{#each c.m_messages}}
    <div class="alert alert-{{Class}} alert-dismissible fade show" role="alert">
      <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="close" c-click="close({{Index}})"></button>
      {{#if Title}}
      <h4 class="alert-heading">{{{Title}}}</h4>
      {{/if}}
      {{#if Body}}
      <pre style="background: inherit; color: inherit; white-space: pre-wrap;">{{{Body}}}</pre>
      {{/if}}
      <hr>
      <div class="float-end"><i>{{Time}}</i></div>
      <br>
    </div>
    {{/each}}
  `
  // }}}
}
