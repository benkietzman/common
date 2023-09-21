// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2021-10-11
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
  // {{{ mounted()
  mounted()
  {
    common.request('applications', {_script: common.centralScript}, (data) =>
    {
      let error = {};
      if (common.response(data, error))
      {
        var bFound = false;
        common.centralMenu.applications = [];
        for (var i = 0; i < data.Response.out.length; i++)
        {
          if (((data.Response.out[i].menu_id == 1 && common.isValid()) || data.Response.out[i].menu_id == 2) && (data.Response.out[i].retirement_date == null || data.Response.out[i].retirement_date == '0000-00-00 00:00:00'))
          {
            common.centralMenu.applications.push(data.Response.out[i]);
          }
        }
        for (var i = 0; !bFound && i < common.centralMenu.applications.length; i++)
        {
          if (common.centralMenu.applications[i].name == common.application)
          {
            bFound = true;
            common.centralMenu.application = common.centralMenu.applications[i];
          }
        }
      }
    });
  },
  // }}}
  // {{{ template
  template:
  `
    <div style="position: relative; z-index: 1000;">
      <div id="central-slide-panel" class="bg-success" style="position: fixed; top: 120px; right: 0px; border-style: solid; border-width: 1px; border-color: #777777;">
        <button id="central-slide-opener" class="btn btn-success glyphicon glyphicon-align-justify" v-on:click="slideMenu()" style="float: left; margin: 0px 0px 0px -40px; border-radius: 10px 0px 00px 10px;"></button>
        <div v-show="common.centralMenu.show" id="central-slide-content" style="padding: 10px;">
          <select class="form-control input-sm" v-on:change="go()" v-model="common.centralMenu.application">
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
  // {{{ mounted()
  mounted()
  {
    common.request('footer', common.footer, (response) =>
    {
      let error = {};
      common.dispatchEvent('commonFooterReady', null);
      if (common.response(response, error))
      {
        common.footer = response.Response.out;
      } 
    }); 
  },
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
    <div style="margin-bottom: 50px;">
      <div v-if="common.menu" class="navbar navbar-default navbar-inverse navbar-fixed-top" role="navigation">
        <div class="container-fluid" id="navfluid">
          <div class="navbar-header">
            <button type="button" class="navbar-toggle" data-toggle="collapse" data-target="#navigationbar">
              <span class="sr-only">Toggle navigation</span>
              <span class="icon-bar"></span>
              <span class="icon-bar"></span>
              <span class="icon-bar"></span>
            </button>
            <a v-if="common.application" class="navbar-brand" href="#/">{{common.application}}</a>
          </div>
          <div class="collapse navbar-collapse" id="navigationbar">
            <ul class="nav navbar-nav">
              <li v-for="item in common.menu.left" :class="item.active"><router-link :to="item.href"><span v-show="item.icon" :class="'glyphicon glyphicon-'+item.icon" style="margin-right:8px;"></span>{{item.value}}</router-link></li>
            </ul>
            <ul class="nav navbar-nav navbar-right">
              <li v-for="item in common.menu.right" :class="item.active"><router-link :to="item.href"><span v-show="item.icon" :class="'glyphicon glyphicon-'+item.icon" style="margin-right:8px;"></span>{{item.value}}</router-link></li>
            </ul>
          </div>
        </div>
      </div>
      <div v-if="common.submenu" class="navbar navbar-default" role="navigation" style="margin-top: 50px; margin-bottom: -50px;">
        <div class="container-fluid" id="navfluid">
          <div class="navbar-header">
            <button type="button" class="navbar-toggle" data-toggle="collapse" data-target="#subnavigationbar">
              <span class="sr-only">Toggle navigation</span>
              <span class="icon-bar"></span>
              <span class="icon-bar"></span>
              <span class="icon-bar"></span>
            </button>
          </div>
          <div class="collapse navbar-collapse" id="subnavigationbar">
            <ul class="nav navbar-nav">
              <li v-for="item in common.submenu.left" :class="item.active"><router-link :to="item.href"><span v-show="item.icon" :class="'glyphicon glyphicon-'+item.icon" style="margin-right:8px;"></span>{{item.value}}</router-link></li>
            </ul>
            <ul class="nav navbar-nav navbar-right">
              <li v-for="item in common.submenu.right" :class="item.active"><router-link :to="item.href"><span v-show="item.icon" :class="'glyphicon glyphicon-'+item.icon" style="margin-right:8px;"></span>{{item.value}}</router-link></li>
            </ul>
          </div>
        </div>
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
    // {{{ getMessages()
    getMessages()
    {
      return common.m_messages;
    }
    // }}}
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
        <pre style="background: inherit; color: inherit; white-space: pre-wrap;">{{message.Body}}</pre>
        <br>
        <div class="pull-right"><i>{{message.Time}}</i></div>
      </div>
    </div>
  `
  // }}}
});
// }}}
