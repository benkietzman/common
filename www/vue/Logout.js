// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2021-10-12
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
export default
{
  // {{{ data
  data()
  {
    let data =
    {
    };
    common.menuSet('Logout as '+common.getFirstName(), null);
    return data;
  },
  // }}}
  // {{{ template
  template: `
    <div>
      <br><br><br>
      <div v-show="message" style="color:red;font-weight:bold;"><br><br>{{message}}<br><br></div>
      <div v-show="info" style="color:orange;"><br><br>{{info}}<br><br></div>
    </div>
  `
  // }}}
}
