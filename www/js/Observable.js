// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-11-04
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////

class Observable
{
  // {{{ constructor()
  constructor()
  {
    this.listeners = [];
    this.v = null;
  }
  // }}}
  // {{{ notify()
  notify()
  {
    this.listeners.forEach(listener => listener(this.v));
  }
  // }}}
  // {{{ subscribe()
  subscribe(listener)
  {
    this.listeners.push(listener);
  }
  // }}}
  // {{{ get value()
  get value()
  {
    return this.v;
  }
  // }}}
  // {{{ set value()
  set value(v)
  {
    if (v !== this.v)
    {
      this.v = v;
      this.notify();
    }
  }
  // }}}
}
