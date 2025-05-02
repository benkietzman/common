///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2015-08-06
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////

var filters = {};
// {{{ commonCapitalize
filters.commonCapitalize = function()
{
  return function(input)
  {
    return (angular.isString(input) && input.length > 0) ? input.charAt(0).toUpperCase() + input.substr(1).toLowerCase() : input;
  }
};
// }}}
// {{{ commonHtml
filters.commonHtml = function ($sce)
{     
  return function (buffer)
  {
    return $sce.trustAsHtml(buffer);
  };
};
// }}}
// {{{ commonNewline
filters.commonNewline = function ()
{     
  return function (buffer)
  {
    return buffer.replace(/\n/g, '<br>');
  };
};
// }}}
// {{{ commonOrderObjectBy
filters.commonOrderObjectBy = function ()
{     
  return function(items, field, reverse)
  {
    var filtered = [];

    angular.forEach(items, function(item)
    {
      filtered.push(item);
    });
    filtered.sort(function (a, b)
    {
      return (a[field] > b[field] ? 1 : -1);
    });
    if (reverse)
    {
      filtered.reverse();
    }

    return filtered;
  };
};
// }}}
// {{{ commonRemoveSpaces
filters.commonRemoveSpaces = function()
{
  return function(input)
  {
    return (angular.isString(input) && input.length > 0) ? input.replace(/ /g, '') : input;
  }
};
// }}}
// {{{ commonTelephone
filters.commonTelephone = function ()
{     
  return function (tel)
  { 
    if (!tel)
    {
      return '';
    }
    var value = tel.toString().trim().replace(/^\+/, '');
    if (value.match(/[^0-9]/))
    {
      return tel;
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
        return tel;
    }
    if (country == 1) 
    {
      country = "";
    }
    number = number.slice(0, 3) + '-' + number.slice(3);
    return (country + " (" + city + ") " + number).trim();
  };
};
// }}}
// {{{ commonURLEncode
filters.commonURLEncode = function()
{
  return window.encodeURIComponent;
};
// }}}
angular.module('commonFilters', []).filter(filters);
