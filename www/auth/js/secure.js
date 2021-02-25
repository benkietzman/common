function bk_login_load()
{
  if (document.bk_login_formlogin)
  {
    document.bk_login_formlogin.bk_login_user_id.focus();
  }
  else if (document.bk_login_formchange)
  {
    document.bk_login_formchange.bk_login_oldpassword.focus();
  }
}

function bk_login_checkchange()
{
  if (document.bk_login_formchange.bk_login_oldpassword.value.length <= 0)
  {
    alert("Please enter an old Pasword.");
    return false;
  }
  else if (document.bk_login_formchange.bk_login_password.value.length <= 0)
  {
    alert("Please enter a new Pasword.");
    return false;
  }
  else if (document.bk_login_formchange.bk_login_confirmpassword.value.length <= 0)
  {
    alert("Please enter a confirming Pasword.");
    return false;
  }
  else if (document.bk_login_formchange.bk_login_oldpassword == document.bk_login_formchange.bk_login_password)
  {
    alert("Old password and new password cannot be the same.");
    return false;
  }
  return true;
}

function bk_login_checklogin()
{
  if (document.bk_login_formlogin.bk_login_user_id.value.length <= 0)
  {
    alert("Please enter a valid User ID.");
    return false;
  }
  else if (document.bk_login_formlogin.bk_login_password && document.bk_login_formlogin.bk_login_password.value.length <= 0)
  {
    alert("Please enter a Pasword.");
    return false;
  }
  return true;
}

function bk_login_changesite()
{
  document.bk_login_formsite.bk_login_action.value = "sitechange";
  document.bk_login_formsite.submit();
}

function bk_login_changeaccount()
{
  document.bk_login_formmodify.bk_login_action.value = "accountchange";
  document.bk_login_formmodify.submit();
}

function bk_login_removeaccount()
{
  if (document.bk_login_formmodify.bk_login_selectaccount.selectedIndex > 0)
  {
    document.bk_login_formmodify.bk_login_action.value = "remove";
    document.bk_login_formmodify.submit();
  }
}

function bk_login_adminchangeaccount()
{
  document.bk_login_formadmin.bk_login_action.value = "adminchange";
  document.bk_login_formadmin.submit();
}

function bk_login_adminremoveaccount()
{
  if (document.bk_login_formadmin.bk_login_selectaccount.selectedIndex > 0)
  {
    document.bk_login_formadmin.bk_login_action.value = "adminremove";
    document.bk_login_formadmin.submit();
  }
}
