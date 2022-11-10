// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-10-20
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////

// {{{ commonBridgeStatus
Vue.component('commonBridgeStatus',
{
  // {{{ data
  data: () => {return {common: common};},
  // }}}
  // {{{ template
  template:
  `
    <div v-show="common.bridgeStatus.stat" style="background: black; border-radius: 10px; bottom: 4px; color: orange; display: inline-block; opacity: 0.8; left: 4px; padding: 8px 16px 12px 16px; position: fixed;">
      {{common.bridgeStatus.stat.bridgeFunction}}
      --&gt;
      {{common.bridgeStatus.stat.bridgeSection}}
      <span v-show="common.bridgeStatus.stat.Function">
        --&gt;
        {{common.bridgeStatus.stat.Function}}
      </span>
      <span v-show="common.bridgeStatus.stat.Activity">
        --&gt;
        {{common.bridgeStatus.stat.Activity}}
      </span>
    </div>
  `
  // }}}
});
// }}}
// {{{ commonCentralMenu
Vue.component('commonCentralMenu',
{
  // {{{ data
  data: () => {return {common: common};},
  // }}}
  // {{{ methods
  methods:
  {
    // {{{ go()
    go()
    {
      document.location.href = common.centralMenu.application.website;
    },
    // }}}
    // {{{ slideMenu()
    slideMenu()
    {
      common.centralMenu.show = !common.centralMenu.show;
    }
    // }}}
  },
  // }}}
  // {{{ template
  template:
  `
    <div style="position: relative; z-index: 1000;">
      <div id="central-slide-panel" class="bg-success" style="position: fixed; top: 120px; right: 0px;">
        <button id="central-slide-opener" class="btn btn-sm btn-success float-start" v-on:click="slideMenu()" style="font-size: 18px; font-weight: bold; margin: 0px 0px 0px -33px; border-radius: 10px 0px 00px 10px;">&#8803;</button>
        <div v-show="common.centralMenu.show" id="central-slide-content" style="padding: 10px;">
          <select class="form-control form-control-sm" v-on:change="go()" v-model="common.centralMenu.application">
            <option v-for="app in common.centralMenu.applications" :value="app">{{app.name}}</option>
          </select>
        </div>
      </div>
    </div>
  `
  // }}}
});
// }}}
// {{{ commonFooter
Vue.component('commonFooter',
{
  // {{{ data
  data: () => {return {common: common};},
  // }}}
  // {{{ template
  template:
  `
    <div id="footer">
      <br>
      <div class="container">
        <div class="row">
          <div class="col-md-4 text-muted credit">
            <a v-show="common.footer.email && common.footer.subject" :href="'mailto:' + common.footer.email + '?subject=' + common.footer.subject">Contact Admin</a>
            <span v-show="common.footer.email && common.footer.subject && common.footer.version"> --- </span>
            <span v-show="common.footer.version">{{common.footer.version}}</span>
          </div>
          <div class="col-md-4 hidden-xs text-muted credit" style="font-size:9px;text-align:center;">
            <strong v-show="common.footer.title" style="font-size:11px;">{{common.footer.title}}</strong>
            <span v-show="common.footer.message && common.footer.title"><br></span>
            <span v-show="common.footer.message">{{common.footer.message}}</span>
          </div>
          <div class="col-md-4 text-muted credit" style="font-size:10px;text-align:right;">
            <span v-show="common.footer.year">&copy; {{common.footer.year}}</span> <a v-show="common.footer.company && common.footer.website" :href="common.footer.website" target="_new">{{common.footer.company}}</a>
            <span v-show="common.footer.engineer"><br>Engineered by <a v-show="common.footer.engineer.link" :href="common.footer.engineer.link" :target="common.footer.engineer.target">{{common.footer.engineer.first_name}} {{common.footer.engineer.last_name}}</a><span v-show="!common.footer.engineer.link">{{common.footer.engineer.first_name}} {{common.footer.engineer.last_name}}</span></span>
          </div>
        </div>
      </div>
      <br>
    </div>
  `
  // }}}
});
// }}}
// {{{ commonMenu
Vue.component('commonMenu',
{
  // {{{ data
  data: () => {return {common: common};},
  // }}}
  // {{{ template
  template:
  `
    <div>
      <div class="fixed-top">
        <nav v-if="common.menu" class="navbar navbar-expand-lg navbar-dark bg-secondary bg-gradient">
          <div class="container-fluid">
            <a v-if="common.application" class="navbar-brand" href="#/">{{common.application}}</a>
            <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#navigationbar" aria-controls="navigationbar" aria-expanded="false" aria-label="Toggle navigation">
              <span class="navbar-toggler-icon"></span>
            </button>
            <div class="collapse navbar-collapse" id="navigationbar">
              <ul class="navbar-nav me-auto mb-2 mb-lg-0">
                <li class="nav-item" v-for="item in common.menu.left"><router-link class="nav-link" :class="item.active" :to="item.href">{{item.value}}</router-link></li>
              </ul>
              <ul class="navbar-nav navbar-right">
                <li class="nav-item" v-for="item in common.menu.right"><router-link class="nav-link" :class="item.active" :to="item.href">{{item.value}}</router-link></li>
              </ul>
            </div>
          </div>
        </nav>
        <nav v-if="common.submenu" class="container navbar navbar-expand-lg navbar-dark bg-dark bg-gradient">
          <div class="container-fluid">
            <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#subnavigationbar" aria-controls="subnavigationbar" aria-expanded="false" aria-label="Toggle navigation">
              <span class="navbar-toggler-icon"></span>
            </button>
            <div class="collapse navbar-collapse" id="subnavigationbar">
              <ul class="navbar-nav me-auto mb-2 mb-lg-0">
                <li class="nav-item" v-for="item in common.submenu.left"><router-link class="nav-link" :class="item.active" :to="item.href">{{item.value}}</router-link></li>
              </ul>
              <ul class="navbar-nav navbar-right">
                <li class="nav-item" v-for="item in common.submenu.right"><router-link class="nav-link" :class="item.active" :to="item.href">{{item.value}}</router-link></li>
              </ul>
            </div>
          </div>
        </nav>
      </div>
      <div style="padding: 30px;">
      </div>
      <div v-if="common.submenu" style="padding: 30px;">
      </div>
    </div>
  `
  // }}}
});
// }}}
// {{{ commonMessages
Vue.component('commonMessages',
{
  // {{{ data
  data: () => {return {common: common};},
  // }}}
  // {{{ methods
  methods:
  {
    // {{{ close()
    close()
    {
    },
    // }}}
    getMessages()
    {
      return common.m_messages;
    }
  },
  // }}}
  // {{{ props
  props: ['messages'],
  // }}}
  // {{{ template
  template:
  `
    <div>
      <div v-for="message in getMessages()" :class="'alert alert-' + message.Class + ' alert-dismissible fade in'">
        <button class="close" data-dismiss="alert" aria-label="close" v-on:click="close(message.Index)">&times;</button>
        <strong v-show="message.Title">{{message.Title}}<br></strong>
        {{message.Body}}
        <br>
        <div class="float-end"><i>{{message.Time}}</i></div>
      </div>
    </div>
  `
  // }}}
});
// }}}
