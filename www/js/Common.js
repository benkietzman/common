// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2021-10-12
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
// {{{ Common
class Common
{
  // {{{ constructor()
  constructor(options)
  {
    this.activeMenu = null;
    this.centralMenu = {show: false};
    this.components = {};
    this.debug = false;
    this.footer = {engineer: false};
    this.autoLoads = {};
    this.login = {login: {password: '', title: '', userid: ''}, info: false, message: false, showForm: false};
    this.logout = {info: false, message: false};
    this.m_auth = {};
    this.m_intervals = {};
    this.m_listeners = {};
    this.m_messages = [];
    this.m_store = {};
    this.m_strAuthProtocol = null;
    this.m_strLoginType = null;
    this.m_nUnique = 0;
    this.m_ws = {};
    this.menu = {left: [], right: []};
    this.radialStatus = {stat: false};
    this.submenu = false;
    this.strMenu = null;
    this.strPrevMenu = null;
    this.strPrevSubMenu = null;
    this.strSubMenu = null;
    if (this.isDefined(options.alias))
    {
      this.alias = options.alias;
    }
    if (this.isDefined(options.application))
    {
      this.application = options.application;
      fetch('/include/common/statistic.php',
      {
        method: 'POST',
        headers:
        {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({'Application': this.application})
      });
    }
    if (this.isDefined(options.id))
    {
      this.id = options.id;
    }
    if (!this.isDefined(this.id))
    {
      this.id = 'app';
    }
    if (this.isDefined(options.junction))
    {
      this.junction = options.junction;
      if (this.isDefined(options.email))
      {
        this.email = options.email;
        if (this.isDefined(options.debug) && options.debug)
        {
          this.debug = options.debug;
        }
      }
    }
    if (this.isDefined(options.loads))
    {
      this.loads(options.loads);
    }
    if (this.isDefined(options.routes))
    {
      if (this.isDefined(options.router))
      {
        this.router = options.router;
      }
      else
      {
        this.router = new Navigo('/', {hash: true, strategy: 'ONE'});
      }
      this.routes(options.routes);
    }
    if (this.isDefined(options.script))
    {
      this.script = options.script;
    }
    else
    {
      this.script = 'include/request.php';
    }
    if (this.isDefined(options.footer))
    {
      this.footer = {...this.footer, ...options.footer};
    }
    if ('Notification' in window)
    {
      Notification.requestPermission((permission) => {});
    }
    if (typeof Handlebars !== 'undefined')
    {
      Handlebars.registerHelper('add', (v1, v2) =>
      {
        return (Number(v1) + Number(v2));
      });
      Handlebars.registerHelper('byteShort', (value, decimalLength, thousandsSep, decimalSep) =>
      {
        return this.formatByteShort(value, decimalLength, thousandsSep, decimalSep);
      });
      Handlebars.registerHelper('capitalize', (v) =>
      {
        return (((v instanceof String) && v.length > 0)?v.charAt(0).toUpperCase() + v.substr(1).toLowerCase():v);
      });
      Handlebars.registerHelper('concat', (v1, v2) =>
      {
        return (v1 + v2);
      });
      Handlebars.registerHelper('date', (unTimestamp) =>
      {
        let date = new Date(unTimestamp * 1000);
        return date.getFullYear().toString().padStart(4, '0')+'-'+(date.getMonth() + 1).toString().padStart(2, '0')+'-'+date.getDate().toString().padStart(2, '0');
      });
      Handlebars.registerHelper('datetime', (unTimestamp) =>
      {
        let date = new Date(unTimestamp * 1000);
        return date.getFullYear().toString().padStart(4, '0')+'-'+(date.getMonth() + 1).toString().padStart(2, '0')+'-'+date.getDate().toString().padStart(2, '0')+' '+date.getHours().toString().padStart(2, '0')+':'+date.getMinutes().toString().padStart(2, '0')+':'+date.getSeconds().toString().padStart(2, '0');
      });
      Handlebars.registerHelper('divide', (v1, v2) =>
      {
        return (Number(v1) / Number(v2));
      });
      Handlebars.registerHelper('eachFilter', (a, k, v, options) =>
      {
        let result = '';
        let subks = [];
        if (this.isArray(k))
        {
          for (let i = 0; i < k.length; i++)
          {
            subks.push((this.isObservable(k[i]) || this.isComputed(k[i]))?k[i].v:k[i]);
          }
        }
        else if (this.isObservable(k) || this.isComputed(k))
        {
          subks.push(k.v);
        }
        else
        {
          subks = k.split(',');
        }
        let subv = ((this.isObservable(v) || this.isComputed(v))?v.v:v);
        if (this.isArray(a) || this.isObject(a))
        {
          a.forEach((deepv, deepk) =>
          {
            let m = false;
            for (let i = 0; !m && i < subks.length; i++)
            {
              if (this.isDefined(deepv[subks[i]]) && deepv[subks[i]].search(new RegExp(subv, 'i')) != -1)
              {
                m = true;
              }
            }
            if (m)
            {
              result += options.fn(deepv);
            }
          });
        }
        return result;
      });
      Handlebars.registerHelper('for', (from, to, incr, options) =>
      {
        let result = '';
        for (let i = from; i <= to; i += incr)
        {
          result += options.fn(i);
        }
        return result;
      });
      Handlebars.registerHelper('getUserEmail', () =>
      {
        return this.getUserEmail();
      });
      Handlebars.registerHelper('getUserFirstName', () =>
      {
        return this.getUserFirstName();
      });
      Handlebars.registerHelper('getUserID', () =>
      {
        return this.getUserID();
      });
      Handlebars.registerHelper('getUserLastName', () =>
      {
        return this.getUserLastName();
      });
      Handlebars.registerHelper('ifCond', (v1, operator, v2, options) =>
      {
        switch (operator)
        {
          case '=='  : return (v1 == v2)  ? options.fn(this) : options.inverse(this);
          case '===' : return (v1 === v2) ? options.fn(this) : options.inverse(this);
          case '!='  : return (v1 != v2)  ? options.fn(this) : options.inverse(this);
          case '!==' : return (v1 !== v2) ? options.fn(this) : options.inverse(this);
          case '<'   : return (v1 < v2)   ? options.fn(this) : options.inverse(this);
          case '<='  : return (v1 <= v2)  ? options.fn(this) : options.inverse(this);
          case '>'   : return (v1 > v2)   ? options.fn(this) : options.inverse(this);
          case '>='  : return (v1 >= v2)  ? options.fn(this) : options.inverse(this);
          case '&&'  : return (v1 && v2)  ? options.fn(this) : options.inverse(this);
          case '||'  : return (v1 || v2)  ? options.fn(this) : options.inverse(this);
          default    : return options.inverse(this);
        }
      });
      Handlebars.registerHelper('indexOf', (data, strIndex) =>
      {
        return data[strIndex];
      });
      Handlebars.registerHelper('isGlobalAdmin', (options) =>
      {
        if (this.isGlobalAdmin())
        {
          return options.fn(this);
        }
        else
        {
          return options.inverse(this);
        }
      });
      Handlebars.registerHelper('isLocalAdmin', (application, options) =>
      {
        if (!this.isDefined(options))
        {
          options = application;
          application = null;
        }
        if (this.isLocalAdmin(application))
        {
          return options.fn(this);
        }
        else
        {
          return options.inverse(this);
        }
      });
      Handlebars.registerHelper('isValid', (application, options) =>
      {
        if (!this.isDefined(options))
        {
          options = application;
          application = null;
        }
        if (this.isValid(application))
        {
          return options.fn(this);
        }
        else
        {
          return options.inverse(this);
        }
      });
      Handlebars.registerHelper('json', (context) =>
      {
        return JSON.stringify(context);
      });
      Handlebars.registerHelper('mod', (v1, v2) =>
      {
        return (Number(v1) % Number(v2));
      });
      Handlebars.registerHelper('multiply', (v1, v2) =>
      {
        return (Number(v1) * Number(v2));
      });
      Handlebars.registerHelper('number', (value, decimalLength, thousandsSep, decimalSep) =>
      {
        return this.formatNumber(value, decimalLength, thousandsSep, decimalSep);
      });
      Handlebars.registerHelper('numberShort', (value, decimalLength, thousandsSep, decimalSep) =>
      {
        return this.formatNumberShort(value, decimalLength, thousandsSep, decimalSep);
      });
      Handlebars.registerHelper('removeSpaces', (v) =>
      {
        return (((v instanceof String) && v.length > 0)?v.replace(/ /g, ''):v);
      });
      Handlebars.registerHelper('subtract', (v1, v2) =>
      {
        return (Number(v1) - Number(v2));
      });
      Handlebars.registerHelper('telephone', (v) =>
      {
        if (!v)
        {
          return '';
        }
        var value = v.toString().trim().replace(/^\+/, '');
        if (value.match(/[^0-9]/))
        {
          return v;
        }
        var country, city, number;
        switch (value.length)
        {
          case 10: // +1PPP####### -> C (PPP) ###-####
            country = 1;
            city = value.slice(0, 3);
            number = value.slice(3);
            break;
          case 11: // +CPPP####### -> CCC (PP) ###-####
            country = value[0];
            city = value.slice(1, 4);
            number = value.slice(4);
            break;
          case 12: // +CCCPP####### -> CCC (PP) ###-####
            country = value.slice(0, 3);
            city = value.slice(3, 5);
            number = value.slice(5);
            break;
          default:
            return v;
        }
        if (country == 1)
        {
          country = "";
        }
        number = number.slice(0, 3) + '-' + number.slice(3);
        return (country + " (" + city + ") " + number).trim();
      });
      Handlebars.registerHelper('urlEncode', encodeURIComponent);
    }
  }
  // }}}
  // {{{ addEventListener()
  addEventListener(controller, ev, func)
  {
    if (!this.isDefined(this.m_listeners[controller]))
    {
      this.m_listeners[controller] = {};
    }
    if (this.isDefined(this.m_listeners[controller][ev]))
    {
      this.removeEventListener(controller, ev);
    }
    document.addEventListener(ev, func);
    this.m_listeners[controller][ev] = func;
  }
  // }}}
  // {{{ addInterval()
  addInterval(controller, name, func, interval)
  {
    if (!this.isDefined(this.m_intervals[controller]))
    {
      this.m_intervals[controller] = {};
    }
    if (this.isDefined(this.m_intervals[controller][name]))
    {
      this.removeInterval(controller, name);
    }
    this.m_intervals[controller][name] = setInterval(func, interval);
  }
  // }}}
  // {{{ attachEvent()
  attachEvent(strHandle, callback)
  {
    if (document.addEventListener)
    {
      document.addEventListener(strHandle, callback, false);
    }
    else
    {
      document.attachEvent(strHandle, callback);
    }
  }
  // }}}
  // {{{ auth()
  auth()
  {
    if (this.m_bJwt)
    {
      if (window.localStorage.getItem('sl_wsJwt'))
      {
        let request = {Interface: 'secure', Section: 'secure', 'Function': 'auth', wsJwt: window.localStorage.getItem('sl_wsJwt'), Request: {}};
        this.wsRequest(this.m_strAuthProtocol, request).then((response) =>
        {
          let error = {};
          if (this.wsResponse(response, error))
          {
            this.m_auth = null;
            this.m_auth = response.Response;
            this.m_bHaveAuth = true;
            this.dispatchEvent('commonAuthReady', null);
            this.dispatchEvent('resetMenu', null);
            if (this.isValid())
            {
              if (!this.m_wsRequestID && !this.m_bConnecting)
              {
                this.m_bConnecting = true;
                let request = {Interface: 'live', 'Function': 'connect'};
                request.Request = {};
                this.wsRequest(this.m_strAuthProtocol, request).then((response) =>
                {
                  let error = {};
                  if (this.wsResponse(response, error))
                  {
                    this.m_wsRequestID = response.wsRequestID;
                    this.m_bConnecting = false;
                    this.dispatchEvent('commonWsConnected', null);
                  }
                });
              }
            }
            else if (this.m_wsRequestID)
            {
              this.m_bConnecting = true;
              let request = {Interface: 'live', 'Function': 'disconnect', wsRequestID: this.m_wsRequestID};
              request.Request = {};
              this.wsRequest(this.m_strAuthProtocol, request).then((response) =>
              {
                this.m_wsRequestID = null;
                this.m_bConnecting = false;
              });
            }
          }
        });
      }
      else
      {
        this.dispatchEvent('commonAuthReady', null);
        this.dispatchEvent('resetMenu', null);
      }
    }
    else
    {
      this.request('auth', null, (response) =>
      {
        let error = {};
        if (this.response(response, error))
        {
          this.m_auth = null;
          this.m_auth = response.Response.out;
          this.m_bHaveAuth = true;
          this.dispatchEvent('commonAuthReady', null);
          this.dispatchEvent('resetMenu', null);
        }
      });
    }
  }
  // }}}
  // {{{ bind()
  bind(id, name)
  {
    if (this.isDefined(id) && !this.isNull(id))
    {
      if (!this.isDefined(name) || this.isNull(name))
      {
        name = id;
      }
      let s = common.getStore(name);
      if (this.isObject(s))
      {
        document.querySelectorAll('#' + id + ' [c-model]').forEach(e =>
        {
          // {{{ init
          let o = null;
          if (typeof _ !== 'undefined')
          {
            o = _.get(s, e.getAttribute('c-model'));
            if (!this.isDefined(o))
            {
              o = new Observable;
              _.set(s, e.getAttribute('c-model'), o);
              if (e.getAttribute('id'))
              {
                o.id(e.getAttribute('id'));
              }
            }
            if (!this.isObservable(o) && !this.isComputed(o))
            {
              o = new Observable(o);
              _.set(s, e.getAttribute('c-model'), o);
              if (e.getAttribute('id'))
              {
                o.id(e.getAttribute('id'));
              }
            }
          }
          else
          {
            if (!this.isDefined(s[e.getAttribute('c-model')]))
            {
              s[e.getAttribute('c-model')] = new Observable;
              if (e.getAttribute('id'))
              {
                s[e.getAttribute('c-model')].id(e.getAttribute('id'));
              }
            }
            o = s[e.getAttribute('c-model')];
            if (!this.isObservable(o) && !this.isComputed(o))
            {
              o = new Observable(o);
              s[e.getAttribute('c-model')] = o;
              if (e.getAttribute('id'))
              {
                s[e.getAttribute('c-model')].id(e.getAttribute('id'));
              }
            }
          }
          // }}}
          // {{{ value
          if (this.isDefined(e.value))
          {
            if (this.isDefined(e.type) && (e.type == 'checkbox' || e.type == 'radio' || e.type == 'select-multiple'))
            {
              if (e.type == 'checkbox')
              {
                let v = o.v;
                document.querySelectorAll('#' + id + ' [c-model]').forEach(sube =>
                {
                  if (e.getAttribute('c-model') == sube.getAttribute('c-model'))
                  {
                    let s = false;
                    if (this.isArray(v))
                    {
                      for (let i = 0; !s && i < v.length; i++)
                      {
                        let sv = ((e.hasAttribute('c-json'))?JSON.stringify(v[i]):v[i]);
                        if (sube.value == sv)
                        {
                          s = true;
                        }
                      }
                    }
                    else
                    {
                      let sv = ((e.hasAttribute('c-json'))?JSON.stringify(v):v);
                      if (sube.value == sv)
                      {
                        s = true;
                      }
                    }
                    if (sube.checked != s)
                    {
                      sube.checked = s;
                    }
                  }
                });
              }
              else if (e.type == 'radio')
              {
                let sv = ((e.hasAttribute('c-json'))?JSON.stringify(o.v):o.v);
                document.querySelectorAll('#' + id + ' [c-model]').forEach(sube =>
                {
                  if (e.getAttribute('c-model') == sube.getAttribute('c-model'))
                  {
                    let s = ((sube.value == sv)?true:false);
                    if (sube.checked != s)
                    {
                      sube.checked = s;
                    }
                  }
                });
              }
              else if (e.type == 'select-multiple')
              {
                let v = o.v;
                if (!this.isArray(v))
                {
                  v = [o.v];
                }
                for (let i = 0; i < e.options.length; i++)
                {
                  let s = false;
                  for (let j = 0; !s && j < v.length; j++)
                  {
                    let sv = ((e.hasAttribute('c-json'))?JSON.stringify(v[j]):v[j]);
                    if (e.options[i].value == sv)
                    {
                      s = true;
                    }
                  }
                  if (e.options[i].selected != s)
                  {
                    e.options[i].selected = s;
                  }
                }
              }
            }
            else
            {
              e.value = ((e.hasAttribute('c-json'))?JSON.stringify(o.v):o.v);
            }
            o.subscribe('bind', () =>
            {
              if (this.isDefined(e.type) && (e.type == 'checkbox' || e.type == 'radio' || e.type == 'select-multiple'))
              {
                if (e.type == 'checkbox')
                {
                  let v = o.v;
                  document.querySelectorAll('#' + id + ' [c-model]').forEach(sube =>
                  {
                    if (e.getAttribute('c-model') == sube.getAttribute('c-model'))
                    {
                      let s = false;
                      if (this.isArray(v))
                      {
                        for (let i = 0; !s && i < v.length; i++)
                        {
                          let sv = ((e.hasAttribute('c-json'))?JSON.stringify(v[i]):v[i]);
                          if (sube.value == sv)
                          {
                            s = true;
                          }
                        }
                      }
                      else
                      {
                        let sv = ((e.hasAttribute('c-json'))?JSON.stringify(v):v);
                        if (sube.value == sv)
                        {
                          s = true;
                        }
                      }
                      if (sube.checked != s)
                      {
                        sube.checked = s;
                      }
                    }
                  });
                }
                else if (e.type == 'radio')
                {
                  let sv = ((e.hasAttribute('c-json'))?JSON.stringify(o.v):o.v);
                  document.querySelectorAll('#' + id + ' [c-model]').forEach(sube =>
                  {
                    if (e.getAttribute('c-model') == sube.getAttribute('c-model'))
                    {
                      let s = ((sube.value == sv)?true:false);
                      if (sube.checked != s)
                      {
                        sube.checked = s;
                      }
                    }
                  });
                }
                else if (e.type == 'select-multiple')
                {
                  let v = o.v;
                  if (!this.isArray(v))
                  {
                    v = [o.v];
                  }
                  for (let i = 0; i < e.options.length; i++)
                  {
                    let s = false;
                    for (let j = 0; !s && j < v.length; j++)
                    {
                      let sv = ((e.hasAttribute('c-json'))?JSON.stringify(v[j]):v[j]);
                      if (e.options[i].value == sv)
                      {
                        s = true;
                      }
                    }
                    if (e.options[i].selected != s)
                    {
                      e.options[i].selected = s;
                    }
                  }
                }
              }
              else
              {
                e.value = ((e.hasAttribute('c-json'))?JSON.stringify(o.v):o.v);
              }
            });
          }
          else if (this.isDefined(e.innerHTML))
          {
            e.innerHTML = ((e.hasAttribute('c-json'))?JSON.stringify(o.v):o.v);
            o.subscribe('bind', () => {e.innerHTML = ((e.hasAttribute('c-json'))?JSON.stringify(o.v):o.v);});
          }
          // }}}
          // {{{ change
          if (e.hasAttribute('c-change'))
          {
            e.onchange = () =>
            {
              let c = false;
              if (this.isDefined(e.type) && (e.type == 'checkbox' || e.type == 'radio' || e.type == 'select-multiple'))
              {
                if (e.type == 'checkbox')
                {
                  let v = [];
                  document.querySelectorAll('#' + id + ' [c-model]').forEach(sube =>
                  {
                    if (e.getAttribute('c-model') == sube.getAttribute('c-model') && sube.checked)
                    {
                      v.push(((sube.hasAttribute('c-json'))?JSON.parse(sube.value):sube.value));
                    }
                  });
                  if (v.length == 0)
                  {
                    v = false;
                  }
                  else if (v.length == 1)
                  {
                    v = v[0];
                  }
                  c = ((JSON.stringify(o.v) != JSON.stringify(v))?true:false);
                  if (c)
                  {
                    o.v = v;
                  }
                }
                else if (e.type == 'radio')
                {
                  let v = '';
                  document.querySelectorAll('#' + id + ' [c-model]').forEach(sube =>
                  {
                    if (e.getAttribute('c-model') == sube.getAttribute('c-model') && sube.checked)
                    {
                      v = ((sube.hasAttribute('c-json'))?JSON.parse(sube.value):sube.value);
                    }
                  });
                  c = ((JSON.stringify(o.v) != JSON.stringify(v))?true:false);
                  if (c)
                  {
                    o.v = v;
                  }
                }
                else if (e.type == 'select-multiple')
                {
                  let v = [];
                  for (let i = 0; i < e.options.length; i++)
                  {
                    if (e.options[i].selected)
                    {
                      v.push(((e.hasAttribute('c-json'))?JSON.parse(e.options[i].value):e.options[i].value));
                    }
                  }
                  c = ((JSON.stringify(o.v) != JSON.stringify(v))?true:false);
                  if (c)
                  {
                    o.v = v;
                  }
                }
              }
              else
              {
                let v = ((e.hasAttribute('c-json'))?JSON.parse(e.value):e.value);
                c = ((o.v != v)?true:false);
                if (c)
                {
                  o.v = v;
                }
              }
              if (this.isDefined(o.onchange))
              {
                o.onchange();
              }
              eval('s.' + e.getAttribute('c-change'));
              if (c && e.hasAttribute('c-render'))
              {
                this.render(this.id, this.name, this.component);
                if (this.isDefined(o.id))
                {
                  o.e().focus();
                }
              }
            };
          }
          else
          {
            e.onchange = () =>
            {
              let v = ((e.hasAttribute('c-json'))?JSON.parse(e.value):e.value);
              let c = ((o.v != v)?true:false);
              if (this.isDefined(e.type) && (e.type == 'checkbox' || e.type == 'radio' || e.type == 'select-multiple'))
              {
                if (e.type == 'checkbox')
                {
                  let v = [];
                  document.querySelectorAll('#' + id + ' [c-model]').forEach(sube =>
                  {
                    if (e.getAttribute('c-model') == sube.getAttribute('c-model') && sube.checked)
                    {
                      v.push(((sube.hasAttribute('c-json'))?JSON.parse(sube.value):sube.value));
                    }
                  });
                  if (v.length == 0)
                  {
                    v = false;
                  }
                  else if (v.length == 1)
                  {
                    v = v[0];
                  }
                  c = ((JSON.stringify(o.v) != JSON.stringify(v))?true:false);
                  if (c)
                  {
                    o.v = v;
                  }
                }
                else if (e.type == 'radio')
                {
                  let v = '';
                  document.querySelectorAll('#' + id + ' [c-model]').forEach(sube =>
                  {
                    if (e.getAttribute('c-model') == sube.getAttribute('c-model') && sube.checked)
                    {
                      v = ((sube.hasAttribute('c-json'))?JSON.parse(sube.value):sube.value);
                    }
                  });
                  c = ((JSON.stringify(o.v) != JSON.stringify(v))?true:false);
                  if (c)
                  {
                    o.v = v;
                  }
                }
                else if (e.type == 'select-multiple')
                {
                  let v = [];
                  for (let i = 0; i < e.options.length; i++)
                  {
                    if (e.options[i].selected)
                    {
                      v.push(((e.hasAttribute('c-json'))?JSON.parse(e.options[i].value):e.options[i].value));
                    }
                  }
                  c = ((JSON.stringify(o.v) != JSON.stringify(v))?true:false);
                  if (c)
                  {
                    o.v = v;
                  }
                }
              }
              else
              {
                let v = ((e.hasAttribute('c-json'))?JSON.parse(e.value):e.value);
                c = ((o.v != v)?true:false);
                if (c)
                {
                  o.v = v;
                }
              }
              if (this.isDefined(o.onchange))
              {
                o.onchange();
              }
              if (c && e.hasAttribute('c-render'))
              {
                this.render(this.id, this.name, this.component);
                if (this.isDefined(o.id))
                {
                  o.e().focus();
                }
              }
            };
          }
          // }}}
          // {{{ click
          if (e.hasAttribute('c-click'))
          {
            e.onclick = () =>
            {
              if (this.isDefined(o.onclick))
              {
                o.onclick();
              }
              eval('s.' + e.getAttribute('c-click'));
            };
          }
          else
          {
            e.onclick = () =>
            {
              if (this.isDefined(o.onclick))
              {
                o.onclick();
              }
            };
          }
          // }}}
          // {{{ keydown
          if (e.hasAttribute('c-keydown'))
          {
            e.onkeydown = () =>
            {
              if (this.isDefined(o.onkeydown))
              {
                o.onkeydown();
              }
              eval('s.' + e.getAttribute('c-keydown'));
            };
          }
          else
          {
            e.onkeydown = () =>
            {
              if (this.isDefined(o.onkeydown))
              {
                o.onkeydown();
              }
            };
          }
          // }}}
          // {{{ keyup
          if (e.hasAttribute('c-keyup'))
          {
            e.onkeyup = () =>
            {
              let v = ((e.hasAttribute('c-json'))?JSON.parse(e.value):e.value);
              let c = ((o.v != v)?true:false);
              if (c && this.isDefined(e.type) && (e.type == 'password' || e.type == 'text' || e.type == 'textarea'))
              {
                o.v = v;
              }
              if (this.isDefined(o.onkeyup))
              {
                o.onkeyup();
              }
              eval('s.' + e.getAttribute('c-keyup'));
              if (c && e.hasAttribute('c-render'))
              {
                this.render(this.id, this.name, this.component);
                if (this.isDefined(o.id))
                {
                  o.e().focus();
                }
              }
            };
          }
          else
          {
            e.onkeyup = () =>
            {
              let v = ((e.hasAttribute('c-json'))?JSON.parse(e.value):e.value);
              let c = ((o.v != v)?true:false);
              if (c && this.isDefined(e.type) && (e.type == 'password' || e.type == 'text' || e.type == 'textarea'))
              {
                o.v = v;
              }
              if (this.isDefined(o.onkeyup))
              {
                o.onkeyup();
              }
              if (c && e.hasAttribute('c-render'))
              {
                this.render(this.id, this.name, this.component);
                if (this.isDefined(o.id))
                {
                  o.e().focus();
                }
              }
            };
          }
          // }}}
        });
      }
    }
    else
    {
      this.bind(this.id, this.name);
    }
  }
  // }}}
  // {{{ clearMenu()
  clearMenu()
  {
    this.menu = null;
    this.menu = {left: [], right: []};
  }
  // }}}
  // {{{ detachEvent()
  detachEvent(strHandle, callback)
  {
    if (document.removeEventListener)
    {
      document.removeEventListener(strHandle, callback, false);
    }
    else
    {
      document.detachEvent(strHandle, callback);
    }
  }
  // }}}
  // {{{ dispatchEvent()
  dispatchEvent(strHandle, data)
  {
    let eventHandle = new CustomEvent(strHandle, {'detail': data});
    document.dispatchEvent(eventHandle);
  }
  // }}}
  // {{{ enableJwt()
  enableJwt(bEnable)
  {
    this.m_bJwt = bEnable;
  }
  // }}}
  // {{{ enableJwtInclusion()
  enableJwtInclusion(bEnable)
  {
    this.m_bJwtInclusion = bEnable;
  }
  // }}}
  // {{{ esc()
  esc(v)
  {
    return v.replace(/[\0\x08\x09\x1a\n\r"'\\\%]/g, (char) =>
    {
      switch (char)
      {
        case "\0": return "\\0";
        case "\x08": return "\\b";
        case "\x09": return "\\t";
        case "\x1a": return "\\z";
        case "\n": return "\\n";
        case "\r": return "\\r";
        case "\"":
        case "'":
        case "\\":
        case "%": return "\\"+char;
        default: return char;
      }
    });
  }
  // }}}
  // {{{ existStore()
  existStore(controller)
  {
    let bResult = false;

    if (this.isDefined(this.m_store[controller]))
    {
      bResult = true;
    }

    return bResult;
  }
  // }}}
  // {{{ formatByteShort()
  formatByteShort(value, decimalLength, thousandsSep, decimalSep)
  {
    let bNegative = false;
    let nNumber = Number(value);
    let strSuffix = '';

    if (nNumber < 0)
    {
      bNegative = true;
      nNumber *= -1;
    }
    if (nNumber >= 1024)
    {
      strSuffix = 'KB';
      nNumber /= 1024;
      if (nNumber >= 1024)
      {
        strSuffix = 'MB';
        nNumber /= 1024;
        if (nNumber >= 1024)
        {
          strSuffix = 'GB';
          nNumber /= 1024;
          if (nNumber >= 1024)
          {
            strSuffix = 'TB';
            nNumber /= 1024;
            if (nNumber >= 1024)
            {
              strSuffix = 'PB';
              nNumber /= 1024;
              if (nNumber >= 1024)
              {
                strSuffix = 'EB';
                nNumber /= 1024;
                if (nNumber >= 1024)
                {
                  strSuffix = 'ZB';
                  nNumber /= 1024;
                  if (nNumber >= 1024)
                  {
                    strSuffix = 'YB';
                    nNumber /= 1024;
                  }
                }
              }
            }
          }
        }
      }
    }
    nNumber = this.formatNumber(nNumber, decimalLength, thousandsSep, decimalSep);

    return ((bNegative)?'-':'') + nNumber + strSuffix;
  }
  // }}}
  // {{{ formatNumber()
  formatNumber(value, decimalLength, thousandsSep, decimalSep)
  {
    if (!this.isDefined(decimalLength) || this.isNull(decimalLength) || this.isObject(decimalLength))
    {
      decimalLength = 2;
    }
    if (!this.isDefined(thousandsSep) || this.isNull(thousandsSep) || this.isObject(thousandsSep))
    {
      thousandsSep = ',';
    }
    let num = Number(value).toFixed(Math.max(0, ~~decimalLength));
    if (decimalSep)
    {
      num = num.replace('.', decimalSep);
    }

    return num.replace(new RegExp('\\d(?=(\\d{3})+' + ((decimalLength > 0)?'\\D':'$') + ')', 'g'), '$&' + thousandsSep);
  }
  // }}}
  // {{{ formatNumberShort()
  formatNumberShort(value, decimalLength, thousandsSep, decimalSep)
  {
    let bNegative = false;
    let nNumber = Number(value);
    let strSuffix = '';

    if (nNumber < 0)
    {
      bNegative = true;
      nNumber *= -1;
    }
    if (nNumber >= 1000)
    {
      strSuffix = 'K';
      nNumber /= 1000;
      if (nNumber >= 1000)
      {
        strSuffix = 'M';
        nNumber /= 1000;
        if (nNumber >= 1000)
        {
          strSuffix = 'B';
          nNumber /= 1000;
          if (nNumber >= 1000)
          {
            strSuffix = 'T';
            nNumber /= 1000;
            if (nNumber >= 1000)
            {
              strSuffix = 'P';
              nNumber /= 1000;
              if (nNumber >= 1000)
              {
                strSuffix = 'E';
                nNumber /= 1000;
                if (nNumber >= 1000)
                {
                  strSuffix = 'Z';
                  nNumber /= 1000;
                  if (nNumber >= 1000)
                  {
                    strSuffix = 'Y';
                    nNumber /= 1000;
                    if (nNumber >= 1000)
                    {
                      strSuffix = 'R';
                      nNumber /= 1000;
                      if (nNumber >= 1000)
                      {
                        strSuffix = 'Q';
                        nNumber /= 1000;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    nNumber = this.formatNumber(nNumber, decimalLength, thousandsSep, decimalSep);

    return ((bNegative)?'-':'') + nNumber + strSuffix;
  }
  // }}}
  // {{{ getCookie()
  getCookie(strName)
  {
    let bFound = false;
    let strValue = '';
    let strSubName = strName+'=';
    let strCookies = decodeURIComponent(document.cookie);
    let cookies = strCookies.split(';');

    for (let i = 0; !bFound && i < cookies.length; i++)
    {
      let strCookie = cookies[i];
      while (strCookie.charAt(0) == ' ')
      {
        strCookie = strCookie.substring(1);
      }
      if (strCookie.indexOf(strSubName) == 0)
      {
        bFound = true;
        strValue = strCookie.substring(strSubName.length, strCookie.length);
      }
    }

    return strValue;
  }
  // }}}
  // {{{ getParam()
  getParam(nav, param)
  {
    let result = null;

    if (this.isObject(nav.data) && this.isDefined(nav.data[param]))
    {
      result = nav.data[param];
    }
    else if (this.isObject(nav.params) && this.isDefined(nav.params[param]))
    {
      result = nav.params[param];
    }

    return result;
  }
  // }}}
  // {{{ getRedirectPath()
  getRedirectPath()
  {
    return this.m_strRedirectPath;
  }
  // }}}
  // {{{ getStore()
  getStore(controller)
  {
    if (!this.isDefined(this.m_store[controller]))
    {
      this.putStore(controller, {});
    }

    return this.m_store[controller];
  }
  // }}}
  // {{{ getUserEmail()
  getUserEmail()
  {
    return this.m_auth.email;
  };
  // }}}
  // {{{ getUserFirstName()
  getUserFirstName()
  {
    return this.m_auth.first_name;
  };
  // }}}
  // {{{ getUserID()
  getUserID()
  {
    return this.m_auth.userid;
  };
  // }}}
  // {{{ getUserLastName()
  getUserLastName()
  {
    return this.m_auth.last_name;
  };
  // }}}
  // {{{ isArray()
  isArray(variable)
  {
    let bResult = false;

    if (typeof variable === 'object' && Array.isArray(variable))
    {
      bResult = true;
    }

    return bResult;
  }
  // }}}
  // {{{ isComputed()
  isComputed(variable)
  {
    let bResult = false;

    if (this.isObject(variable) && variable instanceof Computed)
    {
      bResult = true;
    }

    return bResult;
  }
  // }}}
  // {{{ isCookie()
  isCookie(strName)
  {
    let bFound = false;
    let strSubName = strName+'=';
    let strCookies = decodeURIComponent(document.cookie);
    let cookies = strCookies.split(';');

    for (let i = 0; !bFound && i < cookies.length; i++)
    {
      let strCookie = cookies[i];
      while (strCookie.charAt(0) == ' ')
      {
        strCookie = strCookie.substring(1);
      }
      if (strCookie.indexOf(strSubName) == 0)
      {
        bFound = true;
      }
    }

    return bFound;
  }
  // }}}
  // {{{ isDefined()
  isDefined(variable)
  {
    let bResult = false;

    if (typeof variable !== 'undefined')
    {
      bResult = true;
    }

    return bResult;
  }
  // }}}
  // {{{ isGlobalAdmin()
  isGlobalAdmin()
  {
    let bResult = false;

    if (this.isDefined(this.m_auth) && this.m_auth.admin)
    {
      bResult = true;
    }

    return bResult;
  }
  // }}}
  // {{{ isLocalAdmin()
  isLocalAdmin(strApplication)
  {
    let bResult = false;

    if (this.isDefined(this.m_auth) && ((this.isDefined(this.m_auth.admin) && this.m_auth.admin) || (strApplication != null && this.isDefined(this.m_auth.apps) && this.isDefined(this.m_auth.apps[strApplication]) && this.m_auth.apps[strApplication])))
    {
      bResult = true;
    }

    return bResult;
  }
  // }}}
  // {{{ isNull()
  isNull(variable)
  {
    let bResult = false;

    if (this.isDefined(variable) && variable === null)
    {
      bResult = true;
    }

    return bResult;
  }
  // }}}
  // {{{ isObject()
  isObject(variable)
  {
    let bResult = false;

    if (typeof variable === 'object' && !Array.isArray(variable) && variable !== null)
    {
      bResult = true;
    }

    return bResult;
  }
  // }}}
  // {{{ isObservable()
  isObservable(variable)
  {
    let bResult = false;

    if (this.isObject(variable) && variable instanceof Observable)
    {
      bResult = true;
    }

    return bResult;
  }
  // }}}
  // {{{ isParam()
  isParam(nav, param)
  {
    return ((this.isObject(nav.data) && this.isDefined(nav.data[param])) || (this.isObject(nav.params) && this.isDefined(nav.params[param])));
  }
  // }}}
  // {{{ isValid()
  isValid(strApplication)
  {
    let bResult = false;

    if (this.isDefined(this.m_auth) && ((this.isDefined(this.m_auth.admin) && this.m_auth.admin) || (strApplication != null && this.isDefined(this.m_auth.apps) && this.isDefined(this.m_auth.apps[strApplication])) || (strApplication == null && this.isDefined(this.m_auth.userid) && this.m_auth.userid != null && this.m_auth.userid.length > 0)))
    {
      bResult = true;
    }

    return bResult;
  }
  // }}}
  // {{{ load()
  async load(id)
  {
    if (this.isDefined(this.autoLoads[id]))
    {
      if (this.isDefined(this.autoLoads[id].controller))
      {
        this.autoLoads[id].controller(id);
      }
      this.render(id, this.autoLoads[id]);
    }
  }
  // }}}
  // {{{ loadModal()
  loadModal(controller, id, show)
  {
    if (controller)
    {
      this.update(controller);
    }
    document.querySelectorAll('div.modal-backdrop').forEach(e =>
    {
      e.parentNode.removeChild(e);
    });
    document.querySelectorAll('body').forEach(e =>
    {
      e.style.overflow = 'auto';
      e.style.paddingRight = '';
    });
    document.querySelectorAll('div.fixed-top').forEach(e =>
    {
      e.style.paddingRight = '';
    });
    let bFirst = true;
    document.querySelectorAll('#'+id).forEach(loadModal =>
    {
      if (bFirst)
      {
        bFirst = false;
      }
      else
      {
        loadModal.remove();
      }
    });
    let modal = new bootstrap.Modal(document.getElementById(id));
    if (show)
    {
      modal.show();
    }
    else
    {
      modal.hide();
    }
  }
  // }}}
  // {{{ loads()
  async loads(data)
  {
    this.autoLoads = null;
    this.autoLoads = {};
    for (let id of Object.keys(data))
    {
      let c = await import(data[id]);
      this.autoLoads[id] = c.default;
      this.load(id);
    }
  }
  // }}}
  // {{{ processLogin()
  processLogin()
  {
    this.login.info = 'Processing login...';
    this.render(this.id, 'Login', this.component);
    if (this.m_bJwt)
    {
      let request = null;
      request = {Interface: 'secure', Section: 'secure', 'Function': 'getSecurityModule', Request: {}};
      this.wsRequest(this.m_strAuthProtocol, request).then((response) =>
      {
        this.login.info = null;
        if (this.m_strLoginType == null)
        {
          this.m_strLoginType = 'password';
          if (this.isDefined(response.Response.Module) && response.Response.Module != '')
          {
            this.m_strLoginType = response.Response.Module;
          }
        }
        fetch('/include/common_addons/auth/modules.json',
        {
          method: 'GET',
          headers:
          {
            'Content-Type': 'application/json'
          }
        })
        .then((response) =>
        {
          let request = null;
          response = ((response.status == 200)?response.json():{});
          request = {Interface: 'secure', Section: 'secure', 'Function': 'process', Request: this.simplify(this.login.login)};
          request.Request.Type = this.m_strLoginType;
          if (window.localStorage.getItem('sl_uniqueID'))
          {
            request.Request.UniqueID = window.localStorage.getItem('sl_uniqueID');
          }
          if (this.isDefined(response[this.m_strLoginType]) && this.isDefined(response[this.m_strLoginType]['cookie']) && this.isCookie(response[this.m_strLoginType]['cookie']))
          {
            request.Request.Data = this.getCookie(response[this.m_strLoginType]['cookie']);
          }
          this.wsRequest(this.m_strAuthProtocol, request).then((response) =>
          {
            let error = {};
            this.login.info = null;
            if (this.wsResponse(response, error))
            {
              if (this.isDefined(response.Error))
              {
                if (this.isDefined(response.Error.Message) && response.Error.Message.length > 0 && response.Error.Message.search('Please provide the User.') == -1)
                {
                  this.login.message = response.Error.Message;
                }
                else if (response.Error.length > 0 && response.Error.search('Please provide the User.') == -1)
                {
                  this.login.message = response.Error;
                }
              }
              if (this.isDefined(response.Response.auth))
              {
                this.m_auth = null;
                this.m_auth = response.Response.auth;
                this.m_bHaveAuth = true;
                if (this.isDefined(response.Response.jwt))
                {
                  window.localStorage.setItem('sl_wsJwt', response.Response.jwt);
                }
                if (this.isDefined(this.m_auth.login_title))
                {
                  if (!this.isDefined(this.login.login) || !this.login.login)
                  {
                    this.login.login = {};
                  }
                  this.login.login.title = this.m_auth.login_title;
                  if (this.login.login.title.length <= 30 || this.login.login.title.substr(this.login.login.title.length - 30, 30) != ' (please wait for redirect...)')
                  {
                    this.login.showForm = true;
                  }
                }
                if (this.isValid())
                {
                  this.dispatchEvent('resetMenu', null);
                  document.location.href = this.m_strRedirectPath;
                }
                else
                {
                  let request = {Interface: 'secure', Section: 'secure', 'Function': 'login'};
                  request.Request = {Type: this.m_strLoginType, Return: document.location.href};
                  this.wsRequest(this.m_strAuthProtocol, request).then((response) =>
                  {
                    let error = {};
                    this.login.info = null;
                    if (this.wsResponse(response, error))
                    {
                      if (common.isDefined(response.Response))
                      {
                        if (common.isDefined(response.Response.UniqueID) && response.Response.UniqueID.length > 0)
                        {
                          window.localStorage.setItem('sl_uniqueID', response.Response.UniqueID);
                        }
                        if (this.isDefined(response.Response.Redirect) && response.Response.Redirect.length > 0)
                        {
                          this.dispatchEvent('resetMenu', null);
                          document.location.href = response.Response.Redirect;
                        }
                      }
                    }
                    else
                    {
                      this.login.message = error.message;
                    }
                    this.render(this.id, 'Login', this.component);
                  });
                }
              }
              if (this.isDefined(error.message))
              {
                this.login.message = error.message;
              }
            }
            else
            {
              this.login.message = error.message;
            }
            this.render(this.id, 'Login', this.component);
          });
        });
      });
    }
    else
    {
      this.login.Application = this.application;
      if (this.m_strLoginType != '')
      {
        this.login.LoginType = this.m_strLoginType;
      }
      this.request('authProcessLogin', this.simplify(this.login.login), (result) =>
      {
        let error = {};
        this.login.info = null;
        if (this.response(result, error))
        {
          this.m_auth = null;
          this.m_auth = result.data.Response.out;
          this.m_bHaveAuth = true;
          if (this.isDefined(this.m_auth.login_title))
          {
            this.login.login.title = this.m_auth.login_title;
            if (this.login.login.title.length <= 30 || this.login.login.title.substr(this.login.login.title.length - 30, 30) != ' (please wait for redirect...)')
            {
              this.login.showForm = true;
            }
          }
          if (this.isValid())
          {
            this.dispatchEvent('resetMenu', null);
            document.location.href = this.m_strRedirectPath;
          }
          else
          {
            let login = {};
            login.Application = this.application;
            if (this.m_strLoginType != '')
            {
              login.LoginType = this.m_strLoginType;
            }
            login.Redirect = this.getRedirectPath();
            this.request('authLogin', login, (result) =>
            {
              let error = {};
              this.login.info = null;
              if (this.response(result, error))
              {
                if (this.isDefined(result.data.Response.out.Status) && result.data.Response.out.Status == 'okay')
                {
                  if (this.isDefined(result.data.Response.out.Redirect) && result.data.Response.out.Redirect.length > 0)
                  {
                    this.dispatchEvent('resetMenu', null);
                    document.location.href = result.data.Response.out.Redirect;
                  }
                }
                else if (this.isDefined(result.data.Response.out.Error) && result.data.Response.out.Error.length > 0)
                {
                  this.login.message = result.data.Response.out.Error;
                }
                else
                {
                  this.login.message = 'Encountered an unknown error.';
                }
              }
              else
              {
                this.login.message = error.message;
              }
              this.render(this.id, 'Login', this.component);
            });
          }
        }
        else
        {
          this.login.message = error.message;
        }
        this.render(this.id, 'Login', this.component);
      });
    }
  }
  // }}}
  // {{{ processLogout()
  processLogout()
  {
    this.logout.info = 'Processing logout...';
    if (this.isValid())
    {
      if (this.m_bJwt)
      {
        let request = null;
        request = {Interface: 'secure', Section: 'secure', 'Function': 'getSecurityModule', Request: {}};
        this.wsRequest(this.m_strAuthProtocol, request).then((response) =>
        {
          this.logout.info = null;
          if (this.m_strLoginType == null)
          {
            this.m_strLoginType = 'password';
            if (this.isDefined(response.Response) && this.isDefined(response.Response.Module) && response.Response.Module != '')
            {
              this.m_strLoginType = response.Response.Module;
            }
          }
          let request = {Interface: 'secure', Section: 'secure', 'Function': 'logout'};
          request.Request = {Type: this.m_strLoginType, Return: this.getRedirectPath()};
          this.wsRequest(this.m_strAuthProtocol, request).then((response) =>
          {
            let error = {};
            this.logout.info = null;
            if (this.wsResponse(response, error))
            {
              if (this.isDefined(response.Response.Redirect) && response.Response.Redirect.length > 0)
              {
                this.m_bHaveAuth = false;
                this.m_auth = null;
                this.m_auth = {admin: false, apps: {}};
                window.localStorage.removeItem('sl_wsJwt');
                window.localStorage.removeItem('sl_uniqueID');
                this.dispatchEvent('resetMenu', null);
                document.location.href = response.Response.Redirect;
              }
            }
            else
            {
              this.logout.message = error.message;
            }
          });
        });
      }
      else
      {
        let logout = {};
        logout.Application = this.getApplication();
        if (this.m_strLoginType != '')
        {
          logout.LoginType = this.m_strLoginType;
        }
        logout.Redirect = this.getRedirectPath();
        this.request('authLogout', logout, (result) =>
        {
          let error = {};
          this.logout.info = null;
          if (this.response(result, error))
          {
            if (common.isDefined(result.data.Response.out.Status) && result.data.Response.out.Status == 'okay')
            {
              if (common.isDefined(result.data.Response.out.Redirect) && result.data.Response.out.Redirect.length > 0)
              {
                this.dispatchEvent('resetMenu', null);
                document.location.href = result.data.Response.out.Redirect;
              }
              else
              {
                this.logout.message = 'Redirect not found.';
              }
            }
            else if (this.isDefined(result.data.Response.out.Error) && result.data.Response.out.Error.length > 0)
            {
              this.logout.message = result.data.Response.out.Error;
            }
            else
            {
              this.logout.message = 'Encountered an unknown error.';
            }
          }
          else
          {
            this.logout.message = error.message;
          }
        });
      }
    }
    else
    {
      this.m_bHaveAuth = false;
      this.m_auth = null;
      this.m_auth = {admin: false, apps: {}};
      if (this.m_bJwt)
      {
        window.localStorage.removeItem('sl_wsJwt');
        window.localStorage.removeItem('sl_uniqueID');
      }
      this.dispatchEvent('resetMenu', null);
      document.location.href = this.m_strRedirectPath;
    }
  }
  // }}}
  // {{{ pushDebugMessage()
  pushDebugMessage(strMessage)
  {
    if (this.isDefined(this.debug) && this.debug)
    {
      let messages = null;
      if (window.localStorage.getItem('sl_debugMessages'))
      {
        messages = JSON.parse(window.localStorage.getItem('sl_debugMessages'));
      }
      else
      {
        messages = [];
      }
      messages.push(strMessage);
      window.localStorage.setItem('sl_debugMessages', JSON.stringify(messages));
    }
  }
  // }}}
  // {{{ putStore()
  putStore(controller, data)
  {
    if (!this.isDefined(data) || this.isNull(data))
    {
      data = {};
    }
    this.m_store[controller] = data;
  }
  // }}}
  // {{{ removeDebugMessages()
  removeDebugMessages()
  {
    if (window.localStorage.getItem('sl_debugMessages'))
    {
      window.localStorage.removeItem('sl_debugMessages');
    }
  }
  // }}}
  // {{{ removeEventListener()
  removeEventListener(controller, ev)
  {
    if (this.isDefined(this.m_listeners[controller]) && this.isDefined(this.m_listeners[controller][ev]))
    {
      document.removeEventListener(ev, this.m_listeners[controller][ev]);
      delete this.m_listeners[controller][ev];
    }
    if (this.m_listeners[controller].length == 0)
    {
      delete this.m_listeners[controller];
    }
  }
  // }}}
  // {{{ removeInterval()
  removeInterval(controller, name)
  {
    if (this.isDefined(this.m_intervals[controller]) && this.isDefined(this.m_intervals[controller][name]))
    {
      clearInterval(this.m_intervals[controller][name]);
      delete this.m_intervals[controller][name];
    }
    if (this.m_intervals[controller].length == 0)
    {
      delete this.m_intervals[controller];
    }
  }
  // }}}
  // {{{ render()
  render(id, name, component)
  {
    if (this.isDefined(id) && !this.isNull(id))
    {
      if ((!this.isDefined(name) || this.isNull(name)) && this.isDefined(this.autoLoads[id]))
      {
        name = this.autoLoads[id];
      }
      if (!this.isDefined(component) || this.isNull(component))
      {
        component = name;
        name = id;
      }
      let s = common.getStore(name);
      document.getElementById(id).innerHTML = Handlebars.compile(component.template)(s);
      document.querySelectorAll('#' + id + ' [autofocus]').forEach(e =>
      {
        e.focus();
      });
      document.querySelectorAll('#' + id + ' [c-change]').forEach(e =>
      {
        e.onchange = () => eval('s.' + e.getAttribute('c-change'));
      });
      document.querySelectorAll('#' + id + ' [c-click]').forEach(e =>
      {
        e.onclick = () => eval('s.' + e.getAttribute('c-click'));
      });
      document.querySelectorAll('#' + id + ' [c-keydown]').forEach(e =>
      {
        e.onkeydown = () => eval('s.' + e.getAttribute('c-keydown'));
      });
      document.querySelectorAll('#' + id + ' [c-keyup]').forEach(e =>
      {
        e.onkeyup = () => eval('s.' + e.getAttribute('c-keyup'));
      });
      this.bind(id, name);
      this.dispatchEvent('render', {id: id, name: name});
    }
    else
    {
      for (let id of Object.keys(this.autoLoads))
      {
        this.render(id, this.autoLoads[id]);
      }
      this.render(this.id, this.name, this.component);
    }
  }
  // }}}
  // {{{ request()
  request(strFunction, request, callback)
  {
    let strScript = null;
    if (this.isDefined(this.script))
    {
      strScript = this.script;
    }
    if (request && this.isDefined(request._script))
    {
      strScript = request._script;
      delete request._script;
    }
    if (this.isDefined(strScript))
    {
      let data = {'Function': strFunction};
      if (request != null)
      {
        data.Arguments = request;
      }
      fetch(strScript,
      {
        method: 'POST',
        headers:
        {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(data)
      })
      .then(response => response.json())
      .then(callback);
    }
  }
  // }}}
  // {{{ resetMenu()
  resetMenu()
  {
    if (this.isValid())
    {
      this.menu.right[this.menu.right.length] = {value: 'Logout as ' + this.getUserFirstName(), href: '/Logout', icon: 'box-arrow-right', active: null};
    }
    else
    {
      this.menu.right[this.menu.right.length] = {value: 'Login', href: '/Login', icon: 'box-arrow-in-right', active: null};
    }
    this.setMenu(this.strMenu, this.strSubMenu);
  }
  // }}}
  // {{{ response()
  response(response, error)
  {
    let bResult = false;

    if (this.isDefined(response.Status) && response.Status == 'okay')
    {
      bResult = true;
    }
    else if (this.isDefined(response.Error))
    {
      error.message = response.Error;
    }
    else
    {
      error.message = 'Encountered an unknown error.';
    }

    return bResult;
  }
  // }}}
  // {{{ routes()
  routes(data)
  {
    for (let i = 0; i < data.length; i++)
    {
      if (this.isDefined(data[i].component))
      {
        if (this.isDefined(data[i].name) && this.isDefined(data[i].path))
        {
          this.router.on(data[i].path, async (nav) =>
          {
            let c = null;
            if (this.isDefined(this.name) && this.name != '')
            {
              if (this.isDefined(this.m_listeners[this.name]))
              {
                for (let [ev, func] of Object.entries(this.m_listeners[this.name]))
                {
                  document.removeEventListener(ev, func);
                }
                delete this.m_listeners[this.name];
              }
              if (this.isDefined(this.m_intervals[this.name]))
              {
                for (let [name, handle] of Object.entries(this.m_intervals[this.name]))
                {
                  clearInterval(handle);
                }
                delete this.m_intervals[this.name];
              }
            }
            let path = data[i].path;
            this.name = data[i].name;
            if (this.isDefined(this.components[path]))
            {
              this.component = c = this.components[path];
            }
            else
            {
              let component = await import(data[i].component);
              this.components[path] = this.component = c = component.default;
            }
            if (this.isDefined(c.controller))
            {
              c.controller(this.id, nav);
            }
            this.render(this.id, data[i].name, c);
          });
        }
      }
      else if (this.isDefined(data[i].default))
      {
        this.router.notFound(() => this.router.navigate(data[i].default));
      }
    }
    this.router.resolve();
  }
  // }}}
  // {{{ scope()
  scope(controller, data)
  {
    return this.store(controller, data);
  }
  // }}}
  // {{{ sendDebugEmail()
  sendDebugEmail()
  {
    if (this.isDefined(this.junction) && this.isDefined(this.email))
    {
      if (window.localStorage.getItem('sl_debugMessages'))
      {
        let messages = JSON.parse(window.localStorage.getItem('sl_debugMessages'));
        let strHtml = '';
        for (let i = 0; i < messages.length; i++)
        {
          strHtml += '<li>' + messages[i] + '</li>';
        }
        if (!this.isNull(strHtml))
        {
          this.sendEmail(this.email, 'Common Debug', '<html><body><ul>'+strHtml+'</ul></body></html>', null);
        }
      }
    }
  }
  // }}}
  // {{{ sendEmail()
  sendEmail(strTo, strSubject, strHtml, strText)
  {
    if (this.isDefined(this.junction))
    {
      let request = {Service: 'email', To: strTo, Subject: strSubject};
      if (!this.isNull(strHtml))
      {
        request.HTML = strHtml;
      }
      if (!this.isNull(strText))
      {
        request.Text = strText;
      }
      this.junction.request([request], true, (response) =>
      {
        let error = {};
        if (!this.junction.response(response, error))
        {
          console.log(response);
        }
      });
    }
  }
  // }}}
  // {{{ setCookie()
  setCookie(strName, strValue)
  {
    const d = new Date();
    d.setTime(d.getTime()+(365*24*60*60*1000));
    let strExpires = 'expires'+d.toUTCString();
    document.cookie = strName+'='+strValue+';'+strExpires+';path=/';
  }
  // }}}
  // {{{ setMenu()
  setMenu(strMenu, strSubMenu)
  {
    let nActiveLeft = -1;
    let menu = this.menu;
    let submenu = this.submenu;
    if (!this.isDefined(strSubMenu))
    {
      strSubMenu = null;
    }
    this.menu = null;
    this.submenu = null;
    this.strPrevMenu = this.strMenu;
    this.strPrevSubMenu = this.strSubMenu;
    this.strMenu = strMenu;
    this.strSubMenu = strSubMenu;
    for (let i = 0; i < menu.left.length; i++)
    {
      if (menu.left[i].value == strMenu)
      {
        nActiveLeft = i;
        menu.left[i].active = 'active';
        this.activeMenu = strMenu;
      }
      else
      {
        menu.left[i].active = null;
      }
    }
    let nActiveRight = -1;
    for (let i = 0; i < menu.right.length; i++)
    {
      if (menu.right[i].value == strMenu)
      {
        nActiveRight = i;
        menu.right[i].active = 'active';
        this.activeMenu = strMenu;
      }
      else
      {
        menu.right[i].active = null;
      }
    }
    if (nActiveLeft >= 0 && menu.left[nActiveLeft].submenu)
    {
      submenu = menu.left[nActiveLeft].submenu;
      for (let i = 0; i < submenu.left.length; i++)
      {
        if (submenu.left[i].value == strSubMenu)
        {
          submenu.left[i].active = 'active';
        }
        else
        {
          submenu.left[i].active = null;
        }
      }
      for (let i = 0; i < submenu.right.length; i++)
      {
        if (submenu.right[i].value == strSubMenu)
        {
          submenu.right[i].active = 'active';
        }
        else
        {
          submenu.right[i].active = null;
        }
      }
    }
    else if (nActiveRight >= 0 && menu.right[nActiveRight].submenu)
    {
      submenu = menu.right[nActiveRight].submenu;
      for (let i = 0; i < submenu.left.length; i++)
      {
        if (submenu.left[i].value == strSubMenu)
        {
          submenu.left[i].active = 'active';
        }
        else
        {
          submenu.left[i].active = null;
        }
      }
      for (let i = 0; i < submenu.right.length; i++)
      {
        if (submenu.right[i].value == strSubMenu)
        {
          submenu.right[i].active = 'active';
        }
        else
        {
          submenu.right[i].active = null;
        }
      }
    }
    else
    {
      submenu = false;
    }
    this.menu = menu;
    this.submenu = submenu;
    this.load('menu');
  }
  // }}}
  // {{{ setRedirectPath()
  setRedirectPath(strPath)
  {
    this.m_strRedirectPath = strPath;
  }
  // }}}
  // {{{ setRequestPath()
  setRequestPath(strPath)
  {
    this.m_strRequestPath = strPath;
  }
  // }}}
  // {{{ setSecureLogin()
  setSecureLogin(bSecureLogin)
  {
    this.m_bSecureLogin = bSecureLogin;
    if (bSecureLogin && document.location.protocol !== 'https:')
    {
      document.location.protocol = 'https:';
    }
  };
  // }}}
  // {{{ setSecurity()
  setSecurity(strLoginType)
  {
    this.m_strLoginType = strLoginType;
  };
  // }}}
  // {{{ simplify()
  simplify(data)
  {
    let simple = null;

    if (this.isObservable(data) || this.isComputed(data))
    {
      simple = this.simplify(data.v);
    }
    else if (this.isArray(data))
    {
      simple = [];
      for (let i = 0; i < data.length; i++)
      {
        simple[i] = this.simplify(data[i]);
      }
    }
    else if (this.isObject(data))
    {
      simple = {};
      for (let key of Object.keys(data))
      {
        simple[key] = this.simplify(data[key]);
      }
    }
    else
    {
      simple = data;
    }

    return simple;
  }
  // }}}
  // {{{ store()
  store(controller, data)
  {
    if (!this.existStore(controller))
    {
      this.putStore(controller, data);
    }

    return this.getStore(controller);
  }
  // }}}
  // {{{ tableSort()
  tableSort(strID, nCol, bNumeric)
  {
    let table, tableID, rows, switching, i, x, xn, y, yn, shouldSwitch, dir, switchcount = 0;
    tableID = document.getElementById(strID);
    table = tableID.getElementsByTagName('tbody')[0];
    switching = true;
    dir = 'asc';
    while (switching)
    {
      switching = false;
      rows = table.getElementsByTagName('tr');
      for (i = 0; i < (rows.length - 1); i++)
      {
        shouldSwitch = false;
        x = rows[i].getElementsByTagName('TD')[nCol].innerHTML.toLowerCase();
        y = rows[i+1].getElementsByTagName('TD')[nCol].innerHTML.toLowerCase();
        if (bNumeric)
        {
          x = x.replace('$', '');
          y = y.replace('$', '');
          x = x.replace(',', '');
          y = y.replace(',', '');
          x = x.replace('%', '');
          y = y.replace('%', '');
          if (x.length > 1)
          {
            if (x[x.length-1] == 'k')
            {
              x = Number(x.substr(0, (x.length - 1)));
              x *= 1000;
            }
            else if (x[x.length-1] == 'm')
            {
              x = Number(x.substr(0, (x.length - 1)));
              x *= 1000000;
            }
            else if (x[x.length-1] == 'b' || x[x.length-1] == 'g')
            {
              x = Number(x.substr(0, (x.length - 1)));
              x *= 1000000000;
            }
            else if (x[x.length-1] == 't')
            {
              x = Number(x.substr(0, (x.length - 1)));
              x *= 1000000000000;
            }
          }
          if (y.length > 1)
          {
            if (y[y.length-1] == 'k')
            {
              y = Number(y.substr(0, (y.length - 1)));
              y *= 1000;
            }
            else if (y[y.length-1] == 'm')
            {
              y = Number(y.substr(0, (y.length - 1)));
              y *= 1000000;
            }
            else if (y[y.length-1] == 'b' || y[y.length-1] == 'g')
            {
              y = Number(y.substr(0, (y.length - 1)));
              y *= 1000000000;
            }
            else if (y[y.length-1] == 't')
            {
              y = Number(y.substr(0, (y.length - 1)));
              y *= 1000000000000;
            }
          }
          x = Number(x);
          y = Number(y);
        }
        if (dir == 'asc')
        {
          if (x > y)
          {
            shouldSwitch = true;
            break;
          }
        }
        else if (dir == 'desc')
        {
          if (x < y)
          {
            shouldSwitch = true;
            break;
          }
        }
      }
      if (shouldSwitch)
      {
        rows[i].parentNode.insertBefore(rows[i + 1], rows[i]);
        switching = true;
        switchcount ++;
      }
      else
      {
        if (switchcount == 0 && dir == 'asc')
        {
          dir = 'desc';
          switching = true;
        }
      }
    }
  }
  // }}}
  // {{{ update()
  update(name)
  {
    if (this.name == name)
    {
      this.render(this.id, this.name, this.component);
    }
  }
  // }}}
  // {{{ wsCreate()
  wsCreate(strName, strServer, strPort, bSecure, strProtocol)
  {
    let bResult = false;

    if (this.application.length > 0 && strName.length > 0 && strPort.length > 0 && strProtocol.length > 0 && (bSecure == true || bSecure == false) && strServer.length > 0)
    {
      let date = new Date();
      if (!this.isDefined(this.m_ws[strName]))
      {
        this.m_ws[strName] = {};
      }
      this.m_ws[strName].Application = this.application;
      this.m_ws[strName].Attempts = 1;
      this.m_ws[strName].Connected = false;
      this.m_ws[strName].Port = strPort;
      this.m_ws[strName].Protocol = strProtocol;
      if (this.m_strAuthProtocol == null || strProtocol == 'radial')
      {
        this.m_strAuthProtocol = strProtocol;
      }
      this.m_ws[strName].Secure = bSecure;
      this.m_ws[strName].Server = strServer;
      this.m_ws[strName].ws = new WebSocket(((bSecure)?'wss':'ws')+'://'+strServer+':'+strPort, [strProtocol]);
      // {{{ WebSocket::onopen()
      this.m_ws[strName].ws.onopen = (e) =>
      {
        this.m_ws[strName].Connected = true;
        this.dispatchEvent('commonWsReady', null);
        this.dispatchEvent('commonWsReady_' + this.application, null);
        this.dispatchEvent('commonWsReady_' + this.application + '_' + strName, null);
        this.dispatchEvent('commonWsReady_' + strName, null);
        this.m_ws[strName].Attempts = 1;
        if (this.m_ws[strName].modalInstance != null)
        {
          this.m_ws[strName].modalInstance.close();
          this.m_ws[strName].modalInstance = null;
        }
        this.auth();
      };
      // }}}
      // {{{ WebSocket::onclose()
      this.m_ws[strName].ws.onclose = (e) =>
      {
        let maxInterval = (Math.pow(2, this.m_ws[strName].Attempts) - 1) * 1000;
        console.log(e);
        this.m_bConnecting = false;
        this.m_bSentJwt = false;
        this.m_wsRequestID = null;
        this.dispatchEvent('commonWsClose_' + this.application, e);
        this.dispatchEvent('commonWsClose_' + this.application + '_' + strName, e);
        if (this.m_ws[strName].Connected)
        {
          this.m_ws[strName].Connected = false;
        }
        if (maxInterval > 30 * 1000)
        {
          maxInterval = 30 * 1000;
        }
        setTimeout(() =>
        {
          this.m_ws[strName].Attempts++;
          this.wsCreate(strName, strServer, strPort, bSecure, strProtocol);
        }, (Math.random() * maxInterval));
      };
      // }}}
      // {{{ WebSocket::onerror()
      this.m_ws[strName].ws.onerror = (e) =>
      {
        console.log(e);
        this.dispatchEvent('commonWsError_'+this.application, e);
        this.dispatchEvent('commonWsError_'+this.application+'_'+strName, e);
      };
      // }}}
      // {{{ WebSocket::onmessage()
      this.m_ws[strName].ws.onmessage = (e) =>
      {
        let response = JSON.parse(e.data);
        if (this.m_bJwt && strProtocol == 'radial' && response.Error && response.Error == 'Failed: exp')
        {
          response.Error = 'Session expired.  Please login again.';
          this.m_auth = null;
          this.m_auth = {admin: false, apps: {}};
          this.m_bSentJwt = false;
          window.localStorage.removeItem('sl_wsJwt');
          window.localStorage.removeItem('sl_uniqueID');
          delete response.Error;
          delete response.wsJwt;
          delete response.wsRequestID;
          delete response.Response;
          delete response.Status;
          this.dispatchEvent('commonAuthReady', null);
          this.dispatchEvent('resetMenu', null);
        }
        else if (!this.m_bJwt && strProtocol == 'radial' && this.m_ws[strName].SetRadialCredentials && this.m_ws[strName].SetRadialCredentials.Script && this.m_ws[strName].SetRadialCredentials.Script.length > 0 && this.m_ws[strName].SetRadialCredentials['Function'] && this.m_ws[strName].SetRadialCredentials['Function'].length > 0 && response.Status && response.Status == 'error' && response.Error && (response.Error == 'Your access has been denied.' || response.Error == 'Please provide the User.'))
        {
          if (this.isCookie('PHPSESSID'))
          {
            if (strProtocol == 'radial' && !this.m_bSetRadialCredentials)
            {
              this.m_bSetRadialCredentials = true;
              fetch(this.m_ws[strName].SetRadialCredentials['Script'],
              {
                method: 'POST',
                headers:
                {
                  'Content-Type': 'application/json'
                },
                body: JSON.stringify({'Function': this.m_ws[strName].SetRadialCredentials['Function']})
              })
              .then(response => response.json())
              .then((response) =>
              {
                this.m_bSetRadialCredentials = false;
              });
            }
            delete response.Error;
            delete response.wsRequestID;
            delete response.Response;
            delete response.Status;
            response.wsSessionID = this.getCookie('PHPSESSID');
            this.m_ws[strName].ws.send(JSON.stringify(response));
          }
          else if (this.isValid())
          {
            alert('Your session has expired.  Please login again.');
            document.location.href = '#/Logout';
          }
        }
        else
        {
          if (this.isDefined(response.wsController))
          {
            this.dispatchEvent(response.wsController, response);
          }
          else
          {
            this.dispatchEvent('commonWsMessage_' + this.application, response);
            this.dispatchEvent('commonWsMessage_' + this.application + '_' + strName, response);
          }
          if (response.wsRequestID && response.wsRequestID == this.m_wsRequestID && response.Action)
          {
            if (response.Action == 'alert')
            {
              if (response.Message)
              {
                alert(response.Message);
              }
            }
            else if (response.Action == 'audio')
            {
              if ('Audio' in window)
              {
                if (response.Media)
                {
                  let audio = new Audio(response.Media);
                  audio.play();
                }
              }
            }
            else if (response.Action == 'location')
            {
              if (response.Location)
              {
                document.location.href = response.Location;
              }
            }
            else if (response.Action == 'message')
            {
              let date = null;
              if (this.isDefined(response.Time) && response.Time != '')
              {
                date = new Date(response.Time * 1000);
              }
              else
              {
                date = new Date();
              }
              response.Time = date.getFullYear() + '-' + (date.getMonth() + 1) + '-' + date.getDate() + ' ' + date.getHours() + ':' + date.getMinutes() + ':' + date.getSeconds();
              this.m_messages.push(response);
              this.m_messages[this.m_messages.length - 1].Index = (this.m_messages.length - 1);
              this.render('messages', this.autoLoads['messages']);
            }
            else if (response.Action == 'notification' && response.Title && response.Body && 'Notification' in window)
            {
              Notification.requestPermission(function (permission)
              {
                let nDuration = 5000;
                if (response.Duration)
                {
                  nDuration = response.Duration * 1000;
                }
                let notification = new Notification(response.Title, {body: response.Body, icon: null, dir: 'auto'});
                setTimeout(() =>
                {
                  notification.close();
                }, nDuration);
              });
            }
          }
        }
        if (response.radialPurpose && response.radialPurpose == 'status')
        {
          this.dispatchEvent('radialPurpose_status', response);
        }
      };
      // }}}
    }

    return bResult;
  }
  // }}}
  // {{{ wsRequest()
  wsRequest(strName, request)
  {
    let deferred = {};
    let promise = new Promise(function (resolve, reject)
    {
      deferred.resolve = resolve;
      deferred.reject = reject;
    });
    deferred.promise = promise;
    if (this.isDefined(this.m_ws[strName]))
    {
      let unHandle = this.wsUnique(strName);
      this.wsSend(strName, unHandle, request);
      this.attachEvent(unHandle, (data) =>
      {
        deferred.resolve(data.detail);
      });
    }

    return deferred.promise;
  }
  // }}}
  // {{{ wsResponse()
  wsResponse(response, error)
  {
    let bResult = false;

    if (response.Status && response.Status == 'okay')
    {
      bResult = true;
    }
    else if (error != null)
    {
      if (response.Error)
      {
        if (response.Error != 'Please provide the wsJwt.')
        {
          error.message = response.Error;
        }
      }
      else
      {
        error.message = 'Encountered an unknown error.';
      }
    }

    return bResult;
  }
  // }}}
  // {{{ wsSend()
  wsSend(strName, strController, request)
  {
    if (this.isDefined(this.m_ws[strName]) && this.isDefined(this.m_ws[strName].Connected) && this.m_ws[strName].Connected)
    {
      request.reqApp = this.application;
      request.wsController = strController;
      if (this.m_bJwt && (this.m_bJwtInclusion || !this.m_bSentJwt))
      {
        if (window.localStorage.getItem('sl_wsJwt'))
        {
          this.m_bSentJwt = true;
          request.wsJwt = window.localStorage.getItem('sl_wsJwt');
        }
      }
      if (this.isDefined(this.m_wsSessionID))
      {
        if (!this.isCookie('PHPSESSID'))
        {
          this.setCookie('PHPSESSID', this.m_wsSessionID);
        }
      }
      else if (this.isCookie('PHPSESSID'))
      {
        this.m_wsSessionID = this.getCookie('PHPSESSID');
      }
      request.wsSessionID = this.m_wsSessionID;
      this.m_ws[strName].ws.send(JSON.stringify(request));
    }
  }
  // }}}
  // {{{ wsSetBridgeCredentials()
  wsSetBridgeCredentials(strName, strScript, strFunction)
  {
    if (!this.isDefined(this.m_ws[strName]))
    {
      this.m_ws[strName] = {};
    }
    this.m_ws[strName].SetBridgeCredentials = {Script: strScript, 'Function': strFunction};
  }
  // }}}
  // {{{ wsSetRadialCredentials()
  wsSetRadialCredentials(strName, strScript, strFunction)
  {
    if (!this.isDefined(this.m_ws[strName]))
    {
      this.m_ws[strName] = {};
    }
    this.m_ws[strName].SetRadialCredentials = {Script: strScript, 'Function': strFunction};
  }
  // }}}
  // {{{ wsUnique()
  wsUnique()
  {
    let nUnique = 0;

    nUnique = this.m_nUnique++;

    return nUnique;
  }
  // }}}
}
// }}}
// {{{ Observable
class Observable
{
  // {{{ constructor()
  constructor(val)
  {
    this.listeners = {};
    this.val = ((typeof val !== 'undefined')?val:'');
  }
  // }}}
  // {{{ e()
  e()
  {
    let r = null;
    if (typeof this.id !== 'undefined')
    {
      r = document.getElementById(this.id);
    }
    return r;
  }
  // }}}
  // {{{ id()
  id(v)
  {
    this.id = v;
  }
  // }}}
  // {{{ notify()
  notify()
  {
    for (let [name, listener] of Object.entries(this.listeners))
    {
      listener(this.val);
    }
  }
  // }}}
  // {{{ subscribe()
  subscribe(name, listener)
  {
    if (typeof this.listeners[name] !== 'undefined')
    {
      delete this.listeners[name];
    }
    this.listeners[name] = listener;
  }
  // }}}
  // {{{ get v()
  get v()
  {
    return this.value;
  }
  // }}}
  // {{{ get value()
  get value()
  {
    return this.val;
  }
  // }}}
  // {{{ set v()
  set v(val)
  {
    this.value = val;
  }
  // }}}
  // {{{ set value()
  set value(val)
  {
    if (this.val !== val)
    {
      this.val = val;
      this.notify();
    }
  }
  // }}}
}
// }}}
// {{{ Computed
class Computed extends Observable
{
  // {{{ constructor()
  constructor(val, deps)
  {
    super(val());
    const listener = () =>
    {
      this.val = val();
      this.notify();
    }
    deps.forEach(dep => dep.subscribe('computed', listener));
  }
  // }}}
  // {{{ e()
  e()
  {
    let r = null;
    if (typeof this.id !== 'undefined')
    {
      r = document.getElementById(this.id);
    }
    return r;
  }
  // }}}
  // {{{ id()
  id(v)
  {
    this.id = v;
  }
  // }}}
  // {{{ get v()
  get v()
  {
    return this.value;
  }
  // }}}
  // {{{ get value()
  get value()
  {
    return this.val;
  }
  // }}}
  // {{{ set v()
  set v(val)
  {
    this.value = val;
  }
  // }}}
  // {{{ set value()
  set value(val)
  {
    throw "Cannot set computed property";
  }
  // }}}
}
// }}}
