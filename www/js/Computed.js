// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-11-04
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////

class Computed extends Observable
{
  // {{{ constructor()
  constructor(value, deps)
  {
    super(value());
    const listener = () =>
    {
      this.v = value();
      this.notify();
    }
    deps.forEach(dep => dep.subscribe(listener));
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
    throw "Cannot set computed property";
  }
  // }}}
}
