// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Storage
// -------------------------------------
// file       : Storage.cpp
// author     : Ben Kietzman
// begin      : 2021-04-07
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

/*! \file Storage.cpp
* \brief Storage Class
*
* Provides storage functionality.
*/
// {{{ includes
#include "Storage"
// }}}
extern "C++"
{ 
  namespace common
  {
    // {{{ Storage()
    Storage::Storage()
    {
      lock();
      m_ptStorage = new Json;
      unlock();
    }
    // }}}
    // {{{ ~Storage()
    Storage::~Storage()
    {
      lock();
      delete m_ptStorage;
      unlock();
    }
    // }}}
    // {{{ add()
    bool Storage::add(list<string> keys, Json *ptData, string &strError)
    {
      bool bResult = false;

      if (request("add", keys, ptData, strError))
      {
        bResult = true;
      }

      return bResult;
    }
    // }}}
    // {{{ get()
    Json *Storage::get()
    {
      Json *ptStorage;

      lock();
      ptStorage = new Json(m_ptStorage);
      unlock();

      return ptStorage;
    }
    // }}}
    // {{{ lock()
    void Storage::lock()
    {
      m_mutexStorage.lock();
    }
    // }}}
    // {{{ ptr()
    Json *Storage::ptr()
    {
      return m_ptStorage;
    }
    // }}}
    // {{{ put()
    void Storage::put(Json *ptStorage)
    {
      lock();
      delete m_ptStorage;
      m_ptStorage = new Json(ptStorage);
      unlock();
    }
    // }}}
    // {{{ remove()
    bool Storage::remove(list<string> keys, string &strError)
    {
      bool bResult = false;

      if (request("remove", keys, NULL, strError))
      {
        bResult = true;
      }

      return bResult;
    }
    // }}}
    // {{{ request()
    bool Storage::request(const string strAction, list<string> keys, Json *ptData, string &strError)
    {
      bool bResult = false;
      Json *ptCurrent;

      lock();
      ptCurrent = m_ptStorage;
      // {{{ add or update
      if (strAction == "add" || strAction == "update")
      {
        if (ptData != NULL)
        {
          bResult = true;
          if (keys.empty())
          {
            if (strAction == "add")
            {
              m_ptStorage->clear();
            }
          }
          else
          {
            for (list<string>::iterator j = keys.begin(); j != keys.end(); j++)
            {
              list<string>::iterator nextIter = j;
              nextIter++;
              if (ptCurrent->m.find(*j) == ptCurrent->m.end())
              {
                ptCurrent->m[*j] = new Json;
              }
              if (nextIter == keys.end() && strAction == "add")
              {
                delete ptCurrent->m[*j];
                ptCurrent->m[*j] = new Json;
              }
              if (ptCurrent->m[*j]->t == 's')
              {
                ptCurrent->m[*j]->v.clear();
              }
              ptCurrent = ptCurrent->m[*j];
            }
          }
          ptCurrent->merge(ptData, false, false);
        }
        else
        {
          strError = "Please provide valid Data.";
        }
      }
      // }}}
      // {{{ remove
      else if (strAction == "remove")
      {
        bResult = true;
        for (list<string>::iterator j = keys.begin(); bResult && j != keys.end(); j++)
        {
          list<string>::iterator nextIter = j;
          nextIter++;
          if (ptCurrent->m.find(*j) != ptCurrent->m.end())
          {
            if (nextIter == keys.end())
            {
              delete ptCurrent->m[*j];
              ptCurrent->m.erase(*j);
            }
            else
            {
              ptCurrent = ptCurrent->m[*j];
            } 
          }
          else
          {
            bResult = false;
          } 
        }
        if (!bResult)
        {
          strError = "Failed to find key.";
        } 
      } 
      // }}}
      // {{{ retrieve
      else if (strAction == "retrieve")
      {
        bResult = true;
        if (!keys.empty())
        {
          bool bFound = false;
          for (list<string>::iterator j = keys.begin(); bResult && !bFound && j != keys.end(); j++)
          {
            list<string>::iterator nextIter = j;
            nextIter++;
            if (ptCurrent->m.find(*j) != ptCurrent->m.end())
            {
              if (nextIter == keys.end())
              {
                bFound = true;
                ptData->merge(ptCurrent->m[*j], true, false);
              }
              else
              {
                ptCurrent = ptCurrent->m[*j];
              }
            }
            else
            {
              bResult = false;
            }
          }
          if (!bResult)
          {
            strError = "Failed to find key.";
          }
        }
        else
        {
          ptData->merge(ptCurrent, true, false);
        }
      }
      // }}}
      // {{{ retrieveKeys
      else if (strAction == "retrieveKeys")
      {
        bResult = true;
        if (!keys.empty())
        {
          bool bFound = false;
          for (list<string>::iterator j = keys.begin(); bResult && !bFound && j != keys.end(); j++)
          {
            list<string>::iterator nextIter = j;
            nextIter++;
            if (ptCurrent->m.find(*j) != ptCurrent->m.end())
            {
              if (nextIter == keys.end())
              {
                Json *ptKeys = new Json;
                bFound = true;
                for (map<string, Json *>::iterator k = ptCurrent->m[*j]->m.begin(); k != ptCurrent->m[*j]->m.end(); k++)
                {
                  ptKeys->push_back(k->first);
                }
                ptData->merge(ptKeys, true, false);
                delete ptKeys;
              }
              else
              {
                ptCurrent = ptCurrent->m[*j];
              }
            }
            else
            {
              bResult = false;
            }
          }
          if (!bResult)
          {
            strError = "Failed to find key.";
          }
        }
        else
        {
          Json *ptKeys = new Json;
          for (map<string, Json *>::iterator j = ptCurrent->m.begin(); j != ptCurrent->m.end(); j++)
          {
            ptKeys->push_back(j->first);
          }
          ptData->merge(ptKeys, true, false);
          delete ptKeys;
        }
      }
      // }}}
      // {{{ invalid
      else
      {
        strError = "Please provide a valid Action.";
      }
      // }}}
      unlock();

      return bResult;
    }
    // }}}
    // {{{ retrieve()
    bool Storage::retrieve(list<string> keys, Json *ptData, string &strError)
    {
      bool bResult = false;

      if (request("retrieve", keys, ptData, strError))
      {
        bResult = true;
      }

      return bResult;
    }
    // }}}
    // {{{ retrieveKeys()
    bool Storage::retrieveKeys(list<string> keys, Json *ptData, string &strError)
    {
      bool bResult = false;

      if (request("retrieveKeys", keys, ptData, strError))
      {
        bResult = true;
      }

      return bResult;
    }
    // }}}
    // {{{ unlock()
    void Storage::unlock()
    {
      m_mutexStorage.unlock();
    }
    // }}}
    // {{{ update()
    bool Storage::update(list<string> keys, Json *ptData, string &strError)
    {
      bool bResult = false;

      if (request("update", keys, ptData, strError))
      {
        bResult = true;
      }

      return bResult;
    }
    // }}}
  }
}
