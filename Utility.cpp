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
    // {{{ fdread()
    bool Utility::fdread(int fdSocket, string &strBuffer, int &nReturn)
    {
      bool bResult = true;
      char szBuffer[65536];
      int nSize = 65536;

      if ((nReturn = read(fdSocket, szBuffer, nSize)) > 0)
      {
        strBuffer.append(szBuffer, nReturn);
      }
      else
      {
        bResult = false;
      }

      return bResult;
    }
    // }}}
    // {{{ fdwrite()
    bool Utility::fdwrite(int fdSocket, string &strBuffer, int &nReturn)
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
    // {{{ socketType()
    bool Utility::socketType(int fdSocket, common_socket_type &eType, string &strError)
    {
      bool bResult = false;
      char cChar;
      int nReturn;

      if ((nReturn = recv(fdSocket, &cChar, 1, MSG_PEEK)) > 0)
      {
        bResult = true;
        // Checking SSL handshake packet type:
        eType = ((cChar == 0x14 /* Change Cipher Spec */ || cChar == 0x15 /* Alert */ || cChar == 0x16 /* Handshake */ || cChar == 0x17 /* Application Data */)?COMMON_SOCKET_ENCRYPTED:COMMON_SOCKET_UNENCRYPTED);
      }
      else
      {
        strError = strerror(errno);
      }

      return bResult;
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
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        CONF_modules_load_file(NULL, NULL, 0); // Configure OpenSSL using standard configuration file and application.
      }
    }
    SSL_CTX *Utility::sslInit(const bool bSslServer, string &strError)
    {
      long lErrCode;
      const char* lcpErrMsg;
      stringstream ssMessage;
      SSL_CTX *ctx = NULL;
      SSL_METHOD *method;

      sslInit();
      method = (SSL_METHOD *)((bSslServer)?TLS_server_method():TLS_client_method());
      ERR_clear_error();
      ctx = SSL_CTX_new(method);
      if (ctx == NULL)
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
    SSL_CTX *Utility::sslInitClient(string &strError)
    {
      return sslInit(false, strError);
    }
    // }}}
    // {{{ sslInitServer()
    SSL_CTX *Utility::sslInitServer(string &strError)
    {
      return sslInit(true, strError);
    }
    // }}}
    // }}}
    // {{{ sslLoadCertKey()
    bool Utility::sslLoadCertKey(SSL_CTX *ctx, const string strCertificate, const string strPrivateKey, string &strError)
    {
      bool bResult = true;
      string strFunction;

      if (SSL_CTX_use_certificate_file(ctx, strCertificate.c_str(), SSL_FILETYPE_PEM) <= 0)
      {
        bResult = false;
        strFunction = "SSL_CTX_use_certificate_file";
      }
      if (SSL_CTX_use_PrivateKey_file(ctx, strPrivateKey.c_str(), SSL_FILETYPE_PEM) <= 0)
      {
        bResult = false;
        strFunction = "SSL_CTX_use_PrivateKey_file";
      }
      if (!SSL_CTX_check_private_key(ctx))
      {
        bResult = false;
        strFunction = "SSL_CTX_check_private_key";
      }
      if (!bResult)
      {
        strError = sslstrerror();
      }

      return bResult;
    }
    // }}}
    // {{{ sslread()
    bool Utility::sslread(SSL *ssl, string &strBuffer, int &nReturn)
    {
      bool bBlocking = false, bDone = false, bResult = true;;
      char szBuffer[65536];
      int nPending, nSize = 65536;
      long lArg, lArgOrig;

      if ((lArg = lArgOrig = fcntl(SSL_get_fd(ssl), F_GETFL, NULL)) >= 0 && !(lArg & O_NONBLOCK))
      {
        bBlocking = true;
        lArg |= O_NONBLOCK;
        fcntl(SSL_get_fd(ssl), F_SETFL, lArg);
      }
      while ((nPending = SSL_pending(ssl)) > 0)
      {
        bDone = true;
        if (nPending > nSize)
        {
          nPending = nSize;
        }
        if ((nReturn = SSL_read(ssl, szBuffer, nPending)) > 0)
        {
          strBuffer.append(szBuffer, nReturn);
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
      if (!bDone)
      {
        if ((nReturn = SSL_read(ssl, szBuffer, nSize)) > 0)
        {
          strBuffer.append(szBuffer, nReturn);
          while ((nPending = SSL_pending(ssl)) > 0)
          {
            if (nPending > nSize)
            {
              nPending = nSize;
            }
            if ((nReturn = SSL_read(ssl, szBuffer, nPending)) > 0)
            {
              strBuffer.append(szBuffer, nReturn);
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
      }
      if (bBlocking)
      {
        fcntl(SSL_get_fd(ssl), F_SETFL, lArgOrig);
      }

      return bResult;
    }
    // }}}
    // {{{ sslwrite()
    bool Utility::sslwrite(SSL *ssl, string &strBuffer, int &nReturn)
    {
      bool bBlocking = false, bResult = true;;
      long lArg, lArgOrig;

      if ((lArg = lArgOrig = fcntl(SSL_get_fd(ssl), F_GETFL, NULL)) >= 0 && !(lArg & O_NONBLOCK))
      {
        bBlocking = true;
        lArg |= O_NONBLOCK;
        fcntl(SSL_get_fd(ssl), F_SETFL, lArg);
      }
      if ((nReturn = SSL_write(ssl, strBuffer.c_str(), strBuffer.size())) > 0)
      {
        strBuffer.erase(0, nReturn);
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
        ssError << ERR_lib_error_string(ulError) << "::" << ERR_func_error_string(ulError) << "(" << ulError << "," << szError << ") " << ERR_reason_error_string(ulError);
      }

      return ssError.str();
    }
    string Utility::sslstrerror(SSL *ssl, int nReturn)
    {
      stringstream ssError;

      ssError << sslstrerror();
      if (ssError.str().empty())
      {
        int nError;
        switch ((nError = SSL_get_error(ssl, nReturn)))
        {
          case SSL_ERROR_NONE : ssError << "[SSL_ERROR_NONE] The TLS/SSL I/O operation completed."; break;
          case SSL_ERROR_ZERO_RETURN : ssError << "[SSL_ERROR_ZERO_RETURN] The TLS/SSL connection has been closed."; break;
          case SSL_ERROR_WANT_READ : ssError << "[SSL_ERROR_WANT_READ] The operation did not complete; the same TLS/SSL I/O function should be called again later."; break;
          case SSL_ERROR_WANT_WRITE : ssError << "[SSL_ERROR_WANT_WRITE] The operation did not complete; the same TLS/SSL I/O function should be called again later."; break;
          case SSL_ERROR_WANT_CONNECT : ssError << "[SSL_ERROR_WANT_CONNECT] The operation did not complete; the same TLS/SSL I/O function should be called again later."; break;
          case SSL_ERROR_WANT_ACCEPT : ssError << "[SSL_ERROR_WANT_ACCEPT) The operation did not complete; the same TLS/SSL I/O function should be called again later."; break;
          case SSL_ERROR_WANT_X509_LOOKUP : ssError << "[SSL_ERROR_WANT_X509_LOOKUP] The operation did not complete because an application callback set by SSL_CTX_set_client_cert_cb() has asked to be called again."; break;
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
          default : ssError << " [" << nError << "] Caught an unknown error.";
        }
      }

      return ssError.str();
    }
    // }}}
  }
}
