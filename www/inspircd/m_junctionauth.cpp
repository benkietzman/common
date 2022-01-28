// vim: syntax=cpp
// vim600: fdm=marker
///////////////////////////////////////////
// ModuleJunctionAuth
// -------------------------------------
// file       : m_junctionauth.cpp
// author     : Ben Kietzman
// begin      : 2022-01-27
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

/// $CompilerFlags: find_compiler_flags("openssl" "")
/// $LinkerFlags: find_linker_flags("openssl" "-lssl -lcrypto")

/*! \file m_junctionauth.cpp
* \brief ModuleJunctionAuth Class
*/
// {{{ includes
#include <cerrno>
#include <cstring>
#include <ctime>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "inspircd.h"
// }}}
// {{{ global variables
enum AuthState
{
  AUTH_STATE_NONE = 0,
  AUTH_STATE_BUSY = 1,
  AUTH_STATE_FAIL = 2
};
// }}}
// {{{ ModuleJunctionAuth
class ModuleJunctionAuth : public Module
{
  private:
  // {{{ member variables
  std::string m_strKillReason;
  std::string m_strAllowPattern;
  std::string m_strPort;
  std::string m_strRequest;
  std::string m_strServer;
  LocalIntExt pendingExt;
  // }}}
  public:
  // {{{ ModuleJunctionAuth()
  ModuleJunctionAuth() : pendingExt("junctionauth-wait", ExtensionItem::EXT_USER, this)
  {
  }
  // }}}
  // {{{ GetVersion()
  Version GetVersion() CXX11_OVERRIDE
  {
    return Version("Allows connecting users to be authenticated against an arbitrary Service Junction request.", VF_VENDOR);
  }
  // }}}
  // {{{ OnUserRegister()
  ModResult OnUserRegister(LocalUser* user) CXX11_OVERRIDE
  {
    // {{{ prep work
    bool bResult = false;
    ConfigTag* tag = user->MyClass->config;
    // }}}
    if (tag->getBool("usejunctionauth", true) && (m_strAllowPattern.empty() || InspIRCd::Match(user->nick, m_strAllowPattern)) && !pendingExt.get(user))
    {
      std::string strError;
      SSL_CTX *ctx = NULL;
      SSL_METHOD *method = (SSL_METHOD *)SSLv23_client_method();
      pendingExt.set(user, AUTH_STATE_BUSY);
      if ((ctx = SSL_CTX_new(method)) != NULL)
      {
        addrinfo hints, *result;
        int nReturn;
        memset(&hints, 0, sizeof(addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        if ((nReturn = getaddrinfo(m_strServer.c_str(), m_strPort.c_str(), &hints, &result)) == 0)
        {
          addrinfo *rp;
          bool bConnected[3] = {false, false, false};
          int fdSocket;
          SSL *ssl = NULL;
          for (rp = result; !bConnected[2] && rp != NULL; rp = rp->ai_next)
          {
            bConnected[0] = bConnected[1] = false;
            if ((fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
            {
              bConnected[0] = true;
              if (connect(fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
              {
                bConnected[1] = true;
                if ((ssl = sslConnect(ctx, fdSocket, strError)) != NULL)
                {
                  bConnected[2] = true;
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
          if (bConnected[2])
          {
            bool bExit = false;
            size_t unPosition;
            std::string strBuffer[2], strLine, strResponse;
            time_t CEnd, CStart, CTimeout = 30;
            time(&CStart);
            strBuffer[1] = m_strRequest + "\nend\n";
            while ((unPosition = strBuffer[1].find("$ident")) != std::string::npos)
            {
              std::string strIdent = user->ident;
              if ((unPosition = strIdent.find("@")) != std::string::npos)
              {
                strIdent.erase(unPosition, (strIdent.size() - unPosition));
              }
              strBuffer[1].replace(unPosition, 6, strIdent);
            }
            while ((unPosition = strBuffer[1].find("$nick")) != std::string::npos)
            {
              strBuffer[1].replace(unPosition, 5, user->nick);
            }
            while ((unPosition = strBuffer[1].find("$pass")) != std::string::npos)
            {
              strBuffer[1].replace(unPosition, 5, user->password);
            }
            while ((unPosition = strBuffer[1].find("\\n")) != std::string::npos)
            {
              strBuffer[1][unPosition] = '\n';
            }
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
                  if (sslRead(ssl, strBuffer[0], nReturn))
                  {
                    while ((unPosition = strBuffer[0].find("\n")) != std::string::npos)
                    {
                      strLine = strBuffer[0].substr(0, unPosition);
                      strResponse.append(strLine);
                      strBuffer[0].erase(0, unPosition + 1);
                      if (strLine == "end")
                      {
                        bExit = true;
                        if (strResponse.find("\"Status\":\"okay\"") != std::string::npos)
                        {
                          bResult = true;
                        }
                        else if ((unPosition = strResponse.find("\"Error\":\"")) != std::string::npos)
                        {
                          std::string strPartial = strResponse.substr((unPosition + 9), (strResponse.size() - (unPosition + 9)));
                          if ((unPosition = strPartial.find("\"")) != std::string::npos)
                          {
                            strError = strPartial.substr(0, unPosition);
                          }
                        }
                      }
                    }
                  }
                  else
                  {
                    bExit = true;
                    if (nReturn < 0)
                    {
                      std::stringstream ssError;
                      ssError << "SSL_read(" << SSL_get_error(ssl, nReturn) << ") " << sslstrerror(ssl, nReturn);
                      strError = ssError.str();
                    }
                  }
                }
                if (fds[0].fd == fdSocket && (fds[0].revents & POLLOUT))
                {
                  if (!sslWrite(ssl, strBuffer[1], nReturn))
                  {
                    bExit = true;
                    if (nReturn < 0)
                    {
                      std::stringstream ssError;
                      ssError << "sslWrite(" << SSL_get_error(ssl, nReturn) << ") " << sslstrerror(ssl, nReturn);
                      strError = ssError.str();
                    }
                  }
                }
              }
              else if (nReturn < 0)
              {
                std::stringstream ssError;
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
            if (bResult && strResponse.empty())
            {
              bResult = false;
              strError = "Failed to receive any data from the underlying service within the Service Junction.";
            }
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(fdSocket);
          }
          else if (!bConnected[1])
          {
            std::stringstream ssError;
            ssError << ((!bConnected[0])?"socket":"connect") << "(" << errno << ") " << strerror(errno);
            strError = ssError.str();
          }
          else
          {
            strError = (std::string)"sslConnect() " + strError;
          }
          freeaddrinfo(result);
        }
        else
        {
          std::stringstream ssError;
          ssError << "getaddrinfo(" << nReturn << ") " << gai_strerror(nReturn);
          strError = ssError.str();
        }
        SSL_CTX_free(ctx);
      }
      else
      {
        long lErrCode = ERR_get_error();
        const char *lcpErrMsg;
        std::stringstream ssError;
        ssError << "SSL_CTX_new(" << lErrCode << ") " << lcpErrMsg;
        strError = ssError.str();
      }
      if (bResult)
      {
        pendingExt.set(user, AUTH_STATE_NONE);
      }
      else
      {
        if (strError.empty() && m_strKillReason.empty())
        {
          strError = "Encountered an unknown error.";
        }
        if (!strError.empty())
        {
          m_strKillReason = strError;
        }
        pendingExt.set(user, AUTH_STATE_FAIL);
      }
    }

    return MOD_RES_PASSTHRU;
  }
  // }}}
  // {{{ OnCheckReady()
  ModResult OnCheckReady(LocalUser* user) CXX11_OVERRIDE
  {
    ModResult modResult = MOD_RES_PASSTHRU;

    switch (pendingExt.get(user))
    {
      case AUTH_STATE_NONE: modResult = MOD_RES_PASSTHRU; break;
      case AUTH_STATE_BUSY: modResult = MOD_RES_DENY; break;
      case AUTH_STATE_FAIL: ServerInstance->Users->QuitUser(user, m_strKillReason); modResult = MOD_RES_DENY; break;
    }

    return modResult;
  }
  // }}}
  // {{{ ReadConfig()
  void ReadConfig(ConfigStatus& status) CXX11_OVERRIDE
  {
    ConfigTag* conf = ServerInstance->Config->ConfValue("junctionauth");
    m_strAllowPattern = conf->getString("allowpattern");
    m_strKillReason = conf->getString("killreason");
    m_strPort = conf->getString("port");
    m_strRequest = conf->getString("request");
    m_strServer = conf->getString("server");
  }
  // }}}
  // {{{ sslConnect()
  SSL *sslConnect(SSL_CTX *ctx, int fdSocket, std::string &strError)
  {
    bool bGood = false;
    int nReturn;
    SSL *ssl = NULL;

    ERR_clear_error();
    if ((ssl = SSL_new(ctx)) == NULL)
    {
      strError = (std::string)"SSL_new() " + sslstrerror();
    }
    else if (!SSL_set_fd(ssl, fdSocket))
    {
      strError = (std::string)"SSL_set_fd() " + sslstrerror();
    }
    else if ((nReturn = SSL_connect(ssl)) != 1)
    {
      strError = (std::string)"SSL_connect() " + sslstrerror(ssl, nReturn);
    }
    else
    {
      bGood = true;
    }
    if (!bGood && ssl != NULL)
    {
      SSL_shutdown(ssl);
      SSL_free(ssl);
      ssl = NULL;
    }
    
    return ssl;
  }
  // }}}
  // {{{ sslRead()
  bool sslRead(SSL *ssl, std::string &strBuffer, int &nReturn)
  {
    std::string strRead;
    
    return sslRead(ssl, strBuffer, strRead, nReturn);
  }
  bool sslRead(SSL *ssl, std::string &strBuffer, std::string &strRead, int &nReturn)
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
  // {{{ sslWrite()
  bool sslWrite(SSL *ssl, std::string &strBuffer, int &nReturn)
  {
    bool bBlocking = false, bResult = true;
    long lArg, lArgOrig;

    if ((lArg = lArgOrig = fcntl(SSL_get_fd(ssl), F_GETFL, NULL)) >= 0 && !(lArg & O_NONBLOCK))
    {
      bBlocking = true;
      lArg |= O_NONBLOCK;
      fcntl(SSL_get_fd(ssl), F_SETFL, lArg);
    }
    if ((nReturn = SSL_write(ssl, strBuffer.c_str(), ((strBuffer.size() < 65536)?strBuffer.size():65536))) > 0)
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
  std::string sslstrerror()
  {
    bool bFirst = true;
    char szError[1024];
    std::stringstream ssError;
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
  std::string sslstrerror(SSL *ssl, int nReturn)
  {
    std::stringstream ssError;

    ssError << sslstrerror();
    if (ssError.str().empty())
    {
      int nError;
      ssError.str("");
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
};
// }}}
MODULE_INIT(ModuleJunctionAuth)
