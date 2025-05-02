/* -*- c++ -*- */
///////////////////////////////////////////
// Acorn
// -------------------------------------
// file       : Acorn.cpp
// author     : Ben Kietzman
// begin      : 2018-12-27
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
/*! \file Acorn.cpp
* \brief Acorn Class
*/
// {{{ includes
#include "Acorn"
// }}}
extern "C++"
{
  namespace common
  {
    #ifdef COMMON_LINUX
    sem_t m_semaAcornRequestLock;
    #endif
    #ifdef COMMON_SOLARIS
    sema_t m_semaAcornRequestLock;
    #endif
    #ifdef COMMON_LINUX
    sem_t m_semaAcornThrottle;
    #endif
    #ifdef COMMON_SOLARIS
    sema_t m_semaAcornThrottle;
    #endif
    // {{{ Acorn()
    Acorn::Acorn(string &strError)
    {
      #ifdef COMMON_LINUX
      sem_init(&m_semaAcornRequestLock, 0, 10);
      #endif
      #ifdef COMMON_SOLARIS
      sema_init(&m_semaAcornRequestLock, 10, USYNC_THREAD, NULL);
      #endif
      m_bUseSingleSocket = false;
      m_unThrottle = 0;
      m_unUniqueID = 0;
      m_pUtility = new Utility(strError);
    }
    // }}}
    // {{{ ~Acorn()
    Acorn::~Acorn()
    {
      if (m_bUseSingleSocket)
      {
        useSingleSocket(false);
      }
      delete m_pUtility;
      #ifdef COMMON_LINUX
      sem_destroy(&m_semaAcornRequestLock);
      #endif
      #ifdef COMMON_SOLARIS
      sema_destroy(&m_semaAcornRequestLock);
      #endif
      if (m_unThrottle > 0)
      {
        #ifdef COMMON_LINUX
        sem_destroy(&m_semaAcornThrottle);
        #endif
        #ifdef COMMON_SOLARIS
        sema_destroy(&m_semaAcornThrottle);
        #endif
      }
    }
    // }}}
    // {{{ email()
    bool Acorn::email(const string strFrom, list<string> toList, list<string> ccList, list<string> bccList, const string strSubject, const string strText, const string strHTML, list<string> fileList, string &strError)
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
            char szBuffer[32768];
            ssize_t nReturn;
            string strData;
            while ((nReturn = read(fdFile, szBuffer, 32768)) > 0)
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
    bool Acorn::email(const string strFrom, list<string> toList, list<string> ccList, list<string> bccList, const string strSubject, const string strText, const string strHTML, map<string, string> fileMap, string &strError)
    {
      bool bResult = false;
      Json *ptRequest = new Json, *ptResponse = new Json;

      if (!m_strApplication.empty())
      {
        ptRequest->insert("reqApp", m_strApplication);
      }
      if (!m_strProgram.empty())
      {
        ptRequest->insert("reqProg", m_strProgram);
      }
      ptRequest->insert("Acorn", "email");
      ptRequest->m["Request"] = new Json;
      if (!strFrom.empty())
      {
        ptRequest->m["Request"]->insert("From", strFrom);
      }
      if (!toList.empty())
      {
        ptRequest->m["Request"]->insert("To", "");
        for (list<string>::iterator i = toList.begin(); i != toList.end(); i++)
        {
          if (i != toList.begin())
          {
            ptRequest->m["Request"]->m["To"]->v += ",";
          }
          ptRequest->m["Request"]->m["To"]->v += (*i);
        }
      }
      if (!ccList.empty())
      {
        ptRequest->m["Request"]->insert("CC", "");
        for (list<string>::iterator i = ccList.begin(); i != ccList.end(); i++)
        {
          if (i != ccList.begin())
          {
            ptRequest->m["Request"]->m["CC"]->v += ",";
          }
          ptRequest->m["Request"]->m["CC"]->v += (*i);
        }
      }
      if (!bccList.empty())
      {
        ptRequest->m["Request"]->insert("BCC", "");
        for (list<string>::iterator i = bccList.begin(); i != bccList.end(); i++)
        {
          if (i != bccList.begin())
          {
            ptRequest->m["Request"]->m["BCC"]->v += ",";
          }
          ptRequest->m["Request"]->m["BCC"]->v += (*i);
        }
      }
      if (!strSubject.empty())
      {
        ptRequest->m["Request"]->insert("Subject", strSubject);
      }
      if (!strText.empty())
      {
        ptRequest->m["Request"]->insert("Text", strText);
      }
      if (!strHTML.empty())
      {
        ptRequest->m["Request"]->insert("HTML", strHTML);
      }
      if (!fileMap.empty())
      {
        ptRequest->m["Request"]->m["Attachments"] = new Json;
        for (map<string, string>::iterator i = fileMap.begin(); i != fileMap.end(); i++)
        {
          string strData;
          Json *ptJson = new Json;
          m_manip.encodeBase64(i->second, strData);
          ptRequest->m["Request"]->m["Attachments"];
          ptJson->insert("Name", i->first);
          ptJson->insert("Data", strData);
          ptJson->insert("Encode", "base64");
          ptRequest->m["Request"]->m["Attachments"]->push_back(ptJson);
          delete ptJson;
        }
      }
      if (request(ptRequest, ptResponse, strError))
      {
        bResult = true;
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
      delete ptRequest;
      delete ptResponse;

      return bResult;
    }
    // }}}
    // {{{ jwt()
    bool Acorn::jwt(const string strSigner, const string strSecret, string &strPayload, Json *ptPayload, string &strError)
    {
      bool bDecode = false, bResult = false;
      string strJson;
      Json *ptRequest = new Json, *ptResponse = new Json;

      if (!m_strApplication.empty())
      {
        ptRequest->insert("reqApp", m_strApplication);
      }
      if (!m_strProgram.empty())
      {
        ptRequest->insert("reqProg", m_strProgram);
      }
      ptRequest->insert("Acorn", "jwt");
      if (!strPayload.empty())
      {
        bDecode = true;
        ptRequest->insert("Function", "decode");
      }
      else
      {
        ptRequest->insert("Function", "encode");
      }
      ptRequest->m["Request"] = new Json;
      ptRequest->m["Request"]->insert("Signer", strSigner);
      ptRequest->m["Request"]->insert("Secret", strSecret);
      if (bDecode)
      {
        ptRequest->m["Request"]->insert("Payload", strPayload);
      }
      else
      {
        ptRequest->m["Request"]->m["Payload"] = new Json(ptPayload);
      }
      if (request(ptRequest, ptResponse, strError))
      {
        if (ptResponse->m.find("Status") != ptResponse->m.end() && ptResponse->m["Status"]->v == "okay")
        {
          if (ptResponse->m.find("Response") != ptResponse->m.end())
          {
            if (ptResponse->m["Response"]->m.find("Payload") != ptResponse->m["Response"]->m.end())
            {
              if (bDecode)
              {
                bResult = true;
                ptPayload->merge(ptResponse->m["Response"]->m["Payload"], true, false);
              }
              else if (!ptResponse->m["Response"]->m["Payload"]->v.empty())
              {
                bResult = true;
                strPayload = ptResponse->m["Response"]->m["Payload"]->v;
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
          }
          else
          {
            strError = "Failed to receive data.";
          }
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
      delete ptRequest;
      delete ptResponse;

      return bResult;
    }
    // }}}
    // {{{ logger()
    bool Acorn::logger(const string strApplication, const string strUser, const string strPassword, const string strMessage, map<string, string> label, string &strError)
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
    bool Acorn::logger(const string strUser, const string strPassword, const string strMessage, map<string, string> label, string &strError)
    {
      return logger(m_strApplication, strUser, strPassword, strMessage, label, strError);
    }
    bool Acorn::logger(const string strApplication, const string strUser, const string strPassword, const string strFunction, const string strMessage, map<string, string> label, list<Json *> &result, string &strError)
    {
      bool bResult = false;
      list<string> in, out;
      string strJson;
      Json *ptRequest = new Json, *ptResponse = new Json;

      ptRequest->insert("Application", strApplication);
      if (!m_strApplication.empty())
      {
        ptRequest->insert("reqApp", m_strApplication);
      }
      if (!m_strProgram.empty())
      {
        ptRequest->insert("reqProg", m_strProgram);
      }
      if (!m_strTimeout.empty())
      {
        ptRequest->insert("Timeout", m_strTimeout);
      }
      ptRequest->insert("Acorn", "logger");
      ptRequest->insert("User", strUser);
      ptRequest->insert("Password", strPassword);
      ptRequest->insert("Function", strFunction);
      ptRequest->insert("Message", strMessage);
      ptRequest->m["Label"] = new Json(label);
      if (request(ptRequest, ptResponse, strError))
      {
        if (ptResponse->m.find("Status") != ptResponse->m.end() && ptResponse->m["Status"]->v == "okay")
        {
          bResult = true;
          if (ptResponse->m.find("Response") != ptResponse->m.end())
          {
            for (list<Json *>::iterator i = ptResponse->m["Response"]->l.begin(); i != ptResponse->m["Response"]->l.end(); i++)
            {
              result.push_back(new Json(*i));
            }
          }
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
      delete ptRequest;
      delete ptResponse;

      return bResult;
    }
    bool Acorn::logger(const string strUser, const string strPassword, const string strFunction, const string strMessage, map<string, string> label, list<Json *> &result, string &strError)
    {
      return logger(m_strApplication, strUser, strPassword, strFunction, strMessage, label, result, strError);
    }
    // }}}
    // {{{ mssqlQuery()
    bool Acorn::mssqlQuery(const string strUser, const string strPassword, const string strServer, const string strQuery, list<map<string, string> > &result, string &strError)
    {
      bool bResult = false;
      Json *ptRequest = new Json, *ptResponse = new Json;
      
      for (list<map<string, string> >::iterator i = result.begin(); i != result.end(); i++)
      {
        i->clear();
      } 
      result.clear();
      if (!m_strApplication.empty())
      {
        ptRequest->insert("reqApp", m_strApplication);
      }
      if (!m_strProgram.empty()) 
      { 
        ptRequest->insert("reqProg", m_strProgram);
      }
      ptRequest->insert("Acorn", "mssql");
      ptRequest->insert("User", strUser);
      ptRequest->insert("Password", strPassword);
      ptRequest->insert("Server", strServer);
      ptRequest->m["Request"] = new Json;
      ptRequest->m["Request"]->insert("Query", strQuery);
      if (request(ptRequest, ptResponse, strError))
      {
        if (ptResponse->m.find("Status") != ptResponse->m.end() && ptResponse->m["Status"]->v == "okay")
        {
          bResult = true;
          if (ptResponse->m.find("Response") != ptResponse->m.end())
          {
            for (list<Json *>::iterator rowIter = ptResponse->m["Response"]->l.begin(); rowIter != ptResponse->m["Response"]->l.end(); rowIter++)
            {
              map<string, string> row;
              (*rowIter)->flatten(row, true);
              result.push_back(row);
              row.clear();
            }
          }
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
      delete ptRequest;
      delete ptResponse;

      return bResult;
    }
    // }}}
    // {{{ mssqlUpdate()
    bool Acorn::mssqlUpdate(const string strUser, const string strPassword, const string strServer, const string strUpdate, string &strError)
    {
      bool bResult = false;
      Json *ptRequest = new Json, *ptResponse = new Json;
      list<map<string, string> > result;

      if (!m_strApplication.empty())
      {
        ptRequest->insert("reqApp", m_strApplication);
      }
      if (!m_strProgram.empty())
      {
        ptRequest->insert("reqProg", m_strProgram);
      }
      ptRequest->insert("Acorn", "mssql");
      ptRequest->insert("User", strUser);
      ptRequest->insert("Password", strPassword);
      ptRequest->insert("Server", strServer);
      ptRequest->m["Request"] = new Json;
      ptRequest->m["Request"]->insert("Update", strUpdate);
      if (request(ptRequest, ptResponse, strError))
      {
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
      delete ptRequest;
      delete ptResponse;

      return bResult;
    }
    // }}}
    // {{{ mysqlQuery()
    bool Acorn::mysqlQuery(const string strUser, const string strPassword, const string strServer, const string strDatabase, const string strQuery, list<map<string, string> > &result, string &strError)
    {
      bool bResult = false;
      Json *ptRequest = new Json, *ptResponse = new Json;

      for (list<map<string, string> >::iterator i = result.begin(); i != result.end(); i++)
      {
        i->clear();
      }
      result.clear();
      if (!m_strApplication.empty())
      {
        ptRequest->insert("reqApp", m_strApplication);
      }
      if (!m_strProgram.empty())
      {
        ptRequest->insert("reqProg", m_strProgram);
      }
      ptRequest->insert("Acorn", "mysql");
      ptRequest->insert("User", strUser);
      ptRequest->insert("Password", strPassword);
      ptRequest->insert("Server", strServer);
      ptRequest->insert("Database", strDatabase);
      ptRequest->m["Request"] = new Json;
      ptRequest->m["Request"]->insert("Query", strQuery);
      if (request(ptRequest, ptResponse, strError))
      {
        if (ptResponse->m.find("Status") != ptResponse->m.end() && ptResponse->m["Status"]->v == "okay")
        {
          bResult = true;
          if (ptResponse->m.find("Response") != ptResponse->m.end())
          {
            for (list<Json *>::iterator rowIter = ptResponse->m["Response"]->l.begin(); rowIter != ptResponse->m["Response"]->l.end(); rowIter++)
            {
              map<string, string> row;
              (*rowIter)->flatten(row, true);
              result.push_back(row);
              row.clear();
            }
          }
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
      delete ptRequest;
      delete ptResponse;

      return bResult;
    }
    // }}}
    // {{{ mysqlUpdate()
    bool Acorn::mysqlUpdate(const string strUser, const string strPassword, const string strServer, const string strDatabase, const string strUpdate, string &strID, string &strRows, string &strError)
    {
      bool bResult = false;
      Json *ptRequest = new Json, *ptResponse = new Json;
      list<map<string, string> > result;

      if (!m_strApplication.empty())
      {
        ptRequest->insert("reqApp", m_strApplication);
      }
      if (!m_strProgram.empty())
      {
        ptRequest->insert("reqProg", m_strProgram);
      }
      ptRequest->insert("Acorn", "mysql");
      ptRequest->insert("User", strUser);
      ptRequest->insert("Password", strPassword);
      ptRequest->insert("Server", strServer);
      ptRequest->insert("Database", strDatabase);
      ptRequest->m["Request"] = new Json;
      ptRequest->m["Request"]->insert("Update", strUpdate);
      if (request(ptRequest, ptResponse, strError))
      {
        if (ptResponse->m.find("Status") != ptResponse->m.end() && ptResponse->m["Status"]->v == "okay")
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
        else if (ptResponse->m.find("Error") != ptResponse->m.end() && !ptResponse->m["Error"]->v.empty())
        {
          strError = ptResponse->m["Error"]->v;
        }
        else
        {
          strError = "Encountered an unknown error.";
        }
      }
      delete ptRequest;
      delete ptResponse;

      return bResult;
    }
    bool Acorn::mysqlUpdate(const string strUser, const string strPassword, const string strServer, const string strDatabase, const string strUpdate, string &strError)
    {
      string strID, strRows;

      return mysqlUpdate(strUser, strPassword, strServer, strDatabase, strUpdate, strID, strRows, strError);
    }
    // }}}
    // {{{ oracleQuery()
    bool Acorn::oracleQuery(const string strSchema, const string strPassword, const string strTnsName, const string strQuery, list<map<string, string> > &result, string &strError)
    {
      bool bResult = false;
      Json *ptRequest = new Json, *ptResponse = new Json;

      for (list<map<string, string> >::iterator i = result.begin(); i != result.end(); i++)
      {
        i->clear();
      }
      result.clear();
      if (!m_strApplication.empty())
      {
        ptRequest->insert("reqApp", m_strApplication);
      }
      if (!m_strProgram.empty())
      {
        ptRequest->insert("reqProg", m_strProgram);
      }
      ptRequest->insert("Acorn", "oracle");
      ptRequest->insert("Schema", strSchema);
      ptRequest->insert("Password", strPassword);
      ptRequest->insert("tnsName", strTnsName);
      ptRequest->m["Request"] = new Json;
      ptRequest->m["Request"]->insert("Query", strQuery);
      if (request(ptRequest, ptResponse, strError))
      {
        if (ptResponse->m.find("Status") != ptResponse->m.end() && ptResponse->m["Status"]->v == "okay")
        {
          bResult = true;
          if (ptResponse->m.find("Response") != ptResponse->m.end())
          {
            for (list<Json *>::iterator rowIter = ptResponse->m["Response"]->l.begin(); rowIter != ptResponse->m["Response"]->l.end(); rowIter++)
            {
              map<string, string> row;
              (*rowIter)->flatten(row, true);
              result.push_back(row);
              row.clear();
            }
          }
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
      delete ptRequest;
      delete ptResponse;

      return bResult;
    }
    // }}}
    // {{{ oracleUpdate()
    bool Acorn::oracleUpdate(const string strSchema, const string strPassword, const string strTnsName, const string strUpdate, string &strID, string &strRows, string &strError)
    {
      bool bResult = false;
      Json *ptRequest = new Json, *ptResponse = new Json;
      list<map<string, string> > result;

      if (!m_strApplication.empty())
      {
        ptRequest->insert("reqApp", m_strApplication);
      }
      if (!m_strProgram.empty())
      {
        ptRequest->insert("reqProg", m_strProgram);
      }
      ptRequest->insert("Acorn", "oracle");
      ptRequest->insert("Schema", strSchema);
      ptRequest->insert("Password", strPassword);
      ptRequest->insert("tnsName", strTnsName);
      ptRequest->m["Request"] = new Json;
      ptRequest->m["Request"]->insert("Update", strUpdate);
      if (request(ptRequest, ptResponse, strError))
      {
        if (ptResponse->m.find("Status") != ptResponse->m.end() && ptResponse->m["Status"]->v == "okay")
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
        else if (ptResponse->m.find("Error") != ptResponse->m.end() && !ptResponse->m["Error"]->v.empty())
        {
          strError = ptResponse->m["Error"]->v;
        }
        else
        {
          strError = "Encountered an unknown error.";
        }
      }
      delete ptRequest;
      delete ptResponse;

      return bResult;
    }
    bool Acorn::oracleUpdate(const string strSchema, const string strPassword, const string strTnsName, const string strUpdate, string &strError)
    {
      string strID, strRows;

      return oracleUpdate(strSchema, strPassword, strTnsName, strUpdate, strID, strRows, strError);
    }
    // }}}
    // {{{ request()
    bool Acorn::request(Json *ptRequest, Json *ptResponse, string &strError)
    {
      return request(ptRequest, ptResponse, 0, strError);
    }
    bool Acorn::request(Json *ptRequest, Json *ptResponse, time_t CTimeout, string &strError)
    {
      bool bResult = false;
      time_t CEnd, CStart;

      if (m_unThrottle > 0)
      {
        #ifdef COMMON_LINUX
        sem_wait(&m_semaAcornThrottle);
        #endif
        #ifdef COMMON_SOLARIS
        sema_wait(&m_semaAcornThrottle);
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
          char szBuffer[32768];
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
          ptJson->insert("acornUniqueID", ssUniqueID.str(), 'n');
          ptJson->json(strBuffer);
          strBuffer.append("\n");
          delete ptJson;
          acornreqdata *ptData = new acornreqdata;
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
                if ((nReturn = read(readpipe[0], szBuffer, 32768)) > 0)
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
                    if (ptJson->m.find("acornUniqueID") != ptJson->m.end())
                    {
                      delete ptJson->m["acornUniqueID"];
                      ptJson->m.erase("acornUniqueID");
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
        list<string> server;
        utility()->readConf(strError);
        if (utility()->conf()->m.find("Load Balancer") != utility()->conf()->m.end())
        {
          server.push_back(utility()->conf()->m["Load Balancer"]->v);
        }
        if (!server.empty())
        {
          bool bDone = false;
          for (list<string>::iterator i = server.begin(); !bDone && i != server.end(); i++)
          {
            bool bConnected = false, bAddrInfo = false, bSocket = false;
            int fdSocket = -1, nReturn = -1;
            string strServer;
            unsigned int unAttempt = 0, unPick = 0, unSeed = time(NULL);
            vector<string> acornServer;
            for (int j = 1; !m_manip.getToken(strServer, (*i), j, ",", true).empty(); j++)
            {
              acornServer.push_back(m_manip.trim(strServer, strServer));
            }
            srand(unSeed);
            unPick = rand_r(&unSeed) % acornServer.size();
            #ifdef COMMON_LINUX
            sem_wait(&m_semaAcornRequestLock);
            #endif
            #ifdef COMMON_SOLARIS
            sema_wait(&m_semaAcornRequestLock);
            #endif
            while (!bConnected && unAttempt++ < acornServer.size())
            {
              addrinfo hints, *result;
              bAddrInfo = bSocket = false;
              if (unPick == acornServer.size())
              {
                unPick = 0;
              }
              strServer = acornServer[unPick];
              memset(&hints, 0, sizeof(addrinfo));
              hints.ai_family = AF_UNSPEC;
              hints.ai_socktype = SOCK_STREAM;
              m_mutexGetAddrInfo.lock();
              nReturn = getaddrinfo(strServer.c_str(), "22676", &hints, &result);
              m_mutexGetAddrInfo.unlock();
              if (nReturn == 0)
              {
                addrinfo *rp;
                bAddrInfo = true;
                for (rp = result; !bConnected && rp != NULL; rp = rp->ai_next)
                {
                  bSocket = false;
                  if ((fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
                  {
                    bSocket = true;
                    if (connect(fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
                    {
                      bConnected = true;
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
            sem_post(&m_semaAcornRequestLock);
            #endif
            #ifdef COMMON_SOLARIS
            sema_post(&m_semaAcornRequestLock);
            #endif
            acornServer.clear();
            if (bConnected)
            {
              bool bExit = false;
              char szBuffer[32768];
              size_t unPosition;
              string strBuffer[2], strLine, strTrim;
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
                    if ((nReturn = read(fdSocket, szBuffer, 32768)) > 0)
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
                  if (fds[0].fd == fdSocket && (fds[0].revents & POLLOUT))
                  {
                    if ((nReturn = write(fdSocket, strBuffer[1].c_str(), strBuffer[1].size())) > 0)
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
              close(fdSocket);
            }
            else
            {
              stringstream ssError;
              if (!bAddrInfo)
              {
                ssError << "getaddrinfo(" << nReturn << ") " << gai_strerror(nReturn);
              }
              else
              {
                ssError << ((!bSocket)?"socket":"connect") << "(" << errno << ") " << strerror(errno);
              }
              strError = ssError.str();
            }
          }
          if (!bResult && strError.empty())
          {
            strError = "Acorn request failed without returning an error message.";
          }
        }
        else
        {
          strError = (string)"Please provide the Load Balancer server via the " + utility()->getConfPath() + (string)" file.";
        }
        server.clear();
      }
      if (m_unThrottle > 0)
      {
        #ifdef COMMON_LINUX
        sem_post(&m_semaAcornThrottle);
        #endif
        #ifdef COMMON_SOLARIS
        sema_post(&m_semaAcornThrottle);
        #endif
      }

      return bResult;
    }
    // }}}
    // {{{ requestThread()
    void Acorn::requestThread()
    {
      size_t unSleep = 1;

      while (m_bUseSingleSocket)
      {
        list<string> server;
        string strError;
        utility()->readConf(strError);
        if (utility()->conf()->m.find("Load Balancer") != utility()->conf()->m.end())
        {
          server.push_back(utility()->conf()->m["Load Balancer"]->v);
        }
        if (!server.empty())
        {
          for (list<string>::iterator i = server.begin(); m_bUseSingleSocket && i != server.end(); i++)
          {
            bool bConnected = false;
            int fdSocket = -1, nReturn = -1;
            string strServer;
            unsigned int unAttempt = 0, unPick = 0, unSeed = time(NULL);
            vector<string> acornServer;
            for (int j = 1; !m_manip.getToken(strServer, (*i), j, ",", true).empty(); j++)
            {
              acornServer.push_back(m_manip.trim(strServer, strServer));
            }
            srand(unSeed);
            unPick = rand_r(&unSeed) % acornServer.size();
            while (m_bUseSingleSocket && !bConnected && unAttempt++ < acornServer.size())
            {
              addrinfo hints, *result;
              if (unPick == acornServer.size())
              {
                unPick = 0;
              }
              strServer = acornServer[unPick];
              memset(&hints, 0, sizeof(addrinfo));
              hints.ai_family = AF_UNSPEC;
              hints.ai_socktype = SOCK_STREAM;
              m_mutexGetAddrInfo.lock();
              nReturn = getaddrinfo(strServer.c_str(), "22676", &hints, &result);
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
                      bConnected = true;
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
            acornServer.clear();
            if (bConnected)
            {
              bool bExit = false;
              char szBuffer[32768];
              size_t unPosition;
              string strBuffer[2], strLine, strTrim;
              unSleep = 1;
              while (m_bUseSingleSocket && !bExit)
              {
                size_t unIndex = 1;
                pollfd *fds = new pollfd[m_requests.size() + 1];
                m_mutexRequests.lock();
                for (map<int, acornreqdata *>::iterator j = m_requests.begin(); j != m_requests.end(); j++)
                {
                  if (!j->second->bSent)
                  {
                    j->second->bSent = true;
                    strBuffer[1] += j->second->strBuffer[0];
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
                    if ((nReturn = read(fdSocket, szBuffer, 32768)) > 0)
                    {
                      strBuffer[0].append(szBuffer, nReturn);
                      while ((unPosition = strBuffer[0].find("\n")) != string::npos)
                      {
                        Json *ptJson = new Json(strBuffer[0].substr(0, unPosition));
                        strBuffer[0].erase(0, unPosition + 1);
                        if (ptJson->m.find("acornUniqueID") != ptJson->m.end() && !ptJson->m["acornUniqueID"]->v.empty())
                        {
                          size_t unUniqueID = atoi(ptJson->m["acornUniqueID"]->v.c_str());
                          string strResponse;
                          ptJson->json(strResponse);
                          m_mutexRequests.lock();
                          if (m_requests.find(unUniqueID) != m_requests.end())
                          {
                            m_requests[unUniqueID]->strBuffer[1] = strResponse + "\n";
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
                    if ((nReturn = write(fdSocket, strBuffer[1].c_str(), strBuffer[1].size())) > 0)
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
                    for (map<int, acornreqdata *>::iterator j = m_requests.begin(); j != m_requests.end(); j++)
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
              for (map<int, acornreqdata *>::iterator j = m_requests.begin(); j != m_requests.end(); j++)
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
      for (map<int, acornreqdata *>::iterator i = m_requests.begin(); i != m_requests.end(); i++)
      {
        if (i->second->fdSocket != -1)
        {
          close(i->second->fdSocket);
          i->second->fdSocket = -1;
        }
      }
    }
    // }}}
    // {{{ setApplication()
    void Acorn::setApplication(const string strApplication)
    {
      m_strApplication = strApplication;
    }
    // }}}
    // {{{ setThrottle()
    void Acorn::setThrottle(const size_t unThrottle)
    {
      if (unThrottle > 0)
      {
        if (unThrottle != m_unThrottle)
        {
          if (m_unThrottle > 0)
          {
            #ifdef COMMON_LINUX
            sem_destroy(&m_semaAcornThrottle);
            #endif
            #ifdef COMMON_SOLARIS
            sema_destroy(&m_semaAcornThrottle);
            #endif
          }
          #ifdef COMMON_LINUX
          sem_init(&m_semaAcornThrottle, 0, unThrottle);
          #endif
          #ifdef COMMON_SOLARIS
          sema_init(&m_semaAcornThrottle, unThrottle, USYNC_THREAD, NULL);
          #endif
        }
      }
      else if (m_unThrottle > 0)
      {
        #ifdef COMMON_LINUX
        sem_destroy(&m_semaAcornThrottle);
        #endif
        #ifdef COMMON_SOLARIS
        sema_destroy(&m_semaAcornThrottle);
        #endif
      }
      m_unThrottle = unThrottle;
    }
    // }}}
    // {{{ setTimeout()
    void Acorn::setTimeout(const string strTimeout)
    {
      m_strTimeout = strTimeout;
    }
    // }}}
    // {{{ useSingleSocket()
    void Acorn::useSingleSocket(const bool bUseSingleSocket)
    {
      if (m_bUseSingleSocket != bUseSingleSocket)
      {
        m_bUseSingleSocket = bUseSingleSocket;
        if (m_bUseSingleSocket)
        {
          m_pThreadRequest = new thread([this](){requestThread();});
          #ifdef COMMON_LINUX
          pthread_setname_np(m_pThreadRequest->native_handle(), "AC_requestThrea");
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
    Utility *Acorn::utility()
    {
      return m_pUtility;
    }
    // }}}
  }
}
