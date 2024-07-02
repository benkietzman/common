// vim: syntax=cpp
// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Utility
// -------------------------------------
// file       : Utility.cpp
// author     : Ben Kietzman
// begin      : 2013-11-30
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

/*! \file Utility.cpp
* \brief Utility Class
*/
// {{{ includes
#include "Utility"
// }}}
extern "C++"
{
  namespace common
  {
    // {{{ Utility()
    Utility::Utility(string &strError)
    {
      m_bSslInit = false;
      m_ulModifyTime = 0;
      m_strConf = "/etc/central.conf";
      m_conf = new Json;
      readConf(strError);
    }
    // }}}
    // {{{ ~Utility()
    Utility::~Utility()
    {
      if (m_conf != NULL)
      {
        delete m_conf;
      }
    }
    // }}}
    // {{{ conf()
    Json *Utility::conf()
    {
      return m_conf;
    }
    // }}}
    // {{{ connect()
    bool Utility::connect(const string strServer, const string strPort, int &fdSocket, string &strError, const bool bProxy)
    {
      return connect(strServer, strPort, ((bProxy)?m_strProxyServer:""), ((bProxy)?m_strProxyPort:""), fdSocket, strError);
    }
    bool Utility::connect(const string strServer, const string strPort, const string strProxyServer, const string strProxyPort, int &fdSocket, string &strError)
    {
      bool bResult = false;

      if (!strServer.empty())
      {
        if (!strPort.empty())
        {
          addrinfo hints, *result;
          bool bUseProxy = false;
          int nReturn;
          string strDestServer = strServer, strDestPort = strPort;
          time_t CTime[2];
          memset(&hints, 0, sizeof(addrinfo));
          hints.ai_family = AF_UNSPEC;
          hints.ai_socktype = SOCK_STREAM;
          if (!strProxyServer.empty() && !strProxyPort.empty())
          {
            bUseProxy = true;
            strDestServer = strProxyServer;
            strDestPort = strProxyPort;
          }
          time(&(CTime[0]));
          CTime[1] = CTime[0];
          if ((nReturn = getaddrinfo(strDestServer.c_str(), strDestPort.c_str(), &hints, &result)) == 0)
          {
            bool bConnected[2] = {false, false}, bTimeout = false;
            for (addrinfo *rp = result; !bConnected[1] && rp != NULL; rp = rp->ai_next)
            {
              bConnected[0] = false;
              if ((fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
              {
                bool bDone = false;
                bConnected[0] = true;
                fdNonBlocking(fdSocket, strError);
                while (!bDone && (CTime[1] - CTime[0]) <= 5)
                {
                  if (::connect(fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
                  {
                    bDone = bConnected[1] = true;
                  }
                  else if (errno != EAGAIN && errno != EALREADY && errno != EINPROGRESS)
                  {
                    bDone = true;
                  }
                  else
                  {
                    msleep(100);
                    time(&(CTime[1]));
                  }
                }
                fdBlocking(fdSocket, strError);
                if (!bConnected[1])
                {
                  close(fdSocket);
                  fdSocket = -1;
                }
                if (!bDone)
                {
                  bTimeout = true;
                }
              }
            }
            freeaddrinfo(result);
            if (bConnected[1])
            {
              if (bUseProxy)
              {
                bool bExit = false;
                char cChar;
                size_t unPosition;
                string strBuffers[2];
                stringstream ssBuffer;
                ssBuffer << "CONNECT " << strServer << ":" << strPort << " HTTP/1.0\r\n\r\n";
                strBuffers[1] = ssBuffer.str();
                while (!bExit && (CTime[1] - CTime[0]) <= 5)
                {
                  pollfd fds[1];
                  fds[0].fd = fdSocket;
                  fds[0].events = POLLIN;
                  if (!strBuffers[1].empty())
                  {
                    fds[0].events |= POLLOUT;
                  }
                  if ((nReturn = poll(fds, 1, 250)) > 0)
                  {
                    if (fds[0].revents & POLLIN)
                    {
                      if ((nReturn = read(fdSocket, &cChar, 1)) > 0)
                      {
                        strBuffers[0].push_back(cChar);
                        if ((unPosition = strBuffers[0].find("\r\n\r\n")) != string::npos)
                        {
                          string strCode, strLine, strValue;
                          stringstream ssData(strBuffers[0].substr(0, unPosition)), ssLine;
                          getline(ssData, strLine);
                          ssLine.str(strLine);
                          ssLine >> strValue >> strCode;
                          if (strCode == "200")
                          {
                            bExit = bResult = true;
                          }
                          else
                          {
                            strError = strLine;
                            while (!bExit && getline(ssData, strLine))
                            {
                              if (strLine.size() >= 15 && strLine.substr(0, 15) == "X-Squid-Error: ")
                              {
                                bExit = true;
                                strError = strLine.substr(15, (strLine.size() - 15));
                              }
                            }
                            if (!bExit)
                            {
                              bExit = true;
                              strError = strLine;
                            }
                          }
                        }
                      }
                      else
                      {
                        bExit = true;
                        if (nReturn < 0)
                        {
                          stringstream ssError;
                          ssError << "read(" << errno << ") error:  " << strerror(errno);
                          strError = ssError.str();
                        }
                      }
                    }
                    if (fds[0].revents & POLLOUT)
                    {
                      if ((nReturn = write(fdSocket, strBuffers[1].c_str(), strBuffers[1].size())) > 0)
                      {
                        strBuffers[1].erase(0, nReturn);
                      }
                      else
                      {
                        bExit = true;
                        if (nReturn < 0)
                        {
                          stringstream ssError;
                          ssError << "write(" << errno << ") error:  " << strerror(errno);
                          strError = ssError.str();
                        }
                      }
                    }
                  }
                  else if (nReturn < 0)
                  {
                    stringstream ssError;
                    bExit = true;
                    ssError << "poll(" << errno << ") error:  " << strerror(errno);
                    strError = ssError.str();
                  }
                  time(&(CTime[1]));
                }
                if (!bResult)
                {
                  close(fdSocket);
                  fdSocket = -1;
                  if (!bExit)
                  {
                    strError = "Timed out due to proxy connection taking longer than five seconds.";
                  }
                }
              }
              else
              {
                bResult = true;
              }
            }
            else if (bTimeout)
            {
              strError = "Timed out due to connection taking longer than five seconds.";
            }
            else
            {
              stringstream ssError;
              ssError << ((!bConnected[0])?"socket":"connect") << "(" << errno << ") error:  " << strerror(errno);
              strError = ssError.str();
            }
          }
          else
          {
            stringstream ssError;
            ssError << "getaddrinfo(" << nReturn << ") error:  " << gai_strerror(nReturn);
            strError = ssError.str();
          }
        }
        else
        {
          strError = "Please provide the Port.";
        }
      }
      else
      {
        strError = "Please provide the Server.";
      }

      return bResult;
    }
    // }}}
    // {{{ daemonize()
    void Utility::daemonize()
    {
      int nPid = 0;

      if (getpid() == 1)
      {
        return;
      }
      nPid = fork();
      if (nPid < 0)
      {
        exit(1);
      }
      if (nPid > 0)
      {
        exit(0);
      }
      setsid();
      close(1);
      close(2);
      (void)(dup(open("/dev/null", O_RDWR))+1);
      signal(SIGCHLD, SIG_IGN);
      signal(SIGTSTP, SIG_IGN);
      signal(SIGTTOU, SIG_IGN);
      signal(SIGTTIN, SIG_IGN);
    }
    // }}}
    // {{{ fdBlocking()
    bool Utility::fdBlocking(int fdSocket, string &strError)
    {
      bool bResult = false;
      long lArg;
      stringstream ssMessage;

      if ((lArg = fcntl(fdSocket, F_GETFL, NULL)) >= 0)
      {
        lArg &= (~O_NONBLOCK);
        if ((lArg = fcntl(fdSocket, F_SETFL, lArg)) == 0)
        {
          bResult = true;
        }
        else
        {
          ssMessage << "fcntl(F_SETFL," << errno << ") " << strerror(errno);
          strError = ssMessage.str();
        }
      }
      else
      {
        ssMessage << "fcntl(F_GETFL," << errno << ") " << strerror(errno);
        strError = ssMessage.str();
      }

      return bResult;
    }
    // }}}
    // {{{ fdNonBlocking()
    bool Utility::fdNonBlocking(int fdSocket, string &strError)
    {
      bool bResult = false;
      long lArg;
      stringstream ssMessage;

      if ((lArg = fcntl(fdSocket, F_GETFL, NULL)) >= 0)
      {
        lArg |= O_NONBLOCK;
        if ((lArg = fcntl(fdSocket, F_SETFL, lArg)) == 0)
        {
          bResult = true;
        }
        else
        {
          ssMessage << "fcntl(F_SETFL," << errno << ") " << strerror(errno);
          strError = ssMessage.str();
        }
      }
      else
      {
        ssMessage << "fcntl(F_GETFL," << errno << ") " << strerror(errno);
        strError = ssMessage.str();
      }

      return bResult;
    }
    // }}}
    // {{{ fdRead()
    bool Utility::fdRead(int fdSocket, string &strBuffer, int &nReturn)
    {
      string strRead;

      return fdRead(fdSocket, strBuffer, strRead, nReturn);
    }
    bool Utility::fdRead(int fdSocket, string &strBuffer, string &strRead, int &nReturn)
    {
      bool bResult = true;
      char szBuffer[65536];
      int nSize = 65536;

      strRead.clear();
      if ((nReturn = read(fdSocket, szBuffer, nSize)) > 0)
      {
        strBuffer.append(szBuffer, nReturn);
        strRead.append(szBuffer, nReturn);
      }
      else
      {
        bResult = false;
      }

      return bResult;
    }
    // }}}
    // {{{ fdWrite()
    bool Utility::fdWrite(int fdSocket, string &strBuffer, int &nReturn)
    {
      bool bResult = true;

      if ((nReturn = write(fdSocket, strBuffer.c_str(), strBuffer.size())) > 0)
      {
        strBuffer.erase(0, nReturn);
      }
      else
      {
        bResult = false;
      }

      return bResult;
    }
    // }}}
    // {{{ getConfPath()
    string Utility::getConfPath()
    {
      return m_strConf;
    }
    // }}}
    // {{{ getLine()
    bool Utility::getLine(FILE *pFile, string &strLine)
    {
      int nChar;

      strLine.clear();
      while ((nChar = fgetc(pFile)) != EOF && (char)nChar != '\n')
      {
        strLine += (char)nChar;
      }

      return (!strLine.empty() || nChar != EOF);
    }
    bool Utility::getLine(gzFile pgzFile, string &strLine)
    {
      char cChar;
      int nSize = 0;

      strLine.clear();
      while ((nSize = gzread(pgzFile, &cChar, 1)) == 1 && cChar != '\n')
      {
        strLine += cChar;
      }

      return (!strLine.empty() || nSize == 1);
    }
    bool Utility::getLine(int fdFile, string &strLine, const time_t CTimeout, int &nReturn)
    {
      bool bExit = false;
      char cChar;
      time_t CEnd, CStart;

      strLine.clear();
      time(&CStart);
      while (!bExit)
      {
        pollfd fds[1];
        fds[0].fd = fdFile;
        fds[0].events = POLLIN;
        if ((nReturn = poll(fds, 1, 250)) > 0)
        {
          bool bRead = false;
          if (fds[0].fd == fdFile && (fds[0].revents & POLLIN))
          {
            if ((nReturn = read(fdFile, &cChar, 1)) == 1)
            {
              bRead = true;
              if (cChar == '\n')
              {
                bExit = true;
              }
              else
              {
                strLine += cChar;
              }
            }
            else
            {
              bExit = true;
            }
          }
          if (!bRead)
          {
            msleep(250);
          }
        }
        else if (nReturn < 0)
        {
          bExit = true;
        }
        time(&CEnd);
        if (CTimeout > 0 && (CEnd - CStart) > CTimeout)
        {
          bExit = true;
        }
      }

      return (!strLine.empty() || nReturn == 1);
    }
    bool Utility::getLine(int fdFile, string &strLine, int &nReturn)
    {
      return getLine(fdFile, strLine, 0, nReturn);
    }
    bool Utility::getLine(int fdFile, string &strLine)
    {
      int nReturn;

      return getLine(fdFile, strLine, nReturn);
    }
    bool Utility::getLine(ifstream &inFile, string &strLine)
    {
      char cChar;

      strLine.clear();
      while (inFile.get(cChar) && cChar != '\n')
      {
        strLine += cChar;
      }

      return (!strLine.empty() || inFile.good());
    }
    bool Utility::getLine(istream &inStream, string &strLine)
    {
      char cChar;

      strLine.clear();
      while (inStream.get(cChar) && cChar != '\n')
      {
        strLine += cChar;
      }

      return (!strLine.empty() || inStream.good());
    }
    bool Utility::getLine(SSL *ssl, string &strLine, const time_t CTimeout, int &nReturn)
    {
      bool bExit = false;
      char cChar;
      time_t CEnd, CStart;

      strLine.clear();
      time(&CStart);
      while (!bExit)
      {
        bool bRead = false;
        if ((nReturn = SSL_read(ssl, &cChar, 1)) == 1)
        {
          bRead = true;
          if (cChar == '\n')
          {
            bExit = true;
          }
          else
          {
            strLine += cChar;
          }
        }
        else if (nReturn < 0)
        {
          bExit = true;
          switch (SSL_get_error(ssl, nReturn))
          {
            case SSL_ERROR_NONE : break;
            case SSL_ERROR_ZERO_RETURN : break;
            case SSL_ERROR_WANT_READ : bExit = false; break;
            case SSL_ERROR_WANT_WRITE : break;
            case SSL_ERROR_WANT_CONNECT : break;
            case SSL_ERROR_WANT_ACCEPT : break;
            case SSL_ERROR_WANT_X509_LOOKUP : break;
            case SSL_ERROR_SYSCALL : break;
            case SSL_ERROR_SSL : break;
          }
        }
        if (!bRead)
        {
          msleep(250);
        }
        time(&CEnd);
        if (CTimeout > 0 && (CEnd - CStart) > CTimeout)
        {
          bExit = true;
        }
      }

      return (!strLine.empty() || nReturn == 1);
    }
    bool Utility::getLine(SSL *ssl, string &strLine, int &nReturn)
    {
      return getLine(ssl, strLine, 0, nReturn);
    }
    bool Utility::getLine(SSL *ssl, string &strLine)
    {
      int nReturn;

      return getLine(ssl, strLine, nReturn);
    }
    bool Utility::getLine(stringstream &ssData, string &strLine)
    {
      char cChar;

      strLine.clear();
      while (ssData.get(cChar) && cChar != '\n')
      {
        strLine += cChar;
      }

      return (!strLine.empty() || ssData);
    }
    // }}}
    // {{{ isProcessAlreadyRunning()
    bool Utility::isProcessAlreadyRunning(const string strProcess)
    {
      bool bResult = false;
      list<string> procList;

      m_file.directoryList("/proc", procList);
      for (list<string>::iterator i = procList.begin(); !bResult && i != procList.end(); i++)
      {
        if ((*i)[0] != '.' && m_file.directoryExist((string)"/proc/" + *i))
        {
          #ifdef COMMON_LINUX
          if (m_file.fileExist((string)"/proc/" + *i + (string)"/stat"))
          {
            ifstream inStat(((string)"/proc/" + (*i) + (string)"/stat").c_str());
            if (inStat.good())
            {
              pid_t nPid, nPPid;
              string strDaemon, strState;
              inStat >> nPid >> strDaemon >> strState >> nPPid;
              strDaemon.erase(0, 1);
              strDaemon.erase(strDaemon.size() - 1, 1);
              if (strDaemon == strProcess && nPid != getpid())
              {
                bResult = true;
              }
            }
            inStat.close();
          }
          #endif
          #ifdef COMMON_SOLARIS
          if (m_file.fileExist((string)"/proc/" + *i + (string)"/psinfo"))
          {
            ifstream inProc(((string)"/proc/" + *i + (string)"/psinfo").c_str(), ios::in|ios::binary);
            psinfo tInfo;
            if (inProc.good() && inProc.read((char *)&tInfo, sizeof(psinfo)).good() && (string)tInfo.pr_fname == strProcess && atoi(i->c_str()) != getpid())
            {
              bResult = true;
            }
            inProc.close();
          }
          #endif
        }
      }
      procList.clear();

      return bResult;
    }
    // }}}
    // {{{ msleep()
    void Utility::msleep(const unsigned long ulMilliSec)
    {
      struct timespec rqtp, *rmtp = NULL;

      rqtp.tv_sec = (time_t)(ulMilliSec / 1000);
      rqtp.tv_nsec = (ulMilliSec - ((ulMilliSec / 1000) * 1000)) * 1000000L;
      nanosleep(&rqtp, rmtp);
      if (rmtp != NULL)
      {
        delete rmtp;
      }
    }
    // }}}
    // {{{ readConf()
    bool Utility::readConf(string &strError)
    {
      bool bResult = false;
      struct stat tStat;

      m_mutexConf.lock();
      if (stat(m_strConf.c_str(), &tStat) == 0)
      {
        if (m_ulModifyTime != tStat.st_mtime)
        {
          ifstream inFile(m_strConf.c_str());
          if (inFile.good())
          {
            string strLine;
            m_ulModifyTime = tStat.st_mtime;
            if (getLine(inFile, strLine))
            {
              bResult = true;
              if (m_conf != NULL)
              {
                delete m_conf;
              }
              m_conf = new Json(strLine);
            }
            else
            {
              strError = "Failed to read central configuration line.";
            }
          }
          else
          {
            strError = "Failed to open central configuration file for reading.";
          }
          inFile.close();
        }
      }
      else
      {
        strError = "Unable to locate central configuration file.";
      }
      m_mutexConf.unlock();

      return bResult;
    }
    // }}}
    // {{{ setConfPath()
    void Utility::setConfPath(const string strPath, string &strError)
    {
      m_strConf = strPath;
      m_ulModifyTime = 0;
      readConf(strError);
    }
    // }}}
    // {{{ setProxy()
    void Utility::setProxy(const string strServer, const string strPort)
    {
      m_strProxyServer = strServer;
      m_strProxyPort = strPort;
    }
    // }}}
    // {{{ socketType()
    bool Utility::socketType(int fdSocket, common_socket_type &eType, string &strError)
    {
      bool bRetry;

      return socketType(fdSocket, eType, bRetry, strError);
    }
    bool Utility::socketType(int fdSocket, common_socket_type &eType, bool &bRetry, string &strError)
    {
      bool bResult = false;
      char cChar;
      int nReturn;

      bRetry = false;
      if ((nReturn = recv(fdSocket, &cChar, 1, MSG_PEEK)) > 0)
      {
        bResult = true;
        // Checking SSL handshake packet type:
        eType = ((cChar == 0x14 /* Change Cipher Spec */ || cChar == 0x15 /* Alert */ || cChar == 0x16 /* Handshake */ || cChar == 0x17 /* Application Data */)?COMMON_SOCKET_ENCRYPTED:COMMON_SOCKET_UNENCRYPTED);
      }
      else if (errno == EAGAIN)
      {
        bRetry = true;
      }
      else
      {
        strError = strerror(errno);
      }

      return bResult;
    }
    // }}}
    // {{{ sslAccept()
    SSL *Utility::sslAccept(SSL_CTX *ctx, int fdSocket, string &strError)
    {
      bool bRetry;

      return sslAccept(ctx, fdSocket, bRetry, strError);
    }
    SSL *Utility::sslAccept(SSL_CTX *ctx, int fdSocket, bool &bRetry, string &strError)
    {
      bool bGood = false;
      int nReturn;
      SSL *ssl = NULL;

      bRetry = false; 
      ERR_clear_error();
      if ((ssl = SSL_new(ctx)) == NULL)
      {
        strError = (string)"SSL_new() " + sslstrerror();
      }
      else if (!SSL_set_fd(ssl, fdSocket))
      {
        strError = (string)"SSL_set_fd() " + sslstrerror();
      }
      else if ((nReturn = SSL_accept(ssl)) <= 0)
      {
        strError = (string)"SSL_accept() " + sslstrerror(ssl, nReturn, bRetry);
      }
      else
      {
        bGood = true;
      }
      if (!bRetry && !bGood && ssl != NULL)
      {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        ssl = NULL;
      }

      return ssl;
    }
    // }}}
    // {{{ sslConnect()
    SSL *Utility::sslConnect(SSL_CTX *ctx, int fdSocket, string &strError)
    {
      bool bRetry;

      return sslConnect(ctx, fdSocket, bRetry, strError);
    }
    SSL *Utility::sslConnect(SSL_CTX *ctx, int fdSocket, bool &bRetry, string &strError)
    {
      bool bGood = false;
      int nReturn;
      SSL *ssl = NULL;

      bRetry = false; 
      ERR_clear_error();
      if ((ssl = SSL_new(ctx)) == NULL)
      {
        strError = (string)"SSL_new() " + sslstrerror();
      }
      else if (!SSL_set_fd(ssl, fdSocket))
      {
        strError = (string)"SSL_set_fd() " + sslstrerror();
      }
      else if ((nReturn = SSL_connect(ssl)) != 1)
      {
        strError = (string)"SSL_connect() " + sslstrerror(ssl, nReturn, bRetry);
      }
      else
      {
        bGood = true;
      }
      if (!bRetry && !bGood && ssl != NULL)
      {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        ssl = NULL;
      }

      return ssl;
    }
    SSL *Utility::sslConnect(SSL *ssl, bool &bRetry, string &strError)
    {
      bool bGood = false;
      int nReturn;

      bRetry = false; 
      ERR_clear_error();
      if ((nReturn = SSL_connect(ssl)) != 1)
      {
        strError = (string)"SSL_connect() " + sslstrerror(ssl, nReturn, bRetry);
      }
      else
      {
        bGood = true;
      }
      if (!bRetry && !bGood && ssl != NULL)
      {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        ssl = NULL;
      }

      return ssl;
    }
    // }}}
    // {{{ sslDeinit()
    void Utility::sslDeinit()
    {
      if (m_bSslInit)
      {
        m_bSslInit = false;
        EVP_cleanup();
      }
    }
    // }}}
    // {{{ sslInit
    // {{{ sslInit()
    void Utility::sslInit()
    {
      if (!m_bSslInit)
      {
        m_bSslInit = true;
        SSL_load_error_strings();
        ERR_load_crypto_strings();
        OpenSSL_add_all_algorithms();
        SSL_library_init();
        CONF_modules_load_file(NULL, NULL, 0);
      }
    }
    SSL_CTX *Utility::sslInit(const bool bSslServer, string &strError, const bool bVerifyPeer)
    {
      long lErrCode;
      const char* lcpErrMsg;
      stringstream ssMessage;
      SSL_CTX *ctx = NULL;

      sslInit();
      ERR_clear_error();
      ctx = SSL_CTX_new(((bSslServer)?SSLv23_server_method():SSLv23_client_method()));
      if (ctx != NULL)
      {
        if (bVerifyPeer)
        {
          SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);
          SSL_CTX_set_mode(ctx, SSL_MODE_ENABLE_PARTIAL_WRITE);
          if (SSL_CTX_set_default_verify_paths(ctx) == 1)
          {
            SSL_CTX_set_verify(ctx, ((bSslServer)?SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT:SSL_VERIFY_PEER), NULL);
          }
          else
          {
            lErrCode = ERR_get_error();
            lcpErrMsg = ERR_error_string(lErrCode, NULL);
            ssMessage << "SSL_CTX_set_default_verify_paths(" << lErrCode << ") " << lcpErrMsg;
            strError = ssMessage.str();
            SSL_CTX_free(ctx);
            return NULL;
          }
        }
        else
        {
          SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
        }
      }
      else
      {
        lErrCode = ERR_get_error();
        lcpErrMsg = ERR_error_string(lErrCode, NULL);
        ssMessage << "SSL_CTX_new(" << lErrCode << ") " << lcpErrMsg;
        strError = ssMessage.str();
        return NULL;
      }

      return ctx;
    }
    // }}}
    // {{{ sslInitClient()
    SSL_CTX *Utility::sslInitClient(string &strError, const bool bVerifyPeer)
    {
      return sslInit(false, strError, bVerifyPeer);
    }
    SSL_CTX *Utility::sslInitClient(const string strCertificateChain, const string strPrivateKey, string &strError, const bool bVerifyPeer)
    {
      SSL_CTX *ctx = NULL;

      if ((ctx = sslInitClient(strError, bVerifyPeer)) != NULL)
      {
        if (!sslLoadCertChainKey(ctx, strCertificateChain, strPrivateKey, strError))
        {
          SSL_CTX_free(ctx);
          ctx = NULL;
        }
      }

      return ctx;
    }
    // }}}
    // {{{ sslInitServer()
    SSL_CTX *Utility::sslInitServer(string &strError, const bool bVerifyPeer)
    {
      return sslInit(true, strError, bVerifyPeer);
    }
    SSL_CTX *Utility::sslInitServer(const string strCertificateChain, const string strPrivateKey, string &strError, const bool bVerifyPeer)
    {
      SSL_CTX *ctx = NULL;

      if ((ctx = sslInitServer(strError, bVerifyPeer)) != NULL)
      {
        if (!sslLoadCertChainKey(ctx, strCertificateChain, strPrivateKey, strError))
        {
          SSL_CTX_free(ctx);
          ctx = NULL;
        }
      }

      return ctx;
    }
    // }}}
    // }}}
    // {{{ sslLoadCertChainKey()
    bool Utility::sslLoadCertChainKey(SSL_CTX *ctx, const string strCertificateChain, const string strPrivateKey, string &strError)
    {
      bool bResult = false;

      if (SSL_CTX_use_certificate_chain_file(ctx, strCertificateChain.c_str()) == 1)
      {
        if (SSL_CTX_use_PrivateKey_file(ctx, strPrivateKey.c_str(), SSL_FILETYPE_PEM) == 1)
        {
          if (SSL_CTX_check_private_key(ctx) == 1)
          {
            bResult = true;
          }
          else
          {
            strError = (string)"SSL_CTX_check_private_key() " + sslstrerror();
          }
        }
        else
        {
          strError = (string)"SSL_CTX_use_PrivateKey_file() " + sslstrerror();
        }
      }
      else
      {
        strError = (string)"SSL_CTX_use_certificate_chain_file() " + sslstrerror();
      }

      return bResult;
    }
    // }}}
    // {{{ sslRead()
    bool Utility::sslRead(SSL *ssl, string &strBuffer, int &nReturn)
    {
      string strRead;

      return sslRead(ssl, strBuffer, strRead, nReturn);
    }
    bool Utility::sslRead(SSL *ssl, string &strBuffer, string &strRead, int &nReturn)
    {
      bool bBlocking = false, bResult = true;
      char szBuffer[65536];
      int nPending, nSize = 65536;
      long lArg, lArgOrig;

      strRead.clear();
      if ((lArg = lArgOrig = fcntl(SSL_get_fd(ssl), F_GETFL, NULL)) >= 0 && !(lArg & O_NONBLOCK))
      {
        bBlocking = true;
        lArg |= O_NONBLOCK;
        fcntl(SSL_get_fd(ssl), F_SETFL, lArg);
      }
      if ((nReturn = SSL_read(ssl, szBuffer, nSize)) > 0)
      {
        strBuffer.append(szBuffer, nReturn);
        strRead.append(szBuffer, nReturn);
        while ((nPending = SSL_pending(ssl)) > 0)
        {
          if (nPending > nSize)
          {
            nPending = nSize;
          }
          if ((nReturn = SSL_read(ssl, szBuffer, nPending)) > 0)
          {
            strBuffer.append(szBuffer, nReturn);
            strRead.append(szBuffer, nReturn);
          }
          else
          {
            switch (SSL_get_error(ssl, nReturn))
            {
              case SSL_ERROR_ZERO_RETURN:
              case SSL_ERROR_SYSCALL:
              case SSL_ERROR_SSL: bResult = false; break;
            }
          }
        }
      }
      else
      {
        switch (SSL_get_error(ssl, nReturn))
        {
          case SSL_ERROR_ZERO_RETURN:
          case SSL_ERROR_SYSCALL:
          case SSL_ERROR_SSL: bResult = false; break;
        }
      }
      if (bBlocking)
      {
        fcntl(SSL_get_fd(ssl), F_SETFL, lArgOrig);
      }

      return bResult;
    }
    // }}}
    // {{{ sslstrerror()
    string Utility::sslstrerror()
    {
      bool bFirst = true;
      char szError[1024];
      stringstream ssError;
      unsigned long ulError;

      while ((ulError = ERR_get_error()) != 0)
      {
        if (bFirst)
        {
          bFirst = false;
        }
        else
        {
          ssError << ", ";
        }
        ERR_error_string_n(ulError, szError, 1024);
        ssError << ERR_lib_error_string(ulError) << "(" << ulError << "," << szError << ") " << ERR_reason_error_string(ulError);
      }

      return ssError.str();
    }
    string Utility::sslstrerror(SSL *ssl, int nReturn)
    {
      bool bRetry;

      return sslstrerror(ssl, nReturn, bRetry);
    }
    string Utility::sslstrerror(SSL *ssl, int nReturn, bool &bRetry)
    {
      stringstream ssError;

      bRetry = false;
      ssError << sslstrerror();
      if (ssError.str().empty())
      {
        int nError;
        ssError.str("");
        switch ((nError = SSL_get_error(ssl, nReturn)))
        {
          case SSL_ERROR_NONE : ssError << "[SSL_ERROR_NONE] The TLS/SSL I/O operation completed."; break;
          case SSL_ERROR_ZERO_RETURN : ssError << "[SSL_ERROR_ZERO_RETURN] The TLS/SSL connection has been closed."; break;
          case SSL_ERROR_WANT_READ : bRetry = true; ssError << "[SSL_ERROR_WANT_READ] The operation did not complete; the same TLS/SSL I/O function should be called again later."; break;
          case SSL_ERROR_WANT_WRITE : bRetry = true; ssError << "[SSL_ERROR_WANT_WRITE] The operation did not complete; the same TLS/SSL I/O function should be called again later."; break;
          case SSL_ERROR_WANT_CONNECT : bRetry = true; ssError << "[SSL_ERROR_WANT_CONNECT] The operation did not complete; the same TLS/SSL I/O function should be called again later."; break;
          case SSL_ERROR_WANT_ACCEPT : bRetry = true; ssError << "[SSL_ERROR_WANT_ACCEPT) The operation did not complete; the same TLS/SSL I/O function should be called again later."; break;
          case SSL_ERROR_WANT_X509_LOOKUP : ssError << "[SSL_ERROR_WANT_X509_LOOKUP] The operation did not complete because an application callback set by SSL_CTX_set_client_cert_cb() has asked to be called again."; break;
          #ifdef SSL_ERROR_WANT_RETRY_VERIFY
          case SSL_ERROR_WANT_RETRY_VERIFY : bRetry = true; ssError << "[SSL_ERROR_WANT_RETRY_VERIFY] The operation did not complete; the same TLS/SSL I/O function should be called again later."; break;
          #endif
          case SSL_ERROR_SYSCALL :
          {
            ssError << "[SSL_ERROR_SYSCALL] Some I/O error occurred.  ";
            if (nReturn == 0)
            {
              ssError << "Received invalid EOF.";
            }
            else
            {
              ssError << strerror(errno);
            }
            break;
          }
          case SSL_ERROR_SSL : ssError << "[SSL_ERROR_SSL] A failure in the SSL library occurred, usually a protocol error."; break;
        }
      }
      if (ssError.str().empty())
      {
        ssError.str("");
        ssError << " [" << errno << "] " << strerror(errno);
      }

      return ssError.str();
    }
    // }}}
    // {{{ sslWrite()
    bool Utility::sslWrite(SSL *ssl, string &strBuffer, int &nReturn)
    {
      bool bBlocking = false, bResult = true;
      long lArg, lArgOrig;

      if ((lArg = lArgOrig = fcntl(SSL_get_fd(ssl), F_GETFL, NULL)) >= 0 && !(lArg & O_NONBLOCK))
      {
        bBlocking = true;
        lArg |= O_NONBLOCK;
        fcntl(SSL_get_fd(ssl), F_SETFL, lArg);
      }
      if (SSL_write_ex(ssl, strBuffer.c_str(), ((strBuffer.size() < 65536)?strBuffer.size():65536), &nReturn))
      {
        if (nReturn > 0)
        {
          strBuffer.erase(0, nReturn);
        }
      }
      else
      {
        switch (SSL_get_error(ssl, nReturn))
        {
          case SSL_ERROR_ZERO_RETURN:
          case SSL_ERROR_SYSCALL:
          case SSL_ERROR_SSL: bResult = false; break;
        }
      }
      if (bBlocking)
      {
        fcntl(SSL_get_fd(ssl), F_SETFL, lArgOrig);
      }

      return bResult;
    }
    // }}}
  }
}
