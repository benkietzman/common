// vim: syntax=cpp
// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Json
// -------------------------------------
// file       : Json.cpp
// author     : Ben Kietzman
// begin      : 2011-07-06
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
**************************************************************************/

/*! \file Json.cpp
* \brief Json Functions
*/
// {{{ includes
#include "Xml"
// }}}
extern "C++"
{
  namespace common
  {
    unsigned long long gullUniqueXml = 0;
    // {{{ cleanTicketList()
    void xmlCleanTicketList(struct xmlItem &tItem)
    {
      tItem.strData = "";
      if (!tItem.child.empty())
      {
        for (map<string, struct xmlItem>::iterator i = tItem.child.begin(); i != tItem.child.end(); i++)
        {
          xmlCleanTicketList(i->second);
        }
        tItem.child.clear();
      }
    }
    // }}}
    // {{{ xmlData()
    void xmlData(void *data, const XML_Char *s, int len)
    {
      struct xmlItem *ptItem = (struct xmlItem *)data;
      ptItem->ptCurrent->strData.append(s, len);
    }
    // }}}
    // {{{ xmlEnd()
    void xmlEnd(void *data, const char *el)
    {
      struct xmlItem *ptItem = (struct xmlItem *)data;
      ptItem->ptCurrent = ptItem->ptCurrent->ptParent;
    }
    // }}}
    // {{{ xmlStart()
    void xmlStart(void *data, const char *el, const char *attr[])
    {
      struct xmlItem *ptItem = (struct xmlItem *)data;
      stringstream ssKey;
      if (ptItem->ptCurrent->child.find(el) != ptItem->ptCurrent->child.end())
      {
        ssKey << setw(32) << setfill('0') << gullUniqueXml++ << "#";
      }
      ssKey << el;
      for (int i = 0; attr[i]; i += 2)
      {
        ptItem->ptCurrent->child[ssKey.str()].attr[attr[i]] = attr[i+1];
      }
      ptItem->ptCurrent->child[ssKey.str()].ptParent = ptItem->ptCurrent;
      ptItem->ptCurrent = &ptItem->ptCurrent->child[ssKey.str()];
    }
    // }}}
  }
}
