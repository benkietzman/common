// vim: syntax=cpp
// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// IRC
// -------------------------------------
// file       : Irc.cpp
// author     : Ben Kietzman
// begin      : 2012-11-23
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

/*! \file Irc.cpp
* \brief Irc Class
*/
// {{{ includes
#include "Irc"
// }}}
extern "C++"
{
  namespace common
  {
    // {{{ Irc()
    Irc::Irc()
    {
      m_bConnected = false;
      m_bSSL = false;
      m_unIndex = 0;
    }
    // }}}
    // {{{ ~Irc()
    Irc::~Irc()
    {
      string strError;

      if (m_bConnected)
      {
        for (list<string>::iterator i = m_room.begin(); i != m_room.end(); i++)
        {
          part(*i, strError);
        }
        m_room.clear();
        quit(strError);
        disconnect(strError);
      }
    }
    // }}}
    // {{{ chat()
    bool Irc::chat(const string strLocation, const string strMessage, string &strError)
    {
      bool bResult = false;

      if (!m_strNick.empty() && !strLocation.empty())
      {
        bool bJoined = false;
        if (strLocation[0] != '#' || findRoom(strLocation) != m_room.end())
        {
          bJoined = true;
        }
        else
        {
          bJoined = join(strLocation, strError);
        }
        if (bJoined)
        {
          stringstream ssBuffer;
          ssBuffer << ":" << m_strNick << " PRIVMSG " << strLocation << " :" << strMessage << "\r\n";
          if (write(ssBuffer.str(), strError) == (ssize_t)ssBuffer.str().size())
          {
            bResult = true;
          }
        }
      }

      return bResult;
    }
    // }}}
    // {{{ connect()
    bool Irc::connect(string &strError)
    {
      if (m_strServer.empty())
      {
        m_strServer = "localhost";
      }
      if (m_strPort.empty())
      {
        m_strPort = "6667";
      }

      return connect(m_strServer, m_strPort, strError, m_bSSL);
    }
    bool Irc::connect(const string strServer, const string strPort, string &strError, const bool bSSL)
    {
      if (!m_bConnected)
      {
        int nReturn;
        struct addrinfo hints, *result;
        m_strServer = (!strServer.empty())?strServer:"localhost";
        m_strPort = (!strPort.empty())?strPort:(!bSSL)?"6667":"6697";
        m_bSSL = bSSL;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        if ((nReturn = getaddrinfo(m_strServer.c_str(), m_strPort.c_str(), &hints, &result)) == 0)
        {
          struct addrinfo *rp;
          for (rp = result; !m_bConnected && rp != NULL; rp = rp->ai_next)
          {
            if ((m_fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
            {
              if (::connect(m_fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
              {
                if (!m_bSSL || (m_method = (SSL_METHOD *)SSLv23_client_method()) != NULL)
                {
                  if (!m_bSSL || (m_ctx = SSL_CTX_new(m_method)) != NULL)
                  {
                    if (!m_bSSL || (m_ssl = SSL_new(m_ctx)) != NULL)
                    {
                      if (!m_bSSL || SSL_set_fd(m_ssl, m_fdSocket) == 1)
                      {
                        if (!m_bSSL || SSL_connect(m_ssl) == 1)
                        {
                          string strMessage;
                          m_bConnected = true;
                          if (read(strMessage, strError))
                          {
                            m_message.push_back(strMessage);
                          }
                          else
                          {
                            m_bConnected = false;
                          }
                        }
                        else
                        {
                          if (m_bSSL)
                          {
                            SSL_free(m_ssl);
                            SSL_CTX_free(m_ctx);
                          }
                          close(m_fdSocket);
                          strError = "Failed to connect the SSL handle.";
                        }
                      }
                      else
                      {
                        if (m_bSSL)
                        {
                          SSL_free(m_ssl);
                          SSL_CTX_free(m_ctx);
                        }
                        close(m_fdSocket);
                        strError = "Failed to set the SSL file descriptor.";
                      }
                    }
                    else
                    {
                      if (m_bSSL)
                      {
                        SSL_CTX_free(m_ctx);
                      }
                      close(m_fdSocket);
                      strError = "Failed to create the SSL handle.";
                    }
                  }
                  else
                  {
                    close(m_fdSocket);
                    strError = "Failed to establish the SSL context.";
                  }
                }
                else
                {
                  close(m_fdSocket);
                  strError = "Failed to establish the SSL method.";
                }
              }
              else
              {
                close(m_fdSocket);
                strError = strerror(errno);
              }
            }
            else
            {
              strError = strerror(errno);
            }
          }
          freeaddrinfo(result);
        }
        else
        {
          strError = gai_strerror(nReturn);
        }
      }

      return m_bConnected;
    }
    // }}}
    // {{{ disconnect()
    void Irc::disconnect(string &strError)
    {
      if (m_bConnected)
      {
        if (!m_bQuit)
        {
          quit(strError);
        }
        m_message.clear();
        if (m_bSSL)
        {
          SSL_free(m_ssl);
          SSL_CTX_free(m_ctx);
        }
        close(m_fdSocket);
        m_bConnected = false;
      }
    }
    // }}}
    // {{{ enableSSL()
    void Irc::enableSSL(const bool bEnable)
    {
      m_bSSL = bEnable;
    }
    // }}}
    // {{{ findRoom()
    list<string>::iterator Irc::findRoom(const string strRoom)
    {
      list<string>::iterator roomIter = m_room.end();

      for (list<string>::iterator i = m_room.begin(); i != m_room.end(); i++)
      {
        if (*i == strRoom)
        {
          roomIter = i;
        }
      }

      return roomIter;
    }
    // }}}
    // {{{ getFD()
    int Irc::getFD()
    {
      return m_fdSocket;
    }
    // }}}
    // {{{ getNick()
    string Irc::getNick()
    {
      return m_strNick;
    }
    // }}}
    // {{{ getPort()
    string Irc::getPort()
    {
      return m_strPort;
    }
    // }}}
    // {{{ getServer()
    string Irc::getServer()
    {
      return m_strServer;
    }
    // }}}
    // {{{ getSSL()
    SSL *Irc::getSSL()
    {
      return m_ssl;
    }
    // }}}
    // {{{ good()
    bool Irc::good()
    {
      return m_bConnected;
    }
    // }}}
    // {{{ invite()
    bool Irc::invite(const string strNick, const string strRoom, string &strError)
    {
      bool bResult = false;

      if (!strNick.empty() && !strRoom.empty())
      {
        stringstream ssBuffer;
        ssBuffer << ":" << m_strNick << " INVITE " << strNick << " " << strRoom << "\r\n";
        if (write(ssBuffer.str(), strError) == (ssize_t)ssBuffer.str().size())
        {
          bResult = true;
        }
      }

      return bResult;
    }
    // }}}
    // {{{ join()
    bool Irc::join(const string strRoom, string &strError)
    {
      bool bResult = false;

      if (!m_strNick.empty() && !strRoom.empty() && findRoom(strRoom) == m_room.end())
      {
        stringstream ssBuffer;
        ssBuffer << ":" << m_strNick << " JOIN :" << strRoom << "\r\n";
        if (write(ssBuffer.str(), strError) == (ssize_t)ssBuffer.str().size())
        {
          bool bExit = false;
          string strMessage;
          while (!bExit && read(strMessage, strError))
          {
            string strID, strCommand;
            stringstream ssMessage;
            ssMessage.str(strMessage);
            ssMessage >> strID >> strCommand;
            if (strCommand == "366")
            {
              bExit = bResult = true;
              m_room.push_back(strRoom);
            }
            else
            {
              if (strCommand == "405" || strCommand == "437" || strCommand == "471" || strCommand == "473" || strCommand == "474" || strCommand == "475")
              {
                bExit = true;
                strError = strMessage;
              }
            }
            m_message.push_back(strMessage);
          }
        }
      }

      return bResult;
    }
    // }}}
    // {{{ next()
    bool Irc::next(string &strMessage, string &strError, const long lSec)
    {
      bool bResult = false;
      string strData;

      if (m_message.empty() && read(strData, strError, lSec) && !strData.empty())
      {
        m_message.push_back(strData);
      }
      if (!m_message.empty())
      {
        bResult = true;
        strMessage = m_message.front();
        m_message.pop_front();
      }

      return bResult;
    }
    // }}}
    // {{{ nick()
    bool Irc::nick(const string strNick, string &strError, string strUser, string strName, const int nMode)
    {
      bool bResult = false;

      if (!strNick.empty())
      {
        stringstream ssBuffer;
        if (strUser.empty())
        {
          strUser = strNick;
        }
        if (strName.empty())
        {
          strName = strNick;
        }
        ssBuffer << "NICK " << strNick;
        if (m_unIndex > 0)
        {
          stringstream ssIndex;
          ssIndex << m_unIndex;
          ssBuffer << m_unIndex;
        }
        ssBuffer << "\r\n";
        ssBuffer << "USER " << strUser << " " << nMode << " * :" << strName << "\r\n";
        if (write(ssBuffer.str(), strError) == (ssize_t)ssBuffer.str().size())
        {
          bool bExit = false;
          string strMessage;
          while (!bExit && read(strMessage, strError))
          {
            string strID, strCommand;
            stringstream ssMessage;
            ssMessage.str(strMessage);
            ssMessage >> strID >> strCommand;
            if (strCommand == "004" || strCommand == "005" || strCommand == "376")
            {
              bExit = bResult = true;
              m_strNick = strNick;
              m_strUser = strUser;
              m_strName = strName;
            }
            else if (strCommand == "251")
            {
              bExit = bResult = true;
              m_strNick = strNick;
              if (m_unIndex > 0)
              {
                stringstream ssIndex;
                ssIndex << m_unIndex;
                m_strNick += ssIndex.str();
              }
              m_strUser = strUser;
              m_strName = strName;
            }
            else
            {
              if (strCommand == "433")
              {
                bExit = true;
                m_unIndex++;
                bResult = nick(strNick, strUser, strName);
              }
              else if (strCommand == "431" || strCommand == "432" || strCommand == "436" || strCommand == "437" || strCommand == "462")
              {
                bExit = true;
                strError = strMessage;
              }
            }
            m_message.push_back(strMessage);
          }
        }
      }

      return bResult;
    } 
    // }}}
    // {{{ part()
    bool Irc::part(const string strRoom, string &strError)
    {
      bool bResult = false;
      list<string>::iterator roomIter;

      if (!m_strNick.empty() && !strRoom.empty() && (roomIter = findRoom(strRoom)) != m_room.end())
      {
        stringstream ssBuffer;
        ssBuffer << ":" << m_strNick << " PART :" << strRoom << "\r\n";
        if (write(ssBuffer.str(), strError) == (ssize_t)ssBuffer.str().size())
        {
          bResult = true;
          m_room.erase(roomIter);
        }
      }

      return bResult;
    }
    // }}}
    // {{{ quit()
    bool Irc::quit(string &strError, const string strReason)
    {
      if (!m_bQuit)
      {
        stringstream ssBuffer;
        for (list<string>::iterator i = m_room.begin(); i != m_room.end(); i++)
        {
          part(*i, strError);
        }
        m_room.clear();
        ssBuffer << ":" << m_strNick << " QUIT :" << strReason << "\r\n";
        if (write(ssBuffer.str(), strError) == (ssize_t)ssBuffer.str().size())
        {
          m_bQuit = true;
        }
        m_unIndex = 0;
      }

      return m_bQuit;
    }
    // }}}
    // {{{ read()
    bool Irc::read(string &strMessage, string &strError, const long lSec)
    {
      bool bResult = false;

      strMessage.clear();
      if (m_bConnected)
      {
        char szBuffer[1024];
        int nReturn;
        size_t nPosition;
        while (m_bConnected && !bResult)
        {
          pollfd fds[1];
          fds[0].fd = m_fdSocket;
          fds[0].events = POLLIN;
          if ((nReturn = poll(fds, 1, ((lSec <= 0)?-1:(lSec * 1000)))) > 0)
          {
            if (fds[0].fd == m_fdSocket && (fds[0].revents & POLLIN))
            {
              if ((!m_bSSL && (nReturn = ::read(m_fdSocket, szBuffer, 1024)) > 0) || (m_bSSL && (nReturn = SSL_read(m_ssl, szBuffer, 1024)) > 0))
              {
                m_strBuffer.append(szBuffer, nReturn);
                while ((nPosition = m_strBuffer.find("\r", 0)) != string::npos)
                {
                  m_strBuffer.erase(nPosition, 1);
                }
                while ((nPosition = m_strBuffer.find("\n", 0)) != string::npos)
                {
                  bResult = true;
                  strMessage = m_strBuffer.substr(0, nPosition);
                  if (strMessage.size() >= 4 && strMessage.substr(0, 4) == "PING")
                  {
                    string strPong = strMessage;
                    strPong[1] = 'O';
                    write(strPong + "\r\n", strError);
                  }
                  m_strBuffer.erase(0, nPosition + 1);
                }
              }
              else
              {
                strError = strerror(errno);
                disconnect(strError);
              }
            }
          }
          else if (nReturn < 0)
          {
            strError = strerror(errno);
            disconnect(strError);
          }
        }
      }

      return bResult;
    }
    // }}}
    // {{{ setPort()
    void Irc::setPort(const string strPort)
    {
      m_strPort = strPort;
    } 
    // }}}
    // {{{ setServer()
    void Irc::setServer(const string strServer)
    {
      m_strServer = strServer;
    } 
    // }}}
    // {{{ write()
    ssize_t Irc::write(const string strBuffer, string &strError)
    {
      return write(strBuffer.c_str(), strBuffer.size(), strError);
    } 
    ssize_t Irc::write(const char *pszBuffer, size_t nSize, string &strError)
    {
      ssize_t nReturn = 0;

      if (m_bConnected)
      {
        if (!m_bSSL)
        {
          if ((nReturn = ::write(m_fdSocket, pszBuffer, nSize)) != (ssize_t)nSize)
          {
            strError = strerror(errno);
          }
        }
        else
        {
          if ((nReturn = SSL_write(m_ssl, pszBuffer, nSize)) != (ssize_t)nSize)
          {
            strError = strerror(errno);
          }
        }
      }

      return nReturn;
    } 
    // }}}
  }
}
