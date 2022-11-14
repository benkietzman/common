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
      c: c
    });
  },
  // }}}
  // {{{ template
  template: `
    {{#each c.m_messages}}
    <div class="alert alert-{{Class}} alert-dismissible fade in">
      <button class="close" data-dismiss="alert" aria-label="close">&times;</button>
      {{#Title}}
      <strong>{{Title}}<br></strong>
      {{/Title}}
      {{Body}}
      <br>
      <div class="float-end"><i>{{Time}}</i></div>
    </div>
    {{/each}}
  `
  // }}}
}
