// vim: syntax=cpp
// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Service Junction
// -------------------------------------
// file       : ServiceJunction.cpp
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

/*! \file ServiceJunction.cpp
* \brief ServiceJunction Class
*/
// {{{ includes
#include "ServiceJunction"
// }}}
extern "C++"
{
  namespace common
  {
    #ifdef COMMON_LINUX
    sem_t m_semaServiceJunctionRequestLock;
    #endif
    #ifdef COMMON_SOLARIS
    sema_t m_semaServiceJunctionRequestLock;
    #endif
    #ifdef COMMON_LINUX
    sem_t m_semaServiceJunctionThrottle;
    #endif
    #ifdef COMMON_SOLARIS
    sema_t m_semaServiceJunctionThrottle;
    #endif
    // {{{ ServiceJunction()
    ServiceJunction::ServiceJunction(string &strError)
    {
      #ifdef COMMON_LINUX
      sem_init(&m_semaServiceJunctionRequestLock, 0, 10);
      #endif
      #ifdef COMMON_SOLARIS
      sema_init(&m_semaServiceJunctionRequestLock, 10, USYNC_THREAD, NULL);
      #endif
      m_bDodgeConcentrator = false;
      m_bUseSecureJunction = true;
      m_bUseSingleSocket = false;
      m_ulModifyTime = 0;
      m_unThrottle = 0;
      m_unUniqueID = 0;
      m_pUtility = new Utility(strError);
    }
    // }}}
    // {{{ ~ServiceJunction()
    ServiceJunction::~ServiceJunction()
    {
      if (m_bUseSingleSocket)
      {
        useSingleSocket(false);
      }
      m_loggerLabel.clear();
      delete m_pUtility;
      #ifdef COMMON_LINUX
      sem_destroy(&m_semaServiceJunctionRequestLock);
      #endif
      #ifdef COMMON_SOLARIS
      sema_destroy(&m_semaServiceJunctionRequestLock);
      #endif
      if (m_unThrottle > 0)
      {
        #ifdef COMMON_LINUX
        sem_destroy(&m_semaServiceJunctionThrottle);
        #endif
        #ifdef COMMON_SOLARIS
        sema_destroy(&m_semaServiceJunctionThrottle);
        #endif
      }
    }
    // }}}
    // {{{ bridge()
    bool ServiceJunction::bridge(const string strUser, const string strPassword, Json *ptRequest, Json *ptResponse, string &strError)
    {
      bool bResult = false;
      Json *ptJson, *ptSubJson;
      list<string> in, out;
      string strJson;

      ptJson = new Json(ptRequest);;
      if (!m_strApplication.empty())
      {
        ptJson->insert("reqApp", m_strApplication);
      }
      if (!m_strProgram.empty())
      {
        ptJson->insert("reqProg", m_strProgram);
      }
      if (!m_strTimeout.empty())
      {
        ptJson->insert("Timeout", m_strTimeout);
      }
      ptJson->insert("Service", "bridge");
      ptJson->insert("User", strUser);
      ptJson->insert("Password", strPassword);
      if (ptJson->m.find("Request") != ptJson->m.end())
      {
        ptSubJson = ptJson->m["Request"];
        ptJson->m.erase("Request");
      }
      else
      {
        ptSubJson = new Json;
      }
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      in.push_back(ptSubJson->json(strJson));
      delete ptSubJson;
      if (request(in, out, strError))
      {
        Json *ptStatus = new Json(out.front());
        if (ptStatus->m.find("Status") != ptStatus->m.end() && ptStatus->m["Status"]->v == "okay")
        {
          bResult = true;
        }
        else if (ptStatus->m.find("Error") != ptStatus->m.end())
        {
          if (ptStatus->m["Error"]->m.find("Message") != ptStatus->m["Error"]->m.end() && !ptStatus->m["Error"]->m["Message"]->v.empty())
          {
            strError = ptStatus->m["Error"]->m["Message"]->v;
          }
          else if (!ptStatus->m["Error"]->v.empty())
          {
            strError = ptStatus->m["Error"]->v;
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
        if (out.size() == 2)
        {
          ptResponse->parse(out.back());
        }
        else if (ptStatus->m.find("Response") != ptStatus->m.end())
        {
          ptResponse->merge(ptStatus->m["Response"], true, false);
        }
        delete ptStatus;
      }
      in.clear();
      out.clear();

      return bResult;
    }
    // }}}
    // {{{ curl()
    bool ServiceJunction::curl(const string strURL, const string strType, Json *ptAuth, Json *ptGet, Json *ptPost, Json *ptPut, const string strProxy, string &strCookies, string &strHeader, string &strContent, string &strError, const string strUserAgent, const bool bMobile, const bool bFailOnError, const string strCustomRequest)
    {
      bool bResult = false;
      Json *ptJson;
      list<string> in, out;
      map<string, string> requestArray;
      string strJson;

      if (!m_strApplication.empty())
      {
        requestArray["reqApp"] = m_strApplication;
      }
      if (!m_strProgram.empty())
      {
        requestArray["reqProg"] = m_strProgram;
      }
      if (!m_strTimeout.empty())
      {
        requestArray["Timeout"] = m_strTimeout;
      }
      requestArray["Service"] = "curl";
      ptJson = new Json(requestArray);
      requestArray.clear();
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      ptJson = new Json;
      ptJson->insert("URL", strURL);
      ptJson->insert("Display", "Content,Cookies,Header");
      if (!strCookies.empty())
      {
        ptJson->insert("Cookies", strCookies);
        strCookies.clear();
      }
      if (ptAuth != NULL)
      {
        ptJson->m["Auth"] = new Json(ptAuth);
      }
      if (!strContent.empty())
      {
        ptJson->insert("Content", strContent);
      }
      ptJson->insert("FailOnError", ((bFailOnError)?"yes":"no"));
      if (!strCustomRequest.empty())
      {
        ptJson->insert("CustomRequest", strCustomRequest);
      }
      if (ptGet != NULL)
      {
        ptJson->m["Get"] = new Json(ptGet);
      }
      if (!strHeader.empty())
      {
        ptJson->insert("Header", strHeader);
      }
      if (!strUserAgent.empty())
      {
        ptJson->insert("UserAgent", strUserAgent);
      }
      ptJson->insert("Mobile", ((bMobile)?"yes":"no"));
      if (ptPost != NULL)
      {
        ptJson->m["Post"] = new Json(ptPost);
      }
      if (ptPut != NULL)
      {
        ptJson->m["Put"] = new Json(ptPut);
      }
      if (!strProxy.empty())
      {
        ptJson->insert("Proxy", strProxy);
      }
      if (!strType.empty())
      {
        ptJson->insert("Type", strType);
      }
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      if (request(in, out, strError))
      {
        map<string, string> requestResponse;
        ptJson = new Json(out.front());
        ptJson->flatten(requestResponse, true);
        delete ptJson;
        if (out.size() == 2)
        {
          ptJson = new Json(out.back());
          if (ptJson->m.find("Cookies") != ptJson->m.end())
          {
            strCookies = ptJson->m["Cookies"]->v;
          }
          if (ptJson->m.find("Header") != ptJson->m.end())
          {
            strHeader = ptJson->m["Header"]->v;
          }
          if (ptJson->m.find("Content") != ptJson->m.end())
          {
            strContent = ptJson->m["Content"]->v;
          }
          delete ptJson;
        }
        if (requestResponse.find("Status") != requestResponse.end() && requestResponse["Status"] == "okay")
        {
          bResult = true;
        }
        else if (requestResponse.find("Error") != requestResponse.end() && !requestResponse["Error"].empty())
        {
          strError = requestResponse["Error"];
        }
        else
        {
          strError = "Encountered an unknown error.";
        }
        requestResponse.clear();
      }
      in.clear();
      out.clear();

      return bResult;
    }
    // }}}
    // {{{ dodgeConcentrator()
    void ServiceJunction::dodgeConcentrator(const bool bDodge)
    {
      m_bDodgeConcentrator = bDodge;
    }
    // }}}
    // {{{ email()
    bool ServiceJunction::email(const string strFrom, list<string> toList, list<string> ccList, list<string> bccList, const string strSubject, const string strText, const string strHTML, list<string> fileList, string &strError)
    {
      bool bResult = false;
      map<string, string> fileMap;

      for (list<string>::iterator i = fileList.begin(); i != fileList.end(); i++)
      {
        if (m_file.fileExist(*i))
        {
          int fdFile;
          size_t nPosition;
          string strName = (*i);
          if ((nPosition = i->rfind("/", i->size() - 1)) != string::npos)
          {
            strName = i->substr(nPosition + 1, i->size() - (nPosition + 1));
          }
          if ((fdFile = open(i->c_str(), O_RDONLY)) > 0)
          {
            char szBuffer[65536];
            ssize_t nReturn;
            string strData;
            while ((nReturn = read(fdFile, szBuffer, 65536)) > 0)
            {
              strData.append(szBuffer, nReturn);
            }
            fileMap[strName] = strData;
            close(fdFile);
          }
        }
      }
      bResult = email(strFrom, toList, ccList, bccList, strSubject, strText, strHTML, fileMap, strError);
      fileMap.clear();

      return bResult;
    }
    bool ServiceJunction::email(const string strFrom, list<string> toList, list<string> ccList, list<string> bccList, const string strSubject, const string strText, const string strHTML, map<string, string> fileMap, string &strError)
    {
      bool bResult = false;
      Json *ptJson;
      list<string> in, out;
      map<string, string> requestArray;
      string strJson;

      if (!m_strApplication.empty())
      {
        requestArray["reqApp"] = m_strApplication;
      }
      if (!m_strProgram.empty())
      {
        requestArray["reqProg"] = m_strProgram;
      }
      if (!m_strTimeout.empty())
      {
        requestArray["Timeout"] = m_strTimeout;
      }
      requestArray["Service"] = "email";
      if (!strFrom.empty())
      {
        requestArray["From"] = strFrom;
      }
      if (!toList.empty())
      {
        requestArray["To"] = "";
        for (list<string>::iterator i = toList.begin(); i != toList.end(); i++)
        {
          if (i != toList.begin())
          {
            requestArray["To"] += ",";
          }
          requestArray["To"] += (*i);
        }
      }
      if (!ccList.empty())
      {
        requestArray["CC"] = "";
        for (list<string>::iterator i = ccList.begin(); i != ccList.end(); i++)
        {
          if (i != ccList.begin())
          {
            requestArray["CC"] += ",";
          }
          requestArray["CC"] += (*i);
        }
      }
      if (!bccList.empty())
      {
        requestArray["BCC"] = "";
        for (list<string>::iterator i = bccList.begin(); i != bccList.end(); i++)
        {
          if (i != bccList.begin())
          {
            requestArray["BCC"] += ",";
          }
          requestArray["BCC"] += (*i);
        }
      }
      if (!strSubject.empty())
      {
        requestArray["Subject"] = strSubject;
      }
      if (!strText.empty())
      {
        requestArray["Text"] = strText;
      }
      if (!strHTML.empty())
      {
        requestArray["HTML"] = strHTML;
      }
      ptJson = new Json(requestArray);
      requestArray.clear();
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      for (map<string, string>::iterator i = fileMap.begin(); i != fileMap.end(); i++)
      {
        string strData;
        m_manip.encodeBase64(i->second, strData);
        requestArray["Name"] = i->first;
        requestArray["Data"] = strData;
        requestArray["Encode"] = "base64";
        ptJson = new Json(requestArray);
        requestArray.clear();
        in.push_back(ptJson->json(strJson));
        delete ptJson;
      }
      if (request(in, out, strError) && !out.empty())
      {
        map<string, string> requestResponse;
        bResult = true;
        ptJson = new Json(out.front());
        ptJson->flatten(requestResponse, true);
        delete ptJson;
        if (requestResponse.find("Status") != requestResponse.end() && requestResponse["Status"] == "okay")
        {
          bResult = true;
        }
        else if (requestResponse.find("Error") != requestResponse.end() && !requestResponse["Error"].empty())
        {
          strError = requestResponse["Error"];
        }
        else
        {
          strError = "Encountered an unknown error.";
        }
        requestResponse.clear();
      }
      in.clear();
      out.clear();

      return bResult;
    }
    // }}}
    // {{{ finance()
    bool ServiceJunction::finance(const string strFunction, const string strSymbol, const string strCred, map<string, string> &jsonMap, string &strError)
    {
      bool bResult = false;
      Json *ptJson;
      list<string> in, out;
      map<string, string> requestArray;
      string strJson;

      if (!m_strApplication.empty())
      {
        requestArray["reqApp"] = m_strApplication;
      }
      if (!m_strProgram.empty())
      {
        requestArray["reqProg"] = m_strProgram;
      }
      if (!m_strTimeout.empty())
      {
        requestArray["Timeout"] = m_strTimeout;
      }
      requestArray["Service"] = "finance";
      requestArray["Function"] = strFunction;
      if (!strCred.empty())
      {
        ifstream inCred(strCred.c_str());
        string strLine;
        if (inCred.good())
        {
          while (utility()->getLine(inCred, strLine))
          {
            Json *ptSubJson = new Json(strLine);
            map<string, string> cred;
            ptSubJson->flatten(cred, true);
            delete ptSubJson;
            if (cred["Name"] == strFunction)
            {
              for (map<string, string>::iterator i = cred.begin(); i != cred.end(); i++)
              {
                if (i->first != "Name")
                {
                  requestArray[i->first] = i->second;
                }
              }
            }
            cred.clear();
          }
        }
        inCred.close();
      }
      requestArray["Symbol"] = strSymbol;
      ptJson = new Json(requestArray);
      requestArray.clear();
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      if (request(in, out, strError))
      {
        bool bFirst = true;
        for (list<string>::iterator i = out.begin(); i != out.end(); i++)
        {
          if (bFirst)
          {
            map<string, string> requestResponse;
            bFirst = false;
            ptJson = new Json(*i);
            ptJson->flatten(requestResponse, true);
            delete ptJson;
            if (requestResponse.find("Status") != requestResponse.end() && requestResponse["Status"] == "okay")
            {
              bResult = true;
            }
            else if (requestResponse.find("Error") != requestResponse.end() && !requestResponse["Error"].empty())
            {
              strError = requestResponse["Error"];
            }
            else
            {
              strError = "Encountered an unknown error.";
            }
            requestResponse.clear();
          }
          else
          {
            list<string> removeList;
            ptJson = new Json(*i);
            ptJson->flatten(jsonMap, true);
            delete ptJson;
            for (map<string, string>::iterator j = jsonMap.begin(); j != jsonMap.end(); j++)
            {
              if (j->second.empty())
              {
                removeList.push_back(j->first);
              }
            }
            for (list<string>::iterator j = removeList.begin(); j != removeList.end(); j++)
            {
              jsonMap.erase(*j);
            }
            removeList.clear();
          }
        }
      }
      in.clear();
      out.clear();

      return bResult;
    }
    // }}}
    // {{{ fusionQuery()
    bool ServiceJunction::fusionQuery(const string strUser, const string strPassword, string &strAuth, const string strQuery, list<map<string, string> > &result, string &strError)
    {
      bool bResult = false;
      Json *ptJson;
      list<string> in, out;
      map<string, string> requestArray;
      string strJson;

      for (list<map<string, string> >::iterator i = result.begin(); i != result.end(); i++)
      {
        i->clear();
      }
      result.clear();
      if (!m_strApplication.empty())
      {
        requestArray["reqApp"] = m_strApplication;
      }
      if (!m_strProgram.empty())
      {
        requestArray["reqProg"] = m_strProgram;
      }
      if (!m_strTimeout.empty())
      {
        requestArray["Timeout"] = m_strTimeout;
      }
      requestArray["Service"] = "google";
      requestArray["Application"] = "fusion";
      if (!strAuth.empty())
      {
        requestArray["Auth"] = strAuth;
      }
      else
      {
        requestArray["User"] = strUser;
        requestArray["Password"] = strPassword;
      }
      requestArray["Query"] = strQuery;
      ptJson = new Json(requestArray);
      requestArray.clear();
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      if (request(in, out, strError))
      {
        bool bFirst = true;
        for (list<string>::iterator i = out.begin(); i != out.end(); i++)
        {
          if (bFirst)
          {
            map<string, string> requestResponse;
            bFirst = false;
            ptJson = new Json(*i);
            ptJson->flatten(requestResponse, true);
            delete ptJson;
            if (requestResponse.find("Status") != requestResponse.end() && requestResponse["Status"] == "okay")
            {
              bResult = true;
              if (requestResponse.find("Auth") != requestResponse.end() && !requestResponse["Auth"].empty())
              {
                strAuth = requestResponse["Auth"];
              }
            }
            else if (requestResponse.find("Error") != requestResponse.end() && !requestResponse["Error"].empty())
            {
              strError = requestResponse["Error"];
            }
            else
            {
              strError = "Encountered an unknown error.";
            }
            requestResponse.clear();
          }
          else
          {
            map<string, string> row;
            ptJson = new Json(*i);
            ptJson->flatten(row, true);
            delete ptJson;
            result.push_back(row);
            row.clear();
          }
        }
      }
      in.clear();
      out.clear();

      return bResult;
    }
    // }}}
    // {{{ fusionUpdate()
    bool ServiceJunction::fusionUpdate(const string strUser, const string strPassword, string &strAuth, const string strUpdate, string &strError)
    {
      bool bResult = false;
      Json *ptJson;
      list<map<string, string> > result;
      list<string> in, out;
      map<string, string> requestArray;
      string strJson;

      if (!m_strApplication.empty())
      {
        requestArray["reqApp"] = m_strApplication;
      }
      if (!m_strProgram.empty())
      {
        requestArray["reqProg"] = m_strProgram;
      }
      if (!m_strTimeout.empty())
      {
        requestArray["Timeout"] = m_strTimeout;
      }
      requestArray["Service"] = "google";
      requestArray["Application"] = "fusion";
      if (!strAuth.empty())
      {
        requestArray["Auth"] = strAuth;
      }
      else
      {
        requestArray["User"] = strUser;
        requestArray["Password"] = strPassword;
      }
      requestArray["Update"] = strUpdate;
      ptJson = new Json(requestArray);
      requestArray.clear();
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      if (request(in, out, strError))
      {
        map<string, string> requestResponse;
        ptJson = new Json(out.front());
        ptJson->flatten(requestResponse, true);
        delete ptJson;
        if (requestResponse.find("Status") != requestResponse.end() && requestResponse["Status"] == "okay")
        {
          bResult = true;
          if (requestResponse.find("Auth") != requestResponse.end() && !requestResponse["Auth"].empty())
          {
            strAuth = requestResponse["Auth"];
          }
        }
        else if (requestResponse.find("Error") != requestResponse.end() && !requestResponse["Error"].empty())
        {
          strError = requestResponse["Error"];
        }
        else
        {
          strError = "Encountered an unknown error.";
        }
        requestResponse.clear();
      }
      in.clear();
      out.clear();

      return bResult;
    }
    // }}}
    // {{{ ircBot()
    bool ServiceJunction::ircBot(const string strRoom, const string strMessage, string &strError)
    {
      return ircBot("", strRoom, strMessage, strError);
    }
    bool ServiceJunction::ircBot(const string strSender, const string strRoom, const string strMessage, string &strError)
    {
      bool bResult = false;
      Json *ptJson;
      list<string> in, out;
      map<string, string> requestArray;
      string strJson;

      if (!m_strApplication.empty())
      {
        requestArray["reqApp"] = m_strApplication;
      }
      if (!m_strProgram.empty())
      {
        requestArray["reqProg"] = m_strProgram;
      }
      if (!m_strTimeout.empty())
      {
        requestArray["Timeout"] = m_strTimeout;
      }
      requestArray["Service"] = "ircBot";
      requestArray["Sender"] = strSender;
      requestArray["Room"] = strRoom;
      requestArray["Message"] = strMessage;
      ptJson = new Json(requestArray);
      requestArray.clear();
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      if (request(in, out, strError))
      {
        map<string, string> requestResponse;
        ptJson = new Json(out.front());
        ptJson->flatten(requestResponse, true);
        delete ptJson;
        if (requestResponse.find("Status") != requestResponse.end() && requestResponse["Status"] == "okay")
        {
          bResult = true;
        }
        else
        {
          strError = requestResponse["Error"];
        }
        requestResponse.clear();
      }
      in.clear();
      out.clear();

      return bResult;
    }
    // }}}
    // {{{ jwt()
    bool ServiceJunction::jwt(const string strSigner, const string strSecret, string &strPayload, Json *ptPayload, string &strError)
    {
      bool bDecode = false, bResult = false;
      list<string> in, out;
      string strJson;
      Json *ptJson = new Json;

      if (!m_strApplication.empty())
      {
        ptJson->insert("reqApp", m_strApplication);
      }
      if (!m_strProgram.empty())
      {
        ptJson->insert("reqProg", m_strProgram);
      }
      if (!m_strTimeout.empty())
      {
        ptJson->insert("Timeout", m_strTimeout);
      }
      ptJson->insert("Service", "jwt");
      if (!strPayload.empty())
      {
        bDecode = true;
        ptJson->insert("Function", "decode");
      }
      else
      {
        ptJson->insert("Function", "encode");
      }
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      ptJson = new Json;
      ptJson->insert("Signer", strSigner);
      if (!strSecret.empty())
      {
        ptJson->insert("Secret", strSecret);
      }
      if (bDecode)
      {
        ptJson->insert("Payload", strPayload);
      }
      else
      {
        ptJson->m["Payload"] = new Json(ptPayload);
      }
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      if (request(in, out, strError))
      {
        if (out.size() >= 1)
        {
          Json *ptStatus = new Json(out.front());
          if (ptStatus->m.find("Status") != ptStatus->m.end() && ptStatus->m["Status"]->v == "okay")
          {
            if (out.size() == 2)
            {
              ptJson = new Json(out.back());
              if (ptJson->m.find("Payload") != ptJson->m.end())
              {
                if (bDecode)
                {
                  bResult = true;
                  ptPayload->merge(ptJson->m["Payload"], true, false);
                }
                else if (!ptJson->m["Payload"]->v.empty())
                {
                  bResult = true;
                  strPayload = ptJson->m["Payload"]->v;
                }
                else
                {
                  strError = "Payload was empty within data.";
                }
              }
              else
              {
                strError = "Failed to find Payload within data.";
              }
              delete ptJson;
            }
            else
            {
              strError = "Failed to receive data.";
            }
          }
          else if (ptStatus->m.find("Error") != ptStatus->m.end() && !ptStatus->m["Error"]->v.empty())
          {
            strError = ptStatus->m["Error"]->v;
          }
          else
          {
            strError = "Encountered an unknown error.";
          }
          delete ptStatus;
        }
        else
        {
          strError = "Failed to receive response.";
        }
      }
      in.clear();
      out.clear();

      return bResult;
    }
    // }}}
    // {{{ logger()
    bool ServiceJunction::logger(const string strApplication, const string strUser, const string strPassword, const string strMessage, map<string, string> label, string &strError)
    {
      bool bResult;
      list<Json *> result;

      bResult = logger(strApplication, strUser, strPassword, "log", strMessage, label, result, strError);
      for (list<Json *>::iterator i = result.begin(); i != result.end(); i++)
      {
        delete (*i);
      }
      result.clear();

      return bResult;
    }
    bool ServiceJunction::logger(const string strUser, const string strPassword, const string strMessage, map<string, string> label, string &strError)
    {
      return logger(m_strApplication, strUser, strPassword, strMessage, label, strError);
    }
    bool ServiceJunction::logger(const string strApplication, const string strUser, const string strPassword, const string strFunction, const string strMessage, map<string, string> label, list<Json *> &result, string &strError)
    {
      bool bResult = false;
      Json *ptJson = new Json;
      list<string> in, out;
      string strJson;

      ptJson->insert("Application", strApplication);
      if (!m_strApplication.empty())
      {
        ptJson->insert("reqApp", m_strApplication);
      }
      if (!m_strProgram.empty())
      {
        ptJson->insert("reqProg", m_strProgram);
      }
      if (!m_strTimeout.empty())
      {
        ptJson->insert("Timeout", m_strTimeout);
      }
      ptJson->insert("Service", "logger");
      ptJson->insert("User", strUser);
      ptJson->insert("Password", strPassword);
      ptJson->insert("Function", strFunction);
      ptJson->insert("Message", strMessage);
      ptJson->m["Label"] = new Json(label);
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      if (request(in, out, strError))
      {
        ptJson = new Json(out.front());
        if (ptJson->m.find("Status") != ptJson->m.end() && ptJson->m["Status"]->v == "okay")
        {
          bResult = true;
          out.pop_front();
          for (list<string>::iterator i = out.begin(); i != out.end(); i++)
          {
            Json *ptSubJson = new Json(*i);
            result.push_back(ptSubJson);
          }
        }
        else if (ptJson->m.find("Error") != ptJson->m.end() && !ptJson->m["Error"]->v.empty())
        {
          strError = ptJson->m["Error"]->v;
        }
        else
        {
          strError = "Encountered an unknown error.";
        }
        delete ptJson;
      }
      in.clear();
      out.clear();

      return bResult;
    }
    bool ServiceJunction::logger(const string strUser, const string strPassword, const string strFunction, const string strMessage, map<string, string> label, list<Json *> &result, string &strError)
    {
      return logger(m_strApplication, strUser, strPassword, strFunction, strMessage, label, result, strError);
    }
    void ServiceJunction::logger(list<string> in, list<string> out)
    {
      if (!m_strLoggerApplication.empty() && !m_strLoggerUser.empty() && !m_strLoggerPassword.empty())
      {
        string strService;
        Json *ptJson = new Json(in.front());
        if (ptJson->m.find("Service") != ptJson->m.end() && ptJson->m["Service"]->v != "logger")
        {
          strService = ptJson->m["Service"]->v;
        }
        delete ptJson;
        if (!strService.empty() && strService != "Logger")
        {
          map<string, string> label = m_loggerLabel;
          string strMessage;
          ptJson = new Json;
          ptJson->m["Request"] = new Json;
          for (list<string>::iterator i = in.begin(); i != in.end(); i++)
          {
            Json *ptSubJson = new Json(*i);
            if (i == in.begin())
            {
              string strService;
              if (ptSubJson->m.find("Service") != ptSubJson->m.end())
              {
                strService = ptSubJson->m["Service"]->v;
              }
              if (strService == "ners" && ptSubJson->m.find("GUID") != ptSubJson->m.end() && !ptSubJson->m["GUID"]->v.empty())
              {
                ptSubJson->m["GUID"]->v.clear();
              }
              else if (ptSubJson->m.find("Password") != ptSubJson->m.end() && !ptSubJson->m["Password"]->v.empty())
              {
                ptSubJson->m["Password"]->v = "";
              }
            }
            if (ptSubJson->m.find("companyPassword") != ptSubJson->m.end() && !ptSubJson->m["companyPassword"]->v.empty())
            {
              ptSubJson->m["companyPassword"]->v = "";
            }
            if (ptSubJson->m.find("CompanyPassword") != ptSubJson->m.end() && !ptSubJson->m["CompanyPassword"]->v.empty())
            {
              ptSubJson->m["CompanyPassword"]->v = "";
            }
            if (strService == "wfac")
            {
              if (ptSubJson->m.find("routeToInformation") != ptSubJson->m.end() && ptSubJson->m["routeToInformation"]->m.find("userpassword") != ptSubJson->m["routeToInformation"]->m.end() && !ptSubJson->m["routeToInformation"]->m["userpassword"]->v.empty())
              {
                ptSubJson->m["routeToInformation"]->m["userpassword"]->v.clear();
              }
              if (ptSubJson->m.find("inputRequest") != ptSubJson->m.end() && ptSubJson->m["inputRequest"]->m.find("transactionData") != ptSubJson->m["inputRequest"]->m.end() && !ptSubJson->m["inputRequest"]->m["transactionData"]->l.empty() && ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m.find("data") != ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m.end() && ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m["data"]->m.find("WFA") != ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m["data"]->m.end() && ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m["data"]->m["WFA"]->m.find("REQUEST") != ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m["data"]->m["WFA"]->m.end() && ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m["data"]->m["WFA"]->m["REQUEST"]->m.find("PASSWORD") != ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m["data"]->m["WFA"]->m["REQUEST"]->m.end() && !ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m["data"]->m["WFA"]->m["REQUEST"]->m["PASSWORD"]->v.empty())
               {
                 ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m["data"]->m["WFA"]->m["REQUEST"]->m["PASSWORD"]->v.clear();
               }
            }
            ptJson->m["Request"]->l.push_back(ptSubJson);
          }
          ptJson->m["Response"] = new Json;
          for (list<string>::iterator i = out.begin(); i != out.end(); i++)
          {
            Json *ptSubJson = new Json(*i);
            if (i == out.begin())
            {
              string strService;
              if (ptSubJson->m.find("Service") != ptSubJson->m.end())
              {
                strService = ptSubJson->m["Service"]->v;
              }
              if (strService == "ners" && ptSubJson->m.find("GUID") != ptSubJson->m.end() && !ptSubJson->m["GUID"]->v.empty())
              {
                ptSubJson->m["GUID"]->v.clear();
              }
              else if (strService == "wfac")
              {
                if (ptSubJson->m.find("routeToInformation") != ptSubJson->m.end() && ptSubJson->m["routeToInformation"]->m.find("userpassword") != ptSubJson->m["routeToInformation"]->m.end() && !ptSubJson->m["routeToInformation"]->m["userpassword"]->v.empty())
                {
                  ptSubJson->m["routeToInformation"]->m["userpassword"]->v.clear();
                }
                if (ptSubJson->m.find("inputRequest") != ptSubJson->m.end() && ptSubJson->m["inputRequest"]->m.find("transactionData") != ptSubJson->m["inputRequest"]->m.end() && !ptSubJson->m["inputRequest"]->m["transactionData"]->l.empty() && ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m.find("data") != ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m.end() && ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m["data"]->m.find("WFA") != ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m["data"]->m.end() && ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m["data"]->m["WFA"]->m.find("REQUEST") != ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m["data"]->m["WFA"]->m.end() && ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m["data"]->m["WFA"]->m["REQUEST"]->m.find("PASSWORD") != ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m["data"]->m["WFA"]->m["REQUEST"]->m.end() && !ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m["data"]->m["WFA"]->m["REQUEST"]->m["PASSWORD"]->v.empty())
                 {
                   ptSubJson->m["inputRequest"]->m["transactionData"]->l.front()->m["data"]->m["WFA"]->m["REQUEST"]->m["PASSWORD"]->v.clear();
                 }
              }
              else if (ptSubJson->m.find("Password") != ptSubJson->m.end() && !ptSubJson->m["Password"]->v.empty())
              {
                ptSubJson->m["Password"]->v = "";
              }
            }
            ptJson->m["Response"]->l.push_back(ptSubJson);
          }
          label["Service"] = strService;
          ptJson->json(strMessage);
          delete ptJson;
          if (m_pLoggerPush != NULL)
          {
            label["loggerFunction"] = "message";
            label["loggerMessage"] = strMessage;
            (*m_pLoggerPush)(label);
          }
          else
          {
            list<Json *> result;
            string strError;
            logger(m_strLoggerApplication, m_strLoggerUser, m_strLoggerPassword, "message", strMessage, label, result, strError);
            for (list<Json *>::iterator i = result.begin(); i != result.end(); i++)
            {
              delete (*i);
            }
            result.clear();
          }
          label.clear();
        }
      }
    }
    // }}}
    // {{{ mssqlQuery()
    bool ServiceJunction::mssqlQuery(const string strUser, const string strPassword, const string strServer, const string strQuery, list<map<string, string> > &result, string &strError)
    {
      bool bResult = false;
      Json *ptJson;
      list<string> in, out;
      map<string, string> requestArray;
      string strJson;

      for (list<map<string, string> >::iterator i = result.begin(); i != result.end(); i++)
      {
        i->clear();
      }
      result.clear();
      if (!m_strApplication.empty())
      {
        requestArray["reqApp"] = m_strApplication;
      }
      if (!m_strProgram.empty())
      {
        requestArray["reqProg"] = m_strProgram;
      }
      if (!m_strTimeout.empty())
      {
        requestArray["Timeout"] = m_strTimeout;
      }
      requestArray["Service"] = "mssql";
      requestArray["User"] = strUser;
      requestArray["Password"] = strPassword;
      requestArray["Server"] = strServer;
      requestArray["Query"] = strQuery;
      ptJson = new Json(requestArray);
      requestArray.clear();
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      if (request(in, out, strError))
      {
        bool bFirst = true;
        for (list<string>::iterator i = out.begin(); i != out.end(); i++)
        {
          if (bFirst)
          {
            map<string, string> requestResponse;
            bFirst = false;
            ptJson = new Json(*i);
            ptJson->flatten(requestResponse, true);
            delete ptJson;
            if (requestResponse.find("Status") != requestResponse.end() && requestResponse["Status"] == "okay")
            {
              bResult = true;
            }
            else if (requestResponse.find("Error") != requestResponse.end() && !requestResponse["Error"].empty())
            {
              strError = requestResponse["Error"];
            }
            else
            {
              strError = "Encountered an unknown error.";
            }
            requestResponse.clear();
          }
          else
          {
            map<string, string> row;
            ptJson = new Json(*i);
            ptJson->flatten(row, true);
            delete ptJson;
            result.push_back(row);
            row.clear();
          }
        }
      }
      in.clear();
      out.clear();

      return bResult;
    }
    // }}}
    // {{{ mssqlUpdate()
    bool ServiceJunction::mssqlUpdate(const string strUser, const string strPassword, const string strServer, const string strUpdate, string &strError)
    {
      bool bResult = false;
      Json *ptJson;
      list<map<string, string> > result;
      list<string> in, out;
      map<string, string> requestArray;
      string strJson;

      if (!m_strApplication.empty())
      {
        requestArray["reqApp"] = m_strApplication;
      }
      if (!m_strProgram.empty())
      {
        requestArray["reqProg"] = m_strProgram;
      }
      if (!m_strTimeout.empty())
      {
        requestArray["Timeout"] = m_strTimeout;
      }
      requestArray["Service"] = "mssql";
      requestArray["User"] = strUser;
      requestArray["Password"] = strPassword;
      requestArray["Server"] = strServer;
      requestArray["Update"] = strUpdate;
      ptJson = new Json(requestArray);
      requestArray.clear();
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      if (request(in, out, strError))
      {
        if (out.size() == 1)
        {
          map<string, string> requestResponse;
          ptJson = new Json(out.front());
          ptJson->flatten(requestResponse, true);
          delete ptJson;
          if (requestResponse.find("Status") != requestResponse.end() && requestResponse["Status"] == "okay")
          {
            bResult = true;
          }
          else if (requestResponse.find("Error") != requestResponse.end() && !requestResponse["Error"].empty())
          {
            strError = requestResponse["Error"];
          }
          else
          {
            strError = "Encountered an unknown error.";
          }
          requestResponse.clear();
        }
        else
        {
          stringstream ssError;
          ssError << "Received " << out.size() << " JSON resultant lines when expecting 1 line.";
          strError = ssError.str();
        }
      }
      in.clear();
      out.clear();

      return bResult;
    }
    // }}}
    // {{{ mysqlQuery()
    bool ServiceJunction::mysqlQuery(const string strUser, const string strPassword, const string strServer, const string strDatabase, const string strQuery, list<map<string, string> > &result, string &strError)
    {
      bool bResult = false;
      Json *ptJson;
      list<string> in, out;
      map<string, string> requestArray;
      string strJson;

      for (list<map<string, string> >::iterator i = result.begin(); i != result.end(); i++)
      {
        i->clear();
      }
      result.clear();
      if (!m_strApplication.empty())
      {
        requestArray["reqApp"] = m_strApplication;
      }
      if (!m_strProgram.empty())
      {
        requestArray["reqProg"] = m_strProgram;
      }
      if (!m_strTimeout.empty())
      {
        requestArray["Timeout"] = m_strTimeout;
      }
      requestArray["Service"] = "mysql";
      requestArray["User"] = strUser;
      requestArray["Password"] = strPassword;
      requestArray["Server"] = strServer;
      requestArray["Database"] = strDatabase;
      requestArray["Query"] = strQuery;
      ptJson = new Json(requestArray);
      requestArray.clear();
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      if (request(in, out, strError))
      {
        bool bFirst = true;
        for (list<string>::iterator i = out.begin(); i != out.end(); i++)
        {
          if (bFirst)
          {
            map<string, string> requestResponse;
            bFirst = false;
            ptJson = new Json(*i);
            ptJson->flatten(requestResponse, true);
            delete ptJson;
            if (requestResponse.find("Status") != requestResponse.end() && requestResponse["Status"] == "okay")
            {
              bResult = true;
            }
            else if (requestResponse.find("Error") != requestResponse.end() && !requestResponse["Error"].empty())
            {
              strError = requestResponse["Error"];
            }
            else
            {
              strError = "Encountered an unknown error.";
            }
            requestResponse.clear();
          }
          else
          {
            map<string, string> row;
            ptJson = new Json(*i);
            ptJson->flatten(row, true);
            delete ptJson;
            result.push_back(row);
            row.clear();
          }
        }
      }
      in.clear();
      out.clear();

      return bResult;
    }
    // }}}
    // {{{ mysqlUpdate()
    bool ServiceJunction::mysqlUpdate(const string strUser, const string strPassword, const string strServer, const string strDatabase, const string strUpdate, string &strID, string &strRows, string &strError)
    {
      bool bResult = false;
      Json *ptJson;
      list<map<string, string> > result;
      list<string> in, out;
      map<string, string> requestArray;
      string strJson;

      if (!m_strApplication.empty())
      {
        requestArray["reqApp"] = m_strApplication;
      }
      if (!m_strProgram.empty())
      {
        requestArray["reqProg"] = m_strProgram;
      }
      if (!m_strTimeout.empty())
      {
        requestArray["Timeout"] = m_strTimeout;
      }
      requestArray["Service"] = "mysql";
      requestArray["User"] = strUser;
      requestArray["Password"] = strPassword;
      requestArray["Server"] = strServer;
      requestArray["Database"] = strDatabase;
      requestArray["Update"] = strUpdate;
      ptJson = new Json(requestArray);
      requestArray.clear();
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      if (request(in, out, strError))
      {
        if (out.size() == 1)
        {
          map<string, string> requestResponse;
          ptJson = new Json(out.front());
          ptJson->flatten(requestResponse, true);
          delete ptJson;
          if (requestResponse.find("Status") != requestResponse.end() && requestResponse["Status"] == "okay")
          {
            bResult = true;
            if (requestResponse.find("ID") != requestResponse.end() && !requestResponse["ID"].empty())
            {
              strID = requestResponse["ID"];
            }
            if (requestResponse.find("Rows") != requestResponse.end() && !requestResponse["Rows"].empty())
            {
              strRows = requestResponse["Rows"];
            }
          }
          else if (requestResponse.find("Error") != requestResponse.end() && !requestResponse["Error"].empty())
          {
            strError = requestResponse["Error"];
          }
          else
          {
            strError = "Encountered an unknown error.";
          }
          requestResponse.clear();
        }
        else
        {
          stringstream ssError;
          ssError << "Received " << out.size() << " JSON resultant lines when expecting 1 line.";
          strError = ssError.str();
        }
      }
      in.clear();
      out.clear();

      return bResult;
    }
    bool ServiceJunction::mysqlUpdate(const string strUser, const string strPassword, const string strServer, const string strDatabase, const string strUpdate, string &strError)
    {
      string strID, strRows;

      return mysqlUpdate(strUser, strPassword, strServer, strDatabase, strUpdate, strID, strRows, strError);
    }
    // }}}
    // {{{ oracleQuery()
    bool ServiceJunction::oracleQuery(const string strSchema, const string strPassword, const string strTnsName, const string strQuery, list<map<string, string> > &result, string &strError)
    {
      bool bResult = false;
      Json *ptJson;
      list<string> in, out;
      map<string, string> requestArray;
      string strJson;

      for (list<map<string, string> >::iterator i = result.begin(); i != result.end(); i++)
      {
        i->clear();
      }
      result.clear();
      if (!m_strApplication.empty())
      {
        requestArray["reqApp"] = m_strApplication;
      }
      if (!m_strProgram.empty())
      {
        requestArray["reqProg"] = m_strProgram;
      }
      if (!m_strTimeout.empty())
      {
        requestArray["Timeout"] = m_strTimeout;
      }
      requestArray["Service"] = "oracle";
      requestArray["Schema"] = strSchema;
      requestArray["Password"] = strPassword;
      requestArray["tnsName"] = strTnsName;
      requestArray["Query"] = strQuery;
      ptJson = new Json(requestArray);
      requestArray.clear();
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      if (request(in, out, strError))
      {
        bool bFirst = true;
        for (list<string>::iterator i = out.begin(); i != out.end(); i++)
        {
          if (bFirst)
          {
            map<string, string> requestResponse;
            bFirst = false;
            ptJson = new Json(*i);
            ptJson->flatten(requestResponse, true);
            delete ptJson;
            if (requestResponse.find("Status") != requestResponse.end() && requestResponse["Status"] == "okay")
            {
              bResult = true;
            }
            else if (requestResponse.find("Error") != requestResponse.end() && !requestResponse["Error"].empty())
            {
              strError = requestResponse["Error"];
            }
            else
            {
              strError = "Encountered an unknown error.";
            }
            requestResponse.clear();
          }
          else
          {
            map<string, string> row;
            ptJson = new Json(*i);
            ptJson->flatten(row, true);
            delete ptJson;
            result.push_back(row);
            row.clear();
          }
        }
      }
      in.clear();
      out.clear();

      return bResult;
    }
    // }}}
    // {{{ oracleUpdate()
    bool ServiceJunction::oracleUpdate(const string strSchema, const string strPassword, const string strTnsName, const string strUpdate, string &strID, string &strRows, string &strError)
    {
      bool bResult = false;
      Json *ptJson;
      list<map<string, string> > result;
      list<string> in, out;
      map<string, string> requestArray;
      string strJson;

      if (!m_strApplication.empty())
      {
        requestArray["reqApp"] = m_strApplication;
      }
      if (!m_strProgram.empty())
      {
        requestArray["reqProg"] = m_strProgram;
      }
      if (!m_strTimeout.empty())
      {
        requestArray["Timeout"] = m_strTimeout;
      }
      requestArray["Service"] = "oracle";
      requestArray["Schema"] = strSchema;
      requestArray["Password"] = strPassword;
      requestArray["tnsName"] = strTnsName;
      requestArray["Update"] = strUpdate;
      ptJson = new Json(requestArray);
      requestArray.clear();
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      if (request(in, out, strError))
      {
        if (out.size() == 1)
        {
          map<string, string> requestResponse;
          ptJson = new Json(out.front());
          ptJson->flatten(requestResponse, true);
          delete ptJson;
          if (requestResponse.find("Status") != requestResponse.end() && requestResponse["Status"] == "okay")
          {
            bResult = true;
            if (requestResponse.find("ID") != requestResponse.end() && !requestResponse["ID"].empty())
            {
              strID = requestResponse["ID"];
            }
            if (requestResponse.find("Rows") != requestResponse.end() && !requestResponse["Rows"].empty())
            {
              strRows = requestResponse["Rows"];
            }
          }
          else if (requestResponse.find("Error") != requestResponse.end() && !requestResponse["Error"].empty())
          {
            strError = requestResponse["Error"];
          }
          else
          {
            strError = "Encountered an unknown error.";
          }
          requestResponse.clear();
        }
        else
        {
          stringstream ssError;
          ssError << "Received " << out.size() << " JSON resultant lines when expecting 1 line.";
          strError = ssError.str();
        }
      }
      in.clear();
      out.clear();

      return bResult;
    }
    bool ServiceJunction::oracleUpdate(const string strSchema, const string strPassword, const string strTnsName, const string strUpdate, string &strError)
    {
      string strID, strRows;

      return oracleUpdate(strSchema, strPassword, strTnsName, strUpdate, strID, strRows, strError);
    }
    // }}}
    // {{{ page()
    bool ServiceJunction::page(const string strUser, const string strMessage, string &strError, const bool bGroup)
    {
      bool bResult = false;
      Json *ptJson;
      list<string> in, out;
      map<string, string> requestArray;
      string strJson;

      if (!m_strApplication.empty())
      {
        requestArray["reqApp"] = m_strApplication;
      }
      if (!m_strProgram.empty())
      {
        requestArray["reqProg"] = m_strProgram;
      }
      if (!m_strTimeout.empty())
      {
        requestArray["Timeout"] = m_strTimeout;
      }
      requestArray["Service"] = "pager";
      if (bGroup)
      {
        requestArray["Group"] = strUser;
      }
      else
      {
        requestArray["User"] = strUser;
      }
      requestArray["Message"] = ((!m_strApplication.empty())?m_strApplication + (string)":  ":"") + strMessage;
      ptJson = new Json(requestArray);
      requestArray.clear();
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      if (request(in, out, strError) && !out.empty())
      {
        map<string, string> requestResponse;
        bResult = true;
        ptJson = new Json(out.front());
        ptJson->flatten(requestResponse, true);
        delete ptJson;
        if (requestResponse.find("Status") != requestResponse.end() && requestResponse["Status"] == "okay")
        {
          bResult = true;
        }
        else if (requestResponse.find("Error") != requestResponse.end() && !requestResponse["Error"].empty())
        {
          strError = requestResponse["Error"];
        }
        else
        {
          strError = "Encountered an unknown error.";
        }
        requestResponse.clear();
      }
      in.clear();
      out.clear();

      return bResult;
    }
    // }}}
    // {{{ pdf()
    bool ServiceJunction::pdf(const string strPdf, list<string> &pages, string &strError)
    {
      return pdf(strPdf, "", "", pages, strError);
    }
    bool ServiceJunction::pdf(const string strPdf, const string strOwnerPassword, const string strUserPassword, list<string> &pages, string &strError)
    {
      bool bResult = false;
      list<string> in, out;
      string strData, strJson;
      Json *ptJson = new Json;

      if (!m_strApplication.empty())
      {
        ptJson->insert("reqApp", m_strApplication);
      }
      if (!m_strProgram.empty())
      {
        ptJson->insert("reqProg", m_strProgram);
      }
      if (!m_strTimeout.empty())
      {
        ptJson->insert("Timeout", m_strTimeout);
      }
      ptJson->insert("Service", "pdf");
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      ptJson = new Json;
      m_manip.encodeBase64(strPdf, strData);
      ptJson->insert("Data", strData);
      if (!strOwnerPassword.empty() || !strUserPassword.empty())
      {
        ptJson->m["Passwords"] = new Json;
        if (!strOwnerPassword.empty())
        {
          ptJson->m["Passwords"]->insert("Owner", strOwnerPassword);
        }
        if (!strUserPassword.empty())
        {
          ptJson->m["Passwords"]->insert("User", strUserPassword);
        }
      }
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      if (request(in, out, strError))
      {
        if (out.size() >= 1)
        {
          Json *ptStatus = new Json(out.front());
          if (ptStatus->m.find("Status") != ptStatus->m.end() && ptStatus->m["Status"]->v == "okay")
          {
            if (out.size() == 2)
            {
              ptJson = new Json(out.back());
              if (ptJson->m.find("Pages") != ptJson->m.end())
              {
                bResult = true;
                for (list<Json *>::iterator i = ptJson->m["Pages"]->l.begin(); i != ptJson->m["Pages"]->l.end(); i++)
                {
                  pages.push_back((*i)->v);
                }
              }
              else
              {
                strError = "Failed to find Pages within data.";
              }
              delete ptJson;
            }
            else
            {
              strError = "Failed to receive data.";
            }
          }
          else if (ptStatus->m.find("Error") != ptStatus->m.end() && !ptStatus->m["Error"]->v.empty())
          {
            strError = ptStatus->m["Error"]->v;
          }
          else
          {
            strError = "Encountered an unknown error.";
          }
          delete ptStatus;
        }
        else
        {
          strError = "Failed to receive response.";
        }
      }
      in.clear();
      out.clear();

      return bResult;
    }
    // }}}
    // {{{ qBot()
    bool ServiceJunction::qBot(const string strRoom, const string strMessage, string &strError)
    {
      return qBot("", strRoom, strMessage, strError);
    }
    bool ServiceJunction::qBot(const string strSender, const string strRoom, const string strMessage, string &strError)
    {
      bool bResult = false;
      Json *ptJson;
      list<string> in, out;
      map<string, string> requestArray;
      string strJson;

      if (!m_strApplication.empty())
      {
        requestArray["reqApp"] = m_strApplication;
      }
      if (!m_strProgram.empty())
      {
        requestArray["reqProg"] = m_strProgram;
      }
      if (!m_strTimeout.empty())
      {
        requestArray["Timeout"] = m_strTimeout;
      }
      requestArray["Service"] = "qBot";
      requestArray["Sender"] = strSender;
      requestArray["Room"] = strRoom;
      requestArray["Message"] = strMessage;
      ptJson = new Json(requestArray);
      requestArray.clear();
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      if (request(in, out, strError))
      {
        map<string, string> requestResponse;
        ptJson = new Json(out.front());
        ptJson->flatten(requestResponse, true);
        delete ptJson;
        if (requestResponse.find("Status") != requestResponse.end() && requestResponse["Status"] == "okay")
        {
          bResult = true;
        }
        else
        {
          strError = requestResponse["Error"];
        }
        requestResponse.clear();
      }
      in.clear();
      out.clear();

      return bResult;
    }
    // }}}
    // {{{ request()
    bool ServiceJunction::request(list<string> in, list<string> &out, string &strError)
    {
      return request(in, out, 0, strError);
    }
    bool ServiceJunction::request(list<string> in, list<string> &out, time_t CTimeout, string &strError)
    {
      bool bResult = false;
      time_t CEnd, CStart;

      if (m_unThrottle > 0)
      {
        #ifdef COMMON_LINUX
        sem_wait(&m_semaServiceJunctionThrottle);
        #endif
        #ifdef COMMON_SOLARIS
        sema_wait(&m_semaServiceJunctionThrottle);
        #endif
      }
      time(&CStart);
      out.clear();
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
      if (in.size() >= 1)
      {
        if (m_bUseSingleSocket)
        {
          int readpipe[2] = {-1, -1};
          if (pipe(readpipe) == 0)
          {
            bool bExit = false, bFirst = true;
            char szBuffer[65536];
            int nReturn;
            size_t unPosition, unUniqueID = 0;
            string strBuffer;
            stringstream ssUniqueID;
            for (list<string>::iterator i = in.begin(); i != in.end(); i++)
            {
              if (i == in.begin())
              {
                Json *ptJson = new Json(in.front());
                m_mutexUnique.lock();
                while (m_requests.find(m_unUniqueID) != m_requests.end())
                {
                  m_unUniqueID++;
                }
                unUniqueID = m_unUniqueID++;
                m_mutexUnique.unlock();
                ssUniqueID << unUniqueID;
                ptJson->insert("ProcessType", "parallel");
                ptJson->insert("sjUniqueID", ssUniqueID.str(), 'n');
                ptJson->json(strBuffer);
                strBuffer += "\n";
                delete ptJson;
              }
              else
              {
                strBuffer += (*i) + "\n";
              }
            }
            sjreqdata *ptData = new sjreqdata;
            ptData->bSent = false;
            ptData->fdSocket = readpipe[1];
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
                    while ((unPosition = strBuffer.find("\n")) != string::npos)
                    {
                      if (bFirst)
                      {
                        string strJson;
                        Json *ptJson = new Json(strBuffer.substr(0, unPosition));
                        bFirst = false;
                        if (ptJson->m.find("ProcessType") != ptJson->m.end())
                        {
                          delete ptJson->m["ProcessType"];
                          ptJson->m.erase("ProcessType");
                        }
                        if (ptJson->m.find("sjUniqueID") != ptJson->m.end())
                        {
                          delete ptJson->m["sjUniqueID"];
                          ptJson->m.erase("sjUniqueID");
                        }
                        out.push_back(ptJson->json(strJson));
                        delete ptJson;
                      }
                      else
                      {
                        out.push_back(strBuffer.substr(0, unPosition));
                      }
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
            if (out.size() >= 1)
            {
              bResult = true;
            }
            else if (strError.empty())
            {
              strError = "Failed to receive any data from the underlying service within the Service Junction.";
            }
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
          if (utility()->conf()->m.find("Load Balancer") != utility()->conf()->m.end())
          {
            server.push_back(utility()->conf()->m["Load Balancer"]->v);
          }
          if (utility()->conf()->m.find("Service Junction") != utility()->conf()->m.end())
          {
            server.push_back(utility()->conf()->m["Service Junction"]->v);
          }
          if (!server.empty())
          {
            bool bDone = false;
            SSL_CTX *ctx = NULL;
            if (m_bUseSecureJunction)
            {
              SSL_METHOD *method = (SSL_METHOD *)SSLv23_client_method();
              if ((ctx = SSL_CTX_new(method)) != NULL)
              {
                //SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
              }
              else
              {
                m_bUseSecureJunction = false;
              }
            }
            for (list<string>::iterator i = server.begin(); !bDone && i != server.end(); i++)
            {
              for (int j = ((m_bUseSecureJunction)?0:1); !bDone && j < 2; j++)
              {
                bool bConnected[4] = {false, false, false, false};
                int fdSocket = -1, nReturn = -1;
                SSL *ssl = NULL;
                string strServer;
                unsigned int unAttempt = 0, unPick = 0, unSeed = time(NULL);
                vector<string> junctionServer;
                for (int k = 1; !m_manip.getToken(strServer, (*i), k, ",", true).empty(); k++)
                {
                  junctionServer.push_back(m_manip.trim(strServer, strServer));
                }
                srand(unSeed);
                unPick = rand_r(&unSeed) % junctionServer.size();
                #ifdef COMMON_LINUX
                sem_wait(&m_semaServiceJunctionRequestLock);
                #endif
                #ifdef COMMON_SOLARIS
                sema_wait(&m_semaServiceJunctionRequestLock);
                #endif
                while (!bConnected[3] && unAttempt++ < junctionServer.size())
                {
                  addrinfo hints, *result;
                  bConnected[0] = bConnected[1] = bConnected[2] = false;
                  if (unPick == junctionServer.size())
                  {
                    unPick = 0;
                  }
                  strServer = junctionServer[unPick];
                  memset(&hints, 0, sizeof(addrinfo));
                  hints.ai_family = AF_UNSPEC;
                  hints.ai_socktype = SOCK_STREAM;
                  m_mutexGetAddrInfo.lock();
                  nReturn = getaddrinfo(strServer.c_str(), ((j == 0)?((m_bDodgeConcentrator)?"5864":"5863"):"5862"), &hints, &result);
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
                          if (j == 1 || (ssl = utility()->sslConnect(ctx, fdSocket, strError)) != NULL)
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
                sem_post(&m_semaServiceJunctionRequestLock);
                #endif
                #ifdef COMMON_SOLARIS
                sema_post(&m_semaServiceJunctionRequestLock);
                #endif
                junctionServer.clear();
                if (bConnected[3])
                {
                  bool bExit = false;
                  size_t unPosition;
                  string strBuffer[2], strLine, strTrim;
                  bDone = true;
                  for (list<string>::iterator k = in.begin(); k != in.end(); k++)
                  {
                    strBuffer[1] += (*k) + "\n";
                  }
                  strBuffer[1] += "end\n";
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
                        if ((j == 0 && utility()->sslRead(ssl, strBuffer[0], nReturn)) || (j == 1 && utility()->fdRead(fdSocket, strBuffer[0], nReturn)))
                        {
                          while ((unPosition = strBuffer[0].find("\n")) != string::npos)
                          {
                            strLine = strBuffer[0].substr(0, unPosition);
                            strBuffer[0].erase(0, unPosition + 1);
                            if (!m_manip.trim(strTrim, strLine).empty() && strTrim != "\"\"")
                            {
                              if (strTrim == "end")
                              {
                                bExit = bResult = true;
                              }
                              else
                              {
                                out.push_back(strLine);
                              }
                            }
                          }
                        }
                        else if (j == 0)
                        {
                          bExit = true;
                          if (SSL_get_error(ssl, nReturn) != SSL_ERROR_ZERO_RETURN)
                          {
                            stringstream ssError;
                            ssError << "Utility::sslRead(" << SSL_get_error(ssl, nReturn) << ") " << utility()->sslstrerror(ssl, nReturn);
                            strError = ssError.str();
                          }
                        }
                        else
                        {
                          bExit = true;
                          if (nReturn < 0)
                          {
                            stringstream ssError;
                            ssError << "Utility::fdRead(" << errno << ") " << strerror(errno);
                            strError = ssError.str();
                          }
                        }
                      }
                      if (fds[0].fd == fdSocket && (fds[0].revents & POLLOUT))
                      {
                        if (j == 0 && !utility()->sslWrite(ssl, strBuffer[1], nReturn))
                        {
                          bExit = true;
                          if (SSL_get_error(ssl, nReturn) != SSL_ERROR_ZERO_RETURN)
                          {
                            stringstream ssError;
                            ssError << "Utility::sslWrite(" << SSL_get_error(ssl, nReturn) << ") " << utility()->sslstrerror(ssl, nReturn);
                            strError = ssError.str();
                          }
                        }
                        else if (j == 1 && !utility()->fdWrite(fdSocket, strBuffer[1], nReturn))
                        {
                          bExit = true;
                          if (nReturn < 0)
                          {
                            stringstream ssError;
                            ssError << "Utility::fdWrite(" << errno << ") " << strerror(errno);
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
                  if (bResult && out.empty())
                  {
                    bResult = false;
                    strError = "Failed to receive any data from the underlying service within the Service Junction.";
                  }
                  if (j == 0)
                  {
                    SSL_shutdown(ssl);
                    SSL_free(ssl);
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
            }
            if (!bResult && strError.empty())
            {
              strError = "Service Junction request failed without returning an error message.";
            }
            if (ctx != NULL)
            {
              SSL_CTX_free(ctx);
            }
          }
          else
          {
            strError = (string)"Please provide the Load Balancer and/or Service Junction servers via the " + utility()->getConfPath() + (string)" file.";
          }
          server.clear();
        }
        logger(in, out);
      }
      else
      {
        strError = "Missing Service Junction request.";
      }
      if (m_unThrottle > 0)
      {
        #ifdef COMMON_LINUX
        sem_post(&m_semaServiceJunctionThrottle);
        #endif
        #ifdef COMMON_SOLARIS
        sema_post(&m_semaServiceJunctionThrottle);
        #endif
      }

      return bResult;
    }
    // }}}
    // {{{ requestThread()
    void ServiceJunction::requestThread()
    {
      size_t unSleep = 1;
      SSL_CTX *ctx = NULL;

      if (m_bUseSecureJunction)
      {
        SSL_METHOD *method = (SSL_METHOD *)SSLv23_client_method();
        if ((ctx = SSL_CTX_new(method)) != NULL)
        {
          //SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
        }
        else
        {
          m_bUseSecureJunction = false;
        }
      }
      while (m_bUseSingleSocket)
      {
        list<string> server;
        string strError;
        utility()->readConf(strError);
        if (utility()->conf()->m.find("Load Balancer") != utility()->conf()->m.end())
        {
          server.push_back(utility()->conf()->m["Load Balancer"]->v);
        }
        if (utility()->conf()->m.find("Service Junction") != utility()->conf()->m.end())
        {
          server.push_back(utility()->conf()->m["Service Junction"]->v);
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
            vector<string> junctionServer;
            for (int k = 1; !m_manip.getToken(strServer, (*i), k, ",", true).empty(); k++)
            {
              junctionServer.push_back(m_manip.trim(strServer, strServer));
            }
            srand(unSeed);
            unPick = rand_r(&unSeed) % junctionServer.size();
            while (m_bUseSingleSocket && !bConnected && unAttempt++ < junctionServer.size())
            {
              addrinfo hints, *result;
              if (unPick == junctionServer.size())
              {
                unPick = 0;
              }
              strServer = junctionServer[unPick];
              memset(&hints, 0, sizeof(addrinfo));
              hints.ai_family = AF_UNSPEC;
              hints.ai_socktype = SOCK_STREAM;
              m_mutexGetAddrInfo.lock();
              nReturn = getaddrinfo(strServer.c_str(), ((!m_bUseSecureJunction)?"5862":"5863"), &hints, &result);
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
                      if (!m_bUseSecureJunction || (ssl = utility()->sslConnect(ctx, fdSocket, strError)) != NULL)
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
            junctionServer.clear();
            if (bConnected)
            {
              bool bExit = false;
              list<string> response;
              size_t unPosition;
              string strBuffer[2], strLine, strTrim;
              unSleep = 1;
              while (m_bUseSingleSocket && !bExit)
              {
                size_t unIndex = 1;
                m_mutexRequests.lock();
                pollfd *fds = new pollfd[m_requests.size() + 1];
                for (map<int, sjreqdata *>::iterator j = m_requests.begin(); j != m_requests.end(); j++)
                {
                  if (!j->second->bSent)
                  {
                    j->second->bSent = true;
                    strBuffer[1] += j->second->strBuffer[0] + "end\n";
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
                    if ((!m_bUseSecureJunction && utility()->fdRead(fdSocket, strBuffer[0], nReturn)) || (m_bUseSecureJunction && utility()->sslRead(ssl, strBuffer[0], nReturn)))
                    {
                      while ((unPosition = strBuffer[0].find("\n")) != string::npos)
                      {
                        strLine = strBuffer[0].substr(0, unPosition);
                        strBuffer[0].erase(0, unPosition + 1);
                        if (!m_manip.trim(strTrim, strLine).empty() && strTrim != "\"\"")
                        {
                          if (strTrim == "end")
                          {
                            if (!response.empty())
                            {
                              Json *ptJson = new Json(response.front());
                              if (ptJson->m.find("sjUniqueID") != ptJson->m.end() && !ptJson->m["sjUniqueID"]->v.empty())
                              {
                                size_t unUniqueID = atoi(ptJson->m["sjUniqueID"]->v.c_str());
                                string strResponse;
                                for (list<string>::iterator j = response.begin(); j != response.end(); j++)
                                {
                                  strResponse += (*j) + "\n";
                                }
                                m_mutexRequests.lock();
                                if (m_requests.find(unUniqueID) != m_requests.end())
                                {
                                  m_requests[unUniqueID]->strBuffer[1] = strResponse;
                                }
                                m_mutexRequests.unlock();
                              }
                              response.clear();
                              delete ptJson;
                            }
                          }
                          else
                          {
                            response.push_back(strLine);
                          }
                        }
                      }
                    }
                    else
                    {
                      bExit = true;
                    }
                  }
                  if (fds[0].revents & POLLOUT)
                  {
                    if ((!m_bUseSecureJunction && !utility()->fdWrite(fdSocket, strBuffer[1], nReturn)) || (m_bUseSecureJunction && !utility()->sslWrite(ssl, strBuffer[1], nReturn)))
                    {
                      bExit = true;
                    }
                  }
                  if (unIndex > 1)
                  {
                    m_mutexRequests.lock();
                    for (map<int, sjreqdata *>::iterator j = m_requests.begin(); j != m_requests.end(); j++)
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
              for (map<int, sjreqdata *>::iterator j = m_requests.begin(); j != m_requests.end(); j++)
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
              if (m_bUseSecureJunction)
              {
                SSL_shutdown(ssl);
                SSL_free(ssl);
              }
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
      for (map<int, sjreqdata *>::iterator i = m_requests.begin(); i != m_requests.end(); i++)
      {
        if (i->second->fdSocket != -1)
        {
          close(i->second->fdSocket);
          i->second->fdSocket = -1;
        }
      }
      m_mutexRequests.unlock();
      if (ctx != NULL)
      {
        SSL_CTX_free(ctx);
      }
    }
    // }}}
    // {{{ setApplication()
    void ServiceJunction::setApplication(const string strApplication)
    {
      m_strApplication = strApplication;
    }
    // }}}
    // {{{ setLogger()
    void ServiceJunction::setLogger(const string strApplication, const string strUser, const string strPassword, void (*pPush)(map<string, string> &), map<string, string> label)
    {
      m_strLoggerApplication = strApplication;
      m_strLoggerUser = strUser;
      m_strLoggerPassword = strPassword;
      m_pLoggerPush = pPush;
      m_loggerLabel = label;
    }
    // }}}
    // {{{ setProgram()
    void ServiceJunction::setProgram(const string strProgram)
    {
      m_strProgram = strProgram;
    }
    // }}}
    // {{{ setThrottle()
    void ServiceJunction::setThrottle(const size_t unThrottle)
    {
      if (unThrottle > 0)
      {
        if (unThrottle != m_unThrottle)
        {
          if (m_unThrottle > 0)
          {
            #ifdef COMMON_LINUX
            sem_destroy(&m_semaServiceJunctionThrottle);
            #endif
            #ifdef COMMON_SOLARIS
            sema_destroy(&m_semaServiceJunctionThrottle);
            #endif
          }
          #ifdef COMMON_LINUX
          sem_init(&m_semaServiceJunctionThrottle, 0, unThrottle);
          #endif
          #ifdef COMMON_SOLARIS
          sema_init(&m_semaServiceJunctionThrottle, unThrottle, USYNC_THREAD, NULL);
          #endif
        }
      }
      else if (m_unThrottle > 0)
      {
        #ifdef COMMON_LINUX
        sem_destroy(&m_semaServiceJunctionThrottle);
        #endif
        #ifdef COMMON_SOLARIS
        sema_destroy(&m_semaServiceJunctionThrottle);
        #endif
      }
      m_unThrottle = unThrottle;
    }
    // }}}
    // {{{ setTimeout()
    void ServiceJunction::setTimeout(const string strTimeout)
    {
      m_strTimeout = strTimeout;
    }
    // }}}
    // {{{ sysInfo()
    bool ServiceJunction::sysInfo(Json *ptRequest, Json *ptResponse, string &strError)
    {
      bool bResult = false;
      list<string> in, out;
      string strJson;
      Json *ptJson = new Json(ptRequest);

      utility()->readConf(strError);
      ptJson->insert("Service", "sysInfo");
      if (!m_strApplication.empty())
      {
        ptJson->insert("reqApp", m_strApplication);
      }
      if (!m_strProgram.empty())
      {
        ptJson->insert("reqProg", m_strProgram);
      }
      if (!m_strTimeout.empty())
      {
        ptJson->insert("Timeout", m_strTimeout);
      }
      if (utility()->conf()->m.find("System Information") != utility()->conf()->m.end())
      {
        ptJson->insert("sysInfoServer", utility()->conf()->m["System Information"]->v);
      }
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      if (request(in, out, strError))
      {
        if (out.size() >= 1)
        {
          ptJson = new Json(out.front());
          if (ptJson->m.find("Status") != ptJson->m.end() && ptJson->m["Status"]->v == "okay")
          {
            bResult = true;
          }
          else if (ptJson->m.find("Error") != ptJson->m.end() && !ptJson->m["Error"]->v.empty())
          {
            strError = ptJson->m["Error"]->v;
          }
          else
          {
            strError = "Encountered an unknown error.";
          }
          delete ptJson;
          if (out.size() == 2)
          {
            ptResponse->parse(out.back());
          }
        }
        else
        {
          strError = "Failed to receive any data from the underlying service within the Service Junction.";
        }
      }
      in.clear();
      out.clear();

      return bResult;
    }
    bool ServiceJunction::sysInfo(const string strServer, const string strProcess, Json *ptResponse, string &strError)
    {
      bool bResult = false;
      Json *ptRequest = new Json;

      ptRequest->insert("Action", ((!strProcess.empty())?"process":"system"));
      ptRequest->insert("Server", strServer);
      if (!strProcess.empty())
      {
        ptRequest->insert("Process", strProcess);
      }
      bResult = sysInfo(ptRequest, ptResponse, strError);
      delete ptRequest;

      return bResult;
    }
    // }}}
    // {{{ teradataQuery()
    bool ServiceJunction::teradataQuery(const string strUser, const string strPassword, const string strURL, const string strQuery, list<map<string, string> > &result, string &strError)
    {
      bool bResult = false;
      Json *ptJson;
      list<string> in, out;
      map<string, string> requestArray;
      string strJson;

      for (list<map<string, string> >::iterator i = result.begin(); i != result.end(); i++)
      {
        i->clear();
      }
      result.clear();
      if (!m_strApplication.empty())
      {
        requestArray["reqApp"] = m_strApplication;
      }
      if (!m_strProgram.empty())
      {
        requestArray["reqProg"] = m_strProgram;
      }
      if (!m_strTimeout.empty())
      {
        requestArray["Timeout"] = m_strTimeout;
      }
      requestArray["Service"] = "teradata";
      requestArray["User"] = strUser;
      requestArray["Password"] = strPassword;
      requestArray["URL"] = strURL;
      requestArray["Query"] = strQuery;
      ptJson = new Json(requestArray);
      requestArray.clear();
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      if (request(in, out, strError))
      {
        bool bFirst = true;
        for (list<string>::iterator i = out.begin(); i != out.end(); i++)
        {
          if (bFirst)
          {
            map<string, string> requestResponse;
            bFirst = false;
            ptJson = new Json(*i);
            ptJson->flatten(requestResponse, true);
            delete ptJson;
            if (requestResponse.find("Status") != requestResponse.end() && requestResponse["Status"] == "okay")
            {
              bResult = true;
            }
            else if (requestResponse.find("Error") != requestResponse.end() && !requestResponse["Error"].empty())
            {
              strError = requestResponse["Error"];
            }
            else
            {
              strError = "Encountered an unknown error.";
            }
            requestResponse.clear();
          }
          else
          {
            map<string, string> row;
            ptJson = new Json(*i);
            ptJson->flatten(row, true);
            delete ptJson;
            result.push_back(row);
            row.clear();
          }
        }
      }
      in.clear();
      out.clear();

      return bResult;
    }
    // }}}
    // {{{ teradataUpdate()
    bool ServiceJunction::teradataUpdate(const string strUser, const string strPassword, const string strURL, const string strUpdate, string &strError)
    {
      bool bResult = false;
      Json *ptJson;
      list<map<string, string> > result;
      list<string> in, out;
      map<string, string> requestArray;
      string strJson;

      if (!m_strApplication.empty())
      {
        requestArray["reqApp"] = m_strApplication;
      }
      if (!m_strProgram.empty())
      {
        requestArray["reqProg"] = m_strProgram;
      }
      if (!m_strTimeout.empty())
      {
        requestArray["Timeout"] = m_strTimeout;
      }
      requestArray["Service"] = "teradata";
      requestArray["User"] = strUser;
      requestArray["Password"] = strPassword;
      requestArray["URL"] = strURL;
      requestArray["Update"] = strUpdate;
      ptJson = new Json(requestArray);
      requestArray.clear();
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      if (request(in, out, strError))
      {
        if (out.size() == 1)
        {
          map<string, string> requestResponse;
          ptJson = new Json(out.front());
          ptJson->flatten(requestResponse, true);
          delete ptJson;
          if (requestResponse.find("Status") != requestResponse.end() && requestResponse["Status"] == "okay")
          {
            bResult = true;
          }
          else if (requestResponse.find("Error") != requestResponse.end() && !requestResponse["Error"].empty())
          {
            strError = requestResponse["Error"];
          }
          else
          {
            strError = "Encountered an unknown error.";
          }
          requestResponse.clear();
        }
        else
        {
          stringstream ssError;
          ssError << "Received " << out.size() << " JSON resultant lines when expecting 1 line.";
          strError = ssError.str();
        }
      }
      in.clear();
      out.clear();

      return bResult;
    }
    // }}}
    // {{{ useSecureJunction()
    void ServiceJunction::useSecureJunction(const bool bUseSecureJunction)
    {
      m_bUseSecureJunction = bUseSecureJunction;
    }
    // }}}
    // {{{ useSingleSocket()
    void ServiceJunction::useSingleSocket(const bool bUseSingleSocket)
    {
      if (m_bUseSingleSocket != bUseSingleSocket)
      {
        m_bUseSingleSocket = bUseSingleSocket;
        if (m_bUseSingleSocket)
        {
          m_pThreadRequest = new thread([this](){requestThread();});
          #ifdef COMMON_LINUX
          pthread_setname_np(m_pThreadRequest->native_handle(), "SJ_requestThrea");
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
    Utility *ServiceJunction::utility()
    {
      return m_pUtility;
    }
    // }}}
  }
}
