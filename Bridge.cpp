// vim: syntax=cpp
// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Bridge
// -------------------------------------
// file       : Bridge.cpp
// author     : Ben Kietzman
// begin      : 2020-01-13
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

/*! \file Bridge.cpp
* \brief Bridge Class
*/
// {{{ includes
#include "Bridge"
// }}}
extern "C++"
{
  namespace common
  {
    #ifdef COMMON_LINUX
    sem_t m_semaBridgeRequestLock;
    #endif
    #ifdef COMMON_SOLARIS
    sema_t m_semaBridgeRequestLock;
    #endif
    #ifdef COMMON_LINUX
    sem_t m_semaBridgeThrottle;
    #endif
    #ifdef COMMON_SOLARIS
    sema_t m_semaBridgeThrottle;
    #endif
    // {{{ Bridge()
    Bridge::Bridge(string &strError)
    {
      m_bAuthenticated = false;
      m_bFailed = false;
      m_bUseSecureBridge = true;
      m_ctx = NULL;
      m_eSocketType = COMMON_SOCKET_UNKNOWN;
      m_fdSocket = -1;
      m_ssl = NULL;
      #ifdef COMMON_LINUX
      sem_init(&m_semaBridgeRequestLock, 0, 10);
      #endif
      #ifdef COMMON_SOLARIS
      sema_init(&m_semaBridgeRequestLock, 10, USYNC_THREAD, NULL);
      #endif
      m_CTime[0] = 0;
      m_CTime[1] = 0;
      m_unThrottle = 0;
      m_unUniqueID = 0;
      m_pUtility = new Utility(strError);
    }
    // }}}
    // {{{ ~Bridge()
    Bridge::~Bridge()
    {
      string strError;
      if (m_fdSocket != -1)
      {
        disconnect(strError);
      }
      m_buffer[0].clear();
      m_buffer[1].clear();
      delete m_pUtility;
      #ifdef COMMON_LINUX
      sem_destroy(&m_semaBridgeRequestLock);
      #endif
      #ifdef COMMON_SOLARIS
      sema_destroy(&m_semaBridgeRequestLock);
      #endif
      if (m_unThrottle > 0)
      {
        #ifdef COMMON_LINUX
        sem_destroy(&m_semaBridgeThrottle);
        #endif
        #ifdef COMMON_SOLARIS
        sema_destroy(&m_semaBridgeThrottle);
        #endif
      }
    }
    // }}}
    // {{{ application()
    bool Bridge::application(const string strApplication, Json *ptMessage, string &strError)
    {
      bool bResult = false;
      Json *ptRequest = new Json, *ptResponse = new Json;

      ptRequest->insert("Section", "application");
      ptRequest->insert("Application", strApplication);
      ptRequest->m["Request"] = new Json(ptMessage);
      if (request(ptRequest, ptResponse, strError))
      {
        bResult = true;
        if (ptResponse->m.find("Response") != ptResponse->m.end())
        {
          ptMessage->merge(ptResponse->m["Response"], true, false);
        }
      }
      delete ptRequest;
      delete ptResponse;

      return bResult;
    }
    // }}}
    // {{{ connect()
    bool Bridge::connect(string &strError)
    {
      bool bResult = false;

      if (m_fdSocket == -1)
      {
        if (!m_strUser.empty())
        {
          if (!m_strPassword.empty())
          {
            time(&(m_CTime[1]));
            if (!m_bFailed || (m_CTime[1] - m_CTime[0]) > 30)
            {
              list<string> server;
              m_CTime[0] = m_CTime[1];
              utility()->readConf(strError);
              if (utility()->conf()->m.find("Load Balancer") != utility()->conf()->m.end())
              {
                server.push_back(utility()->conf()->m["Load Balancer"]->v);
              }
              if (utility()->conf()->m.find("Bridge Server") != utility()->conf()->m.end())
              {
                server.push_back(utility()->conf()->m["Bridge Server"]->v);
              }
              if (!server.empty())
              {
                bool bConnected[4] = {false, false, false, false}, bGood = false;
                int fdSocket = -1, nReturn = -1;
                for (list<string>::iterator i = server.begin(); !bGood && i != server.end(); i++)
                {
                  string strServer;
                  unsigned int unAttempt = 0, unPick = 0, unSeed = time(NULL);
                  vector<string> bridgeServer;
                  for (int j = 1; !m_manip.getToken(strServer, (*i), j, ",", true).empty(); j++)
                  {
                    bridgeServer.push_back(m_manip.trim(strServer, strServer));
                  }
                  srand(unSeed);
                  unPick = rand_r(&unSeed) % bridgeServer.size();
                  while (!bGood && unAttempt++ < bridgeServer.size())
                  {
                    addrinfo hints, *result;
                    bConnected[0] = false;
                    if (unPick == bridgeServer.size())
                    {
                      unPick = 0;
                    }
                    strServer = bridgeServer[unPick];
                    memset(&hints, 0, sizeof(addrinfo));
                    hints.ai_family = AF_UNSPEC;
                    hints.ai_socktype = SOCK_STREAM;
                    m_mutexGetAddrInfo.lock();
                    nReturn = getaddrinfo(strServer.c_str(), "12199", &hints, &result);
                    m_mutexGetAddrInfo.unlock();
                    if (nReturn == 0)
                    {
                      addrinfo *rp;
                      bConnected[0] = true;
                      for (rp = result; !bGood && rp != NULL; rp = rp->ai_next)
                      {
                        bConnected[1] = bConnected[2] = false;
                        if ((fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
                        {
                          bConnected[1] = true;
                          if (::connect(fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
                          {
                            bConnected[2] = true;
                            if (m_bUseSecureBridge)  
                            {
                              if ((m_ssl = utility()->sslConnect(m_ctx, fdSocket, strError)) != NULL) 
                              {
                                m_eSocketType = COMMON_SOCKET_ENCRYPTED;
                                bConnected[3] = true;
                                bGood = true;
                              }
                            }
                            else
                            {
                              m_eSocketType = COMMON_SOCKET_UNENCRYPTED;
                              bGood = true;
                            }
                          }
                          else
                          {
                            close(fdSocket);
                          }
                        }
                      }
                      freeaddrinfo(result);
                    }
                    unPick++;
                  }
                  bridgeServer.clear();
                }
                if (bGood)
                {
                  string strJson;
                  Json *ptRequest = new Json;
                  bResult = true;
                  m_fdSocket = fdSocket;
                  ptRequest->insert("Section", "bridge");
                  ptRequest->insert("Function", "application");
                  ptRequest->insert("User", m_strUser);
                  ptRequest->insert("Password", m_strPassword);
                  if (!m_strUserID.empty())
                  {
                    ptRequest->insert("UserID", m_strUserID);
                  }
                  ptRequest->m["Request"] = new Json;
                  m_buffer[1].push_back(ptRequest->json(strJson));
                  delete ptRequest;
                }
                else
                {
                  stringstream ssError;
                  if (!bConnected[0])
                  {
                    ssError << "getaddrinfo(" << nReturn << ") " << gai_strerror(nReturn);
                  }
                  else if (!bConnected[2])
                  {
                    ssError << ((!bConnected[1])?"socket":"connect") << "(" << errno << ") " << strerror(errno);
                  }
                  else
                  {
                    ssError << "Central::utilty()->sslConnect() error [" << strError << "]  ";
                  }
                  strError = ssError.str();
                }
              }
              else
              {
                strError = (string)"Please provide the Load Balancer server via the " + utility()->getConfPath() + (string)" file.";
              }
              server.clear();
            }
            else
            {
              strError = "Retrying a failed connection too quickly.";
            }
          }
          else
          {
            strError = "Please provide the Password.";
          }
        }
        else
        {
          strError = "Please provide the User.";
        }
      }

      return bResult;
    }
    // }}}
    // {{{ dbFree()
    void Bridge::dbFree(list<map<string, string> > *result)
    {
      if (result != NULL)
      {
        for (list<map<string, string> >::iterator i = result->begin(); i != result->end(); i++)
        {
          i->clear();
        }
        result->clear();
        delete result;
        result = NULL;
      }
    }
    // }}}
    // {{{ dbQuery()
    list<map<string, string> > *Bridge::dbQuery(const string strDatabase, const string strQuery, string &strError)
    {
      unsigned long long ullRows;

      return dbQuery(strDatabase, strQuery, ullRows, strError);
    }
    list<map<string, string> > *Bridge::dbQuery(const string strDatabase, const string strQuery, unsigned long long &ullRows, string &strError)
    {
      list<map<string, string> > *result = NULL;
      Json *ptRequest = new Json, *ptResponse = new Json;

      ptRequest->insert("Section", "database");
      ptRequest->insert("Database", strDatabase);
      ptRequest->m["Request"] = new Json;
      ptRequest->m["Request"]->insert("Query", strQuery);
      if (request(ptRequest, ptResponse, strError))
      {
        result = new list<map<string, string> >;
        if (ptResponse->m.find("Response") != ptResponse->m.end() && !ptResponse->m["Response"]->l.empty())
        {
          for (list<Json *>::iterator i = ptResponse->m["Response"]->l.begin(); i != ptResponse->m["Response"]->l.end(); i++)
          {
            map<string, string> row;
            (*i)->flatten(row, true);
            result->push_back(row);
            row.clear();
          }
        }
      }
      delete ptRequest;
      delete ptResponse;

      return result;
    }
    // }}}
    // {{{ dbUpdate()
    bool Bridge::dbUpdate(const string strDatabase, const string strUpdate, string &strError)
    {
      unsigned long long ullID, ullRows;

      return dbUpdate(strDatabase, strUpdate, ullID, ullRows, strError);
    }
    bool Bridge::dbUpdate(const string strDatabase, const string strUpdate, unsigned long long &ullID, unsigned long long &ullRows, string &strError)
    {
      bool bResult = false;
      Json *ptRequest = new Json, *ptResponse = new Json;

      ullID = ullRows = 0;
      ptRequest->insert("Section", "database");
      ptRequest->insert("Database", strDatabase);
      ptRequest->m["Request"] = new Json;
      ptRequest->m["Request"]->insert("Update", strUpdate);
      if (request(ptRequest, ptResponse, strError))
      {
        bResult = true;
        if (ptResponse->m.find("Response") != ptResponse->m.end())
        {
          if (ptResponse->m["Response"]->m.find("ID") != ptResponse->m["Response"]->m.end() && !ptResponse->m["Response"]->m["ID"]->v.empty())
          {
            stringstream ssID(ptResponse->m["Response"]->m["ID"]->v);
            ssID >> ullID;
          }
          if (ptResponse->m["Response"]->m.find("Rows") != ptResponse->m["Response"]->m.end() && !ptResponse->m["Response"]->m["Rows"]->v.empty())
          {
            stringstream ssRows(ptResponse->m["Response"]->m["Rows"]->v);
            ssRows >> ullRows;
          }
        }
      }
      delete ptRequest;
      delete ptResponse;

      return bResult;
    }
    // }}}
    // {{{ disconnect()
    bool Bridge::disconnect(string &strError)
    {
      bool bResult = false;

      if (m_fdSocket != -1)
      {
        if (close(m_fdSocket) == 0)
        {
          bResult = true;
        }
        else
        {
          strError = strerror(errno);
        }
        m_fdSocket = -1;
        if (m_eSocketType == COMMON_SOCKET_ENCRYPTED)
        {
          SSL_shutdown(m_ssl);
          SSL_free(m_ssl);
        }
        m_eSocketType = COMMON_SOCKET_UNKNOWN;
      }
      m_bAuthenticated = false;
      m_strBuffer[0].clear();
      if (!m_strBuffer[1].empty() && !m_strLine.empty())
      {
        m_buffer[1].push_front(m_strLine);
      }
      m_strBuffer[1].clear();
      m_strLine.clear();

      if ((m_eSocketType == COMMON_SOCKET_ENCRYPTED) && (m_ssl != NULL))
      {
        SSL_shutdown(m_ssl);
        SSL_free(m_ssl);
        m_ssl = NULL;
      }
      m_eSocketType = COMMON_SOCKET_UNKNOWN;

      return bResult;
    }
    // }}}
    // {{{ getMessages()
    bool Bridge::getMessages(list<Json *> &messages, string &strError)
    {
      bool bResult = false;

      if (getMessages(strError))
      {
        bResult = true;
        m_mutexResource.lock();
        while (!m_buffer[0].empty())
        {
          messages.push_back(new Json(m_buffer[0].front()));
          m_buffer[0].pop_front();
        }
        m_mutexResource.unlock();
      }

      return bResult;
    }
    bool Bridge::getMessages(string &strError)
    {
      bool bResult = false;

      m_mutexResource.lock();
      if ((m_fdSocket == -1) && (m_bUseSecureBridge) && (m_ctx == NULL))
      {
        strError = "SSL CTX is not initialized.";
        bResult = false;
      }
      else
      if (m_fdSocket != -1 || connect(strError))
      {
        time(&(m_CTime[1]));
        if (!m_buffer[0].empty() && (m_CTime[1] - m_CTime[0]) < 5)
        {
          bResult = true;
        }
        else
        {
          bool bClose = false, bExit = false;
          int nReturn;
          size_t unPosition;
          string strLine;
          m_CTime[0] = m_CTime[1];
          while (!bExit)
          {
            pollfd fds[1];
            fds[0].fd = m_fdSocket;
            fds[0].events = POLLIN;
            if (m_strBuffer[1].empty() && !m_buffer[1].empty())
            {
              m_strBuffer[1].append(m_buffer[1].front()+"\n");
              m_strLine = m_buffer[1].front();
              m_buffer[1].pop_front();
            }
            if (!m_strBuffer[1].empty())
            {
              fds[0].events |= POLLOUT;
            }
            if ((nReturn = poll(fds, 1, 10)) > 0)
            {
              if (fds[0].fd == m_fdSocket && (fds[0].revents & POLLIN))
              {
                if (((m_eSocketType == COMMON_SOCKET_ENCRYPTED) && utility()->sslRead(m_ssl, m_strBuffer[0], nReturn)) || ((m_eSocketType == COMMON_SOCKET_UNENCRYPTED) && utility()->fdRead(m_fdSocket, m_strBuffer[0], nReturn)))
                {
                  while ((unPosition = m_strBuffer[0].find("\n")) != string::npos)
                  {
                    if (m_bAuthenticated)
                    {
                      bResult = true;
                      m_buffer[0].push_back(m_strBuffer[0].substr(0, unPosition));
                    }
                    else
                    {
                      Json *ptResponse = new Json(m_strBuffer[0].substr(0, unPosition));
                      if (ptResponse->m.find("Section") != ptResponse->m.end() && ptResponse->m["Section"]->v == "bridge")
                      {
                        if (ptResponse->m.find("Function") != ptResponse->m.end() && ptResponse->m["Function"]->v == "application")
                        {
                          if (ptResponse->m.find("Status") != ptResponse->m.end() && ptResponse->m["Status"]->v == "okay")
                          {
                            m_bAuthenticated = true;
                            m_bFailed = false;
                          }
                          else
                          {
                            bClose = bExit = true;
                            if (ptResponse->m.find("Error") != ptResponse->m.end())
                            {
                              if (!ptResponse->m["Error"]->v.empty())
                              {
                                strError = ptResponse->m["Error"]->v;
                              }
                              else if (ptResponse->m["Error"]->m.find("Message") != ptResponse->m["Error"]->m.end() && !ptResponse->m["Error"]->m["Message"]->v.empty())
                              {
                                strError = ptResponse->m["Error"]->m["Message"]->v;
                              }
                              else
                              {
                                strError = "Encountered an unknown error.";
                              }
                            }
                            else
                            {
                              strError = "Encountered an unknown error.";
                            }
                          }
                        }
                        else
                        {
                          bClose = bExit = true;
                          strError = "Please authenticate against the application Function within the bridge Section.";
                        }
                      }
                      else
                      {
                        stringstream ssMessage;
                        bClose = bExit = true;
                        ssMessage << "Please authenticate against the bridge Section. --- " << ptResponse;
                        strError = ssMessage.str();
                      }
                      delete ptResponse;
                    }
                    m_strBuffer[0].erase(0, unPosition + 1);
                  }
                }
                else
                {
                  stringstream ssError;
                  bClose = bExit = true;
                  if (m_eSocketType == COMMON_SOCKET_ENCRYPTED)
                  {
                    ssError << "Central::utility()->sslRead(" << SSL_get_error(m_ssl, nReturn) << ") error [" << m_fdSocket << "]:  " << utility()->sslstrerror();
                  }
                  else
                  {
                    ssError << "Central::utility()->fdRead(" << errno << ") error [" << m_fdSocket << "]:  " << strerror(errno);
                  }
                  strError = ssError.str();
                }
              }
              if (fds[0].fd == m_fdSocket && (fds[0].revents & POLLOUT))
              {
                if (((m_eSocketType == COMMON_SOCKET_ENCRYPTED) && utility()->sslWrite(m_ssl, m_strBuffer[1], nReturn)) || ((m_eSocketType == COMMON_SOCKET_UNENCRYPTED) && utility()->fdWrite(m_fdSocket, m_strBuffer[1], nReturn)))
                {
                  m_strBuffer[1].erase(0, nReturn);
                }
                else
                {
                  stringstream ssError;
                  bClose = bExit = true;
                  if (m_eSocketType == COMMON_SOCKET_ENCRYPTED)
                  {
                    ssError << "Central::utility()->sslWrite(" << SSL_get_error(m_ssl, nReturn) << ") error [" << m_fdSocket << "]:  " <<  utility()->sslstrerror();
                  }
                  else
                  {
                    ssError << "Central::utility()->fdWrite(" << errno << ") error [" << m_fdSocket << "]:  " << strerror(errno);
                  }

                  strError = ssError.str();
                }
              }
            }
            else if (nReturn < 0)
            {
              stringstream ssError;
              bClose = bExit = true;
              ssError << "poll(" << errno << ") " << strerror(errno);
              strError = ssError.str();
            }
            else
            {
              bExit = bResult = true;
            }
          }
          if (bClose)
          {
            disconnect(strError);
          }
        }
      }
      m_mutexResource.unlock();

      return bResult;
    }
    // }}}
    // {{{ putMessages()
    bool Bridge::putMessages(list<Json *> &messages, string &strError)
    {
      bool bResult = true;
      string strJson;

      m_mutexResource.lock();
      while (!messages.empty())
      {
        m_buffer[1].push_back(messages.front()->json(strJson));
        delete messages.front();
        messages.pop_front();
      }
      m_mutexResource.unlock();

      return bResult;
    }
    // }}}
    // {{{ request()
    bool Bridge::request(Json *ptRequest, Json *ptResponse, string &strError)
    {
      return request(ptRequest, ptResponse, 0, strError);
    }
    bool Bridge::request(Json *ptRequest, Json *ptResponse, time_t CTimeout, string &strError)
    {
      bool bResult = false;
      list<string> server;
      time_t CEnd, CStart;

      if (m_bUseSecureBridge && (m_ctx == NULL))
      {
        strError = "SSL CTX is not initialized.";
        return (false);
      }
      if (!m_strUser.empty() && (ptRequest->m.find("User") == ptRequest->m.end() || ptRequest->m["User"]->v.empty()))
      {
        ptRequest->insert("User", m_strUser);
      }
      if (!m_strPassword.empty() && (ptRequest->m.find("Password") == ptRequest->m.end() || ptRequest->m["Password"]->v.empty()))
      {
        ptRequest->insert("Password", m_strPassword);
      }
      if (!m_strUserID.empty() && (ptRequest->m.find("UserID") == ptRequest->m.end() || ptRequest->m["UserID"]->v.empty()))
      {
        ptRequest->insert("UserID", m_strUserID);
      }
      if (m_unThrottle > 0)
      {
        #ifdef COMMON_LINUX
        sem_wait(&m_semaBridgeThrottle);
        #endif
        #ifdef COMMON_SOLARIS
        sema_wait(&m_semaBridgeThrottle);
        #endif
      }
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
      utility()->readConf(strError);
      if (utility()->conf()->m.find("Load Balancer") != utility()->conf()->m.end())
      {
        server.push_back(utility()->conf()->m["Load Balancer"]->v);
      }
      if (utility()->conf()->m.find("Bridge Server") != utility()->conf()->m.end())
      {
        server.push_back(utility()->conf()->m["Bridge Server"]->v);
      }
      if (!server.empty())
      {
        bool bDone = false;
        for (list<string>::iterator i = server.begin(); !bDone && i != server.end(); i++)
        {
          bool bConnected[4] = {false, false, false, false}, bGood = false;
          int fdSocket = -1, nReturn = -1;
          string strServer;
          unsigned int unAttempt = 0, unPick = 0, unSeed = time(NULL);
          vector<string> bridgeServer;
          for (int j = 1; !m_manip.getToken(strServer, (*i), j, ",", true).empty(); j++)
          {
            bridgeServer.push_back(m_manip.trim(strServer, strServer));
          }
          srand(unSeed);
          unPick = rand_r(&unSeed) % bridgeServer.size();
          #ifdef COMMON_LINUX
          sem_wait(&m_semaBridgeRequestLock);
          #endif
          #ifdef COMMON_SOLARIS
          sema_wait(&m_semaBridgeRequestLock);
          #endif
          while (!bGood && unAttempt++ < bridgeServer.size())
          {
            addrinfo hints, *result;
            bConnected[0] = false;
            if (unPick == bridgeServer.size())
            {
              unPick = 0;
            }
            strServer = bridgeServer[unPick];
            memset(&hints, 0, sizeof(addrinfo));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            m_mutexGetAddrInfo.lock();
            nReturn = getaddrinfo(strServer.c_str(), "12199", &hints, &result);
            m_mutexGetAddrInfo.unlock();
            if (nReturn == 0)
            {
              addrinfo *rp;
              bConnected[0] = true;
              for (rp = result; !bGood && rp != NULL; rp = rp->ai_next)
              {
                bConnected[1] = bConnected[2] = bConnected[3] = false;
                if ((fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
                {
                  bConnected[1] = true;
                  if (::connect(fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
                  {
                    bConnected[2] = true;
                    if (m_bUseSecureBridge)
                    {
                      if ((m_ssl = utility()->sslConnect(m_ctx, fdSocket, strError)) != NULL)
                      {
                        bConnected[3] = true;
                        bGood = true;
                        m_eSocketType = COMMON_SOCKET_ENCRYPTED;
                      }
                    }
                    else
                    {
                      bGood = true;
                      m_eSocketType = COMMON_SOCKET_UNENCRYPTED;
                    }
                  }
                  else
                  {
                    close(fdSocket);
                  }
                }
              }
              freeaddrinfo(result);
            }
            unPick++;
          }
          #ifdef COMMON_LINUX
          sem_post(&m_semaBridgeRequestLock);
          #endif
          #ifdef COMMON_SOLARIS
          sema_post(&m_semaBridgeRequestLock);
          #endif
          bridgeServer.clear();
          if (bGood)
          {
            bool bExit = false;
            size_t unPosition;
            string strBuffer[2], strLine;
            bDone = true;
            ptRequest->json(strBuffer[1]);
            strBuffer[1].append("\n");
            while (!bExit)
            {
              pollfd fds[1];
              fds[0].fd = fdSocket;
              fds[0].events = POLLIN;
              if (!strBuffer[1].empty())
              {
                fds[0].events |= POLLOUT;
              }
              if ((nReturn = poll(fds, 1, 250)) > 0)
              {
                if (fds[0].fd == fdSocket && (fds[0].revents & POLLIN))
                {
                  if (((m_eSocketType == COMMON_SOCKET_ENCRYPTED) && utility()->sslRead(m_ssl, strBuffer[0], nReturn)) || ((m_eSocketType == COMMON_SOCKET_UNENCRYPTED) && utility()->fdRead(fdSocket, strBuffer[0], nReturn)))
                  {
                    if ((unPosition = strBuffer[0].find("\n")) != string::npos)
                    {
                      bExit = true;
                      ptResponse->parse(strBuffer[0].substr(0, unPosition));
                      strBuffer[0].erase(0, unPosition + 1);
                      if (ptResponse->m.find("Status") != ptResponse->m.end() && ptResponse->m["Status"]->v == "okay")
                      {
                        bResult = true;
                      }
                      else if (ptResponse->m.find("Error") != ptResponse->m.end())
                      {
                        if (!ptResponse->m["Error"]->v.empty())
                        {
                          strError = ptResponse->m["Error"]->v;
                        }
                        else if (ptResponse->m["Error"]->m.find("Message") != ptResponse->m["Error"]->m.end() && !ptResponse->m["Error"]->m["Message"]->v.empty())
                        {
                          strError = ptResponse->m["Error"]->m["Message"]->v;
                        }
                        else
                        {
                          strError = "Encountered an unknown error.";
                        }
                      }
                      else
                      {
                        strError = "Encountered an unknown error.";
                      }
                    }
                  }
                  else
                  {
                    bExit = true;
                    if (nReturn < 0)
                    {
                      stringstream ssError;
                      if (m_eSocketType == COMMON_SOCKET_ENCRYPTED)
                      {
                        ssError << "Central::utility()->sslRead(" << SSL_get_error(m_ssl, nReturn) << ") error [" << m_fdSocket << "]:  " << utility()->sslstrerror();
                      }
                      else
                      {
                        ssError << "Central::utility()->fdRead(" << errno << ") error [" << m_fdSocket << "]:  " << strerror(errno);
                      }
                      strError = ssError.str();
                    }
                  }
                }
                if (fds[0].fd == fdSocket && (fds[0].revents & POLLOUT))
                {
                  if (((m_eSocketType == COMMON_SOCKET_ENCRYPTED) && utility()->sslWrite(m_ssl, strBuffer[1], nReturn)) || ((m_eSocketType == COMMON_SOCKET_UNENCRYPTED) && utility()->fdWrite(fdSocket, strBuffer[1], nReturn)))
                  {
                    strBuffer[1].erase(0, nReturn);
                  }
                  else
                  {
                    bExit = true;
                    if (nReturn < 0)
                    {
                      stringstream ssError;
                      if (m_eSocketType == COMMON_SOCKET_ENCRYPTED)
                      {
                        ssError << "Central::utility()->sslWrite(" << SSL_get_error(m_ssl, nReturn) << ") error [" << fdSocket << "]:  " <<  utility()->sslstrerror();
                      }
                      else
                      {
                        ssError << "Central::utility()->fdWrite(" << errno << ") error [" << fdSocket << "]:  " << strerror(errno);
                      }
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
            close(fdSocket);
          }
          else
          {
            stringstream ssError;
            if (!bConnected[0])
            {
              ssError << "getaddrinfo(" << nReturn << ") " << gai_strerror(nReturn);
            }
            else if (!bConnected[2])
            {
              ssError << ((!bConnected[1])?"socket":"connect") << "(" << errno << ") " << strerror(errno);
            }
            else
            {
              ssError << "Central::utilty()->sslConnect() error [" << strError << "]  ";
            }
            strError = ssError.str();
          }
        }
        if (!bResult && strError.empty())
        {
          strError = "Bridge request failed without returning an error message.";
        }
      }
      else
      {
        strError = (string)"Please provide the Load Balancer server via the " + utility()->getConfPath() + (string)" file.";
      }
      server.clear();
      if (m_unThrottle > 0)
      {
        #ifdef COMMON_LINUX
        sem_post(&m_semaBridgeThrottle);
        #endif
        #ifdef COMMON_SOLARIS
        sema_post(&m_semaBridgeThrottle);
        #endif
      }

      return bResult;
    }
    // }}}
    // {{{ setCredentials()
    void Bridge::setCredentials(const string strUser, const string strPassword, const string strUserID)
    {
      m_strUser = strUser;
      m_strPassword = strPassword;
      m_strUserID = strUserID;
    }
    // }}}
    // {{{ setThrottle()
    void Bridge::setThrottle(const size_t unThrottle)
    {
      if (unThrottle > 0)
      {
        if (unThrottle != m_unThrottle)
        {
          if (m_unThrottle > 0)
          {
            #ifdef COMMON_LINUX
            sem_destroy(&m_semaBridgeThrottle);
            #endif
            #ifdef COMMON_SOLARIS
            sema_destroy(&m_semaBridgeThrottle);
            #endif
          }
          #ifdef COMMON_LINUX
          sem_init(&m_semaBridgeThrottle, 0, unThrottle);
          #endif
          #ifdef COMMON_SOLARIS
          sema_init(&m_semaBridgeThrottle, unThrottle, USYNC_THREAD, NULL);
          #endif
        }
      }
      else if (m_unThrottle > 0)
      {
        #ifdef COMMON_LINUX
        sem_destroy(&m_semaBridgeThrottle);
        #endif
        #ifdef COMMON_SOLARIS
        sema_destroy(&m_semaBridgeThrottle);
        #endif
      }
      m_unThrottle = unThrottle;
    }
    // }}}
    // {{{ setTimeout()
    void Bridge::setTimeout(const string strTimeout)
    {
      m_strTimeout = strTimeout;
    }
    // }}}
    // {{{ passwordVerify()
    bool Bridge::passwordVerify(const string strApplication, const string strType, const string strUser, const string strPassword, string &strError)
    {
      bool bResult = false;
      Json *ptRequest = new Json, *ptResponse = new Json;

      ptRequest->insert("Section", "password");
      ptRequest->insert("Function", "verify");
      ptRequest->m["Request"] = new Json;
      ptRequest->m["Request"]->insert("Application", strApplication);
      ptRequest->m["Request"]->insert("Password", strPassword);
      ptRequest->m["Request"]->insert("Type", strType);
      ptRequest->m["Request"]->insert("User", strUser);
      if (request(ptRequest, ptResponse, strError))
      {
        bResult = true;
      }
      delete ptRequest;
      delete ptResponse;

      return bResult;
    }
    // }}}
    // {{{ useSecureBridge()
    void Bridge::useSecureBridge(bool bUseSecureBridge, SSL_CTX *ctx)
    {
      m_bUseSecureBridge = bUseSecureBridge;
      m_ctx = ctx;
    }
    // }}}
    // {{{ utility()
    Utility *Bridge::utility()
    {
      return m_pUtility;
    }
    // }}}
  }
}
