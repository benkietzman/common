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
    app.$refs.menu.set('Login', null);
    return data;
  },
  // }}}
  // {{{ template
  template: `
    <div>
      <br><br><br>
      <div v-show="message" style="color:red;font-weight:bold;"><br><br>{{message}}<br><br></div>
      <div v-show="info" style="color:orange;"><br><br>{{info}}<br><br></div>
      <div v-show="showForm" class="row" style="width:50%;">
        <h3 class="page-header">{{login.title}}</h3>
        <div class="col-md-5" style="padding:10px;"><input class="form-control" type="text" id="login_userid" v-model="login.userid" maxlength="20" v-on:keyup="$event.keyCode == 13 && submit()" placeholder="User"></div>
        <div class="col-md-5" style="padding:10px;"><input class="form-control" type="password" v-model="login.password" maxlength="64" v-on:keyup="$event.keyCode == 13 && submit()" placeholder="Password"></div>
        <div class="col-md-2" style="padding:10px;"><button class="btn btn-primary" v-on:click="submit()" style="float:right;">Login</button></div>
      </div>
    </div>
  `
  // }}}
}
