/* -*- c++ -*- */
///////////////////////////////////////////
// Archive
// -------------------------------------
// file       : Archive.cpp
// author     : Ben Kietzman
// begin      : 2010-03-03
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
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
