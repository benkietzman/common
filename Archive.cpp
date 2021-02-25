// vim: syntax=cpp
// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Archive
// -------------------------------------
// file       : Archive.cpp
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

/*! \file Archive.cpp
* \brief Archive Class
*/
// {{{ includes
#include "Archive"
// }}}
extern "C++"
{
  namespace common
  {
    // {{{ Archive()
    Archive::Archive()
    {
    }
    // }}}
    // {{{ ~Archive
    Archive::~Archive()
    {
    }
    // }}}
    // {{{ Auto()
    // The Auto function is dependent on the file extension.
    // For that reason it will not Compress, GZip, or BZip2.
    bool Archive::Auto(const string strFile, list<string> fileList)
    {
      bool bResult = false;
      size_t nPosition = 0;
      string strExtension;

      if ((nPosition = strFile.rfind(".", strFile.size() - 1)) != string::npos)
      {
        strExtension = strFile.substr(nPosition + 1, 3);
      }
      if (strExtension == "gz")
      {
        bResult = gunzip(strFile);
      }
      else if (strExtension == "tar")
      {
        if (!fileList.empty())
        {
          bResult = tar(strFile, fileList);
        }
        else
        {
          bResult = untar(strFile);
        }
      }

      return bResult;
    }

    bool Archive::Auto(const string strFile, const string strFiles)
    {
      bool bResult = true;
      list<string> fileList;
      if (!strFiles.empty())
      {
        fileList.push_back(strFiles);
      }
      bResult = Auto(strFile, fileList);
      fileList.clear();
      return bResult;
    }
    // }}}
    // {{{ AutoUntar()
    bool Archive::AutoUntar(const string strFile)
    {
      bool bResult = false;
      size_t nPosition = 0;
      string strExtension, strCommand;

      if ((nPosition = strFile.rfind(".", strFile.size() - 1)) != string::npos)
      {
        strExtension = strFile.substr(nPosition + 1, 3);
      }
      if (strExtension == "gz")
      {
        if ((bResult = gunzip(strFile)) && strFile.size() >= 7 && strFile.substr(strFile.size() - 7, 4) == ".tar")
        {
          bResult = untar(strFile.substr(0, strFile.size() - 3));
        }
      }
      else if (strExtension == "tar")
      {
        bResult = untar(strFile);
      }

      return bResult;
    }
    // }}}
    // {{{ tar()
    bool Archive::tar(const string strFile, list<string> fileList)
    {
      bool bResult = true;
      TAR *ptTar;

      if (tar_open(&ptTar, (char *)strFile.c_str(), NULL, O_WRONLY | O_CREAT, 0644, 0) == 0)
      {
        for (list<string>::iterator i = fileList.begin(); i != fileList.end(); i++)
        {
          tar_append_tree(ptTar, (char *)(*i).c_str(), (char *)(*i).c_str());
        }
        tar_append_eof(ptTar);
        tar_close(ptTar);
      }
      else
      {
        bResult = false;
      }

      return bResult;
    }

    bool Archive::tar(const string strFile, const string strFiles)
    {
      bool bResult = true;
      list<string> fileList;
      if (!strFiles.empty())
      {
        fileList.push_back(strFiles);
      }
      bResult = tar(strFile, fileList);
      fileList.clear();
      return bResult;
    }
    // }}}
    // {{{ untar()
    bool Archive::untar(const string strFile, const bool bKeepOriginal)
    {
      bool bResult = true;
      TAR *ptTar;

      if (tar_open(&ptTar, (char *)strFile.c_str(), NULL, O_RDONLY, 0, 0) == 0 && tar_extract_all(ptTar, NULL) == 0)
      {
        tar_close(ptTar);
      }
      else
      {
        bResult = false;
      }

      return bResult;
    }
    // }}}
    // {{{ gzip()
    bool Archive::gzip(const string strFile, const bool bKeepOriginal)
    {
      bool bResult = true;
      ifstream inFile;
      gzFile gzoutFile;
      int nLength = 1;
      char cBuffer;

      inFile.open(strFile.c_str(), ios::in|ios::binary);
      if (inFile.good() && (gzoutFile = gzopen((strFile + (string)".gz").c_str(), "wb")))
      {
        inFile.read(&cBuffer, nLength);
        while (inFile.good())
        {
          gzwrite(gzoutFile, &cBuffer, nLength);
          inFile.read(&cBuffer, nLength);
        }
        gzclose(gzoutFile);
      }
      else
      {
        bResult = false;
      }
      inFile.close();
      if (bResult && !bKeepOriginal)
      {
        remove(strFile.c_str());
      }

      return bResult;
    }
    // }}}
    // {{{ gunzip()
    bool Archive::gunzip(const string strFile, const bool bKeepOriginal)
    {
      bool bResult = true;
      gzFile gzinFile = NULL;
      ofstream outFile;
      int nLength = 1;
      char cBuffer;

      if (strFile.substr(strFile.size() - 3, 3) == ".gz")
      {
        outFile.open(strFile.substr(0, strFile.size() - 3).c_str(), ios::out|ios::binary);
        if (outFile.good() && (gzinFile = gzopen(strFile.c_str(), "rb")))
        {
          while (gzread(gzinFile, &cBuffer, nLength) > 0)
          {
            outFile.write(&cBuffer, nLength);
          }
        }
        else
        {
          bResult = false;
        }
        gzclose(gzinFile);
        outFile.close();
        if (bResult && !bKeepOriginal)
        {
          remove(strFile.c_str());
        }
      }

      return bResult;
    }
    // }}}
  }
}
