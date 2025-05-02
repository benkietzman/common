/* -*- c++ -*- */
///////////////////////////////////////////
// File
// -------------------------------------
// file       : File.cpp
// author     : Ben Kietzman
// begin      : 2010-03-03
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
/*! \file File.cpp
* \brief File Class
*/
// {{{ includes
#include "File"
// }}}
extern "C++"
{
  namespace common
  {
    // {{{ File()
    File::File()
    {
    }
    // }}}
    // {{{ ~File()
    File::~File()
    {
    }
    // }}}
    // {{{ fileExist()
    bool File::fileExist(const string strPath)
    {
      bool bResult = false;
      struct stat tStat;

      if (stat(strPath.c_str(), &tStat) == 0 && S_ISREG(tStat.st_mode))
      {
        bResult = true;
      }

      return bResult;
    }
    // }}}
    // {{{ fileEmpty()
    bool File::fileEmpty(const string strPath)
    {
      bool bResult = true;

      if (fileSize(strPath))
      {
        bResult = false;
      }

      return bResult;
    }
    // }}}
    // {{{ fileSize()
    long File::fileSize(const string strPath)
    {
      long lSize = 0;
      struct stat tStat;

      if (stat(strPath.c_str(), &tStat) == 0)
      {
        lSize = tStat.st_size;
      }

      return lSize;
    }
    // }}}
    // {{{ fileAccessDate()
    bool File::fileAccessDate(const string strPath, int &nYear, int &nMonth, int &nDay, int &nHour, int &nMinute, int &nSecond)
    {
      bool bResult = false;
      struct stat tStat;

      if (stat(strPath.c_str(), &tStat) == 0)
      {
        struct tm tTime;
        bResult = true;
        localtime_r(&tStat.st_atime, &tTime);
        nYear = tTime.tm_year + 1900;
        nMonth = tTime.tm_mon + 1;
        nDay = tTime.tm_mday;
        nHour = tTime.tm_hour;
        nMinute = tTime.tm_min;
        nSecond = tTime.tm_sec;
      }

      return bResult;
    }
    // }}}
    // {{{ fileChangeDate()
    bool File::fileChangeDate(const string strPath, int &nYear, int &nMonth, int &nDay, int &nHour, int &nMinute, int &nSecond)
    {
      bool bResult = false;
      struct stat tStat;

      if (stat(strPath.c_str(), &tStat) == 0)
      {
        struct tm tTime;
        bResult = true;
        localtime_r(&tStat.st_ctime, &tTime);
        nYear = tTime.tm_year + 1900;
        nMonth = tTime.tm_mon + 1;
        nDay = tTime.tm_mday;
        nHour = tTime.tm_hour;
        nMinute = tTime.tm_min;
        nSecond = tTime.tm_sec;
      }

      return bResult;
    }
    // }}}
    // {{{ fileModifyDate()
    bool File::fileModifyDate(const string strPath, int &nYear, int &nMonth, int &nDay, int &nHour, int &nMinute, int &nSecond)
    {
      bool bResult = false;
      struct stat tStat;

      if (stat(strPath.c_str(), &tStat) == 0)
      {
        struct tm tTime;
        bResult = true;
        localtime_r(&tStat.st_mtime, &tTime);
        nYear = tTime.tm_year + 1900;
        nMonth = tTime.tm_mon + 1;
        nDay = tTime.tm_mday;
        nHour = tTime.tm_hour;
        nMinute = tTime.tm_min;
        nSecond = tTime.tm_sec;
      }

      return bResult;
    }
    // }}}
    // {{{ copy()
    bool File::copy(const string strOldPath, const string strNewPath)
    {
      bool bResult = false, bCopied = false;
      char cBuffer = '\0';
      ifstream inFile;
      ofstream outFile;

      if (fileExist(strOldPath))
      {
        inFile.open(strOldPath.c_str(), ios::in|ios::binary);
        outFile.open(strNewPath.c_str(), ios::out|ios::binary);
        if (inFile.good() && outFile.good())
        {
          bCopied = true;
          while(inFile.read(&cBuffer, 1).good())
          {
            outFile.write(&cBuffer, 1);
          }
        }
        inFile.close();
        outFile.close();
        if (bCopied && fileSize(strOldPath) == fileSize(strNewPath))
        {
          bResult = true;
        }
      }

      return bResult;
    }
    // }}}
    // {{{ rename()
    bool File::rename(const string strOldPath, const string strNewPath)
    {
      bool bResult = false;

      if (std::rename(strOldPath.c_str(), strNewPath.c_str()) == 0)
      {
        bResult = true;
      }

      return bResult;
    }
    // }}}
    // {{{ remove()
    bool File::remove(const string strPath)
    {
      bool bResult = false;

      if (std::remove(strPath.c_str()) == 0)
      {
        bResult = true;
      }

      return bResult;
    }
    // }}}
    // {{{ directoryExist()
    bool File::directoryExist(const string strPath)
    {
      bool bResult = false;
      struct stat tStat;

      if (stat(strPath.c_str(), &tStat) == 0 && S_ISDIR(tStat.st_mode))
      {
        bResult = true;
      }

      return bResult;
    }
    // }}}
    // {{{ directoryList()
    list<string> File::directoryList(const string strPath, list<string> &dirList)
    {
      DIR *dirp;

      if ((dirp = opendir(strPath.c_str())))
      {
        struct dirent *entry;
        while ((entry = readdir(dirp)) != NULL)
        {
          dirList.push_back(entry->d_name);
        }
        closedir(dirp);
      }
      dirList.sort();

      return dirList;
    }
    // }}}
    // {{{ makeDirectory()
    bool File::makeDirectory(const string strPath)
    {
      bool bResult = false;
      mode_t mode = 00777;

      if (mkdir(strPath.c_str(), mode) == 0)
      {
        bResult = true;
      }

      return bResult;
    }
    // }}}
    // {{{ establishDirectory()
    bool File::establishDirectory(const string strPath)
    {
      bool bResult = true;
      mode_t mode = 00777;
      size_t unIndex = 0;

      if (strPath[0] == '/')
      {
        unIndex = 1;
      }
      for (size_t nPosition = unIndex; bResult && nPosition != string::npos; nPosition = strPath.find("/", nPosition))
      {
        if (!directoryExist(strPath.substr(0, nPosition)) && mkdir(strPath.substr(0, nPosition).c_str(), mode) != 0)
        {
          bResult = false;
        }
      }
      if (!directoryExist(strPath) && mkdir(strPath.c_str(), mode) != 0)
      {
        bResult = false;
      }

      return bResult;
    }
    // }}}
    // {{{ removeDirectory()
    bool File::removeDirectory(const string strPath)
    {
      bool bResult = true;
      list<string> dirList;

      directoryList(strPath, dirList);
      if (!dirList.empty())
      {
        list<string> removeList;
        for (list<string>::iterator i = dirList.begin(); i != dirList.end(); i++)
        {
          if ((*i) != "." && (*i) != "..")
          {
            if (directoryExist(strPath + (string)"/" + (*i)))
            {
              removeList.push_back(*i);
            }
            else if (!remove(strPath + (string)"/" + (*i)))
            {
              bResult = false;
            }
          }
        }
        for (list<string>::iterator i = removeList.begin(); i != removeList.end(); i++)
        {
          if (!removeDirectory(strPath + (string)"/" + (*i)))
          {
            bResult = false;
          }
        }
        dirList.clear();
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
    // {{{ findLine()
    bool File::findLine(ifstream &inFile, const bool bResetFilePointer, const bool bEatLine, const string strString)
    {
      streampos current_position;
      bool bResult = false, bOriginalState = false;
      string strTemp;

      if (inFile.good())
      {
        bOriginalState = true;
        if (bResetFilePointer)
        {
          inFile.clear();
          inFile.seekg(0, ios::beg);
        }
        current_position = inFile.tellg();
      }
      while (!bResult && getline(inFile, strTemp).good())
      {
        if (strTemp.find(strString, 0) != string::npos)
        {
          if (!bEatLine)
          {
            inFile.seekg(current_position);
          }
          bResult = true;
        }
        else
        {
          current_position = inFile.tellg();
        }
      }
      if (!inFile.good() && bOriginalState)
      {
        inFile.clear();
      }

      return bResult;
    }
    // }}}
  }
}
