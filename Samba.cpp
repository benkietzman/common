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
      m_pDirectoryItem = NULL;
    }
    // }}}
    // {{{ ~Samba()
    Samba::~Samba()
    {
      m_directoryList.clear();
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
    bool Samba::directoryExist(const string strPath)
    {
      bool bResult = false;
      smbc_dirent *pDirectoryEntry = getDirectoryEntry(strPath);

      if (pDirectoryEntry != NULL && pDirectoryEntry->smbc_type == SMBC_DIR)
      {
        bResult = true;
      }

      return bResult;
    }
    // }}}
    // {{{ directoryList()
    list<string> Samba::directoryList(const string strPath)
    {
      bool bDone = false;
      int nDirectory = smbc_opendir(getPath(strPath).c_str());

      m_directoryList.clear();
      while (!bDone && nDirectory >= 0)
      {
        smbc_dirent *pDirectoryItem = NULL;
        if ((pDirectoryItem = smbc_readdir(nDirectory)) != NULL)
        {
          m_directoryList.push_back(pDirectoryItem->name);
        }
        else
        {
          bDone = true;
        }
      }
      if (nDirectory >= 0)
      {
        smbc_closedir(nDirectory);
      }
      m_directoryList.sort();
      m_directoryIterator = m_directoryList.begin();

      return m_directoryList;
    }
    // }}}
    // {{{ establishDirectory()
    bool Samba::establishDirectory(const string strPath)
    {
      bool bResult = true;
      size_t unIndex = 0;

      if (strPath[0] == '/')
      {
        unIndex = 1;
      }
      for (size_t nPosition = unIndex; bResult && nPosition != string::npos; nPosition = strPath.find("/", nPosition))
      {
        if (!directoryExist(getPath(strPath.substr(0, nPosition))) && !makeDirectory(getPath(strPath.substr(0, nPosition))))
        {
          bResult = false;
        }
      }
      if (!directoryExist(getPath(strPath)) && !makeDirectory(getPath(strPath)))
      {
        bResult = false;
      }

      return bResult;
    }
    // }}}
    // {{{ fetchDirectoryItem()
    bool Samba::fetchDirectoryItem(string &strItem)
    {
      bool bResult = true;

      if (m_directoryIterator != m_directoryList.end())
      {
        strItem = *m_directoryIterator;
        m_directoryIterator++;
      }
      else
      {
        bResult = false;
      }

      return bResult;
    }
    // }}}
    // {{{ fileAccessDate()
    bool Samba::fileAccessDate(const string strPath, int &nYear, int &nMonth, int &nDay, int &nHour, int &nMinute, int &nSecond)
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

      return bResult;
    }
    // }}}
    // {{{ fileChangeDate()
    bool Samba::fileChangeDate(const string strPath, int &nYear, int &nMonth, int &nDay, int &nHour, int &nMinute, int &nSecond)
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

      return bResult;
    }
    // }}}
    // {{{ fileEmpty()
    bool Samba::fileEmpty(const string strPath)
    {
      bool bResult = true;

      if (fileSize(strPath))
      {
        bResult = false;
      }

      return bResult;
    }
    // }}}
    // {{{ fileExist()
    bool Samba::fileExist(const string strPath)
    {
      bool bResult = false;
      smbc_dirent *pDirectoryEntry = getDirectoryEntry(strPath);

      if (pDirectoryEntry != NULL && pDirectoryEntry->smbc_type == SMBC_FILE)
      {
        bResult = true;
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
    long Samba::fileSize(const string strPath)
    {
      long lSize = 0;
      struct stat tStat;

      if (smbc_stat(getPath(strPath).c_str(), &tStat) == 0)
      {
        lSize = tStat.st_size;
      }

      return lSize;
    }
    // }}}
    // {{{ get()
    bool Samba::get(const string strRemotePath, const string strLocalPath)
    {
      bool bResult = true;
      int nInFile = smbc_open(getPath(strRemotePath).c_str(), O_RDONLY, 0666);
      ofstream outFile(strLocalPath.c_str(), ios::out|ios::binary);

      if (nInFile >= 0 && outFile)
      {
        long lSize = 0;
        char szBuffer[4096] = "\0";
        while ((lSize = smbc_read(nInFile, &szBuffer, 4096)) > 0)
        {
          outFile.write(szBuffer, lSize);
        }
        smbc_close(nInFile);
      }
      else
      {
        bResult = false;
      }
      outFile.close();

      return bResult;
    }
    bool Samba::get(const string strRemotePath, stringstream &ssLocalData)
    {
      bool bResult = true;
      int nInFile = smbc_open(getPath(strRemotePath).c_str(), O_RDONLY, 0666);

      if (nInFile >= 0)
      {
        long lSize = 0;
        char szBuffer[4096] = "\0";
        while ((lSize = smbc_read(nInFile, &szBuffer, 4096)) > 0)
        {
          ssLocalData.write(szBuffer, lSize);
        }
        smbc_close(nInFile);
      }
      else
      {
        bResult = false;
      }

      return bResult;
    }
    // }}}
    // {{{ getDirectoryEntry()
    smbc_dirent *Samba::getDirectoryEntry(const string strPath)
    {
      bool bDone = false;
      size_t nPosition = 0;
      int nDirectory = 0;
      string strDirectory;
      string strFilename;

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
      nDirectory = smbc_opendir(getPath(strDirectory).c_str());
      while (!bDone && nDirectory >= 0)
      {
        if ((m_pDirectoryItem = smbc_readdir(nDirectory)) == NULL || (string)m_pDirectoryItem->name == strFilename)
        {
          bDone = true;
        }
      }
      if (nDirectory >= 0)
      {
        smbc_closedir(nDirectory);
      }

      return m_pDirectoryItem;
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
    bool Samba::init(const string strServer, const string strShare, const string strWorkgroup, const string strUsername, const string strPassword)
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

      return bResult;
    }
    // }}}
    // {{{ makeDirectory()
    bool Samba::makeDirectory(const string strPath)
    {
      bool bResult = false;
      mode_t mode = 00777;

      if (smbc_mkdir(getPath(strPath).c_str(), mode) == 0)
      {
        bResult = true;
      }

      return bResult;
    }
    // }}}
    // {{{ put()
    bool Samba::put(const string strLocalPath, const string strRemotePath)
    {
      bool bResult = true;
      int nOutFile = smbc_open(getPath(strRemotePath).c_str(), O_CREAT|O_WRONLY, 0666);
      ifstream inFile(strLocalPath.c_str(), ios::in|ios::binary);

      if (nOutFile >= 0 && inFile)
      {
        char cBuffer = '\0';
        while (inFile.read(&cBuffer, 1))
        {
          smbc_write(nOutFile, &cBuffer, 1);
        }
        smbc_close(nOutFile);
      }
      else
      {
        bResult = false;
      }
      inFile.close();

      return bResult;
    }
    bool Samba::put(stringstream &ssLocalData, const string strRemotePath)
    {
      bool bResult = true;
      int nOutFile = smbc_open(getPath(strRemotePath).c_str(), O_CREAT|O_WRONLY, 0666);

      if (nOutFile >= 0)
      {
        char cBuffer = '\0';
        while (ssLocalData.read(&cBuffer, 1))
        {
          smbc_write(nOutFile, &cBuffer, 1);
        }
        smbc_close(nOutFile);
      }
      else
      {
        bResult = false;
      }

      return bResult;
    }
    // }}}
    // {{{ remove()
    bool Samba::remove(const string strPath)
    {
      bool bResult = false;

      if (smbc_unlink(getPath(strPath).c_str()) == 0)
      {
        bResult = true;
      }

      return bResult;
    }
    // }}}
    // {{{ removeDirectory()
    bool Samba::removeDirectory(const string strPath)
    {
      bool bResult = true;

      if (!directoryList(strPath).empty())
      {
        string strItem;
        list<string> directoryList;
        while (fetchDirectoryItem(strItem))
        {
          if (strItem != "." && strItem != "..")
          {
            if (directoryExist(strPath + (string)"/" + strItem))
            {
              directoryList.push_back(strItem);
            }
            else if (!remove(strPath + (string)"/" + strItem))
            {
              bResult = false;
            }
          }
        }
        for (list<string>::iterator i = directoryList.begin(); i != directoryList.end(); i++)
        {
          if (!removeDirectory(strPath + (string)"/" + *i))
          {
            bResult = false;
          }
        }
        directoryList.clear();
        if (bResult && !remove(strPath))
        {
          bResult = false;
        }
      }
      else
      {
        bResult = false;
      }

      return bResult;
    }
    // }}}
    // {{{ rename()
    bool Samba::rename(const string strOldPath, const string strNewPath)
    {
      bool bResult = false;

      if (smbc_rename(getPath(strOldPath).c_str(), getPath(strNewPath).c_str()) == 0)
      {
        bResult = true;
      }

      return bResult;
    }
    // }}}
  }
}
