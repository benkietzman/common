// vim: syntax=cpp
// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Radial
// -------------------------------------
// file       : Radial.cpp
// author     : Ben Kietzman
// begin      : 2022-07-27
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

/*! \file Radial.cpp
* \brief Radial Class
*/
// {{{ includes
#include "Radial"
// }}}
extern "C++"
{
namespace common
{
// {{{ semaphores
#ifdef COMMON_LINUX
sem_t m_semaRadialRequestLock;
#endif
#ifdef COMMON_SOLARIS
sema_t m_semaRadialRequestLock;
#endif
#ifdef COMMON_LINUX
sem_t m_semaRadialThrottle;
#endif
#ifdef COMMON_SOLARIS
sema_t m_semaRadialThrottle;
#endif
// }}}
// {{{ Radial()
Radial::Radial(string &strError)
{
  #ifdef COMMON_LINUX
  sem_init(&m_semaRadialRequestLock, 0, 10);
  #endif
  #ifdef COMMON_SOLARIS
  sema_init(&m_semaRadialRequestLock, 10, USYNC_THREAD, NULL);
  #endif
  m_bUseSingleSocket = false;
  m_unThrottle = 0;
  m_unUniqueID = 0;
  m_pUtility = new Utility(strError);
}
// }}}
// {{{ ~Radial()
Radial::~Radial()
{
  if (m_bUseSingleSocket)
  {
    useSingleSocket(false);
  }
  delete m_pUtility;
  #ifdef COMMON_LINUX
  sem_destroy(&m_semaRadialRequestLock);
  #endif
  #ifdef COMMON_SOLARIS
  sema_destroy(&m_semaRadialRequestLock);
  #endif
  if (m_unThrottle > 0)
  {
    #ifdef COMMON_LINUX
    sem_destroy(&m_semaRadialThrottle);
    #endif
    #ifdef COMMON_SOLARIS
    sema_destroy(&m_semaRadialThrottle);
    #endif
  }
}
// }}}
// {{{ alert()
bool Radial::alert(const string strUser, const string strMessage, string &strError)
{
  bool bResult = false;
  Json *ptRequest = new Json, *ptResponse = new Json;

  ptRequest->i("Interface", "alert");
  ptRequest->m["Request"] = new Json;
  ptRequest->m["Request"]->i("User", strUser);
  ptRequest->m["Request"]->i("Message", strMessage);
  if (request(ptRequest, ptResponse, strError))
  {
    bResult = true;
  }
  delete ptRequest;
  delete ptResponse;

  return bResult;
}
// }}}
// {{{ database
// {{{ databaseFree()
void Radial::databaseFree(list<map<string, string> > *result)
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
// {{{ databaseQuery()
list<map<string, string> > *Radial::databaseQuery(const string strDatabase, const string strQuery, string &strError)
{
  unsigned long long ullRows;

  return databaseQuery(strDatabase, strQuery, ullRows, strError);
}
list<map<string, string> > *Radial::databaseQuery(const string strDatabase, const string strQuery, unsigned long long &ullRows, string &strError)
{
  list<map<string, string> > *result = NULL;
  Json *ptRequest = new Json, *ptResponse = new Json;

  ptRequest->i("Interface", "database");
  ptRequest->i("Database", strDatabase);
  ptRequest->i("Query", strQuery);
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
// {{{ databaseUpdate()
bool Radial::databaseUpdate(const string strDatabase, const string strUpdate, string &strError)
{
  unsigned long long ullID, ullRows;

  return databaseUpdate(strDatabase, strUpdate, ullID, ullRows, strError);
}
bool Radial::databaseUpdate(const string strDatabase, const string strUpdate, unsigned long long &ullID, unsigned long long &ullRows, string &strError)
{
  bool bResult = false;
  Json *ptRequest = new Json, *ptResponse = new Json;

  ullID = ullRows = 0;
  ptRequest->i("Interface", "database");
  ptRequest->i("Database", strDatabase);
  ptRequest->i("Update", strUpdate);
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
// {{{ dbFree()
void Radial::dbFree(list<map<string, string> > *result)
{
  databaseFree(result);
}
// }}}
// {{{ dbQuery()
list<map<string, string> > *Radial::dbQuery(const string strDatabase, const string strQuery, string &strError)
{
  return databaseQuery(strDatabase, strQuery, strError);
}
list<map<string, string> > *Radial::dbQuery(const string strDatabase, const string strQuery, unsigned long long &ullRows, string &strError)
{
  return databaseQuery(strDatabase, strQuery, ullRows, strError);
}
// }}}
// {{{ dbUpdate()
bool Radial::dbUpdate(const string strDatabase, const string strUpdate, string &strError)
{
  return databaseUpdate(strDatabase, strUpdate, strError);
}
bool Radial::dbUpdate(const string strDatabase, const string strUpdate, unsigned long long &ullID, unsigned long long &ullRows, string &strError)
{
  return databaseUpdate(strDatabase, strUpdate, ullID, ullRows, strError);
}
// }}}
// }}}
// {{{ hub
// {{{ hubList()
bool Radial::hubList(map<string, map<string, string> > &interfaces, string &strError)
{
  bool bResult = false;
  Json *ptRequest = new Json, *ptResponse = new Json;

  ptRequest->i("Interface", "hub");
  ptRequest->i("Function", "list");
  if (request(ptRequest, ptResponse, strError))
  {
    bResult = true;
    if (ptResponse->m.find("Response") != ptResponse->m.end())
    {
      for (auto &interface : ptResponse->m["Response"]->m)
      {
        map<string, string> item;
        interface.second->flatten(item, true, false);
        interfaces[interface.first] = item;
      }
    }
  }
  delete ptRequest;
  delete ptResponse;

  return bResult;
}
// }}}
// {{{ hubPing()
bool Radial::hubPing(string &strError)
{
  bool bResult = false;
  Json *ptRequest = new Json, *ptResponse = new Json;

  ptRequest->i("Interface", "hub");
  ptRequest->i("Function", "ping");
  if (request(ptRequest, ptResponse, strError))
  {
    bResult = true;
  }
  delete ptRequest;
  delete ptResponse;

  return bResult;
}
// }}}
// }}}
// {{{ ircChat()
bool Radial::ircChat(const string strTarget, const string strMessage, string &strError)
{
  bool bResult = false;
  Json *ptRequest = new Json, *ptResponse = new Json;

  ptRequest->i("Interface", "irc");
  ptRequest->i("Function", "chat");
  ptRequest->i("Target", strTarget);
  ptRequest->i("Message", strMessage);
  if (request(ptRequest, ptResponse, strError))
  {
    bResult = true;
  }
  delete ptRequest;
  delete ptResponse;

  return bResult;
}
// }}}
// {{{ linkStatus()
bool Radial::linkStatus(list<string> &nodes, string &strMaster, string &strError)
{
  bool bResult = false;
  Json *ptRequest = new Json, *ptResponse = new Json;

  ptRequest->i("Interface", "link");
  ptRequest->i("Function", "status");
  if (request(ptRequest, ptResponse, strError))
  {
    bResult = true;
    if (ptResponse->m.find("Response") != ptResponse->m.end())
    {
      if (ptResponse->m["Response"]->m.find("Master") != ptResponse->m["Response"]->m.end() && !ptResponse->m["Response"]->m["Master"]->v.empty())
      {
        strMaster = ptResponse->m["Response"]->m["Master"]->v;
      }
      if (ptResponse->m["Response"]->m.find("Node") != ptResponse->m["Response"]->m.end() && !ptResponse->m["Response"]->m["Node"]->v.empty())
      {
        nodes.push_back(ptResponse->m["Response"]->m["Node"]->v);
      }
      if (ptResponse->m["Response"]->m.find("Links") != ptResponse->m["Response"]->m.end() && !ptResponse->m["Response"]->m["Links"]->l.empty())
      {
        for (auto &ptLink : ptResponse->m["Response"]->m["Links"]->l)
        {
          nodes.push_back(ptLink->v);
        }
      }
      nodes.sort();
      nodes.unique();
    }
  }
  delete ptRequest;
  delete ptResponse;

  return bResult;
}
// }}}
// {{{ mysqlQuery()
bool Radial::mysqlQuery(const string strUser, const string strPassword, const string strServer, const string strDatabase, const string strQuery, list<map<string, string> > &result, string &strError)
{
  bool bResult = false;
  Json *ptRequest = new Json, *ptResponse = new Json;
  string strJson;

  result.clear();
  ptRequest->i("Interface", "mysql");
  ptRequest->i("User", strUser);
  ptRequest->i("Password", strPassword);
  ptRequest->i("Server", strServer);
  ptRequest->i("Database", strDatabase);
  ptRequest->i("Query", strQuery);
  if (request(ptRequest, ptResponse, strError))
  {
    bResult = true;
    if (ptResponse->m.find("Response") != ptResponse->m.end())
    {
      for (auto &i : ptResponse->m["Response"]->l)
      {
        map<string, string> row;
        i->flatten(row, true);
        result.push_back(row);
      }
    }
  }
  delete ptRequest;
  delete ptResponse;

  return bResult;
}
// }}}
// {{{ mysqlUpdate()
bool Radial::mysqlUpdate(const string strUser, const string strPassword, const string strServer, const string strDatabase, const string strUpdate, string &strID, string &strRows, string &strError)
{
  bool bResult = false;
  list<map<string, string> > result;
  map<string, string> requestArray;
  Json *ptRequest = new Json, *ptResponse = new Json;

  ptRequest->i("Interface", "mysql");
  ptRequest->i("User", strUser);
  ptRequest->i("Password", strPassword);
  ptRequest->i("Server", strServer);
  ptRequest->i("Database", strDatabase);
  ptRequest->i("Update", strUpdate);
  if (request(ptRequest, ptResponse, strError))
  {
    bResult = true;
    if (ptResponse->m.find("ID") != ptResponse->m.end() && !ptResponse->m["ID"]->v.empty())
    {
      strID = ptResponse->m["ID"]->v;
    }
    if (ptResponse->m.find("Rows") != ptResponse->m.end() && !ptResponse->m["Rows"]->v.empty())
    {
      strRows = ptResponse->m["Rows"]->v;
    }
  }
  delete ptRequest;
  delete ptResponse;

  return bResult;
}
bool Radial::mysqlUpdate(const string strUser, const string strPassword, const string strServer, const string strDatabase, const string strUpdate, string &strError)
{
  string strID, strRows;

  return mysqlUpdate(strUser, strPassword, strServer, strDatabase, strUpdate, strID, strRows, strError);
}
// }}}
// {{{ request
// {{{ request()
bool Radial::request(Json *ptRequest, Json *ptResponse, string &strError)
{
  return request(ptRequest, ptResponse, 0, strError);
}
bool Radial::request(Json *ptRequest, Json *ptResponse, time_t CTimeout, string &strError)
{
  bool bResult = false;
  time_t CEnd, CStart;

  if (!m_strUser.empty() && (ptRequest->m.find("User") == ptRequest->m.end() || ptRequest->m["User"]->v.empty()))
  {
    ptRequest->i("User", m_strUser);
  }
  if (!m_strPassword.empty() && (ptRequest->m.find("Password") == ptRequest->m.end() || ptRequest->m["Password"]->v.empty()))
  {
    ptRequest->i("Password", m_strPassword);
  }
  if (!m_strUserID.empty() && (ptRequest->m.find("UserID") == ptRequest->m.end() || ptRequest->m["UserID"]->v.empty()))
  {
    ptRequest->i("UserID", m_strUserID);
  }
  if (m_unThrottle > 0)
  {
    #ifdef COMMON_LINUX
    sem_wait(&m_semaRadialThrottle);
    #endif
    #ifdef COMMON_SOLARIS
    sema_wait(&m_semaRadialThrottle);
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
      radialreqdata *ptData = new radialreqdata;
      m_mutexRequests.lock();
      while (m_requests.find(m_unUniqueID) != m_requests.end())
      {
        m_unUniqueID++;
      }
      unUniqueID = m_unUniqueID++;
      ssUniqueID << unUniqueID;
      ptRequest->i("rUniqueID", ssUniqueID.str(), 'n');
      ptRequest->j(strBuffer);
      strBuffer += "\n";
      ptData->bSent = false;
      ptData->fdSocket = readpipe[1];
      ptData->strBuffer[0] = strBuffer;
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
                bExit = true;
                ptResponse->parse(strBuffer.substr(0, unPosition));
                strBuffer.erase(0, (unPosition + 1));
                if (ptResponse->m.find("rUniqueID") != ptResponse->m.end())
                {
                  delete ptResponse->m["rUniqueID"];
                  ptResponse->m.erase("rUniqueID");
                }
                if (ptResponse->m.find("Status") != ptResponse->m.end() && ptResponse->m["Status"]->v == "okay")
                {
                  bResult = true;
                }
                else if (ptResponse->m.find("Error") != ptResponse->m.end() && !ptResponse->m["Error"]->v.empty())
                {
                  strError = ptResponse->m["Error"]->v;
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
    list<string> server;
    utility()->readConf(strError);
    if (!m_strServer.empty())
    {
      server.push_back(m_strServer);
    }
    else
    {
      if (utility()->conf()->m.find("Load Balancer") != utility()->conf()->m.end())
      {
        server.push_back(utility()->conf()->m["Load Balancer"]->v);
      }
      if (utility()->conf()->m.find("Radial") != utility()->conf()->m.end())
      {
        server.push_back(utility()->conf()->m["Radial"]->v);
      }
    }
    if (!server.empty())
    {
      bool bDone = false;
      SSL_METHOD *method = (SSL_METHOD *)SSLv23_client_method();
      SSL_CTX *ctx = SSL_CTX_new(method);
      if (ctx != NULL)
      {
        //SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
      }
      for (list<string>::iterator i = server.begin(); !bDone && i != server.end(); i++)
      {
        bool bConnected[4] = {false, false, false, false};
        int fdSocket = -1, nReturn = -1;
        SSL *ssl = NULL;
        string strServer;
        unsigned int unAttempt = 0, unPick = 0, unSeed = time(NULL);
        vector<string> radialServer;
        for (int j = 1; !m_manip.getToken(strServer, (*i), j, ",", true).empty(); j++)
        {
          radialServer.push_back(m_manip.trim(strServer, strServer));
        }
        srand(unSeed);
        unPick = rand_r(&unSeed) % radialServer.size();
        #ifdef COMMON_LINUX
        sem_wait(&m_semaRadialRequestLock);
        #endif
        #ifdef COMMON_SOLARIS
        sema_wait(&m_semaRadialRequestLock);
        #endif
        while (!bConnected[3] && unAttempt++ < radialServer.size())
        {
          addrinfo hints, *result;
          bConnected[0] = bConnected[1] = bConnected[2] = false;
          if (unPick == radialServer.size())
          {
            unPick = 0;
          }
          strServer = radialServer[unPick];
          memset(&hints, 0, sizeof(addrinfo));
          hints.ai_family = AF_UNSPEC;
          hints.ai_socktype = SOCK_STREAM;
          m_mutexGetAddrInfo.lock();
          nReturn = getaddrinfo(strServer.c_str(), "7234", &hints, &result);
          m_mutexGetAddrInfo.unlock();
          if (nReturn == 0)
          {
            addrinfo *rp;
            bConnected[0] = true;
            for (rp = result; !bConnected[3] && rp != NULL; rp = rp->ai_next)
            {
              bConnected[1] = bConnected[2] = false;
              if ((fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
              {
                bConnected[1] = true;
                if (connect(fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
                {
                  bConnected[2] = true;
                  if ((ssl = utility()->sslConnect(ctx, fdSocket, strError)) != NULL)
                  {
                    bConnected[3] = true;
                  }
                  else
                  {
                    close(fdSocket);
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
        sem_post(&m_semaRadialRequestLock);
        #endif
        #ifdef COMMON_SOLARIS
        sema_post(&m_semaRadialRequestLock);
        #endif
        radialServer.clear();
        if (bConnected[3])
        {
          bool bExit = false;
          size_t unPosition;
          string strBuffer[2];
          bDone = true;
          ptRequest->j(strBuffer[1]);
          strBuffer[1] += "\n";
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
                if (utility()->sslRead(ssl, strBuffer[0], nReturn))
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
                    else if (ptResponse->m.find("Error") != ptResponse->m.end() && !ptResponse->m["Error"]->v.empty())
                    {
                      strError = ptResponse->m["Error"]->v;
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
                  if (SSL_get_error(ssl, nReturn) != SSL_ERROR_ZERO_RETURN)
                  {
                    stringstream ssError;
                    ssError << "Utility::sslRead(" << SSL_get_error(ssl, nReturn) << ") " << utility()->sslstrerror(ssl, nReturn);
                    strError = ssError.str();
                  }
                }
              }
              if (fds[0].fd == fdSocket && (fds[0].revents & POLLOUT))
              {
                if (!utility()->sslWrite(ssl, strBuffer[1], nReturn))
                {
                  bExit = true;
                  if (SSL_get_error(ssl, nReturn) != SSL_ERROR_ZERO_RETURN)
                  {
                    stringstream ssError;
                    ssError << "Utility::sslWrite(" << SSL_get_error(ssl, nReturn) << ") " << utility()->sslstrerror(ssl, nReturn);
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
          SSL_shutdown(ssl);
          SSL_free(ssl);
          close(fdSocket);
        }
        else
        {
          stringstream ssError;
          if (!bConnected[0])
          {
            ssError << "getaddrinfo(" << nReturn << ") " << gai_strerror(nReturn);
          }
          else if (!bConnected[1])
          {
            ssError << "socket(" << errno << ") " << strerror(errno);
          }
          else if (!bConnected[2])
          {
            ssError << "connect(" << errno << ") " << strerror(errno);
          }
          else
          {
            ssError << "Utility::sslConnect() " << strError;
          }
          strError = ssError.str();
        }
      }
      if (!bResult && strError.empty())
      {
        strError = "Radial request failed without returning an error message.";
      }
      if (ctx != NULL)
      {
        SSL_CTX_free(ctx);
      }
    }
    else
    {
      strError = (string)"Please provide the Load Balancer and/or Radial servers via the " + utility()->getConfPath() + (string)" file.";
    }
    server.clear();
  }
  if (m_unThrottle > 0)
  {
    #ifdef COMMON_LINUX
    sem_post(&m_semaRadialThrottle);
    #endif
    #ifdef COMMON_SOLARIS
    sema_post(&m_semaRadialThrottle);
    #endif
  }

  return bResult;
}
// }}}
// {{{ requestThread()
void Radial::requestThread()
{
  size_t unSleep = 1;
  SSL_METHOD *method = (SSL_METHOD *)SSLv23_client_method();
  SSL_CTX *ctx = SSL_CTX_new(method);

  if ((ctx = SSL_CTX_new(method)) != NULL)
  {
    //SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    while (m_bUseSingleSocket)
    {
      list<string> server;
      string strError;
      utility()->readConf(strError);
      if (!m_strServer.empty())
      {
        server.push_back(m_strServer);
      }
      else
      {
        if (utility()->conf()->m.find("Load Balancer") != utility()->conf()->m.end())
        {
          server.push_back(utility()->conf()->m["Load Balancer"]->v);
        }
        if (utility()->conf()->m.find("Radial") != utility()->conf()->m.end())
        {
          server.push_back(utility()->conf()->m["Radial"]->v);
        }
      }
      if (!server.empty())
      {
        for (list<string>::iterator i = server.begin(); m_bUseSingleSocket && i != server.end(); i++)
        {
          bool bConnected = false;
          int fdSocket = -1, nReturn = -1;
          SSL *ssl = NULL;
          string strServer;
          unsigned int unAttempt = 0, unPick = 0, unSeed = time(NULL);
          vector<string> radialServer;
          for (int j = 1; !m_manip.getToken(strServer, (*i), j, ",", true).empty(); j++)
          {
            radialServer.push_back(m_manip.trim(strServer, strServer));
          }
          srand(unSeed);
          unPick = rand_r(&unSeed) % radialServer.size();
          while (m_bUseSingleSocket && !bConnected && unAttempt++ < radialServer.size())
          {
            addrinfo hints, *result;
            if (unPick == radialServer.size())
            {
              unPick = 0;
            }
            strServer = radialServer[unPick];
            memset(&hints, 0, sizeof(addrinfo));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            m_mutexGetAddrInfo.lock();
            nReturn = getaddrinfo(strServer.c_str(), "7234", &hints, &result);
            m_mutexGetAddrInfo.unlock();
            if (nReturn == 0)
            {
              addrinfo *rp;
              for (rp = result; m_bUseSingleSocket && !bConnected && rp != NULL; rp = rp->ai_next)
              {
                if ((fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
                {
                  if (connect(fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
                  {
                    if ((ssl = utility()->sslConnect(ctx, fdSocket, strError)) != NULL)
                    {
                      bConnected = true;
                    }
                    else
                    {
                      close(fdSocket);
                      fdSocket = -1;
                    }
                  }
                  else
                  {
                    close(fdSocket);
                    fdSocket = -1;
                  }
                }
              }
              freeaddrinfo(result);
            }
            unPick++;
          }
          radialServer.clear();
          if (bConnected)
          {
            bool bExit = false;
            list<string> response;
            size_t unPosition;
            string strBuffer[2];
            unSleep = 1;
            while (m_bUseSingleSocket && !bExit)
            {
              size_t unIndex = 1;
              m_mutexRequests.lock();
              pollfd *fds = new pollfd[m_requests.size() + 1];
              for (map<int, radialreqdata *>::iterator j = m_requests.begin(); j != m_requests.end(); j++)
              {
                if (!j->second->bSent)
                {
                  j->second->bSent = true;
                  strBuffer[1].append(j->second->strBuffer[0]);
                }
                if (j->second->fdSocket != -1 && !j->second->strBuffer[1].empty())
                {
                  fds[unIndex].fd = j->second->fdSocket;
                  fds[unIndex].events = POLLOUT;
                  unIndex++;
                }
              }
              m_mutexRequests.unlock();
              fds[0].fd = fdSocket;
              fds[0].events = POLLIN;
              if (!strBuffer[1].empty())
              {
                fds[0].events |= POLLOUT;
              }
              if ((nReturn = poll(fds, unIndex, 250)) > 0)
              {
                if (fds[0].revents & POLLIN)
                {
                  if (utility()->sslRead(ssl, strBuffer[0], nReturn))
                  {
                    while ((unPosition = strBuffer[0].find("\n")) != string::npos)
                    {
                      Json *ptJson = new Json(strBuffer[0].substr(0, unPosition));
                      strBuffer[0].erase(0, (unPosition + 1));
                      if (ptJson->m.find("rUniqueID") != ptJson->m.end() && !ptJson->m["rUniqueID"]->v.empty())
                      {
                        size_t unUniqueID = atoi(ptJson->m["rUniqueID"]->v.c_str());
                        m_mutexRequests.lock();
                        if (m_requests.find(unUniqueID) != m_requests.end())
                        {
                          ptJson->j(m_requests[unUniqueID]->strBuffer[1]);
                          m_requests[unUniqueID]->strBuffer[1] += "\n";
                        }
                        m_mutexRequests.unlock();
                      }
                      delete ptJson;
                    }
                  }
                  else
                  {
                    bExit = true;
                  }
                }
                if (fds[0].revents & POLLOUT)
                {
                  if (!utility()->sslWrite(ssl, strBuffer[1], nReturn))
                  {
                    bExit = true;
                  }
                }
                if (unIndex > 1)
                {
                  m_mutexRequests.lock();
                  for (map<int, radialreqdata *>::iterator j = m_requests.begin(); j != m_requests.end(); j++)
                  {
                    for (size_t k = 1; k < unIndex; k++)
                    {
                      if (fds[k].fd == j->second->fdSocket && fds[k].revents & POLLOUT)
                      {
                        if ((nReturn = write(j->second->fdSocket, j->second->strBuffer[1].c_str(), j->second->strBuffer[1].size())) > 0)
                        {
                          j->second->strBuffer[1].erase(0, nReturn);
                          if (j->second->strBuffer[1].empty())
                          {
                            close(j->second->fdSocket);
                            j->second->fdSocket = -1;
                          }
                        }
                        else
                        {
                          close(j->second->fdSocket);
                          j->second->fdSocket = -1;
                        }
                      }
                    }
                  }
                  m_mutexRequests.unlock();
                }
              }
              delete[] fds;
            }
            response.clear();
            m_mutexRequests.lock();
            for (map<int, radialreqdata *>::iterator j = m_requests.begin(); j != m_requests.end(); j++)
            {
              if (j->second->bSent)
              {
                if (j->second->fdSocket != -1)
                {
                  close(j->second->fdSocket);
                  j->second->fdSocket = -1;
                }
              }
            }
            m_mutexRequests.unlock();
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(fdSocket);
            fdSocket = -1;
          }
        }
      }
      server.clear();
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
    m_mutexRequests.lock();
    for (map<int, radialreqdata *>::iterator i = m_requests.begin(); i != m_requests.end(); i++)
    {
      if (i->second->fdSocket != -1)
      {
        close(i->second->fdSocket);
        i->second->fdSocket = -1;
      }
    }
    m_mutexRequests.unlock();
    SSL_CTX_free(ctx);
  }
  m_bUseSingleSocket = false;
}
// }}}
// }}}
// {{{ set
// {{{ setCredentials()
void Radial::setCredentials(const string strUser, const string strPassword, const string strUserID)
{
  m_strUser = strUser;
  m_strPassword = strPassword;
  m_strUserID = strUserID;
}
// }}}
// {{{ setServer()
void Radial::setServer(const string strServer)
{
  m_strServer = strServer;
}
// }}}
// {{{ setThrottle()
void Radial::setThrottle(const size_t unThrottle)
{
  if (unThrottle > 0)
  {
    if (unThrottle != m_unThrottle)
    {
      if (m_unThrottle > 0)
      {
        #ifdef COMMON_LINUX
        sem_destroy(&m_semaRadialThrottle);
        #endif
        #ifdef COMMON_SOLARIS
        sema_destroy(&m_semaRadialThrottle);
        #endif
      }
      #ifdef COMMON_LINUX
      sem_init(&m_semaRadialThrottle, 0, unThrottle);
      #endif
      #ifdef COMMON_SOLARIS
      sema_init(&m_semaRadialThrottle, unThrottle, USYNC_THREAD, NULL);
      #endif
    }
  }
  else if (m_unThrottle > 0)
  {
    #ifdef COMMON_LINUX
    sem_destroy(&m_semaRadialThrottle);
    #endif
    #ifdef COMMON_SOLARIS
    sema_destroy(&m_semaRadialThrottle);
    #endif
  }
  m_unThrottle = unThrottle;
}
// }}}
// {{{ setTimeout()
void Radial::setTimeout(const string strTimeout)
{
  m_strTimeout = strTimeout;
}
// }}}
// }}}
// {{{ ssh
// {{{ sshConnect()
bool Radial::sshConnect(const string strServer, const string strPort, const string strUser, const string strPassword, string &strSession, string &strData, string &strError)
{
  bool bResult = false;
  Json *ptRequest = new Json, *ptResponse = new Json;

  ptRequest->i("Interface", "ssh");
  ptRequest->i("Function", "connect");
  ptRequest->m["Request"] = new Json;
  ptRequest->m["Request"]->i("Server", strServer);
  ptRequest->m["Request"]->i("Port", strPort);
  ptRequest->m["Request"]->i("User", strUser);
  ptRequest->m["Request"]->i("Password", strPassword);
  if (request(ptRequest, ptResponse, strError))
  {
    bResult = true;
  }
  if (ptResponse->m.find("Session") != ptResponse->m.end() && !ptResponse->m["Session"]->v.empty())
  {
    strSession = ptResponse->m["Session"]->v;
  }
  if (ptResponse->m.find("Response") != ptResponse->m.end() && !ptResponse->m["Response"]->v.empty())
  {
    strData = ptResponse->m["Response"]->v;
  }
  delete ptRequest;
  delete ptResponse;

  return bResult;
}
// }}}
// {{{ sshDisconnect()
bool Radial::sshDisconnect(string &strSession, string &strError)
{
  bool bResult = false;
  Json *ptRequest = new Json, *ptResponse = new Json;

  ptRequest->i("Interface", "ssh");
  ptRequest->i("Function", "disconnect");
  ptRequest->i("Session", strSession);
  if (request(ptRequest, ptResponse, strError))
  {
    bResult = true;
  }
  if (ptResponse->m.find("Session") == ptResponse->m.end() || ptResponse->m["Session"]->v.empty())
  {
    strSession.clear();
  }
  delete ptRequest;
  delete ptResponse;

  return bResult;
}
// }}}
// {{{ sshSend()
bool Radial::sshSend(string &strSession, const string strCommand, string &strData, string &strError)
{
  bool bResult = false;
  Json *ptRequest = new Json, *ptResponse = new Json;

  ptRequest->i("Interface", "ssh");
  ptRequest->i("Function", "send");
  ptRequest->i("Session", strSession);
  ptRequest->i("Request", strCommand);
  if (request(ptRequest, ptResponse, strError))
  {
    bResult = true;
  }
  if (ptResponse->m.find("Session") == ptResponse->m.end() || ptResponse->m["Session"]->v.empty())
  {
    strSession.clear();
  }
  if (ptResponse->m.find("Response") != ptResponse->m.end() && !ptResponse->m["Response"]->v.empty())
  {
    strData = ptResponse->m["Response"]->v;
  }
  delete ptRequest;
  delete ptResponse;

  return bResult;
}
// }}}
// }}}
// {{{ storage
// {{{ storageAdd()
bool Radial::storageAdd(list<string> keys, Json *ptData, string &strError)
{
  bool bResult = false;
  Json *ptRequest = new Json, *ptResponse = new Json;

  ptRequest->i("Interface", "storage");
  ptRequest->i("Function", "add");
  ptRequest->i("Keys", keys);
  ptRequest->i("Request", ptData);
  if (request(ptRequest, ptResponse, strError))
  {
    bResult = true;
  }
  delete ptRequest;
  delete ptResponse;

  return bResult;
}
// }}}
// {{{ storageRemove()
bool Radial::storageRemove(list<string> keys, string &strError)
{
  bool bResult = false;
  Json *ptRequest = new Json, *ptResponse = new Json;

  ptRequest->i("Interface", "storage");
  ptRequest->i("Function", "remove");
  ptRequest->i("Keys", keys);
  if (request(ptRequest, ptResponse, strError))
  {
    bResult = true;
  }
  delete ptRequest;
  delete ptResponse;

  return bResult;
}
// }}}
// {{{ storageRetrieve()
bool Radial::storageRetrieve(list<string> keys, Json *ptData, string &strError)
{
  bool bResult = false;
  Json *ptRequest = new Json, *ptResponse = new Json;

  ptRequest->i("Interface", "storage");
  ptRequest->i("Function", "retrieve");
  ptRequest->i("Keys", keys);
  if (request(ptRequest, ptResponse, strError))
  {
    bResult = true;
    if (ptResponse->m.find("Response") != ptResponse->m.end())
    {
      ptData->merge(ptResponse->m["Response"], true, false);
    }
  }
  delete ptRequest;
  delete ptResponse;

  return bResult;
}
// }}}
// {{{ storageRetrieveKeys()
bool Radial::storageRetrieveKeys(list<string> inKeys, list<string> &outKeys, string &strError)
{
  bool bResult = false;
  Json *ptRequest = new Json, *ptResponse = new Json;

  ptRequest->i("Interface", "storage");
  ptRequest->i("Function", "retrieve");
  ptRequest->i("Keys", inKeys);
  if (request(ptRequest, ptResponse, strError))
  {
    bResult = true;
    if (ptResponse->m.find("Response") != ptResponse->m.end())
    {
      for (auto &ptKey : ptResponse->m["Response"]->l)
      {
        outKeys.push_back(ptKey->v);
      }
    }
  }
  delete ptRequest;
  delete ptResponse;

  return bResult;
}
// }}}
// {{{ storageUpdate()
bool Radial::storageUpdate(list<string> keys, Json *ptData, string &strError)
{
  bool bResult = false;
  Json *ptRequest = new Json, *ptResponse = new Json;

  ptRequest->i("Interface", "storage");
  ptRequest->i("Function", "update");
  ptRequest->i("Keys", keys);
  ptRequest->i("Request", ptData);
  if (request(ptRequest, ptResponse, strError))
  {
    bResult = true;
  }
  delete ptRequest;
  delete ptResponse;

  return bResult;
}
// }}}
// }}}
// {{{ terminal
// {{{ terminalConnect()
bool Radial::terminalConnect(radialTerminal &tInfo, const string strServer, const string strPort, const bool bWait, string &strError)
{
  return terminalRequest(tInfo, "connect", {{"Server", strServer}, {"Port", strPort}, {"Wait", ((bWait)?"1":"0")}}, strError);;
}
// }}}
// {{{ terminalCtrl()
bool Radial::terminalCtrl(radialTerminal &tInfo, const char cData, const bool bWait, string &strError)
{
  stringstream ssData;

  ssData << cData;

  return terminalRequest(tInfo, "sendCtrl", {{"Data", ssData.str()}, {"Wait", ((bWait)?"1":"0")}}, strError);
}
// }}}
// {{{ terminalDisconnect()
bool Radial::terminalDisconnect(radialTerminal &tInfo, string &strError)
{
  return terminalRequest(tInfo, "disconnect", strError);
}
// }}}
// {{{ terminalDown()
bool Radial::terminalDown(radialTerminal &tInfo, const size_t unCount, const bool bWait, string &strError)
{
  stringstream ssCount;

  ssCount << unCount;

  return terminalRequest(tInfo, "down", {{"Count", ssCount.str()}, {"Wait", ((bWait)?"1":"0")}}, strError);
}
// }}}
// {{{ terminalEnter()
bool Radial::terminalEnter(radialTerminal &tInfo, const bool bWait, string &strError)
{
  return terminalRequest(tInfo, "enter", {{"Wait", ((bWait)?"1":"0")}}, strError);
}
// }}}
// {{{ terminalEscape()
bool Radial::terminalEscape(radialTerminal &tInfo, const bool bWait, string &strError)
{
  return terminalRequest(tInfo, "escape", {{"Wait", ((bWait)?"1":"0")}}, strError);
}
// }}}
// {{{ terminalFunction()
bool Radial::terminalFunction(radialTerminal &tInfo, const int nKey, string &strError)
{
  stringstream ssKey;

  ssKey << nKey;

  return terminalRequest(tInfo, "function", {{"Data", ssKey.str()}}, strError);
}
// }}}
// {{{ terminalGetSocketTimeout()
bool Radial::terminalGetSocketTimeout(radialTerminal &tInfo, int &nShort, int &nLong, string &strError)
{
  bool bResult = false;
  Json *ptJson = new Json;

  if (terminalRequest(tInfo, "getSocketTimeout", {}, ptJson, strError))
  {
    bResult = true;
    if (ptJson->m.find("Short") != ptJson->m.end() && !ptJson->m["Short"]->v.empty())
    {
      stringstream ssShort(ptJson->m["Short"]->v);
      ssShort >> nShort;
    }
    if (ptJson->m.find("Long") != ptJson->m.end() && !ptJson->m["Long"]->v.empty())
    {
      stringstream ssLong(ptJson->m["Long"]->v);
      ssLong >> nLong;
    }
  }
  delete ptJson;

  return bResult;
}
// }}}
// {{{ terminalHome()
bool Radial::terminalHome(radialTerminal &tInfo, const bool bWait, string &strError)
{
  return terminalRequest(tInfo, "home", {{"Wait", ((bWait)?"1":"0")}}, strError);
}
// }}}
// {{{ terminalKey()
bool Radial::terminalKey(radialTerminal &tInfo, const char cData, const size_t unCount, const bool bWait, string &strError)
{
  stringstream ssData, ssCount;

  ssData << cData;
  ssCount << unCount;

  return terminalRequest(tInfo, "key", {{"Data", ssData.str()}, {"Count", ssCount.str()}, {"Wait", ((bWait)?"1":"0")}}, strError);
}
// }}}
// {{{ terminalKeypadEnter()
bool Radial::terminalKeypadEnter(radialTerminal &tInfo, const bool bWait, string &strError)
{
  return terminalRequest(tInfo, "keypadEnter", {{"Wait", ((bWait)?"1":"0")}}, strError);
}
// }}}
// {{{ terminalLeft()
bool Radial::terminalLeft(radialTerminal &tInfo, const size_t unCount, const bool bWait, string &strError)
{
  stringstream ssCount;

  ssCount << unCount;

  return terminalRequest(tInfo, "left", {{"Count", ssCount.str()}, {"Wait", ((bWait)?"1":"0")}}, strError);
}
// }}}
// {{{ terminalRequest()
bool Radial::terminalRequest(radialTerminal &tInfo, const string strFunction, string &strError)
{
  return terminalRequest(tInfo, strFunction, {}, strError);
}
bool Radial::terminalRequest(radialTerminal &tInfo, const string strFunction, map<string, string> data, string &strError)
{
  bool bResult = false;
  Json *ptJson = new Json;

  if (terminalRequest(tInfo, strFunction, data, ptJson, strError))
  {
    bResult = true;
  }
  delete ptJson;

  return bResult;
}
bool Radial::terminalRequest(radialTerminal &tInfo, const string strFunction, map<string, string> data, Json *ptJson, string &strError)
{
  bool bResult = false;
  string strJson;
  Json *ptRequest = new Json, *ptResponse = new Json;

  ptRequest->i("Interface", "terminal");
  ptRequest->i("Function", strFunction);
  if (!data.empty())
  {
    ptRequest->m["Request"] = new Json(data);
  }
  else
  {
    ptRequest->m["Request"] = new Json;
  }
  if (!tInfo.strSession.empty())
  {
    ptRequest->m["Request"]->i("Session", tInfo.strSession);
  }
  if (request(ptRequest, ptResponse, strError))
  {
    bResult = true;
  }
  if (ptResponse->m.find("Response") != ptResponse->m.end())
  {
    ptJson->parse(ptResponse->m["Response"]->j(strJson));
    if (ptJson->m.find("Session") != ptJson->m.end() && !ptJson->m["Session"]->v.empty())
    {
      tInfo.strSession = ptJson->m["Session"]->v;
    }
    else
    {
      tInfo.strSession.clear();
    }
    if (ptJson->m.find("Screen") != ptJson->m.end() && !ptJson->m["Screen"]->l.empty())
    {
      tInfo.screen.clear();
      for (auto &i : ptJson->m["Screen"]->l)
      {
        tInfo.screen.push_back(i->v);
      }
    }
    if (ptJson->m.find("Col") != ptJson->m.end() && !ptJson->m["Col"]->v.empty())
    {
      stringstream ssCol(ptJson->m["Col"]->v);
      ssCol >> tInfo.unCol;
    }
    if (ptJson->m.find("Cols") != ptJson->m.end() && !ptJson->m["Cols"]->v.empty())
    {
      stringstream ssCols(ptJson->m["Cols"]->v);
      ssCols >> tInfo.unCols;
    }
    if (ptJson->m.find("Row") != ptJson->m.end() && !ptJson->m["Row"]->v.empty())
    {
      stringstream ssRow(ptJson->m["Row"]->v);
      ssRow >> tInfo.unRow;
    }
    if (ptJson->m.find("Rows") != ptJson->m.end() && !ptJson->m["Rows"]->v.empty())
    {
      stringstream ssRows(ptJson->m["Rows"]->v);
      ssRows >> tInfo.unRows;
    }
  }
  delete ptRequest;
  delete ptResponse;

  return bResult;
}
// }}}
// {{{ terminalRight()
bool Radial::terminalRight(radialTerminal &tInfo, const size_t unCount, const bool bWait, string &strError)
{
  stringstream ssCount;

  ssCount << unCount;

  return terminalRequest(tInfo, "right", {{"Count", ssCount.str()}, {"Wait", ((bWait)?"1":"0")}}, strError);
}
// }}}
// {{{ terminalScreen()
bool Radial::terminalScreen(radialTerminal &tInfo, string &strError)
{
  return terminalRequest(tInfo, "screen", strError);
}
// }}}
// {{{ terminalSend()
bool Radial::terminalSend(radialTerminal &tInfo, const string strData, const size_t unCount, const bool bWait, string &strError)
{
  stringstream ssCount;

  ssCount << unCount;

  return terminalRequest(tInfo, "send", {{"Data", strData}, {"Count", ssCount.str()}, {"Wait", ((bWait)?"1":"0")}}, strError);
}
// }}}
// {{{ terminalSetSocketTimeout()
bool Radial::terminalSetSocketTimeout(radialTerminal &tInfo, const int nShort, const int nLong, string &strError)
{
  stringstream ssLong, ssShort;

  ssShort << nShort;
  ssLong << nLong;

  return terminalRequest(tInfo, "setSocketTimeout", {{"Short", ssShort.str()}, {"Long", ssLong.str()}}, strError);
}
// }}}
// {{{ terminalShiftFunction()
bool Radial::terminalShiftFunction(radialTerminal &tInfo, const int nKey, string &strError)
{
  stringstream ssKey;

  ssKey << nKey;

  return terminalRequest(tInfo, "shiftFunction", {{"Data", ssKey.str()}}, strError);
}
// }}}
// {{{ terminalTab()
bool Radial::terminalTab(radialTerminal &tInfo, const size_t unCount, const bool bWait, string &strError)
{
  stringstream ssCount;

  ssCount << unCount;

  return terminalRequest(tInfo, "tab", {{"Count", ssCount.str()}, {"Wait", ((bWait)?"1":"0")}}, strError);
}
// }}}
// {{{ terminalUp()
bool Radial::terminalUp(radialTerminal &tInfo, const size_t unCount, const bool bWait, string &strError)
{
  stringstream ssCount;

  ssCount << unCount;

  return terminalRequest(tInfo, "up", {{"Count", ssCount.str()}, {"Wait", ((bWait)?"1":"0")}}, strError);
}
// }}}
// {{{ terminalWait()
bool Radial::terminalWait(radialTerminal &tInfo, const bool bWait, string &strError)
{
  return terminalRequest(tInfo, "wait", {{"Wait", ((bWait)?"1":"0")}}, strError);
}
// }}}
// }}}
// {{{ useSingleSocket()
void Radial::useSingleSocket(const bool bUseSingleSocket)
{
  if (m_bUseSingleSocket != bUseSingleSocket)
  {
    m_bUseSingleSocket = bUseSingleSocket;
    if (m_bUseSingleSocket)
    {
      m_pThreadRequest = new thread([this](){requestThread();});
      #ifdef COMMON_LINUX
      pthread_setname_np(m_pThreadRequest->native_handle(), "RadialRequest");
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
Utility *Radial::utility()
{
  return m_pUtility;
}
// }}}
}
}
