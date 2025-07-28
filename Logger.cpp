/* -*- c++ -*- */
///////////////////////////////////////////
// Logger
// -------------------------------------
// file       : Logger.cpp
// author     : Ben Kietzman
// begin      : 2023-05-18
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
/*! \file Logger.cpp
* \brief Logger Class
*/
// {{{ includes
#include "Logger"
// }}}
extern "C++"
{
  namespace common
  {
    // {{{ semaphores
    #ifdef COMMON_LINUX
    sem_t m_semaLoggerRequestLock;
    #endif
    #ifdef COMMON_SOLARIS
    sema_t m_semaLoggerRequestLock;
    #endif
    #ifdef COMMON_LINUX
    sem_t m_semaLoggerThrottle;
    #endif
    #ifdef COMMON_SOLARIS
    sema_t m_semaLoggerThrottle;
    #endif
    // }}}
    // {{{ Logger()
    Logger::Logger(string &strError)
    {
      #ifdef COMMON_LINUX
      sem_init(&m_semaLoggerRequestLock, 0, 10);
      #endif
      #ifdef COMMON_SOLARIS
      sema_init(&m_semaLoggerRequestLock, 10, USYNC_THREAD, NULL);
      #endif
      m_bUseSingleSocket = false;
      m_unThrottle = 0;
      m_unUniqueID = 0;
      m_pUtility = new Utility(strError);
    }
    // }}}
    // {{{ ~Logger()
    Logger::~Logger()
    {
      if (m_bUseSingleSocket)
      {
        useSingleSocket(false);
      }
      delete m_pUtility;
      #ifdef COMMON_LINUX
      sem_destroy(&m_semaLoggerRequestLock);
      #endif
      #ifdef COMMON_SOLARIS
      sema_destroy(&m_semaLoggerRequestLock);
      #endif
      if (m_unThrottle > 0)
      {
        #ifdef COMMON_LINUX
        sem_destroy(&m_semaLoggerThrottle);
        #endif
        #ifdef COMMON_SOLARIS
        sema_destroy(&m_semaLoggerThrottle);
        #endif
      }
    }
    // }}}
    // {{{ log()
    bool Logger::log(map<string, string> label, const string strMessage, string &strError)
    {
      bool bResult = false;
      Json *ptRequest = new Json, *ptResponse = new Json;

      ptRequest->i("Function", "log");
      ptRequest->i("Label", label);
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
    // {{{ message()
    bool Logger::message(map<string, string> label, const string strMessage, string &strError)
    {
      bool bResult = false;
      Json *ptRequest = new Json, *ptResponse = new Json;

      ptRequest->i("Function", "message");
      ptRequest->i("Label", label);
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
    // {{{ request
    // {{{ request()
    bool Logger::request(Json *ptRequest, Json *ptResponse, string &strError)
    {
      return request(ptRequest, ptResponse, 0, strError);
    }
    bool Logger::request(Json *ptRequest, Json *ptResponse, time_t CTimeout, string &strError)
    {
      bool bResult = false;
      time_t CEnd, CStart;

      if (!m_strApplication.empty() && (ptRequest->m.find("Application") == ptRequest->m.end() || ptRequest->m["Application"]->v.empty()))
      {
        ptRequest->i("Application", m_strApplication);
      }
      if (!m_strUser.empty() && (ptRequest->m.find("User") == ptRequest->m.end() || ptRequest->m["User"]->v.empty()))
      {
        ptRequest->i("User", m_strUser);
      }
      if (!m_strPassword.empty() && (ptRequest->m.find("Password") == ptRequest->m.end() || ptRequest->m["Password"]->v.empty()))
      {
        ptRequest->i("Password", m_strPassword);
      }
      if (m_unThrottle > 0)
      {
        #ifdef COMMON_LINUX
        sem_wait(&m_semaLoggerThrottle);
        #endif
        #ifdef COMMON_SOLARIS
        sema_wait(&m_semaLoggerThrottle);
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
          loggerreqdata *ptData = new loggerreqdata;
          m_mutexRequests.lock();
          while (m_requests.find(m_unUniqueID) != m_requests.end())
          {
            m_unUniqueID++;
          }
          unUniqueID = m_unUniqueID++;
          ssUniqueID << unUniqueID;
          ptRequest->i("lUniqueID", ssUniqueID.str(), 'n');
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
                    if (ptResponse->m.find("lUniqueID") != ptResponse->m.end())
                    {
                      delete ptResponse->m["lUniqueID"];
                      ptResponse->m.erase("lUniqueID");
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
          if (utility()->conf()->m.find("Logger") != utility()->conf()->m.end())
          {
            server.push_back(utility()->conf()->m["Logger"]->v);
          }
        }
        if (!server.empty())
        {
          bool bDone = false;
          SSL_METHOD *method = (SSL_METHOD *)SSLv23_client_method();
          SSL_CTX *ctx = NULL;
          if ((ctx = SSL_CTX_new(method)) != NULL)
          {
            //SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
            for (list<string>::iterator i = server.begin(); !bDone && i != server.end(); i++)
            {
              bool bConnected[4] = {false, false, false, false};
              int fdSocket = -1, nReturn = -1;
              SSL *ssl = NULL;
              string strServer;
              unsigned int unAttempt = 0, unPick = 0, unSeed = time(NULL);
              vector<string> loggerServer;
              for (int j = 1; !m_manip.getToken(strServer, (*i), j, ",", true).empty(); j++)
              {
                loggerServer.push_back(m_manip.trim(strServer, strServer));
              }
              srand(unSeed);
              unPick = rand_r(&unSeed) % loggerServer.size();
              #ifdef COMMON_LINUX
              sem_wait(&m_semaLoggerRequestLock);
              #endif
              #ifdef COMMON_SOLARIS
              sema_wait(&m_semaLoggerRequestLock);
              #endif
              while (!bConnected[3] && unAttempt++ < loggerServer.size())
              {
                addrinfo hints, *result;
                bConnected[0] = bConnected[1] = bConnected[2] = false;
                if (unPick == loggerServer.size())
                {
                  unPick = 0;
                }
                strServer = loggerServer[unPick];
                memset(&hints, 0, sizeof(addrinfo));
                hints.ai_family = AF_UNSPEC;
                hints.ai_socktype = SOCK_STREAM;
                m_mutexGetAddrInfo.lock();
                nReturn = getaddrinfo(strServer.c_str(), "5647", &hints, &result);
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
              sem_post(&m_semaLoggerRequestLock);
              #endif
              #ifdef COMMON_SOLARIS
              sema_post(&m_semaLoggerRequestLock);
              #endif
              loggerServer.clear();
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
              strError = "Logger request failed without returning an error message.";
            }
            SSL_CTX_free(ctx);
          }
          else
          {
            strError = "Failed to initialize SSL context.";
          }
        }
        else
        {
          strError = (string)"Please provide the Load Balancer and/or Logger servers via the " + utility()->getConfPath() + (string)" file.";
        }
        server.clear();
      }
      if (m_unThrottle > 0)
      {
        #ifdef COMMON_LINUX
        sem_post(&m_semaLoggerThrottle);
        #endif
        #ifdef COMMON_SOLARIS
        sema_post(&m_semaLoggerThrottle);
        #endif
      }

      return bResult;
    }
    // }}}
    // {{{ requestThread()
    void Logger::requestThread()
    {
      size_t unSleep = 1;
      SSL_METHOD *method = (SSL_METHOD *)SSLv23_client_method();
      SSL_CTX *ctx = NULL;

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
            if (utility()->conf()->m.find("Logger") != utility()->conf()->m.end())
            {
              server.push_back(utility()->conf()->m["Logger"]->v);
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
              vector<string> loggerServer;
              for (int j = 1; !m_manip.getToken(strServer, (*i), j, ",", true).empty(); j++)
              {
                loggerServer.push_back(m_manip.trim(strServer, strServer));
              }
              srand(unSeed);
              unPick = rand_r(&unSeed) % loggerServer.size();
              while (m_bUseSingleSocket && !bConnected && unAttempt++ < loggerServer.size())
              {
                addrinfo hints, *result;
                if (unPick == loggerServer.size())
                {
                  unPick = 0;
                }
                strServer = loggerServer[unPick];
                memset(&hints, 0, sizeof(addrinfo));
                hints.ai_family = AF_UNSPEC;
                hints.ai_socktype = SOCK_STREAM;
                m_mutexGetAddrInfo.lock();
                nReturn = getaddrinfo(strServer.c_str(), "5649", &hints, &result);
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
              loggerServer.clear();
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
                  for (map<int, loggerreqdata *>::iterator j = m_requests.begin(); j != m_requests.end(); j++)
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
                          if (ptJson->m.find("lUniqueID") != ptJson->m.end() && !ptJson->m["lUniqueID"]->v.empty())
                          {
                            size_t unUniqueID = atoi(ptJson->m["lUniqueID"]->v.c_str());
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
                      for (map<int, loggerreqdata *>::iterator j = m_requests.begin(); j != m_requests.end(); j++)
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
                for (map<int, loggerreqdata *>::iterator j = m_requests.begin(); j != m_requests.end(); j++)
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
        for (map<int, loggerreqdata *>::iterator i = m_requests.begin(); i != m_requests.end(); i++)
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
    void Logger::setCredentials(const string strApplication, const string strUser, const string strPassword)
    {
      m_strApplication = strApplication;
      m_strUser = strUser;
      m_strPassword = strPassword;
    }
    // }}}
    // {{{ setServer()
    void Logger::setServer(const string strServer)
    {
      m_strServer = strServer;
    }
    // }}}
    // {{{ setThrottle()
    void Logger::setThrottle(const size_t unThrottle)
    {
      if (unThrottle > 0)
      {
        if (unThrottle != m_unThrottle)
        {
          if (m_unThrottle > 0)
          {
            #ifdef COMMON_LINUX
            sem_destroy(&m_semaLoggerThrottle);
            #endif
            #ifdef COMMON_SOLARIS
            sema_destroy(&m_semaLoggerThrottle);
            #endif
          }
          #ifdef COMMON_LINUX
          sem_init(&m_semaLoggerThrottle, 0, unThrottle);
          #endif
          #ifdef COMMON_SOLARIS
          sema_init(&m_semaLoggerThrottle, unThrottle, USYNC_THREAD, NULL);
          #endif
        }
      }
      else if (m_unThrottle > 0)
      {
        #ifdef COMMON_LINUX
        sem_destroy(&m_semaLoggerThrottle);
        #endif
        #ifdef COMMON_SOLARIS
        sema_destroy(&m_semaLoggerThrottle);
        #endif
      }
      m_unThrottle = unThrottle;
    }
    // }}}
    // {{{ setTimeout()
    void Logger::setTimeout(const string strTimeout)
    {
      m_strTimeout = strTimeout;
    }
    // }}}
    // }}}
    // {{{ useSingleSocket()
    void Logger::useSingleSocket(const bool bUseSingleSocket)
    {
      if (m_bUseSingleSocket != bUseSingleSocket)
      {
        m_bUseSingleSocket = bUseSingleSocket;
        if (m_bUseSingleSocket)
        {
          m_pThreadRequest = new thread([this](){requestThread();});
          #ifdef COMMON_LINUX
          pthread_setname_np(m_pThreadRequest->native_handle(), "LoggerRequest");
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
    Utility *Logger::utility()
    {
      return m_pUtility;
    }
    // }}}
  }
}
