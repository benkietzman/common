// vim: syntax=cpp
// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Warden
// -------------------------------------
// file       : Warden.cpp
// author     : Ben Kietzman
// begin      : 2021-04-08
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

/*! \file Warden.cpp
* \brief Warden Class
*/
// {{{ includes
#include "Warden"
// }}}
extern "C++"
{
  namespace common
  {
    // {{{ Warden()
    Warden::Warden(const string strUnix, string &strError)
    {
      m_bUseSingleSocket = false;
      m_strUnix = strUnix;
      m_unUniqueID = 0;
      m_pUtility = new Utility(strError);
    }
    // }}}
    // {{{ ~Warden()
    Warden::~Warden()
    {
      if (m_bUseSingleSocket)
      {
        useSingleSocket(false);
      }
      delete m_pUtility;
    }
    // }}}
    // {{{ request()
    bool Warden::request(Json *ptRequest, Json *ptResponse, string &strError)
    {
      return request(ptRequest, ptResponse, 0, strError);
    }
    bool Warden::request(Json *ptRequest, Json *ptResponse, time_t CTimeout, string &strError)
    {
      bool bResult = false;
      time_t CEnd, CStart;

      time(&CStart);
      if (CTimeout <= 0)
      {
        if (!m_strTimeout.empty())
        {
          CTimeout = atoi(m_strTimeout.c_str());
        }
        else
        {
          CTimeout = 600;
        }
      }
      if (m_bUseSingleSocket)
      {
        int readpipe[2] = {-1, -1};
        if (pipe(readpipe) == 0)
        {
          bool bExit = false;
          char szBuffer[65536];
          int nReturn;
          size_t unPosition, unUniqueID = 0;
          string strBuffer;
          stringstream ssUniqueID;
          Json *ptJson = new Json(ptRequest);
          m_mutexUnique.lock();
          while (m_requests.find(m_unUniqueID) != m_requests.end())
          {
            m_unUniqueID++;
          }
          unUniqueID = m_unUniqueID++;
          m_mutexUnique.unlock();
          ssUniqueID << unUniqueID;
          ptJson->insert("ProcessType", "parallel");
          ptJson->insert("wardenUniqueID", ssUniqueID.str(), 'n');
          ptJson->json(strBuffer);
          strBuffer.append("\n");
          delete ptJson;
          wardenreqdata *ptData = new wardenreqdata;
          ptData->bSent = false;
          ptData->fdUnix = readpipe[1];
          ptData->strBuffer[0] = strBuffer;
          m_mutexRequests.lock();
          m_requests[unUniqueID] = ptData;
          m_mutexRequests.unlock();
          strBuffer.clear();
          while (!bExit)
          {
            pollfd fds[1];
            fds[0].fd = readpipe[0];
            fds[0].events = POLLIN;
            if ((nReturn = poll(fds, 1, 250)) > 0)
            {
              if (fds[0].revents & (POLLHUP | POLLIN))
              {
                if ((nReturn = read(readpipe[0], szBuffer, 65536)) > 0)
                {
                  strBuffer.append(szBuffer, nReturn);
                  if ((unPosition = strBuffer.find("\n")) != string::npos)
                  {
                    string strJson;
                    Json *ptJson = new Json(strBuffer.substr(0, unPosition));
                    if (ptJson->m.find("ProcessType") != ptJson->m.end())
                    {
                      delete ptJson->m["ProcessType"];
                      ptJson->m.erase("ProcessType");
                    }
                    if (ptJson->m.find("wardenUniqueID") != ptJson->m.end())
                    {
                      delete ptJson->m["wardenUniqueID"];
                      ptJson->m.erase("wardenUniqueID");
                    }
                    bResult = true;
                    ptResponse->parse(ptJson->json(strJson));
                    delete ptJson;
                    strBuffer.erase(0, (unPosition + 1));
                  }
                }
                else
                {
                  bExit = true;
                  if (nReturn < 0)
                  {
                    stringstream ssError;
                    ssError << "read(" << errno << ") " << strerror(errno);
                    strError = ssError.str();
                  }
                }
              }
            }
            else if (nReturn < 0)
            {
              bExit = true;
              stringstream ssError;
              ssError << "poll(" << errno << ") " << strerror(errno);
              strError = ssError.str();
            }
            time(&CEnd);
            if ((CEnd - CStart) > CTimeout)
            {
              bExit = true;
              strError = "Timeout expired.";
            }
          }
          close(readpipe[0]);
          m_mutexRequests.lock();
          delete m_requests[unUniqueID];
          m_requests.erase(unUniqueID);
          m_mutexRequests.unlock();
        }
        else
        {
          stringstream ssError;
          ssError << "pipe(" << errno << ") " << strerror(errno);
          strError = ssError.str();
        }
      }
      else
      {
        int fdUnix = -1, nReturn = -1;
        if ((fdUnix = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0)
        {
          sockaddr_un addr;
          memset(&addr, 0, sizeof(sockaddr_un));
          addr.sun_family = AF_UNIX;
          strncpy(addr.sun_path, m_strUnix.c_str(), sizeof(addr.sun_path) - 1);
          if (connect(fdUnix, (sockaddr *)&addr, sizeof(sockaddr_un)) == 0)
          {
            bool bExit = false;
            char szBuffer[65536];
            size_t unPosition;
            string strBuffer[2];
            ptRequest->json(strBuffer[1]);
            strBuffer[1].append("\n");
            while (!bExit)
            {
              pollfd fds[1];
              fds[0].fd = fdUnix;
              fds[0].events = POLLIN;
              if (!strBuffer[1].empty())
              {
                fds[0].events |= POLLOUT;
              }
              if ((nReturn = poll(fds, 1, 250)) > 0)
              {
                if (fds[0].fd == fdUnix && (fds[0].revents & POLLIN))
                {
                  if ((nReturn = read(fdUnix, szBuffer, 65536)) > 0)
                  {
                    strBuffer[0].append(szBuffer, nReturn);
                    if ((unPosition = strBuffer[0].find("\n")) != string::npos)
                    {
                      bExit = bResult = true;
                      ptResponse->parse(strBuffer[0].substr(0, unPosition));
                      strBuffer[0].erase(0, unPosition + 1);
                    }
                  }
                  else
                  {
                    bExit = true;
                    if (nReturn < 0)
                    {
                      stringstream ssError;
                      ssError << "read(" << errno << ") " << strerror(errno);
                      strError = ssError.str();
                    }
                  }
                }
                if (fds[0].fd == fdUnix && (fds[0].revents & POLLOUT))
                {
                  if ((nReturn = write(fdUnix, strBuffer[1].c_str(), strBuffer[1].size())) > 0)
                  {
                    strBuffer[1].erase(0, nReturn);
                  }
                  else
                  {
                    bExit = true;
                    if (nReturn < 0)
                    {
                      stringstream ssError;
                      ssError << "write(" << errno << ") " << strerror(errno);
                      strError = ssError.str();
                    }
                  }
                }
              }
              else if (nReturn < 0)
              {
                stringstream ssError;
                bExit = true;
                ssError << "poll(" << errno << ") " << strerror(errno);
                strError = ssError.str();
              }
              time(&CEnd);
              if ((CEnd - CStart) > CTimeout)
              {
                bExit = true;
                strError = "Timeout expired.";
              }
            }
          }
          else
          {
            stringstream ssError;
            ssError << "connect(" << errno << ") " << strerror(errno);
            strError = ssError.str();
          }
          close(fdUnix);
        }
        else
        {
          stringstream ssError;
          ssError << "socket(" << errno << ") " << strerror(errno);
          strError = ssError.str();
        }
        if (!bResult && strError.empty())
        {
          strError = "Warden request failed without returning an error message.";
        }
      }

      return bResult;
    }
    // }}}
    // {{{ requestThread()
    void Warden::requestThread()
    {
      size_t unSleep = 1;

      while (m_bUseSingleSocket)
      {
        string strError;
        int fdUnix = -1, nReturn = -1;
        if ((fdUnix = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0)
        {
          sockaddr_un addr;
          memset(&addr, 0, sizeof(sockaddr_un));
          addr.sun_family = AF_UNIX;
          strncpy(addr.sun_path, m_strUnix.c_str(), sizeof(addr.sun_path) - 1);
          if (connect(fdUnix, (sockaddr *)&addr, sizeof(sockaddr_un)) == 0)
          {
            bool bExit = false;
            char szBuffer[65536];
            size_t unPosition;
            string strBuffer[2];
            unSleep = 1;
            while (m_bUseSingleSocket && !bExit)
            {
              size_t unIndex = 1;
              pollfd *fds = new pollfd[m_requests.size() + 1];
              m_mutexRequests.lock();
              for (map<int, wardenreqdata *>::iterator i = m_requests.begin(); i != m_requests.end(); i++)
              {
                if (!i->second->bSent)
                {
                  i->second->bSent = true;
                  strBuffer[1] += i->second->strBuffer[0];
                }
                if (i->second->fdUnix != -1 && !i->second->strBuffer[1].empty())
                {
                  fds[unIndex].fd = i->second->fdUnix;
                  fds[unIndex].events = POLLOUT;
                  unIndex++;
                }
              }
              m_mutexRequests.unlock();
              fds[0].fd = fdUnix;
              fds[0].events = POLLIN;
              if (!strBuffer[1].empty())
              {
                fds[0].events |= POLLOUT;
              }
              if ((nReturn = poll(fds, unIndex, 250)) > 0)
              {
                if (fds[0].revents & POLLIN)
                {
                  if ((nReturn = read(fdUnix, szBuffer, 65536)) > 0)
                  {
                    strBuffer[0].append(szBuffer, nReturn);
                    while ((unPosition = strBuffer[0].find("\n")) != string::npos)
                    {
                      Json *ptJson = new Json(strBuffer[0].substr(0, unPosition));
                      if (ptJson->m.find("wardenUniqueID") != ptJson->m.end() && !ptJson->m["wardenUniqueID"]->v.empty())
                      {
                        size_t unUniqueID = atoi(ptJson->m["wardenUniqueID"]->v.c_str());
                        m_mutexRequests.lock();
                        if (m_requests.find(unUniqueID) != m_requests.end())
                        {
                          m_requests[unUniqueID]->strBuffer[1] = strBuffer[0].substr(0, (unPosition + 1));
                        }
                        m_mutexRequests.unlock();
                      }
                      delete ptJson;
                      strBuffer[0].erase(0, (unPosition + 1));
                    }
                  }
                  else
                  {
                    bExit = true;
                  }
                }
                if (fds[0].revents & POLLOUT)
                {
                  if ((nReturn = write(fdUnix, strBuffer[1].c_str(), strBuffer[1].size())) > 0)
                  {
                    strBuffer[1].erase(0, nReturn);
                  }
                  else
                  {
                    bExit = true;
                  }
                }
                if (unIndex > 1)
                {
                  m_mutexRequests.lock();
                  for (map<int, wardenreqdata *>::iterator i = m_requests.begin(); i != m_requests.end(); i++)
                  {
                    for (size_t j = 1; j < unIndex; j++)
                    {
                      if (fds[j].fd == i->second->fdUnix && fds[j].revents & POLLOUT)
                      {
                        if ((nReturn = write(i->second->fdUnix, i->second->strBuffer[1].c_str(), i->second->strBuffer[1].size())) > 0)
                        {
                          i->second->strBuffer[1].erase(0, nReturn);
                          if (i->second->strBuffer[1].empty())
                          {
                            close(i->second->fdUnix);
                            i->second->fdUnix = -1;
                          }
                        }
                        else
                        {
                          close(i->second->fdUnix);
                          i->second->fdUnix = -1;
                        }
                      }
                    }
                  }
                  m_mutexRequests.unlock();
                }
              }
              delete[] fds;
            }
            for (map<int, wardenreqdata *>::iterator i = m_requests.begin(); i != m_requests.end(); i++)
            {
              if (i->second->bSent)
              {
                if (i->second->fdUnix != -1)
                {
                  close(i->second->fdUnix);
                  i->second->fdUnix = -1;
                }
              }
            }
          }
          close(fdUnix);
          fdUnix = -1;
        }
        if (m_bUseSingleSocket)
        {
          for (size_t i = 0; m_bUseSingleSocket && i < (unSleep * 4); i++)
          {
            utility()->msleep(250);
          }
          if (unSleep < 256)
          {
            unSleep *= 2;
          }
        }
      }
      for (map<int, wardenreqdata *>::iterator i = m_requests.begin(); i != m_requests.end(); i++)
      {
        if (i->second->fdUnix != -1)
        {
          close(i->second->fdUnix);
          i->second->fdUnix = -1;
        }
      }
    }
    // }}}
    // {{{ setTimeout()
    void Warden::setTimeout(const string strTimeout)
    {
      m_strTimeout = strTimeout;
    }
    // }}}
    // {{{ useSingleSocket()
    void Warden::useSingleSocket(const bool bUseSingleSocket)
    {
      if (m_bUseSingleSocket != bUseSingleSocket)
      {
        m_bUseSingleSocket = bUseSingleSocket;
        if (m_bUseSingleSocket)
        {
          m_pThreadRequest = new thread([this](){requestThread();});
          #ifdef COMMON_LINUX
          pthread_setname_np(m_pThreadRequest->native_handle(), "P_requestThread");
          #endif
        }
        else
        {
          m_pThreadRequest->join();
          delete m_pThreadRequest;
        }
      }
    }
    // }}}
    // {{{ utility()
    Utility *Warden::utility()
    {
      return m_pUtility;
    }
    // }}}
    // {{{ vault
    // {{{ vault()
    bool Warden::vault(const string strFunction, list<string> keys, Json *ptData, string &strError)
    {
      bool bResult = false;
      Json *ptRequest = new Json, *ptResponse = new Json;

      ptRequest->insert("Module", "vault");
      ptRequest->insert("Function", strFunction);
      ptRequest->insert("Keys", keys);
      if (ptData != NULL)
      {
        ptRequest->insert("Data", ptData);
      }
      if (request(ptRequest, ptResponse, strError))
      {
        bResult = true;
        if (ptResponse->m.find("Data") != ptResponse->m.end())
        {
          string strJson;
          ptData->parse(ptResponse->m["Data"]->json(strJson));
        }
      }
      delete ptRequest;
      delete ptResponse;

      return bResult;
    }
    // }}}
    // {{{ vaultAdd()
    bool Warden::vaultAdd(list<string> keys, Json *ptData, string &strError)
    {
      return vault("add", keys, ptData, strError);
    }
    // }}}
    // {{{ vaultDelete()
    bool Warden::vaultDelete(list<string> keys, string &strError)
    {
      return vault("delete", keys, NULL, strError);
    }
    // }}}
    // {{{ vaultRetrieve()
    bool Warden::vaultRetrieve(list<string> keys, Json *ptData, string &strError)
    {
      return vault("retrieve", keys, ptData, strError);
    }
    // }}}
    // {{{ vaultRetrieveKeys()
    bool Warden::vaultRetrieveKeys(list<string> keys, Json *ptData, string &strError)
    {
      return vault("retrieveKeys", keys, ptData, strError);
    }
    // }}}
    // {{{ vaultUpdate()
    bool Warden::vaultUpdate(list<string> keys, Json *ptData, string &strError)
    {
      return vault("update", keys, ptData, strError);
    }
    // }}}
    // }}}
  }
}