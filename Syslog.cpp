/* -*- c++ -*- */
///////////////////////////////////////////
// Syslog
// -------------------------------------
// file       : Syslog.cpp
// author     : Ben Kietzman
// begin      : 2020-06-01
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
/*! \file Syslog.cpp
* \brief Syslog Class
*/
// {{{ includes
#include "Syslog"
// }}}
extern "C++"
{
  namespace common
  {
    // {{{ Syslog()
    Syslog::Syslog(const string strApplication, const string strProgram, const int nOption, const int nFacility)
    {
      passwd *pPasswd = getpwuid(getuid());
      m_nFacility = nFacility;
      m_nOption = nOption;
      m_strApplication = strApplication;
      m_strProgram = strProgram;
      m_strUser = "N/A";
      if (pPasswd != NULL)
      {
        m_strUser = pPasswd->pw_name;
      }
      openlog(m_strProgram.c_str(), m_nOption, m_nFacility);
    }
    // }}}
    // {{{ ~Syslog()
    Syslog::~Syslog()
    {
      closelog();
    }
    // }}}
    // {{{ commandEnded()
    void Syslog::commandEnded(const string strCommandName, const string strEventDetails, const int nPriority)
    {
      Json *ptMessage = new Json;

      ptMessage->insert("Event Type", "Command Ended");
      ptMessage->insert("Command Name", strCommandName);
      ptMessage->insert("Login ID", m_strUser);
      log(ptMessage, strEventDetails, nPriority);
      delete ptMessage;
    }
    // }}}
    // {{{ commandLaunched()
    void Syslog::commandLaunched(const string strCommandName, const string strEventDetails, const int nPriority)
    {
      Json *ptMessage = new Json;

      ptMessage->insert("Event Type", "Command Launched");
      ptMessage->insert("Command Name", strCommandName);
      ptMessage->insert("Login ID", m_strUser);
      log(ptMessage, strEventDetails, nPriority);
      delete ptMessage;
    }
    // }}}
    // {{{ connectionStarted()
    void Syslog::connectionStarted(const string strEventDetails, const int fdSocket, const bool bResult, const int nPriority)
    {
      Json *ptMessage = new Json;

      ptMessage->insert("Event Type", "Connection Started");
      ptMessage->insert("Action", ((bResult)?"Allowed":"Blocked"));
      if (fdSocket != -1)
      {
        sockaddr_storage addr;
        socklen_t len = sizeof(sockaddr_storage);
        if (getpeername(fdSocket, (sockaddr *)&addr, &len) == 0)
        {
          char szIP[INET6_ADDRSTRLEN];
          if (addr.ss_family == AF_INET)
          {
            sockaddr_in *s = (sockaddr_in *)&addr;
            inet_ntop(AF_INET, &s->sin_addr, szIP, sizeof(szIP));
          }
          else if (addr.ss_family == AF_INET6)
          {
            sockaddr_in6 *s = (sockaddr_in6 *)&addr;
            inet_ntop(AF_INET6, &s->sin6_addr, szIP, sizeof(szIP));
          }
          ptMessage->insert("Source IP", szIP);
        }
      }
      log(ptMessage, strEventDetails, nPriority);
      delete ptMessage;
    }
    // }}}
    // {{{ connectionStartedCommandLaunched()
    void Syslog::connectionStartedCommandLaunched(const string strCommandName, const string strEventDetails, const int fdSocket, const bool bResult, const int nPriority)
    {
      Json *ptMessage = new Json;

      ptMessage->insert("Event Type", "Connection Started & Command Launched");
      ptMessage->insert("Action", ((bResult)?"Allowed":"Blocked"));
      ptMessage->insert("Command Name", strCommandName);
      ptMessage->insert("Login ID", m_strUser);
      if (fdSocket != -1)
      {
        sockaddr_storage addr;
        socklen_t len = sizeof(sockaddr_storage);
        if (getpeername(fdSocket, (sockaddr *)&addr, &len) == 0)
        {
          char szIP[INET6_ADDRSTRLEN];
          if (addr.ss_family == AF_INET)
          {
            sockaddr_in *s = (sockaddr_in *)&addr;
            inet_ntop(AF_INET, &s->sin_addr, szIP, sizeof(szIP));
          }
          else if (addr.ss_family == AF_INET6)
          {
            sockaddr_in6 *s = (sockaddr_in6 *)&addr;
            inet_ntop(AF_INET6, &s->sin6_addr, szIP, sizeof(szIP));
          }
          ptMessage->insert("Source IP", szIP);
        }
      }
      log(ptMessage, strEventDetails, nPriority);
      delete ptMessage;
    }
    // }}}
    // {{{ connectionStopped()
    void Syslog::connectionStopped(const string strEventDetails, const int fdSocket, const int nPriority)
    {
      Json *ptMessage = new Json;

      ptMessage->insert("Event Type", "Connection Stopped");
      ptMessage->insert("Action", "Teardown");
      if (fdSocket != -1)
      {
        sockaddr_storage addr;
        socklen_t len = sizeof(sockaddr_storage);
        if (getpeername(fdSocket, (sockaddr *)&addr, &len) == 0)
        {
          char szIP[INET6_ADDRSTRLEN];
          if (addr.ss_family == AF_INET)
          {
            sockaddr_in *s = (sockaddr_in *)&addr;
            inet_ntop(AF_INET, &s->sin_addr, szIP, sizeof(szIP));
          }
          else if (addr.ss_family == AF_INET6)
          {
            sockaddr_in6 *s = (sockaddr_in6 *)&addr;
            inet_ntop(AF_INET6, &s->sin6_addr, szIP, sizeof(szIP));
          }
          ptMessage->insert("Source IP", szIP);
        }
      }
      log(ptMessage, strEventDetails, nPriority);
      delete ptMessage;
    }
    // }}}
    // {{{ groupCreated()
    void Syslog::groupCreated(const string strEventDetails, const string strLoginID, const bool bResult, const string strProtocol, const string strService, const int nPriority)
    {
      Json *ptMessage = new Json;

      ptMessage->insert("Event Type", "Group Created");
      ptMessage->insert("Login ID", ((!strLoginID.empty())?strLoginID:"N/A"));
      ptMessage->insert("Result", ((bResult)?"Success":"Failure"));
      ptMessage->insert("Protocol", ((!strProtocol.empty())?strProtocol:"N/A"));
      ptMessage->insert("Service", ((!strService.empty())?strService:"N/A"));
      log(ptMessage, strEventDetails, nPriority);
      delete ptMessage;
    }
    // }}}
    // {{{ groupDeleted()
    void Syslog::groupDeleted(const string strEventDetails, const string strLoginID, const bool bResult, const string strProtocol, const string strService, const int nPriority)
    {
      Json *ptMessage = new Json;

      ptMessage->insert("Event Type", "Group Deleted");
      ptMessage->insert("Login ID", ((!strLoginID.empty())?strLoginID:"N/A"));
      ptMessage->insert("Result", ((bResult)?"Success":"Failure"));
      ptMessage->insert("Protocol", ((!strProtocol.empty())?strProtocol:"N/A"));
      ptMessage->insert("Service", ((!strService.empty())?strService:"N/A"));
      log(ptMessage, strEventDetails, nPriority);
      delete ptMessage;
    }
    // }}}
    // {{{ groupModified()
    void Syslog::groupModified(const string strEventDetails, const string strLoginID, const bool bResult, const string strProtocol, const string strService, const int nPriority)
    {
      Json *ptMessage = new Json;

      ptMessage->insert("Event Type", "Group Modified");
      ptMessage->insert("Login ID", ((!strLoginID.empty())?strLoginID:"N/A"));
      ptMessage->insert("Result", ((bResult)?"Success":"Failure"));
      ptMessage->insert("Protocol", ((!strProtocol.empty())?strProtocol:"N/A"));
      ptMessage->insert("Service", ((!strService.empty())?strService:"N/A"));
      log(ptMessage, strEventDetails, nPriority);
      delete ptMessage;
    }
    // }}}
    // {{{ log()
    void Syslog::log(const string strMessage, const int nPriority)
    {
      syslog(nPriority, "%s", strMessage.c_str());
    }
    void Syslog::log(Json *ptMessage, const string strEventDetails, const int nPriority)
    {
      char szBuffer[20] = "\0", szDateTime[37] = "\0";
      int nMillisecond = 0;
      struct timeval tv;
      struct tm tTime;
      string strMessage;
      stringstream ssTimeStamp;

      if (!m_strApplication.empty())
      {
        ptMessage->insert("Application", m_strApplication);
      } 
      if (!m_strProgram.empty())
      {
        ptMessage->insert("Program", m_strProgram);
      }
      if (!strEventDetails.empty())
      {
        ptMessage->insert("Event Details", strEventDetails);
      }
      gettimeofday(&tv, NULL);
      #ifdef COMMON_LINUX
      nMillisecond = lrint(tv.tv_usec/1000.0);
      #endif
      if (nMillisecond >= 1000)
      {
        nMillisecond -= 1000;
        tv.tv_sec++;
      }
      gmtime_r(&tv.tv_sec, &tTime);
      strftime(szBuffer, 20, "%Y-%m-%d %H:%M:%S", &tTime);
      sprintf(szDateTime, "%s.%03d UTC", szBuffer, nMillisecond);
      ptMessage->insert("DateTime", szDateTime);
      ssTimeStamp << tv.tv_sec;
      ptMessage->insert("TimeStamp", ssTimeStamp.str());
      ptMessage->json(strMessage);
      log(strMessage, nPriority);
    }
    // }}}
    // {{{ logoff()
    void Syslog::logoff(const string strEventDetails, const string strLoginID, const bool bResult, const string strProtocol, const string strService, const int nPriority)
    {
      Json *ptMessage = new Json;

      ptMessage->insert("Event Type", "Logoff");
      ptMessage->insert("Login ID", ((!strLoginID.empty())?strLoginID:"N/A"));
      ptMessage->insert("Result", ((bResult)?"Success":"Failure"));
      ptMessage->insert("Protocol", ((!strProtocol.empty())?strProtocol:"N/A"));
      ptMessage->insert("Service", ((!strService.empty())?strService:"N/A"));
      log(ptMessage, strEventDetails, nPriority);
      delete ptMessage;
    }
    // }}}
    // {{{ logon()
    void Syslog::logon(const string strEventDetails, const string strLoginID, const bool bResult, const string strProtocol, const string strService, const int nPriority)
    {
      Json *ptMessage = new Json;

      ptMessage->insert("Event Type", "Logon");
      ptMessage->insert("Login ID", ((!strLoginID.empty())?strLoginID:"N/A"));
      ptMessage->insert("Result", ((bResult)?"Success":"Failure"));
      ptMessage->insert("Protocol", ((!strProtocol.empty())?strProtocol:"N/A"));
      ptMessage->insert("Service", ((!strService.empty())?strService:"N/A"));
      log(ptMessage, strEventDetails, nPriority);
      delete ptMessage;
    }
    // }}}
    // {{{ privilegeLevelChanged()
    void Syslog::privilegeLevelChanged(const string strEventDetails, const string strLoginID, const bool bResult, const string strProtocol, const string strService, const int nPriority)
    {
      Json *ptMessage = new Json;

      ptMessage->insert("Event Type", "Privilege Level Changed");
      ptMessage->insert("Login ID", ((!strLoginID.empty())?strLoginID:"N/A"));
      ptMessage->insert("Result", ((bResult)?"Success":"Failure"));
      ptMessage->insert("Protocol", ((!strProtocol.empty())?strProtocol:"N/A"));
      ptMessage->insert("Service", ((!strService.empty())?strService:"N/A"));
      log(ptMessage, strEventDetails, nPriority);
      delete ptMessage;
    }
    // }}}
    // {{{ userAccountCreated()
    void Syslog::userAccountCreated(const string strEventDetails, const string strLoginID, const bool bResult, const string strProtocol, const string strService, const int nPriority)
    {
      Json *ptMessage = new Json;

      ptMessage->insert("Event Type", "User Account Created");
      ptMessage->insert("Login ID", ((!strLoginID.empty())?strLoginID:"N/A"));
      ptMessage->insert("Result", ((bResult)?"Success":"Failure"));
      ptMessage->insert("Protocol", ((!strProtocol.empty())?strProtocol:"N/A"));
      ptMessage->insert("Service", ((!strService.empty())?strService:"N/A"));
      log(ptMessage, strEventDetails, nPriority);
      delete ptMessage;
    }
    // }}}
    // {{{ userAccountDeleted()
    void Syslog::userAccountDeleted(const string strEventDetails, const string strLoginID, const bool bResult, const string strProtocol, const string strService, const int nPriority)
    {
      Json *ptMessage = new Json;

      ptMessage->insert("Event Type", "User Account Deleted");
      ptMessage->insert("Login ID", ((!strLoginID.empty())?strLoginID:"N/A"));
      ptMessage->insert("Result", ((bResult)?"Success":"Failure"));
      ptMessage->insert("Protocol", ((!strProtocol.empty())?strProtocol:"N/A"));
      ptMessage->insert("Service", ((!strService.empty())?strService:"N/A"));
      log(ptMessage, strEventDetails, nPriority);
      delete ptMessage;
    }
    // }}}
    // {{{ userAccountModified()
    void Syslog::userAccountModified(const string strEventDetails, const string strLoginID, const bool bResult, const string strProtocol, const string strService, const int nPriority)
    {
      Json *ptMessage = new Json;

      ptMessage->insert("Event Type", "User Account Modified");
      ptMessage->insert("Login ID", ((!strLoginID.empty())?strLoginID:"N/A"));
      ptMessage->insert("Result", ((bResult)?"Success":"Failure"));
      ptMessage->insert("Protocol", ((!strProtocol.empty())?strProtocol:"N/A"));
      ptMessage->insert("Service", ((!strService.empty())?strService:"N/A"));
      log(ptMessage, strEventDetails, nPriority);
      delete ptMessage;
    }
    // }}}
  }
}
