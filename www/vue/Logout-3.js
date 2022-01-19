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
  data() {return {common: common};},
  // }}}
  // {{{ mounted()
  mounted()
  {
    common.setMenu('Logout as '+common.getFirstName(), null);
    common.processLogout();
  },
  // }}}
  // {{{ template
  template: `
    <div>
      <br><br><br>
      <div v-show="common.logout.message" style="color:red;font-weight:bold;"><br><br>{{common.logout.message}}<br><br></div>
      <div v-show="common.logout.info" style="color:orange;"><br><br>{{common.logout.info}}<br><br></div>
    </div>
  `
  // }}}
}
