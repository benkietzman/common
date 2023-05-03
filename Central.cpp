// vim: syntax=cpp
// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Central
// -------------------------------------
// file       : Central.cpp
// author     : Ben Kietzman
// begin      : 2011-07-05
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

/*! \file Central.cpp
* \brief Central Class
*/
// {{{ includes
#include "Central"
// }}}
extern "C++"
{
  namespace common
  {
    // {{{ Central()
    Central::Central(string &strError)
    {
      m_pAcorn = new Acorn(strError);
      m_pArchive = new Archive;
      m_pDate = new DateTime;
      m_pFile = new File;
      m_pJunction = new ServiceJunction(strError);
      m_pManip = new StringManip;
      m_pRadial = new Radial(strError);
      m_pUtility = new Utility(strError);
      m_pMysql = NULL;
    }
    // }}}
    // {{{ ~Central()
    Central::~Central()
    {
      for (map<string, list<string> >::iterator i = m_alert.begin(); i != m_alert.end(); i++)
      {
        i->second.clear();
      }
      m_alert.clear();
      for (map<string, commonDatabase *>::iterator i = m_database.begin(); i != m_database.end(); i++)
      {
        i->second->credentials.clear();
        delete i->second;
      }
      m_database.clear();
      delete m_pAcorn;
      delete m_pArchive;
      delete m_pDate;
      delete m_pFile;
      delete m_pJunction;
      delete m_pManip;
      delete m_pUtility;
    }
    // }}}
    // {{{ acorn()
    Acorn *Central::acorn()
    {
      return m_pAcorn;
    }
    // }}}
    // {{{ addDatabase()
    bool Central::addDatabase(const string strName, map<string, string> credentials, string &strError)
    {
      bool bResult = false;

      if (!strName.empty())
      {
        if (credentials.find("Type") != credentials.end() && (credentials["Type"] == "fusion" || credentials["Type"] == "mssql" || credentials["Type"] == "mysql" || credentials["Type"] == "oracle"))
        {
          if (credentials["Type"] == "fusion")
          {
            if ((credentials.find("User") != credentials.end() && !credentials["User"].empty() && credentials.find("Password") != credentials.end() && !credentials["Password"].empty()) || (credentials.find("Auth") != credentials.end() && !credentials["Auth"].empty()))
            {
              bResult = true;
              if (credentials.find("User") == credentials.end() || credentials["User"].empty())
              {
                credentials["User"] = "";
              }
              if (credentials.find("Password") == credentials.end() || credentials["Password"].empty())
              {
                credentials["Password"] = "";
              }
              if (credentials.find("Auth") == credentials.end() || credentials["Auth"].empty())
              {
                credentials["Auth"] = "";
              }
            }
            else
            {
              strError = "Please provide the Google Fusion User and Password or Auth token.";
            }
          }
          else if (credentials["Type"] == "mssql")
          {
            if (credentials.find("User") != credentials.end() && !credentials["User"].empty())
            {
              if (credentials.find("Password") != credentials.end() && !credentials["Password"].empty())
              {
                if (credentials.find("Server") != credentials.end() && !credentials["Server"].empty())
                {
                  bResult = true;
                }
                else
                {
                  strError = "Please provide a MSSQL Server.";
                }
              }
              else
              {
                strError = "Please provide a MSSQL Password.";
              }
            }
            else
            {
              strError = "Please provide a MSSQL User.";
            }
          }
          else if (credentials["Type"] == "mysql")
          {
            if (credentials.find("User") != credentials.end() && !credentials["User"].empty())
            {
              if (credentials.find("Password") != credentials.end() && !credentials["Password"].empty())
              {
                if (credentials.find("Server") != credentials.end() && !credentials["Server"].empty())
                {
                  if (credentials.find("Database") != credentials.end() && !credentials["Database"].empty())
                  {
                    bResult = true;
                  }
                  else
                  {
                    strError = "Please provide a MySQL Database.";
                  }
                }
                else
                {
                  strError = "Please provide a MySQL Server.";
                }
              }
              else
              {
                strError = "Please provide a MySQL Password.";
              }
            }
            else
            {
              strError = "Please provide a MySQL User.";
            }
          }
          else if (credentials["Type"] == "oracle")
          {
            if (credentials.find("Schema") != credentials.end() && !credentials["Schema"].empty())
            {
              if (credentials.find("Password") != credentials.end() && !credentials["Password"].empty())
              {
                if (credentials.find("tnsName") != credentials.end() && !credentials["tnsName"].empty())
                {
                  bResult = true;
                }
                else
                {
                  strError = "Please provide an Oracle tnsName.";
                }
              }
              else
              {
                strError = "Please provide an Oracle Password.";
              }
            }
            else
            {
              strError = "Please provide an Oracle Schema.";
            }
          }
          else if (credentials["Type"] == "teradata")
          {
            if (credentials.find("User") != credentials.end() && !credentials["User"].empty())
            {
              if (credentials.find("Password") != credentials.end() && !credentials["Password"].empty())
              {
                if (credentials.find("URL") != credentials.end() && !credentials["URL"].empty())
                {
                  bResult = true;
                }
                else
                {
                  strError = "Please provide a Teradata URL.";
                }
              }
              else
              {
                strError = "Please provide a Teradata Password.";
              }
            }
            else
            {
              strError = "Please provide a Teradata User.";
            }
          }
          else
          {
            strError = "Please provide a valid Type of database.  (fusion, mssql, mysql, oracle, or teradata)";
          }
          if (bResult)
          {
            if (m_database.find(strName) != m_database.end())
            {
              m_database[strName]->credentials.clear();
              delete m_database[strName];
              m_database.erase(strName);
            }
            m_database[strName] = new commonDatabase;
            m_database[strName]->credentials = credentials;
          }
        }
        else
        {
          strError = "Please provide the Type of database.";
        }
      }
      else
      {
        strError = "Please provide the strName for this database connection.";
      }

      return bResult;
    }
    // }}}
    // {{{ alert()
    bool Central::alert(const string strMessage)
    {
      string strError;

      return alert(strMessage, strError);
    }
    bool Central::alert(const string strMessage, string &strError)
    {
      bool bResult = false;

      if (!m_strApplication.empty())
      {
        if (alert(m_strApplication, strMessage, strError))
        {
          bResult = true;
        }
      }
      else if (notify("", strMessage, strError))
      {
        strError = "Please provide the Central application via setApplication.";
      }

      return bResult;
    }
    bool Central::alert(const string strApplication, const string strMessage, string &strError)
    {
      bool bResult = false;

      if (notify("", strMessage, strError))
      {
        if (!strApplication.empty())
        {
          m_mutexAlert.lock();
          if (m_alert.find(strApplication) == m_alert.end())
          {
            list<string> contact;
            m_alert[strApplication] = contact;
          }
          if (m_alert.find(strApplication) != m_alert.end() && m_alert[strApplication].empty())
          {
            if (utility()->conf()->m.find("Database User") != utility()->conf()->m.end() && !utility()->conf()->m["Database User"]->v.empty() && utility()->conf()->m.find("Database Password") != utility()->conf()->m.end() && !utility()->conf()->m["Database Password"]->v.empty() && utility()->conf()->m.find("Database Server") != utility()->conf()->m.end() && !utility()->conf()->m["Database Server"]->v.empty())
            {
              list<map<string, string> > getUser;
              string strError;
              stringstream ssQuery;
              ssQuery << "select distinct d.userid from application a, application_contact b, contact_type c, person d where a.id=b.application_id and b.type_id=c.id and b.contact_id=d.id and a.name = '" << strApplication << "' and b.notify = 1 and (c.type = 'Primary Developer' or c.type = 'Backup Developer') and d.userid is not null";
              if (junction()->mysqlQuery(utility()->conf()->m["Database User"]->v, utility()->conf()->m["Database Password"]->v, utility()->conf()->m["Database Server"]->v, utility()->conf()->m["Database"]->v, ssQuery.str(), getUser, strError))
              {
                for (list<map<string, string> >::iterator i = getUser.begin(); i != getUser.end(); i++)
                {
                  map<string, string> getUserRow = (*i);
                  m_alert[strApplication].push_back(getUserRow["userid"]);
                  getUserRow.clear();
                }
              }
              for (list<map<string, string> >::iterator i = getUser.begin(); i != getUser.end(); i++)
              {
                i->clear();
              }
              getUser.clear();
            }
          }
          m_mutexAlert.unlock();
          if (m_alert.find(strApplication) != m_alert.end() && !m_alert[strApplication].empty())
          {
            bResult = true;
            for (list<string>::iterator i = m_alert[strApplication].begin(); i != m_alert[strApplication].end(); i++)
            {
              if (!junction()->page((*i), strApplication + (string)":  " + strMessage, strError))
              {
                bResult = false;
              }
            }
          }
          else
          {
            strError = "Please provide application contacts in Central.";
          }
        }
        else
        {
          strError = "Please provide the application.";
        }
      }

      return bResult;
    }
    // }}}
    // {{{ archive()
    Archive *Central::archive()
    {
      return m_pArchive;
    }
    // }}}
    // {{{ date()
    DateTime *Central::date()
    {
      return m_pDate;
    }
    // }}}
    // {{{ file()
    File *Central::file()
    {
      return m_pFile;
    }
    // }}}
    // {{{ free()
    void Central::free(list<map<string, string> > *result)
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
    // {{{ getProcessStatus()
    void Central::getProcessStatus(time_t &CTime, float &fCpu, float &fMem, unsigned long &ulImage, unsigned long &ulResident)
    {
      getProcessStatus(getpid(), CTime, fCpu, fMem, ulImage, ulResident);
    }
    void Central::getProcessStatus(const pid_t nPid, time_t &CTime, float &fCpu, float &fMem, unsigned long &ulImage, unsigned long &ulResident)
    {
      stringstream ssFile, ssStatus;
      ifstream inProc;

      ssFile << "/proc/" << nPid << "/";
      #ifdef COMMON_LINUX
      ifstream inStat((ssFile.str() + "stat").c_str());
      CTime = 0;
      inProc.open("/proc/stat");
      if (inProc.good() && inStat.good())
      {
        long lJiffies = 0, lPageSize = 0;
        long long llBootTime = 0, llStartTime = 0;
        string strField, strLine;
        stringstream ssLine;
        lJiffies = sysconf(_SC_CLK_TCK);
        lPageSize = sysconf(_SC_PAGE_SIZE) / 1024;
        while (llBootTime == 0 && utility()->getLine(inProc, strLine))
        {
          ssLine.str(strLine);
          ssLine >> strField;
          if (strField == "btime")
          {
            ssLine >> llBootTime;
          }
        }
        for (unsigned int i = 0; i < 21; i++)
        {
          inStat >> strField;
        }
        inStat >> llStartTime >> ulImage >> ulResident;
        CTime = llBootTime + (llStartTime / lJiffies);
        ulImage /= 1024;
        ulResident *= lPageSize;
      }
      inProc.close();
      inStat.close();
      #endif
      #ifdef COMMON_SOLARIS
      psinfo *ptInfo = new psinfo;
      ptInfo->pr_pctcpu = 0;
      ptInfo->pr_pctmem = 0;
      ptInfo->pr_size = 0;
      ptInfo->pr_rssize = 0;
      inProc.open((ssFile.str() + "psinfo").c_str(), ios::in|ios::binary);
      if (inProc.good())
      {
        inProc.read((char *)ptInfo, sizeof(psinfo));
        CTime = ptInfo->pr_start.tv_sec;
        fCpu = (float)((float)ptInfo->pr_pctcpu * (float)100 / (float)32768);
        fMem = (float)((float)ptInfo->pr_pctmem * (float)100 / (float)32768);
        ulImage = ptInfo->pr_size;
        ulResident = ptInfo->pr_rssize;
      }
      inProc.close();
      delete ptInfo;
      #endif
    }
    // }}}
    // {{{ junction()
    ServiceJunction *Central::junction()
    {
      return m_pJunction;
    }
    // }}}
    // {{{ log()
    bool Central::log(const string strMessage)
    {
      string strError;

      return log(strMessage, strError);
    }
    bool Central::log(const string strMessage, string &strError)
    {
      bool bResult = false;

      if (!m_strLog.empty())
      {
        bResult = log(m_strLog, m_strPrefix, strMessage, strError, m_strFrequency, m_bLock, m_bTimestamp);
      }
      else
      {
        strError = "Please provide the log path via setLog.";
      }

      return bResult;
    }
    bool Central::log(const string strLog, const string strPrefix, const string strMessage, string &strError, const string strFrequency, const bool bLock, const bool bTimestamp)
    {
      bool bNeedArchive = false, bResult = false;
      unsigned int unFrequency = 0;
      ofstream outLog;
      string strLast[4], strLastLog;
      stringstream ssDay, ssHour, ssLast, ssLog, ssMonth, ssYear;
      struct tm tTime;
      time_t CTime;

      time(&CTime);
      localtime_r(&CTime, &tTime);
      if (strFrequency == "yearly")
      {
        unFrequency = 1;
      }
      else if (strFrequency == "monthly")
      {
        unFrequency = 2;
      }
      else if (strFrequency == "daily")
      {
        unFrequency = 3;
      }
      else if (strFrequency == "hourly")
      {
        unFrequency = 4;
      }
      if (unFrequency > 0)
      {
        strLastLog = strLog + (string)"/." + strPrefix + "last_log.dat";
      }
      if (bLock)
      {
        m_mutexLog.lock();
      }
      ssLog << strLog << '/' << strPrefix;
      if (unFrequency > 0)
      {
        ssYear << setw(4) << setfill('0') << (tTime.tm_year + 1900);
        ssLog << ssYear.str();
      }
      if (unFrequency > 1)
      {
        ssMonth << setw(2) << setfill('0') << (tTime.tm_mon + 1);
        ssLog << ssMonth.str();
      }
      if (unFrequency > 2)
      {
        ssDay << setw(2) << setfill('0') << tTime.tm_mday;
        ssLog << ssDay.str();
      }
      if (unFrequency > 3)
      {
        ssHour << setw(2) << setfill('0') << tTime.tm_hour;
        ssLog << ssHour.str();
      }
      ssLog << ".log";
      if (unFrequency > 0)
      {
        ifstream inLastLog;
        ssLast << strLog << '/' << strPrefix;
        inLastLog.open(strLastLog.c_str());
        if (inLastLog.good())
        {
          for (unsigned int i = 0; i < unFrequency; i++)
          {
            utility()->getLine(inLastLog, strLast[i]);
            if (!strLast[i].empty())
            {
              ssLast << strLast[i];
            }
          }
        }
        inLastLog.close();
        ssLast << ".log";
        if (ssLast.str() != ssLog.str())
        {
          ofstream outLastLog(strLastLog.c_str());
          bNeedArchive = true;
          if (outLastLog.good())
          {
            if (!ssYear.str().empty())
            {
              outLastLog << ssYear.str() << endl;
            }
            if (!ssMonth.str().empty())
            {
              outLastLog << ssMonth.str() << endl;
            }
            if (!ssDay.str().empty())
            {
              outLastLog << ssDay.str() << endl;
            }
            if (!ssHour.str().empty())
            {
              outLastLog << ssHour.str() << endl;
            }
          }
          outLastLog.close();
        }
      }
      outLog.open(ssLog.str().c_str(), ios::out|ios::app);
      if (outLog.good())
      {
        string strLine;
        stringstream ssMessage, ssTimeStamp;
        bResult = true;
        if (bTimestamp)
        {
          ssTimeStamp << put_time(&tTime, "%Y-%m-%d %H:%M:%S") << " ---> ";
        }
        ssMessage.str(strMessage);
        while (getline(ssMessage, strLine))
        {
          if (bTimestamp)
          {
            outLog << ssTimeStamp.str();
          }
          outLog << strLine << endl;
        }
      }
      outLog.close();
      if (bLock)
      {
        m_mutexLog.unlock();
      }
      if (unFrequency > 0 && bNeedArchive)
      {
        if (file()->fileExist(ssLast.str()))
        {
          if (archive()->gzip(ssLast.str()))
          {
            stringstream ssDir;
            ssDir << strLog;
            for (unsigned int i = 0; i < (unFrequency - 1); i++)
            {
              ssDir << '/' << strLast[i];
              if (!file()->directoryExist(ssDir.str()))
              {
                file()->makeDirectory(ssDir.str());
              }
            }
            if (file()->directoryExist(ssDir.str()))
            {
              size_t nPosition;
              if ((nPosition = ssLast.str().rfind("/", ssLast.str().size() - 1)) != string::npos)
              {
                nPosition++;
              }
              else
              {
                nPosition = 0;
              }
              file()->rename(ssLast.str() + (string)".gz", ssDir.str() + "/" + ssLast.str().substr(nPosition, ssLast.str().size() - nPosition) + (string)".gz");
            }
          }
        }
      }

      return bResult;
    }
    // }}}
    // {{{ logf()
    bool Central::logf(const char *pszMessage, ...)
    {
      char *pszBuffer;
      size_t unLength;
      string strMessage;
      va_list vArgs;

      va_start(vArgs, pszMessage);
      unLength = vsnprintf(NULL, 0, pszMessage, vArgs) + 1;
      va_end(vArgs);
      pszBuffer = (char *)malloc(unLength);
      va_start(vArgs, pszMessage);
      vsnprintf(pszBuffer, unLength, pszMessage, vArgs);
      va_end(vArgs);
      strMessage = pszBuffer;
      ::free(pszBuffer);

      return log(strMessage);
    }
    // }}}
    // {{{ manip()
    StringManip *Central::manip()
    {
      return m_pManip;
    }
    // }}}
    // {{{ notify()
    bool Central::notify(const string strMessage)
    {
      return notify("", strMessage);
    }
    bool Central::notify(const string strSubject, const string strMessage)
    {
      string strError;

      return notify(strSubject, strMessage, strError);
    }
    bool Central::notify(const string strSubject, const string strMessage, string &strError)
    {
      bool bResult = false;

      if (log(strMessage, strError))
      {
        if (!m_strEmail.empty())
        {
          struct utsname tServer;
          list<string> toList, ccList, bccList, fileList;
          uname(&tServer);
          toList.push_back(m_strEmail);
          if (junction()->email((string)"root@"+(string)tServer.nodename, toList, ccList, bccList, m_strApplication + (string)((!strSubject.empty())?":  " + strSubject:""), strMessage, "", fileList, strError))
          {
            bResult = true;
          }
        }
        else
        {
          strError = "Please provide the email account via setEmail.";
        }
      }

      return bResult;
    }
    // }}}
    // {{{ putCentralMessage()
    bool Central::putCentralMessage(const string strMessage, string &strError, const bool bAlert)
    {
      bool bResult = false;
      time_t CTime = 0;
      string strValue;
      Json *ptRequest = new Json, *ptResponse = new Json;

      time(&CTime);
      ptRequest->insert("Action", "message");
      ptRequest->insert("Type", ((bAlert)?"alert":"info"));
      if (!m_strApplication.empty())
      {
        ptRequest->insert("Application", m_strApplication);
      }
      ptRequest->insert("Start", manip()->toString(CTime, strValue));
      ptRequest->insert("End", manip()->toString(CTime + 1200, strValue));
      ptRequest->insert("Message", strMessage);
      if (junction()->sysInfo(ptRequest, ptResponse, strError))
      {
        bResult = true;
      }
      delete ptRequest;
      delete ptResponse;

      return bResult;
    }
    // }}}
    // {{{ query()
    list<map<string, string> > *Central::query(const string strName, const string strQuery, string &strError)
    {
      unsigned long long ullRows;

      return query(strName, strQuery, ullRows, strError);
    }
    list<map<string, string> > *Central::query(const string strName, const string strQuery, unsigned long long &ullRows, string &strError)
    {
      list<map<string, string> > *result = NULL;

      ullRows = 0;
      if (m_database.find(strName) != m_database.end())
      {
        if (m_database[strName]->credentials.find("Mutex") != m_database[strName]->credentials.end() && m_database[strName]->credentials["Mutex"] == "yes")
        {
          m_database[strName]->access.lock();
        }
        if (m_database[strName]->credentials["Type"] == "fusion")
        {
          result = new list<map<string, string> >;
          if (junction()->fusionQuery(m_database[strName]->credentials["User"], m_database[strName]->credentials["Password"], m_database[strName]->credentials["Auth"], strQuery, *result, strError))
          {
            ullRows = result->size();
            if (result->empty())
            {
              strError = "No rows returned.";
            }
          }
          else
          {
            delete result;
            result = NULL;
          }
        }
        else if (m_database[strName]->credentials["Type"] == "mssql")
        {
          result = new list<map<string, string> >;
          if (junction()->mssqlQuery(m_database[strName]->credentials["User"], m_database[strName]->credentials["Password"], ((m_database[strName]->credentials.find("ServerRead") != m_database[strName]->credentials.end())?m_database[strName]->credentials["ServerRead"]:m_database[strName]->credentials["Server"]), strQuery, *result, strError))
          {
            ullRows = result->size();
            if (result->empty())
            {
              strError = "No rows returned.";
            }
          }
          else
          {
            delete result;
            result = NULL;
          }
        }
        else if (m_database[strName]->credentials["Type"] == "mysql")
        {
          unsigned long long ullID = 0;
          result = new list<map<string, string> >;
          if (m_pMysql != NULL && (*m_pMysql)("query", strName, strQuery, result, ullID, ullRows, strError))
          {
            if (result->empty())
            {
              strError = "No rows returned.";
            }
          }
          else if (radial()->mysqlQuery(m_database[strName]->credentials["User"], m_database[strName]->credentials["Password"], ((m_database[strName]->credentials.find("ServerRead") != m_database[strName]->credentials.end())?m_database[strName]->credentials["ServerRead"]:m_database[strName]->credentials["Server"]), m_database[strName]->credentials["Database"], strQuery, *result, strError))
          {
            ullRows = result->size();
            if (result->empty())
            {
              strError = "No rows returned.";
            }
          }
          else if (junction()->mysqlQuery(m_database[strName]->credentials["User"], m_database[strName]->credentials["Password"], ((m_database[strName]->credentials.find("ServerRead") != m_database[strName]->credentials.end())?m_database[strName]->credentials["ServerRead"]:m_database[strName]->credentials["Server"]), m_database[strName]->credentials["Database"], strQuery, *result, strError))
          {
            ullRows = result->size();
            if (result->empty())
            {
              strError = "No rows returned.";
            }
          }
          else
          {
            delete result;
            result = NULL;
          }
        }
        else if (m_database[strName]->credentials["Type"] == "oracle")
        {
          result = new list<map<string, string> >;
          if (junction()->oracleQuery(m_database[strName]->credentials["Schema"], m_database[strName]->credentials["Password"], m_database[strName]->credentials["tnsName"], strQuery, *result, strError))
          {
            ullRows = result->size();
            if (result->empty())
            {
              strError = "No rows returned.";
            }
          }
          else
          {
            delete result;
            result = NULL;
          }
        }
        else if (m_database[strName]->credentials["Type"] == "teradata")
        {
          result = new list<map<string, string> >;
          if (junction()->teradataQuery(m_database[strName]->credentials["User"], m_database[strName]->credentials["Password"], m_database[strName]->credentials["URL"], strQuery, *result, strError))
          {
            ullRows = result->size();
            if (result->empty())
            {
              strError = "No rows returned.";
            }
          }
          else
          {
            delete result;
            result = NULL;
          }
        }
        else
        {
          strError = "Invalid database Type.";
        }
        if (m_database[strName]->credentials.find("Mutex") != m_database[strName]->credentials.end() && m_database[strName]->credentials["Mutex"] == "yes")
        {
          m_database[strName]->access.unlock();
        }
      }
      else
      {
        strError = (string)"Failed to find " + strName + (string)" in list of databases.";
      }

      return result;
    }
    // }}}
    // {{{ radial()
    Radial *Central::radial()
    {
      return m_pRadial;
    }
    // }}}
    // {{{ removeDatabase()
    void Central::removeDatabase(const string strName)
    {
      if (m_database.find(strName) != m_database.end())
      {
        m_database[strName]->credentials.clear();
        delete m_database[strName];
        m_database.erase(strName);
      }
    }
    // }}}
    // {{{ request()
    bool Central::request(const string strFunction, const string strJwt, const string strSessionID, Json *ptRequest, Json *ptResponse, string &strError)
    {
      return request(m_strUser, m_strPassword, m_strServer, m_strDatabase, strFunction, strJwt, strSessionID, ptRequest, ptResponse, strError);
    }
    bool Central::request(const string strUser, const string strPassword, const string strServer, const string strDatabase, const string strFunction, const string strJwt, const string strSessionID, Json *ptRequest, Json *ptResponse, string &strError)
    {
      bool bResult = false;
      list<string> in, out;
      string strJson;
      Json *ptJson = new Json;

      ptJson->insert("Service", "central");
      ptJson->insert("User", strUser);
      ptJson->insert("Password", strPassword);
      ptJson->insert("Server", strServer);
      ptJson->insert("Database", strDatabase);
      ptJson->insert("Function", strFunction);
      if (!strJwt.empty())
      {
        ptJson->insert("Jwt", strJwt);
      }
      if (!strSessionID.empty())
      {
        ptJson->insert("SessionID", strSessionID);
      }
      ptJson->insert("reqApp", m_strApplication);
      in.push_back(ptJson->json(strJson));
      delete ptJson;
      in.push_back(ptRequest->json(strJson));
      if (junction()->request(in, out, strError))
      {
        if (out.size() >= 1)
        {
          Json *ptStatus = new Json(out.front());
          if (ptStatus->m.find("Status") != ptStatus->m.end() && ptStatus->m["Status"]->v == "okay")
          {
            bResult = true;
            if (out.size() == 2)
            {
              ptJson = new Json(out.back());
              ptResponse->merge(ptJson, true, false);
              delete ptJson;
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
          strError = "Failed to receive a response.";
        }
      }
      in.clear();
      out.clear();

      return bResult;
    }
    // }}}
    // {{{ setApplication()
    void Central::setApplication(const string strApplication)
    {
      m_strApplication = strApplication;
      acorn()->setApplication(m_strApplication);
      junction()->setApplication(m_strApplication);
    }
    // }}}
    // {{{ setAuth()
    void Central::setAuth(const string strUser, const string strPassword, const string strServer, const string strDatabase)
    {
      m_strUser = strUser;
      m_strPassword = strPassword;
      m_strServer = strServer;
      m_strDatabase = strDatabase;
    }
    // }}}
    // {{{ setEmail()
    void Central::setEmail(const string strEmail)
    {
      m_strEmail = strEmail;
    }
    // }}}
    // {{{ setLog()
    void Central::setLog(const string strLog, const string strPrefix, const string strFrequency, const bool bLock, const bool bTimestamp)
    {
      m_strLog = strLog;
      m_strPrefix = strPrefix;
      m_strFrequency = strFrequency;
      m_bLock = bLock;
      m_bTimestamp = bTimestamp;
    }
    // }}}
    // {{{ setMysql()
    void Central::setMysql(bool (*pMysql)(const string, const string, const string, list<map<string, string> > *, unsigned long long &, unsigned long long &, string &))
    {
      m_pMysql = pMysql;
    }
    // }}}
    // {{{ setRoom()
    void Central::setRoom(const string strRoom)
    {
      m_strRoom = strRoom;
    }
    // }}}
    // {{{ update()
    bool Central::update(const string strName, const string strUpdate, string &strError)
    {
      unsigned long long ullID, ullRows;

      return update(strName, strUpdate, ullID, ullRows, strError);
    }
    bool Central::update(const string strName, const string strUpdate, string &strID, string &strError)
    {
      bool bResult = false;
      unsigned long long ullID, ullRows;

      if (update(strName, strUpdate, ullID, ullRows, strError))
      {
        stringstream ssID;
        bResult = true;
        ssID << ullID;
        strID = ssID.str();
      }

      return bResult;
    }
    bool Central::update(const string strName, const string strUpdate, unsigned long long &ullID, unsigned long long &ullRows, string &strError)
    {
      bool bResult = false;

      ullID = ullRows = 0;
      if (m_database.find(strName) != m_database.end())
      {
        if (m_database[strName]->credentials.find("Mutex") != m_database[strName]->credentials.end() && m_database[strName]->credentials["Mutex"] == "yes")
        {
          m_database[strName]->access.lock();
        }
        if (m_database[strName]->credentials["Type"] == "fusion")
        {
          if (junction()->fusionUpdate(m_database[strName]->credentials["User"], m_database[strName]->credentials["Password"], m_database[strName]->credentials["Auth"], strUpdate, strError))
          {
            bResult = true;
          }
        }
        else if (m_database[strName]->credentials["Type"] == "mssql")
        {
          if (junction()->mssqlUpdate(m_database[strName]->credentials["User"], m_database[strName]->credentials["Password"], m_database[strName]->credentials["Server"], strUpdate, strError))
          {
            bResult = true;
          }
        }
        else if (m_database[strName]->credentials["Type"] == "mysql")
        {
          string strID, strRows;
          if (m_pMysql != NULL && (*m_pMysql)("update", strName, strUpdate, NULL, ullID, ullRows, strError))
          {
            bResult = true;
          }
          else if (radial()->mysqlUpdate(m_database[strName]->credentials["User"], m_database[strName]->credentials["Password"], m_database[strName]->credentials["Server"], m_database[strName]->credentials["Database"], strUpdate, strID, strRows, strError))
          {
            stringstream ssID(strID), ssRows(strRows);
            bResult = true;
            ssID >> ullID;
            ssRows >> ullRows;
          }
          else if (junction()->mysqlUpdate(m_database[strName]->credentials["User"], m_database[strName]->credentials["Password"], m_database[strName]->credentials["Server"], m_database[strName]->credentials["Database"], strUpdate, strID, strRows, strError))
          {
            stringstream ssID(strID), ssRows(strRows);
            bResult = true;
            ssID >> ullID;
            ssRows >> ullRows;
          }
        }
        else if (m_database[strName]->credentials["Type"] == "oracle")
        {
          string strID, strRows;
          if (junction()->oracleUpdate(m_database[strName]->credentials["Schema"], m_database[strName]->credentials["Password"], m_database[strName]->credentials["tnsName"], strUpdate, strID, strRows, strError))
          {
            stringstream ssID(strID), ssRows(strRows);
            bResult = true;
            ssID >> ullID;
            ssRows >> ullRows;
          }
        }
        else if (m_database[strName]->credentials["Type"] == "teradata")
        {
          if (junction()->teradataUpdate(m_database[strName]->credentials["User"], m_database[strName]->credentials["Password"], m_database[strName]->credentials["URL"], strUpdate, strError))
          {
            bResult = true;
          }
        }
        else
        {
          strError = "Invalid database Type.";
        }
        if (m_database[strName]->credentials.find("Mutex") != m_database[strName]->credentials.end() && m_database[strName]->credentials["Mutex"] == "yes")
        {
          m_database[strName]->access.unlock();
        }
      }
      else
      {
        strError = (string)"Failed to find " + strName + (string)" in list of databases.";
      }

      return bResult;
    }
    // }}}
    // {{{ utility()
    Utility *Central::utility()
    {
      return m_pUtility;
    }
    // }}}
  }
}
