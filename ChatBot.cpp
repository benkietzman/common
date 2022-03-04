// vim: syntax=cpp
// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Chat Bot
// -------------------------------------
// file       : ChatBot.cpp
// author     : Ben Kietzman
// begin      : 2018-01-12
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

/*! \file ChatBot.cpp
* \brief ChatBot Class
*/
// {{{ includes
#include "ChatBot"
// }}}
extern "C++"
{
  namespace common
  {
    // {{{ ChatBot()
    ChatBot::ChatBot()
    {
      string strError;

      m_bConnected = false;
      m_bDebug = false;
      m_bQuit = false;
      m_bSecure = true;
      m_ctx = NULL;
      m_fdSocket = -1;
      m_pMessage = NULL;
      m_pUtility = new Utility(strError);
      m_ssl = NULL;
    }
    // }}}
    // {{{ ~ChatBot()
    ChatBot::~ChatBot()
    {
      if (m_bConnected)
      {
        disconnect();
      }
      m_formArguments.clear();
      delete m_pUtility;
    }
    // }}}
    // {{{ analyze()
    void ChatBot::analyze(Json *ptMessage)
    {
      if (m_pMessage != NULL)
      {
        (*m_pMessage)(ptMessage);
      }
      // {{{ Function
      if (ptMessage->m.find("Function") != ptMessage->m.end())
      {
        bool bStatus = false;
        string strFunction = ptMessage->m["Function"]->v, strStatus;
        if (ptMessage->m.find("Status") != ptMessage->m.end())
        {
          bStatus = true;
          strStatus = ptMessage->m["Status"]->v;
        }
        // {{{ connect
        if (strFunction == "connect")
        {
          if (strStatus == "okay")
          {
            if (ptMessage->m.find("Response") != ptMessage->m.end() && ptMessage->m["Response"]->m.find("Rooms") != ptMessage->m["Response"]->m.end())
            {
              for (map<string, Json *>::iterator i = ptMessage->m["Response"]->m["Rooms"]->m.begin(); i != ptMessage->m["Response"]->m["Rooms"]->m.end(); i++)
              {
                m_rooms.push_back(i->first);
              }
              m_rooms.sort();
              m_rooms.unique();
            }
            m_bConnected = true;
          }
          else if (bStatus)
          {
            disconnect();
          }
        }
        // }}}
        // {{{ disconnect
        else if (strFunction == "disconnect")
        {
          close();
        }
        // }}}
        // {{{ join
        else if (strFunction == "join")
        {
          if (strStatus == "okay")
          {
            if (ptMessage->m.find("Request") != ptMessage->m.end() && ptMessage->m["Request"]->m.find("Room") != ptMessage->m["Request"]->m.end() && !ptMessage->m["Request"]->m["Room"]->v.empty())
            {
              m_rooms.push_back(ptMessage->m["Request"]->m["Room"]->v);
              m_rooms.sort();
              m_rooms.unique();
            }
          }
        }
        // }}}
        // {{{ part
        else if (strFunction == "part")
        {
          if (strStatus == "okay")
          {
            if (ptMessage->m.find("Request") != ptMessage->m.end() && ptMessage->m["Request"]->m.find("Room") != ptMessage->m["Request"]->m.end() && !ptMessage->m["Request"]->m["Room"]->v.empty())
            {
              list<string>::iterator removeIter = m_rooms.end();
              for (list<string>::iterator i = m_rooms.begin(); removeIter == m_rooms.end() && i != m_rooms.end(); i++)
              {
                if ((*i) == ptMessage->m["Request"]->m["Room"]->v)
                {
                  removeIter = i;
                }
              }
              if (removeIter != m_rooms.end())
              {
                m_rooms.erase(removeIter);
              }
              if (m_bQuit)
              {
                if (!m_rooms.empty())
                {
                  part(m_rooms.front());
                }
                else
                {
                  disconnect();
                }
              }
            }
          }
        }
        // }}}
      }
      // }}}
    }
    // }}}
    // {{{ close()
    bool ChatBot::close(string &strError)
    {
      bool bResult = false;
      stringstream ssError;

      m_bConnected = false;
      if (m_ssl != NULL)
      {
        SSL_shutdown(m_ssl);
        SSL_free(m_ssl);
        m_ssl = NULL;
      }
      if (m_ctx != NULL)
      {
        SSL_CTX_free(m_ctx);
        m_ctx = NULL;
      }
      if (m_fdSocket == -1)
      {
        if (::close(m_fdSocket) == 0)
        {
          bResult = true;
        }
        else
        {
          ssError.str("");
          ssError << "close(" << errno << ") " << strerror(errno);
          strError = ssError.str();
        }
        m_fdSocket = -1;
      }
      while (!m_messages.empty())
      {
        delete m_messages.front();
        m_messages.pop_front();
      }

      return bResult;
    }
    void ChatBot::close()
    {
      string strError;

      close(strError);
    }
    // }}}
    // {{{ connect()
    bool ChatBot::connect(const string strServer, const string strPort, const string strUser, const string strPassword, const string strName, string &strError)
    {
      bool bResult = false;
      addrinfo hints, *result;
      int fdSocket, nReturn;
      stringstream ssError;

      m_strUser = strUser;
      memset(&hints, 0, sizeof(addrinfo));
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      if (m_bSecure)
      {
        SSL_METHOD *method = (SSL_METHOD *)SSLv23_client_method();
        if ((m_ctx = SSL_CTX_new(method)) == NULL)
        {
          m_bSecure = false;
        }
      }
      if ((nReturn = getaddrinfo(strServer.c_str(), strPort.c_str(), &hints, &result)) == 0)
      {
        bool bSocket = false;
        addrinfo *rp;
        for (rp = result; m_fdSocket == -1 && rp != NULL; rp = rp->ai_next)
        {
          bSocket = false;
          if ((fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
          {
            bSocket = true;
            if (::connect(fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
            {
              if (!m_bSecure || (m_ssl = m_pUtility->sslConnect(m_ctx, fdSocket, strError)) != NULL)
              {
                m_fdSocket = fdSocket;
              }
              else
              {
                ::close(fdSocket);
              }
            }
            else
            {
              ::close(fdSocket);
            }
          }
        }
        freeaddrinfo(result);
        if (m_fdSocket != -1)
        {
          Json *ptMessage = new Json;
          bResult = true;
          ptMessage->insert("Section", "chat");
          ptMessage->insert("Function", "connect");
          ptMessage->insert("User", strUser);
          ptMessage->insert("Password", strPassword);
          ptMessage->m["Request"] = new Json;
          ptMessage->m["Request"]->insert("Name", strName);
          ptMessage->insert("wsRequestID", "-1", 'n');
          putMessage(ptMessage);
        }
        else
        {
          ssError.str("");
          ssError << ((bSocket)?"connect":"socket") << "(" << errno << ") " << strerror(errno);
          strError = ssError.str();
        }
      }
      else
      {
        ssError.str("");
        ssError << "getaddrinfo(" << nReturn << ") " << gai_strerror(nReturn);
        strError = ssError.str();
      }

      return bResult;
    }
    // }}}
    // {{{ disconnect()
    void ChatBot::disconnect()
    {
      putMessage("disconnect");
    }
    // }}}
    // {{{ form()
    string ChatBot::form(const string strIdent, const string strForm, const string strHeader, const string strFooter)
    {
      map<string, string> arguments;

      return form(strIdent, strForm, arguments, strHeader, strFooter);
    }
    string ChatBot::form(const string strIdent, const string strForm, map<string, string> arguments, const string strHeader, const string strFooter)
    {
      stringstream ssForm;

      ssForm << "<form onsubmit=\"fetch('" << m_strFormAction << "', {method: 'POST', cache: 'no-cache', body: new URLSearchParams(new FormData(this))}).then(response =&gt; {}); return false;\">";
      ssForm << "<input type=\"hidden\" name=\"botIdent\" value=\"" << strIdent << "\">";
      for (map<string, string>::iterator i = m_formArguments.begin(); i != m_formArguments.end(); i++)
      {
        ssForm << "<input type=\"hidden\" name=\"" << i->first << "\" value=\"" << i->second << "\">";
      }
      for (map<string, string>::iterator i = arguments.begin(); i != arguments.end(); i++)
      {
        ssForm << "<input type=\"hidden\" name=\"" << i->first << "\" value=\"" << i->second << "\">";
      }
      ssForm << "<div style=\"border-style: solid; border-width: 1px; border-color: #4e5964; border-radius: 10px; background: #2c3742; padding: 10px; color: white;\">";
      if (!m_strFormHeader.empty())
      {
        ssForm << m_strFormHeader;
      }
      if (!strHeader.empty())
      {
        ssForm << strHeader;
      }
      ssForm << strForm;
      if (!strFooter.empty())
      {
        ssForm << strFooter;
      }
      if (!m_strFormFooter.empty())
      {
        ssForm << m_strFormFooter;
      }
      ssForm << "</div>";
      ssForm << "</form>";

      return ssForm.str();
    }
    // }}}
    // {{{ ident()
    string ChatBot::ident(const string strUser)
    {
      char md5string[33];
      MD5_CTX context;
      string strIdent;
      stringstream ssIdent;
      timespec start;
      unsigned char digest[16];

      clock_gettime(CLOCK_REALTIME, &start);
      ssIdent << strUser << "," << start.tv_sec << "," << start.tv_nsec;
      MD5_Init(&context);
      MD5_Update(&context, ssIdent.str().c_str(), ssIdent.str().size());
      MD5_Final(digest, &context);
      for (int i = 0; i < 16; i++)
      {
        sprintf(&md5string[i*2], "%02x", (unsigned int)digest[i]);
      }
      strIdent = md5string;

      return strIdent;
    }
    // }}}
    // {{{ join()
    void ChatBot::join(const string strRoom)
    {
      Json *ptMessage = new Json;

      ptMessage->insert("Room", strRoom);
      putMessage("join", ptMessage);
    }
    // }}}
    // {{{ message()
    void ChatBot::message(const string strTarget, const string strMessage, const string strText)
    {
      Json *ptMessage = new Json;

      ptMessage->insert("Target", strTarget);
      ptMessage->insert("Message", strMessage);
      if (!strText.empty())
      {
        ptMessage->insert("Text", strText);
      }
      putMessage("message", ptMessage);
    }
    // }}}
    // {{{ part()
    void ChatBot::part(const string strRoom)
    {
      Json *ptMessage = new Json;

      ptMessage->insert("Room", strRoom);
      putMessage("part", ptMessage);
    }
    // }}}
    // {{{ ping()
    void ChatBot::ping()
    {
      putMessage("ping");
    }
    // }}}
    // {{{ process()
    bool ChatBot::process(int nTimeout, string &strError)
    {
      bool bResult = false;
      stringstream ssError;

      if (m_fdSocket != -1)
      {
        int nReturn;
        pollfd fds[1];
        size_t unPosition;
        fds[0].fd = m_fdSocket;
        fds[0].events = POLLIN;
        if (!m_strBuffer[1].empty())
        {
          fds[0].events |= POLLOUT;
        }
        else if (!m_messages.empty())
        {
          Json *ptDebug = new Json(m_messages.front());
          if (m_bConnected || (ptDebug->m.find("Function") != ptDebug->m.end() && ptDebug->m["Function"]->v == "connect"))
          {
            stringstream ssJson;
            fds[0].events |= POLLOUT;
            if (m_bDebug)
            {
              if (ptDebug->m.find("Password") != ptDebug->m.end())
              {
                ptDebug->insert("Password", "******");
              }
              cout << "WRITE:  " << ptDebug << endl;
            }
            ssJson << m_messages.front();
            delete m_messages.front();
            m_messages.pop_front();
            m_strBuffer[1] += ssJson.str() + "\n";
          }
          delete ptDebug;
        }
        if ((nReturn = poll(fds, 1, nTimeout)) > 0)
        {
          if (fds[0].revents & POLLIN)
          {
            if ((m_ssl != NULL && m_pUtility->sslRead(m_ssl, m_strBuffer[0], nReturn)) || (m_ssl == NULL && m_pUtility->fdRead(m_fdSocket, m_strBuffer[0], nReturn)))
            {
              bResult = true;
              while ((unPosition = m_strBuffer[0].find("\n")) != string::npos)
              {
                Json *ptMessage = new Json(m_strBuffer[0].substr(0, unPosition));
                m_strBuffer[0].erase(0, (unPosition + 1));
                if (m_bDebug)
                {
                  Json *ptDebug = new Json(ptMessage);
                  if (ptDebug->m.find("Password") != ptDebug->m.end())
                  {
                    ptDebug->insert("Password", "******");
                  }
                  cout << "READ:  " << ptDebug << endl;
                  delete ptDebug;
                }
                analyze(ptMessage);
                delete ptMessage;
              }
            }
            else
            {
              ssError.str("");
              if (m_ssl != NULL)
              {
                ssError << "Utility::sslRead(" << nReturn << ") " << m_pUtility->sslstrerror(m_ssl, nReturn);
              }
              else
              {
                ssError << "Utility::fdRead(" << errno << ") " << strerror(errno);
              }
              strError = ssError.str();
            }
          }
          if (fds[0].revents & POLLOUT)
          {
            if ((m_ssl != NULL && m_pUtility->sslWrite(m_ssl, m_strBuffer[1], nReturn)) || (m_ssl == NULL && m_pUtility->fdWrite(m_fdSocket, m_strBuffer[1], nReturn)))
            {
              bResult = true;
            }
            else
            {
              ssError.str("");
              if (m_ssl != NULL)
              {
                ssError << "Utility::sslWrite(" << nReturn << ") " << m_pUtility->sslstrerror(m_ssl, nReturn);
              }
              else
              {
                ssError << "Utility::fdWrite(" << errno << ") " << strerror(errno);
              }
              strError = ssError.str();
            }
          }
        }
        else if (nReturn == 0 || errno == 4)
        {
          bResult = true;
        }
        else
        {
          ssError.str("");
          ssError << "poll(" << errno << ") " << strerror(errno);
          strError = ssError.str();
        }
        if (!bResult)
        {
          close();
        }
      }
      else
      {
        strError = "Please provide the socket file descriptor.";
        close();
      }

      return bResult;
    }
    // }}}
    // {{{ putMessage()
    void ChatBot::putMessage(Json *ptMessage)
    {
      m_messages.push_back(ptMessage);
    }
    void ChatBot::putMessage(const string strFunction, Json *ptMessage)
    {
      Json *ptRequest = new Json;
 
      ptRequest->insert("Function", strFunction);
      ptRequest->m["Request"] = ((ptMessage != NULL)?ptMessage:new Json);
      m_messages.push_back(ptRequest);
    }
    // }}}
    // {{{ quit()
    void ChatBot::quit()
    {
      m_bQuit = true;
      if (!m_rooms.empty())
      {
        part(m_rooms.front());
      }
      else
      {
        disconnect();
      }
    }
    // }}}
    // {{{ rooms()
    list<string> ChatBot::rooms()
    {
      return m_rooms;
    }
    // }}}
    // {{{ setDebug()
    void ChatBot::setDebug(const bool bDebug)
    {
      m_bDebug = bDebug;
    }
    // }}}
    // {{{ setForm()
    void ChatBot::setForm(const string strAction, const string strHeader, const string strFooter)
    {
      map<string, string> arguments;

      setForm(strAction, arguments, strHeader, strFooter);
    }
    void ChatBot::setForm(const string strAction, map<string, string> arguments, const string strHeader, const string strFooter)
    {
      m_strFormAction = strAction;
      if (!arguments.empty())
      {
        m_formArguments = arguments;
      }
      if (!strHeader.empty())
      {
        m_strFormHeader = strHeader;
      }
      if (!strFooter.empty())
      {
        m_strFormFooter = strFooter;
      }
    }
    // }}}
    // {{{ setFormArguments()
    void ChatBot::setFormArguments(map<string, string> arguments)
    {
      m_formArguments = arguments;
    }
    // }}}
    // {{{ setFormFooter()
    void ChatBot::setFormFooter(const string strFooter)
    {
      m_strFormFooter = strFooter;
    }
    // }}}
    // {{{ setFormHeader()
    void ChatBot::setFormHeader(const string strHeader)
    {
      m_strFormHeader = strHeader;
    }
    // }}}
    // {{{ setMessage()
    void ChatBot::setMessage(void (*pMessage)(Json *))
    {
      m_pMessage = pMessage;
    }
    // }}}
  }
}
