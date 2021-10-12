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
  data: () => {return {};},
  // }}}
  // {{{ props
  props: ['stat'],
  // }}}
  // {{{ template
  template:
  `
    <div v-show="stat" style="background: black; border-radius: 10px; bottom: 4px; color: orange; display: inline-block; opacity: 0.8; left: 4px; padding: 8px 16px 12px 16px; position: fixed;">
      {{stat.bridgeFunction}}
      --&gt;
      {{stat.bridgeSection}}
      <span v-show="stat.Function">
        --&gt;
        {{stat.Function}}
      </span>
      <span v-show="stat.Activity">
        --&gt;
        {{stat.Activity}}
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
  data: () =>
  {
    return {
      application: false,
      applications: false,
      common: false,
      script: false,
      show: false,
      strApplication: false
    };
  },
  // }}}
  // {{{ methods
  methods:
  {
    // {{{ go()
    go()
    {
      document.location.href = this.application.website;
    },
    // }}}
    // {{{ init()
    init(strApplication, common, strScript)
    {
      this.common = common;
      this.strApplication = strApplication;
      this.script = strScript;
      fetch(strScript,
      {
        method: 'POST',
        headers:
        {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({'Function': 'applications'})
      })
      .then(response => response.json())
      .then(data => 
      {
        if (data.Status == 'okay')
        {
          var bFound = false;
          this.applications = [];
          for (var i = 0; i < data.Response.out.length; i++)
          {
            if (((data.Response.out[i].menu_id == 1 && common.isValid('')) || data.Response.out[i].menu_id == 2) && (data.Response.out[i].retirement_date == null || data.Response.out[i].retirement_date == '0000-00-00 00:00:00'))
            {
              this.applications.push(data.Response.out[i]);
            }
          }
          for (var i = 0; !bFound && i < this.applications.length; i++)
          {
            if (this.applications[i].name == this.strApplication)
            {
              bFound = true;
              this.application = this.applications[i];
            }
          }
        }
      });
    },
    // }}}
    // {{{ slideMenu()
    slideMenu()
    {
      this.show = !this.show;
    }
    // }}}
  },
  // }}}
  // {{{ template
  template:
  `
    <div style="position: relative; z-index: 1000;">
      <div id="central-slide-panel" class="bg-success" style="position: fixed; top: 120px; right: 0px; border-style: solid; border-width: 1px; border-color: #777777;">
        <button id="central-slide-opener" class="btn btn-success glyphicon glyphicon-align-justify" v-on:click="slideMenu()" style="float: left; margin: 0px 0px 0px -40px; border-radius: 10px 0px 00px 10px;"></button>
        <div v-show="show" id="central-slide-content" style="padding: 10px;">
          <select class="form-control input-sm" v-on:change="go()" v-model="application">
            <option v-for="app in applications" :value="app">{{app.name}}</option>
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
  data: () =>
  {
    return {
      footer:
      {
        company: false,
        email: false,
        engineer:
        {
          first_name: false,
          last_name: false,
          link: false,
          target: false
        },
        message: false,
        subject: false,
        title: false,
        userid: false,
        version: false,
        website: false,
        year: false
      }
    };
  },
  // }}}
  // {{{ methods
  methods:
  {
    // {{{ set()
    set(strScript, footer)
    {
      this.footer = {...this.footer, ...footer};
      fetch(strScript,
      {
        method: 'POST',
        headers:
        {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({'Function': 'footer', Arguments: this.footer})
      })
      .then(response => response.json())
      .then(data => 
      {
        if (data.Status == 'okay')
        {
          this.footer = data.Response.out;
        }
      });
    }
    // }}}
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
            <a v-show="footer.email && footer.subject" :href="'mailto:' + footer.email + '?subject=' + footer.subject">Contact Admin</a>
            <span v-show="footer.email && footer.subject && footer.version"> --- </span>
            <span v-show="footer.version">{{footer.version}}</span>
          </div>
          <div class="col-md-4 hidden-xs text-muted credit" style="font-size:9px;text-align:center;">
            <strong v-show="footer.title" style="font-size:11px;">{{footer.title}}</strong>
            <span v-show="footer.message && footer.title"><br></span>
            <span v-show="footer.message">{{footer.message}}</span>
          </div>
          <div class="col-md-4 text-muted credit" style="font-size:10px;text-align:right;">
            <span v-show="footer.year">&copy; {{footer.year}}</span> <a v-show="footer.company && footer.website" :href="footer.website" target="_new">{{footer.company}}</a>
            <span v-show="footer.engineer"><br>Engineered by <a v-show="footer.engineer.link" :href="footer.engineer.link" :target="footer.engineer.target">{{footer.engineer.first_name}} {{footer.engineer.last_name}}</a><span v-show="!footer.engineer.link">{{footer.engineer.first_name}} {{footer.engineer.last_name}}</span></span>
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
  data: () =>
  {
    return {
      application: false,
      common: false,
      menu: {left: [], right: []},
      submenu: false
    };
  },
  // }}}
  // {{{ methods
  methods:
  {
    // {{{ init()
    init(strApplication, common)
    {
      this.application = strApplication;
      this.common = common;
    },
    // }}}
    // {{{ reset()
    reset(menu)
    {
      this.menu = menu;
      if (this.common.isValid(''))
      {
        this.menu.right[this.menu.right.length] = {value: 'Logout as ' + this.common.getUserFirstName(), href: '/Logout', icon: 'log-out', active: null};
      }
      else
      {
        this.menu.right[this.menu.right.length] = {value: 'Login', href: '/Login', icon: 'user', active: null};
      }
    },
    // }}}
    // {{{ set()
    set(strMenu, strSubMenu)
    {
      let nActiveLeft = -1;
      this.strPrevMenu = this.strMenu;
      this.strPrevSubMenu = this.strSubMenu;
      this.strMenu = strMenu;
      this.strSubMenu = strSubMenu;
      for (let i = 0; i < this.menu.left.length; i++)
      {
        if (this.menu.left[i].value == strMenu)
        {
          nActiveLeft = i;
          this.menu.left[i].active = 'active';
          this.activeMenu = strMenu;
        }
        else
        {
          this.menu.left[i].active = null;
        }
      }
      let nActiveRight = -1;
      for (let i = 0; i < this.menu.right.length; i++)
      {
        if (this.menu.right[i].value == strMenu)
        {
          nActiveRight = i;
          this.menu.right[i].active = 'active';
          this.activeMenu = strMenu;
        }
        else
        {
          this.menu.right[i].active = null;
        }
      }
      if (nActiveLeft >= 0 && this.menu.left[nActiveLeft].submenu)
      {
        this.submenu = this.menu.left[nActiveLeft].submenu;
        for (var i = 0; i < this.submenu.left.length; i++)
        {
          if (this.submenu.left[i].value == strSubMenu)
          {
            this.submenu.left[i].active = 'active';
          }
          else
          {
            this.submenu.left[i].active = null;
          }
        }
        for (let i = 0; i < this.submenu.right.length; i++)
        {
          if (this.submenu.right[i].value == strSubMenu)
          {
            this.submenu.right[i].active = 'active';
          }
          else
          {
            this.submenu.right[i].active = null;
          }
        }
      }
      else if (nActiveRight >= 0 && this.menu.right[nActiveRight].submenu)
      {
        this.submenu = this.menu.right[nActiveRight].submenu;
        for (let i = 0; i < this.submenu.left.length; i++)
        {
          if (this.submenu.left[i].value == strSubMenu)
          {
            this.submenu.left[i].active = 'active';
          }
          else
          {
            this.submenu.left[i].active = null;
          }
        }
        for (let i = 0; i < this.submenu.right.length; i++)
        {
          if (this.submenu.right[i].value == strSubMenu)
          {
            this.submenu.right[i].active = 'active';
          }
          else
          {
            this.submenu.right[i].active = null;
          }
        }
      }
      else
      {
        this.submenu = false;
      }
    }
    // }}}
  },
  // }}}
  // {{{ template
  template:
  `
    <div style="margin-bottom: 50px;">
      <div v-if="menu" class="navbar navbar-default navbar-inverse navbar-fixed-top" role="navigation">
        <div class="container-fluid" id="navfluid">
          <div class="navbar-header">
            <button type="button" class="navbar-toggle" data-toggle="collapse" data-target="#navigationbar">
              <span class="sr-only">Toggle navigation</span>
              <span class="icon-bar"></span>
              <span class="icon-bar"></span>
              <span class="icon-bar"></span>
            </button>
            <a v-if="application" class="navbar-brand" href="#/">{{application}}</a>
          </div>
          <div class="collapse navbar-collapse" id="navigationbar">
            <ul class="nav navbar-nav">
              <li v-for="item in menu.left" :class="item.active"><router-link :to="item.href"><span v-show="item.icon" :class="'glyphicon glyphicon-'+item.icon" style="margin-right:8px;"></span>{{item.value}}</router-link></li>
            </ul>
            <ul class="nav navbar-nav navbar-right">
              <li v-for="item in menu.right" :class="item.active"><router-link :to="item.href"><span v-show="item.icon" :class="'glyphicon glyphicon-'+item.icon" style="margin-right:8px;"></span>{{item.value}}</router-link></li>
            </ul>
          </div>
        </div>
      </div>
      <div v-if="submenu" class="navbar navbar-default" role="navigation" style="margin-top: 50px;">
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
              <li v-for="item in submenu.left" :class="item.active"><router-link :to="item.href"><span v-show="item.icon" :class="'glyphicon glyphicon-'+item.icon" style="margin-right:8px;"></span>{{item.value}}</router-link></li>
            </ul>
            <ul class="nav navbar-nav navbar-right">
              <li v-for="item in submenu.right" :class="item.active"><router-link :to="item.href"><span v-show="item.icon" :class="'glyphicon glyphicon-'+item.icon" style="margin-right:8px;"></span>{{item.value}}</router-link></li>
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
  data: () => {return {};},
  // }}}
  // {{{ methods
  methods:
  {
    // {{{ close()
    close()
    {
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
      <div v-for="message in messages" :class="'alert alert-' + message.Class + ' alert-dismissible fade in'">
        <button class="close" data-dismiss="alert" aria-label="close" v-on:click="close(message.Index)">&times;</button>
        <strong v-show="message.Title">{{message.Title}}<br></strong>
        {{message.Body}}
      </div>
    </div>
  `
  // }}}
});
// }}}
