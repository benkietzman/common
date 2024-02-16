// vim: syntax=cpp
// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Samba
// -------------------------------------
// file       : Samba.cpp
// author     : Ben Kietzman
// begin      : 2010-03-03
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

/*! \file Samba.cpp
* \brief Samba Class
*/
// {{{ includes
#include "Samba"
// }}}
extern "C++"
{
  namespace common
  {
    // {{{ Samba()
    Samba::Samba()
    {
      char szPid[32] = "\0";

      m_nPid = getpid();
      sprintf(szPid, "%ld", (long int)m_nPid);
      m_strPid = szPid;
    }
    // }}}
    // {{{ ~Samba()
    Samba::~Samba()
    {
    }
    // }}}
    // {{{ authenticate()
    void Samba::authenticate(const char *pszServer, const char *pszShare, char *pszWorkgroup, int nWGLength, char *pszUsername, int nUNLength, char *pszPassword, int nPWLength)
    {
      strcpy(pszWorkgroup, smb_gstrWorkgroup.c_str());
      nWGLength = smb_gstrWorkgroup.size();
      strcpy(pszUsername, smb_gstrUsername.c_str());
      nUNLength = smb_gstrUsername.size();
      strcpy(pszPassword, smb_gstrPassword.c_str());
      nPWLength = smb_gstrPassword.size();
    }
    // }}}
    // {{{ directoryExist()
    bool Samba::directoryExist(const string strPath, string &strError)
    {
      bool bResult = false;
      smbc_dirent *pDirectoryEntry = getDirectoryEntry(strPath, strError);

      bExist = false;
      if (pDirectoryEntry != NULL)
      {
        bResult = true;
        if (pDirectoryEntry->smbc_type == SMBC_DIR)
        {
          bExist = true;
        }
        else
        {
          strError = "Not a directory.";
        }
      }

      return bResult;
    }
    // }}}
    // {{{ directoryList()
    bool Samba::directoryList(const string strPath, list<string> &dirList, string &strError)
    {
      bool bResult = false;
      int nDirectory = smbc_opendir(getPath(strPath).c_str());

      dirList.clear();
      if (nDirectory >= 0)
      {
        bool bDone = false;
        while (!bDone && nDirectory >= 0)
        {
          smbc_dirent *pDirectoryItem = NULL;
          if ((pDirectoryItem = smbc_readdir(nDirectory)) != NULL)
          {
            dirList.push_back(pDirectoryItem->name);
          }
          else
          {
            bDone = true;
          }
        }
        smbc_closedir(nDirectory);
        dirList.sort();
      }
      else
      {
        strError = strerror(errno);
      }

      return bResult;
    }
    // }}}
    // {{{ establishDirectory()
    bool Samba::establishDirectory(const string strPath, string &strError)
    {
      bool bResult = true;
      size_t unIndex = 0;

      if (strPath[0] == '/')
      {
        unIndex = 1;
      }
      for (size_t nPosition = unIndex; bResult && nPosition != string::npos; nPosition = strPath.find("/", nPosition))
      {
        if (!directoryExist(getPath(strPath.substr(0, nPosition)), strError) && !makeDirectory(getPath(strPath.substr(0, nPosition)), strError))
        {
          bResult = false;
        }
      }
      if (!directoryExist(getPath(strPath), strError) && !makeDirectory(getPath(strPath), strError))
      {
        bResult = false;
      }

      return bResult;
    }
    // }}}
    // {{{ fileAccessDate()
    bool Samba::fileAccessDate(const string strPath, int &nYear, int &nMonth, int &nDay, int &nHour, int &nMinute, int &nSecond, string &strError)
    {
      bool bResult = false;
      struct stat tStat;
      struct tm *ptDateTime;

      if (smbc_stat(getPath(strPath).c_str(), &tStat) == 0)
      {
        bResult = true;
        ptDateTime = localtime(&tStat.st_atime);
        nYear = ptDateTime->tm_year + 1900;
        nMonth = ptDateTime->tm_mon + 1;
        nDay = ptDateTime->tm_mday;
        nHour = ptDateTime->tm_hour;
        nMinute = ptDateTime->tm_min;
        nSecond = ptDateTime->tm_sec;
      }
      else
      {
        strError = strerror(errno);
      }

      return bResult;
    }
    // }}}
    // {{{ fileChangeDate()
    bool Samba::fileChangeDate(const string strPath, int &nYear, int &nMonth, int &nDay, int &nHour, int &nMinute, int &nSecond, string &strError)
    {
      bool bResult = false;
      struct stat tStat;
      struct tm *ptDateTime;

      if (smbc_stat(getPath(strPath).c_str(), &tStat) == 0)
      {
        bResult = true;
        ptDateTime = localtime(&tStat.st_ctime);
        nYear = ptDateTime->tm_year + 1900;
        nMonth = ptDateTime->tm_mon + 1;
        nDay = ptDateTime->tm_mday;
        nHour = ptDateTime->tm_hour;
        nMinute = ptDateTime->tm_min;
        nSecond = ptDateTime->tm_sec;
      }
      else
      {
        strError = strerror(errno);
      }

      return bResult;
    }
    // }}}
    // {{{ fileEmpty()
    bool Samba::fileEmpty(const string strPath, bool &bEmpty, string &strError)
    {
      bool bResult = true;
      long lSize;

      if (fileSize(strPath, lSize, strError))
      {
        bResult = false;
      }

      return bResult;
    }
    // }}}
    // {{{ fileExist()
    bool Samba::fileExist(const string strPath, bool &bExist, string &strError)
    {
      bool bResult = false;
      smbc_dirent *pDirectoryEntry = getDirectoryEntry(strPath, strError);

      bExist = false;
      if (pDirectoryEntry != NULL)
      {
        bResult = true;
        if (pDirectoryEntry->smbc_type == SMBC_FILE)
        {
          bExist = true;
        }
        else
        {
          strError = "Not a file.";
        }
      }

      return bResult;
    }
    // }}}
    // {{{ fileModifyDate()
    bool Samba::fileModifyDate(const string strPath, int &nYear, int &nMonth, int &nDay, int &nHour, int &nMinute, int &nSecond)
    {
      bool bResult = false;
      struct stat tStat;
      struct tm *ptDateTime;

      if (smbc_stat(getPath(strPath).c_str(), &tStat) == 0)
      {
        bResult = true;
        ptDateTime = localtime(&tStat.st_mtime);
        nYear = ptDateTime->tm_year + 1900;
        nMonth = ptDateTime->tm_mon + 1;
        nDay = ptDateTime->tm_mday;
        nHour = ptDateTime->tm_hour;
        nMinute = ptDateTime->tm_min;
        nSecond = ptDateTime->tm_sec;
      }

      return bResult;
    }
    // }}}
    // {{{ fileSize()
    bool Samba::fileSize(const string strPath, long &lSize, string &strError)
    {
      bool bResult = false;
      struct stat tStat;

      if (smbc_stat(getPath(strPath).c_str(), &tStat) == 0)
      {
        bResult = true;
        lSize = tStat.st_size;
      }
      else
      {
        strError = strerror(errno);
      }

      return bResult;
    }
    // }}}
    // {{{ get()
    bool Samba::get(const string strRemotePath, const string strLocalPath, string &strError)
    {
      bool bResult = false;
      int nInFile = smbc_open(getPath(strRemotePath).c_str(), O_RDONLY, 0666);

      if (nInFile >= 0)
      {
        ofstream outFile(strLocalPath.c_str(), ios::out|ios::binary);
        if (outFile)
        {
          long lSize = 0;
          char szBuffer[4096] = "\0";
          bResult = true;
          while ((lSize = smbc_read(nInFile, &szBuffer, 4096)) > 0)
          {
            outFile.write(szBuffer, lSize);
          }
        }
        else
        {
          strError = strerror(errno);
        }
        outFile.close();
        smbc_close(nInFile);
      }
      else
      {
        strError = strerror(errno);
      }

      return bResult;
    }
    bool Samba::get(const string strRemotePath, stringstream &ssLocalData, string &strError)
    {
      bool bResult = false;
      int nInFile = smbc_open(getPath(strRemotePath).c_str(), O_RDONLY, 0666);

      if (nInFile >= 0)
      {
        long lSize = 0;
        char szBuffer[4096] = "\0";
        bResult = true;
        while ((lSize = smbc_read(nInFile, &szBuffer, 4096)) > 0)
        {
          ssLocalData.write(szBuffer, lSize);
        }
        smbc_close(nInFile);
      }
      else
      {
        strError = strerror(errno);
      }

      return bResult;
    }
    // }}}
    // {{{ getDirectoryEntry()
    smbc_dirent *Samba::getDirectoryEntry(const string strPath, string &strError)
    {
      size_t nPosition = 0;
      int nDirectory = 0;
      string strDirectory;
      string strFilename;
      smbc_dirent *pDirectoryItem = NULL;

      if ((nPosition = strPath.rfind("/", strPath.size() - 1)) == string::npos)
      {
        strFilename = strPath;
      }
      else
      {
        nPosition++;
        strDirectory = strPath.substr(0, nPosition);
        strFilename = strPath.substr(nPosition, strPath.size() - nPosition);
      }
      if ((nDirectory = smbc_opendir(getPath(strDirectory).c_str())) >= 0)
      {
        bool bDone = false;
        while (!bDone && nDirectory >= 0)
        {
          if ((pDirectoryItem = smbc_readdir(nDirectory)) == NULL || (string)pDirectoryItem->name == strFilename)
          {
            bDone = true;
          }
        }
        smbc_closedir(nDirectory);
        if (pDirectoryItem == NULL)
        {
          strError = "Not found.";
        }
      }
      else
      {
        strError = strerror(errno);
      }

      return pDirectoryItem;
    }
    // }}}
    // {{{ getPath()
    string Samba::getPath(const string strPath)
    {
      m_strPath = (string)"smb://" + m_strServer + (string)"/" + m_strShare;
      if (strPath.empty() || strPath[0] != '/')
      {
        m_strPath += "/";
      }
      m_strPath += strPath;

      return m_strPath;
    }
    // }}}
    // {{{ init()
    bool Samba::init(const string strServer, const string strShare, const string strWorkgroup, const string strUsername, const string strPassword, string &strError)
    {
      bool bResult = false;

      m_strServer = strServer;
      m_strShare = strShare;
      smb_gstrWorkgroup = strWorkgroup;
      smb_gstrUsername = strUsername;
      smb_gstrPassword = strPassword;
      if (smbc_init(Samba::authenticate, 0) == 0)
      {
        bResult = true;
      }
      else
      {
        strError = strerror(errno);
      }

      return bResult;
    }
    // }}}
    // {{{ makeDirectory()
    bool Samba::makeDirectory(const string strPath, string &strError)
    {
      bool bResult = false;
      mode_t mode = 00777;

      if (smbc_mkdir(getPath(strPath).c_str(), mode) == 0)
      {
        bResult = true;
      }
      else
      {
        strError = strerror(errno);
      }

      return bResult;
    }
    // }}}
    // {{{ put()
    bool Samba::put(const string strLocalPath, const string strRemotePath, string &strError)
    {
      bool bResult = false;
      int nOutFile = smbc_open(getPath(strRemotePath).c_str(), O_CREAT|O_WRONLY, 0666);

      if (nOutFile >= 0)
      {
        ifstream inFile(strLocalPath.c_str(), ios::in|ios::binary);
        if (inFile)
        {
          char cBuffer = '\0';
          bResult = true;
          while (inFile.read(&cBuffer, 1))
          {
            smbc_write(nOutFile, &cBuffer, 1);
          }
        }
        else
        {
          strError = strerror(errno);
        }
        inFile.close();
        smbc_close(nOutFile);
      }
      else
      {
        strError = strerror(errno);
      }

      return bResult;
    }
    bool Samba::put(stringstream &ssLocalData, const string strRemotePath, string &strError)
    {
      bool bResult = false;
      int nOutFile = smbc_open(getPath(strRemotePath).c_str(), O_CREAT|O_WRONLY, 0666);

      if (nOutFile >= 0)
      {
        char cBuffer = '\0';
        bResult = true;
        while (ssLocalData.read(&cBuffer, 1))
        {
          smbc_write(nOutFile, &cBuffer, 1);
        }
        smbc_close(nOutFile);
      }
      else
      {
        strError = strerror(errno);
      }

      return bResult;
    }
    // }}}
    // {{{ remove()
    bool Samba::remove(const string strPath, string &strError)
    {
      bool bResult = false;

      if (smbc_unlink(getPath(strPath).c_str()) == 0)
      {
        bResult = true;
      }
      else
      {
        strError = strerror(errno);
      }

      return bResult;
    }
    // }}}
    // {{{ removeDirectory()
    bool Samba::removeDirectory(const string strPath, string &strError)
    {
      bool bResult = true;
      list<string> dirList[2];

      if (directoryList(strPath, dirList[0], strError))
      {
        for (list<string>::iterator i = dirList[0].begin(); i != dirList[0].end(); i++)
        {
          if ((*i) != "." && (*i) != "..")
          {
            if (directoryExist(strPath + (string)"/" + (*i), strError))
            {
              dirList[1].push_back(*i);
            }
            else if (!remove(strPath + (string)"/" + (*i), strError))
            {
              bResult = false;
            }
          }
        }
        for (list<string>::iterator i = dirList[1].begin(); i != dirList[1].end(); i++)
        {
          if (!removeDirectory(strPath + (string)"/" + (*i), strError))
          {
            bResult = false;
          }
        }
        dirList[1].clear();
        if (bResult && !remove(strPath, strError))
        {
          bResult = false;
        }
      }
      dirList[0].clear();

      return bResult;
    }
    // }}}
    // {{{ rename()
    bool Samba::rename(const string strOldPath, const string strNewPath, string &strError)
    {
      bool bResult = false;

      if (smbc_rename(getPath(strOldPath).c_str(), getPath(strNewPath).c_str()) == 0)
      {
        bResult = true;
      }
      else
      {
        strError = strerror(errno);
      }

      return bResult;
    }
    // }}}
  }
}
