// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-11-09
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
export default
{
  // {{{ contoller()
  controller()
  {
    let c = common;
    let s = c.store('menu',
    {
      c: c
    });
  },
  // }}}
  // {{{ template
  template: `
    <div class="fixed-top">
      <nav class="navbar navbar-expand-lg navbar-dark bg-secondary bg-gradient">
        <div class="container-fluid">
          <a class="navbar-brand" href="#/">{{c.application}}</a>
          <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#navigationbar" aria-controls="navigationbar" aria-expanded="false" aria-label="Toggle navigation">
            <span class="navbar-toggler-icon"></span>
          </button>
          <div class="collapse navbar-collapse" id="navigationbar">
            <ul class="navbar-nav me-auto mb-2 mb-lg-0">
              {{#c.menu.left}}
              <li class="nav-item"><a class="nav-link {{active}}" href="{{href}}">{{value}}</a></li>
              {{/c.menu.left}}
            </ul>
            <ul class="navbar-nav navbar-right">
              {{#c.menu.right}}
              <li class="nav-item"><a class="nav-link {{active}}" href="{{href}}">{{value}}</a></li>
              {{/c.menu.right}}
            </ul>
          </div>
        </div>
      </nav>
      <nav class="container navbar navbar-expand-lg navbar-dark bg-dark bg-gradient">
        <div class="container-fluid">
          <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#subnavigationbar" aria-controls="subnavigationbar" aria-expanded="false" aria-label="Toggle navigation">
            <span class="navbar-toggler-icon"></span>
          </button>
          <div class="collapse navbar-collapse" id="subnavigationbar">
            <ul class="navbar-nav me-auto mb-2 mb-lg-0">
              {{#c.submenu.left}}
              <li class="nav-item"><a class="nav-link {{active}}" href="{{href}}">{{value}}</a></li>
              {{/c.submenu.left}}
            </ul>
            <ul class="navbar-nav navbar-right">
              {{#c.submenu.right}}
              <li class="nav-item"><a class="nav-link {{active}}" href="{{href}}">{{value}}</a></li>
              {{/c.submenu.right}}
            </ul>
          </div>
        </div>
      </nav>
    </div>
    <div style="padding: 30px;">
    </div>
    <div v-if="c.submenu" style="padding: 30px;">
    </div>
  `
  // }}}
}
