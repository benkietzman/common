// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-11-09
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
export default
{
  load(id, t)
  {
    let d =
    {
    };

    return d;
  },
  template: `
    <div v-for="message in getMessages()" :class="'alert alert-' + message.Class + ' alert-dismissible fade in'">
      <button class="close" data-dismiss="alert" aria-label="close" v-on:click="close(message.Index)">&times;</button>
      <strong v-show="message.Title">{{message.Title}}<br></strong>
      {{message.Body}}
      <br>
      <div class="pull-right"><i>{{message.Time}}</i></div>
    </div>
  `
}
