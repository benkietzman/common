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
  constructor(value)
  {
    this.listeners = [];
    this.v = value;
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
  set value(val)
  {
    if (val !== this.v)
    {
      this.v = val;
      this.notify();
    }
  }
  // }}}
}
