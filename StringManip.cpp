// vim: syntax=cpp
// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// StringManip
// -------------------------------------
// file       : StringManip
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

/*! \file StringManip.cpp
* \brief StringManip Class
*/
// {{{ includes
#include "StringManip"
// }}}
extern "C++"
{
  namespace common
  {
    // {{{ StringManip()
    StringManip::StringManip()
    {
    }
    // }}}
    // {{{ ~StringManip()
    StringManip::~StringManip()
    {
    }
    // }}}
    // {{{ cleanNonPrintable()
    string StringManip::cleanNonPrintable(string &strClean, const string strString)
    {
      strClean = strString;
      for (size_t nPosition = 0; nPosition < strClean.size(); nPosition++)
      {
        if (!isprint(strString[nPosition]))
        {
          strClean = purgeChar(strClean, strClean, strClean.substr(nPosition--, 1));
        }
      }

      return strClean;
    }
    // }}}
    // {{{ decodeBase64()
    string StringManip::decodeBase64(const string strIn, string &strOut)
    {
      #ifdef COMMON_B64
      base64::decoder b64decoder;
      stringstream ssIn(strIn), ssOut;
      b64decoder.decode(ssIn, ssOut);
      strOut = ssOut.str();
      #else
      #ifdef COMMON_OPENSSL
      BIO *b64, *bmem;
      char *buffer = (char *)malloc(strIn.size());
      memset(buffer, 0, strIn.size());
      b64 = BIO_new(BIO_f_base64());
      bmem = BIO_new_mem_buf((void *)strIn.c_str(), strIn.size());
      bmem = BIO_push(b64, bmem);
      BIO_read(bmem, buffer, strIn.size());
      BIO_free_all(bmem);
      strOut = buffer;
      free(buffer);
      #else
      static const std::string b64_charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
      const char *str = strIn.c_str();
      int in_len, i = 0, j = 0, in_ = 0;
      unsigned char block_4[4], block_3[3];
      strOut.clear();
      in_len = strlen(str);
      while (in_len-- && (str[in_] != '=') && isBase64(str[in_]))
      {
        block_4[i++] = str[in_];
        in_++;
        if (i == 4)
        {
          for (i = 0; i < 4; i++)
          {
            block_4[i] = b64_charset.find(block_4[i]);
          }
          block_3[0] = (block_4[0] << 2) + ((block_4[1] & 0x30) >> 4);
          block_3[1] = ((block_4[1] & 0xf) << 4) + ((block_4[2] & 0x3c) >> 2);
          block_3[2] = ((block_4[2] & 0x3) << 6) + block_4[3];
          for (i = 0; (i < 3); i++)
          {
            strOut += block_3[i];
          }
          i = 0;
        }
      }
      if (i)
      {
        for (j = i; j < 4; j++)
        {
          block_4[j] = 0;
        }
        for (j = 0; j < 4; j++)
        {
          block_4[j] = b64_charset.find(block_4[j]);
        }
        block_3[0] = (block_4[0] << 2) + ((block_4[1] & 0x30) >> 4);
        block_3[1] = ((block_4[1] & 0xf) << 4) + ((block_4[2] & 0x3c) >> 2);
        block_3[2] = ((block_4[2] & 0x3) << 6) + block_4[3];
        for (j = 0; (j < i - 1); j++)
        {
          strOut += block_3[j];
        }
      }
      #endif
      #endif

      return strOut;
    }
    // }}}
    // {{{ decryptAes()
    string StringManip::decryptAes(const string strIn, const string strSecret, string &strOut, string &strError)
    {
      strOut.clear();
      if (!strIn.empty())
      {
        if (!strSecret.empty())
        {
          stringstream ssError;
          unsigned char *puszKey;
          #ifdef COMMON_OPENSSL
          if ((puszKey = (unsigned char *)malloc(SHA512_DIGEST_LENGTH)) != NULL)
          {
            if (SHA512((unsigned char *)strSecret.c_str(), strSecret.size(), puszKey) != NULL)
            {
              int nIn = strIn.size();
              unsigned char *puszIn;
              if ((puszIn = (unsigned char *)malloc(nIn)) != NULL)
              {
                unsigned char *puszOut;
                memcpy(puszIn, strIn.c_str(), nIn);
                if ((puszOut = (unsigned char *)malloc(nIn*16)) != NULL)
                {
                  EVP_CIPHER_CTX *ctx;
                  if ((ctx = EVP_CIPHER_CTX_new()) != NULL)
                  {
                    if (EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, puszKey, NULL))
                    {
                      int nLength;
                      if (EVP_DecryptUpdate(ctx, puszOut, &nLength, puszIn, nIn))
                      {
                        int nOut = nLength;
                        if (EVP_DecryptFinal_ex(ctx, puszOut + nLength, &nLength))
                        {
                          nOut += nLength;
                          strOut.assign((char *)puszOut, nOut);
                        }
                        else
                        {
                          char szBuffer[120];
                          ssError.str("");
                          ssError << "EVP_DecryptFinal_ex(" << ERR_get_error() << ") " << ERR_error_string(ERR_get_error(), szBuffer);
                          strError = ssError.str();
                        }
                      }
                      else
                      {
                        char szBuffer[120];
                        ssError.str("");
                        ssError << "EVP_DecryptUpdate(" << ERR_get_error() << ") " << ERR_error_string(ERR_get_error(), szBuffer);
                        strError = ssError.str();
                      }
                    }
                    else
                    {
                      char szBuffer[120];
                      ssError.str("");
                      ssError << "EVP_DecryptInit_ex(" << ERR_get_error() << ") " << ERR_error_string(ERR_get_error(), szBuffer);
                      strError = ssError.str();
                    }
                    EVP_CIPHER_CTX_free(ctx);
                  }
                  else
                  {
                    char szBuffer[120];
                    ssError.str("");
                    ssError << "EVP_CIPHER_CTX_new(" << ERR_get_error() << ") " << ERR_error_string(ERR_get_error(), szBuffer);
                    strError = ssError.str();
                  }
                  free(puszOut);
                }
                else
                {
                  ssError.str("");
                  ssError << "malloc(" << errno << ") " << strerror(errno);
                  strError = ssError.str();
                }
                free(puszIn);
              }
              else
              {
                char szBuffer[120];
                ssError.str("");
                ssError << "SHA512(" << ERR_get_error() << ") " << ERR_error_string(ERR_get_error(), szBuffer);
                strError = ssError.str();
              }
            }
            else
            {
              ssError.str("");
              ssError << "malloc(" << errno << ") " << strerror(errno);
              strError = ssError.str();
            }
            free(puszKey);
            #endif
          }
          else
          {
            ssError.str("");
            ssError << "malloc(" << errno << ") " << strerror(errno);
            strError = ssError.str();
          }
        }
        else
        {
          strError = "Please provide the Secret.";
        }
      }
      else
      {
        strError = "Please provide the In.";
      }

      return strOut;
    }
    // }}}
    // {{{ encodeBase64()
    string StringManip::encodeBase64(const string strIn, string &strOut)
    {
      #ifdef COMMON_B64
      base64::encoder b64encoder;
      stringstream ssIn(strIn), ssOut;
      b64encoder.encode(ssIn, ssOut);
      strOut = ssOut.str();
      #else
      #ifdef COMMON_OPENSSL
      BIO *bmem, *b64;
      BUF_MEM *bptr;
      char *buffer;
      b64 = BIO_new(BIO_f_base64());
      bmem = BIO_new(BIO_s_mem());
      b64 = BIO_push(b64, bmem);
      BIO_write(b64, strIn.c_str(), strIn.size());
      (void)BIO_flush(b64);
      BIO_get_mem_ptr(b64, &bptr);
      buffer = (char *)malloc(bptr->length); 
      memcpy(buffer, bptr->data, bptr->length-1);
      buffer[bptr->length-1] = 0;
      BIO_free_all(b64);
      strOut = buffer;
      free(buffer);
      #else
      static const char b64_charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
      const char *str = strIn.c_str();
      unsigned char block_3[3];
      unsigned char block_4[4];
      int i = 0, j = 0, size;
      strOut.clear();
      size = strlen(str);
      while (size--)
      {
        block_3[i++] = *(str++);
        if (i == 3)
        {
          block_4[0] = (block_3[0] & 0xfc) >> 2;
          block_4[1] = ((block_3[0] & 0x03) << 4) + ((block_3[1] & 0xf0) >> 4);
          block_4[2] = ((block_3[1] & 0x0f) << 2) + ((block_3[2] & 0xc0) >> 6);
          block_4[3] = block_3[2] & 0x3f;
          for (i = 0; (i < 4) ; i++)
          {
            strOut += b64_charset[block_4[i]];
          }
          i = 0;
        }
      }
      if (i)
      {
        for (j = i; j < 3; j++)
        {
          block_3[j] = '\0';
        }
        block_4[0] = (block_3[0] & 0xfc) >> 2;
        block_4[1] = ((block_3[0] & 0x03) << 4) + ((block_3[1] & 0xf0) >> 4);
        block_4[2] = ((block_3[1] & 0x0f) << 2) + ((block_3[2] & 0xc0) >> 6);
        block_4[3] = block_3[2] & 0x3f;
        for (j = 0; (j < i + 1); j++)
        {
          strOut += b64_charset[block_4[j]];
        }
        while ((i++ < 3))
        {
          strOut += '=';
        }
      }
      #endif
      #endif

      return strOut;
    }
    // }}} 
    // {{{ encryptAes()
    string StringManip::encryptAes(const string strIn, const string strSecret, string &strOut, string &strError)
    {
      strOut.clear();
      if (!strIn.empty())
      {
        if (!strSecret.empty())
        {
          stringstream ssError;
          unsigned char *puszKey;
          #ifdef COMMON_OPENSSL
          if ((puszKey = (unsigned char *)malloc(SHA512_DIGEST_LENGTH)) != NULL)
          {
            if (SHA512((unsigned char *)strSecret.c_str(), strSecret.size(), puszKey) != NULL)
            {
              int nIn = strIn.size();
              unsigned char *puszIn;
              if ((puszIn = (unsigned char *)malloc(nIn)) != NULL)
              {
                unsigned char *puszOut;
                memcpy(puszIn, strIn.c_str(), nIn);
                if ((puszOut = (unsigned char *)malloc(nIn*16)) != NULL)
                {
                  EVP_CIPHER_CTX *ctx;
                  if ((ctx = EVP_CIPHER_CTX_new()) != NULL)
                  {
                    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, puszKey, NULL))
                    {
                      int nLength;
                      if (EVP_EncryptUpdate(ctx, puszOut, &nLength, puszIn, nIn))
                      {
                        int nOut = nLength;
                        if (EVP_EncryptFinal_ex(ctx, puszOut + nLength, &nLength))
                        {
                          nOut += nLength;
                          strOut.assign((char *)puszOut, nOut);
                        }
                        else
                        {
                          char szBuffer[120];
                          ssError.str("");
                          ssError << "EVP_EncryptFinal_ex(" << ERR_get_error() << ") " << ERR_error_string(ERR_get_error(), szBuffer);
                          strError = ssError.str();
                        }
                      }
                      else
                      {
                        char szBuffer[120];
                        ssError.str("");
                        ssError << "EVP_EncryptUpdate(" << ERR_get_error() << ") " << ERR_error_string(ERR_get_error(), szBuffer);
                        strError = ssError.str();
                      }
                    }
                    else
                    {
                      char szBuffer[120];
                      ssError.str("");
                      ssError << "EVP_EncryptInit_ex(" << ERR_get_error() << ") " << ERR_error_string(ERR_get_error(), szBuffer);
                      strError = ssError.str();
                    }
                    EVP_CIPHER_CTX_free(ctx);
                  }
                  else
                  {
                    char szBuffer[120];
                    ssError.str("");
                    ssError << "EVP_CIPHER_CTX_new(" << ERR_get_error() << ") " << ERR_error_string(ERR_get_error(), szBuffer);
                    strError = ssError.str();
                  }
                  free(puszOut);
                }
                else
                {
                  ssError.str("");
                  ssError << "malloc(" << errno << ") " << strerror(errno);
                  strError = ssError.str();
                }
                free(puszIn);
              }
              else
              {
                ssError.str("");
                ssError << "malloc(" << errno << ") " << strerror(errno);
                strError = ssError.str();
              }
            }
            else
            {
              char szBuffer[120];
              ssError.str("");
              ssError << "SHA512(" << ERR_get_error() << ") " << ERR_error_string(ERR_get_error(), szBuffer);
              strError = ssError.str();
            }
            free(puszKey);
          }
          else
          {
            ssError.str("");
            ssError << "malloc(" << errno << ") " << strerror(errno);
            strError = ssError.str();
          }
          #endif
        }
        else
        {
          strError = "Please provide the Secret.";
        }
      }
      else
      {
        strError = "Please provide the In.";
      }

      return strOut;
    }
    // }}}
    // {{{ escape()
    string StringManip::escape(const string strIn, string &strOut)
    {
      size_t unSize = strIn.size();
      stringstream ssResult;

      for (size_t i = 0; i < unSize; i++)
      {
        switch (strIn[i])
        {
          case 0    : ssResult << "\\0";  break;
          case '\n' : ssResult << "\\n";  break;
          case '\r' : ssResult << "\\r";  break;
          case '\\' : ssResult << "\\\\"; break;
          case '\'' : ssResult << "\\'";  break;
          case '"'  : ssResult << "\\\""; break;
          default   : ssResult << strIn[i];
        };
      }
 
      return (strOut = ssResult.str());
    }
    // }}} 
    // {{{ getToken()
    string StringManip::getToken(string &strToken, const string strString, const int nIndex, const string strDelimiter, const bool bIgnoreBlanks)
    {
      size_t nPosition = 0, nSubPosition = 0, nSize = 0;

      if (nIndex > 0)
      {
        if (bIgnoreBlanks)
        {
          while (nPosition + strDelimiter.size() < strString.size() && strString.substr(nPosition, strDelimiter.size()) == strDelimiter)
          {
            nPosition += strDelimiter.size();
          }
        }
        for (int i = 1; i < nIndex && nPosition != string::npos; i++)
        {
          if ((nSubPosition = strString.substr(nPosition, strString.size() - nPosition).find(strDelimiter, 0)) != string::npos)
          {
            nPosition += nSubPosition + strDelimiter.size();
          }
          if (bIgnoreBlanks)
          {
            while (nPosition + strDelimiter.size() < strString.size() && strString.substr(nPosition, strDelimiter.size()) == strDelimiter)
            {
              nPosition += strDelimiter.size();
            }
          }
        }
        if (nSubPosition != string::npos)
        {
          if ((nSize = strString.find(strDelimiter, nPosition)) != string::npos)
          {
            nSize -= nPosition;
          }
          else
          {
            nSize = strString.size() - nPosition;
          }
          strToken = strString.substr(nPosition, nSize);
        }
        else
        {
          strToken = "";
        }
      }
      else
      {
        strToken = "";
      }

      return strToken;
    }
    // }}}
    // {{{ htmlEntityDecode()
    string StringManip::htmlEntityDecode(const string strOld, string &strNew)
    {
      size_t nPosition[3] = {0, 0};
      string strEntity;

      strNew.clear();
      while ((nPosition[1] = strOld.find('&', nPosition[0])) != string::npos)
      {
        bool bDecoded = false;
        strNew += strOld.substr(nPosition[0], nPosition[1] - nPosition[0]);
        if ((nPosition[2] = strOld.find(';', nPosition[1])) != string::npos)
        {
          if (strOld[nPosition[1] + 1] == '#')
          {
            bool bNumeric = true;
            for (size_t i = nPosition[1] + 2; bNumeric && i < nPosition[2]; i++)
            {
              if (!isdigit(strOld[i]))
              {
                bNumeric = false;
              } 
            }
            if (bNumeric)
            {
              bDecoded = true;
              strNew += char(atoi(strOld.substr(nPosition[1] + 2, nPosition[2] - (nPosition[1] + 2)).c_str()));
            } 
          } 
          else
          {
            bDecoded = true;
            strEntity = strOld.substr(nPosition[1] + 1, nPosition[2] - (nPosition[1] + 1));
            if (strEntity == "amp")
            {
              strNew += '&';
            }
            else if (strEntity == "apos")
            {
              strNew += '\'';
            }
            else if (strEntity == "lt")
            {
              strNew += '<';
            }
            else if (strEntity == "gt")
            {
              strNew += '>';
            }
            else if (strEntity == "quot")
            {
              strNew += '"';
            }
            else
            {
              bDecoded = false;
            }
          }
        }
        if (bDecoded)
        {
          nPosition[0] = nPosition[2] + 1;
        }
        else
        {
          strNew += '&';
          nPosition[0] = nPosition[1] + 1;
        }
      }
      if (nPosition[0] < strOld.size())
      {
        strNew += strOld.substr(nPosition[0], strOld.size() - nPosition[0]);
      }

      return strNew;
    }
    // }}}
    // {{{ htmlEntityEncode()
    string StringManip::htmlEntityEncode(const string strOld, string &strNew)
    {
      map<string, string> encodeList;
      size_t nPosition;
      string strValue;

      encodeList["&"] = "&amp;";
      encodeList["'"] = "&apos;";
      encodeList["\""] = "&quot;";
      encodeList["<"] = "&lt;";
      encodeList[">"] = "&gt;";
      strNew = strOld;
      for (map<string, string>::iterator i = encodeList.begin(); i != encodeList.end(); i++)
      {
        nPosition = 0;
        while (nPosition < strNew.size() && (nPosition = strNew.find(i->second, nPosition)) != string::npos)
        {
          strNew.replace(nPosition, i->second.size(), i->first);
          nPosition += i->first.size();
        }
        nPosition = 0;
        while (nPosition < strNew.size() && (nPosition = strNew.find(i->first, nPosition)) != string::npos)
        {
          strNew.replace(nPosition, i->first.size(), i->second);
          nPosition += i->second.size();
        }
        i->second.clear();
      }
      encodeList.clear();
      for (size_t i = 0; i < strNew.size(); i++)
      {
        if (!isprint(strNew[i]))
        {
          strNew.replace(i, 1, (string)"&#" + toString(int(strNew[i]), strValue) + (string)";");
        }
      }

      return strNew;
    }
    // }}}
    // {{{ isBase64()
    int StringManip::isBase64(char c)
    {
      if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c == '+') || (c == '/') || (c == '='))
      {
        return true;
      }
  
      return false;
    }
    // }}}
    // {{{ isNumeric()
    bool StringManip::isNumeric(const string strValue)
    {
      bool bResult = false;

      if (!strValue.empty())
      {
        bResult = true;
        for (size_t i = 0; bResult && i < strValue.size(); i++)
        {
          if ((i != 0 || strValue[i] != '-') && !isdigit(strValue[i]))
          {
            bResult = false;
          }
        }
      }

      return bResult;
    }
    // }}}
    // {{{ lpad()
    string StringManip::lpad(string &strPad, const string strString, const char cPad, const int nPad)
    {
      return pad(strPad, strString, cPad, nPad, 0);
    }
    // }}}
    // {{{ ltrim()
    string StringManip::ltrim(string &strTrim, const string strString)
    {
      return trim(strTrim, strString, 1);
    }
    // }}}
    // {{{ pad()
    string StringManip::pad(string &strPad, const string strString, const char cPad, const int nPad, const int nSide)
    {
      strPad = strString;
      for (unsigned int i = 0; i < nPad - strString.size(); i++)
      {
        (nSide) ? strPad += cPad : strPad = cPad + strPad;
      }

      return strPad;
    }
    // }}}
    // {{{ purgeChar()
    string StringManip::purgeChar(string &strPurge, const string strString, const string strChar)
    {
      size_t nPosition = 0;

      strPurge = strString;
      while ((nPosition = strPurge.find(strChar, 0)) != string::npos)
      {
        strPurge.erase(nPosition, strChar.size());
      }

      return strPurge;
    }
    // }}}
    // {{{ rpad()
    string StringManip::rpad(string &strPad, const string strString, const char cPad, const int nPad)
    {
      return pad(strPad, strString, cPad, nPad, 1);
    }
    // }}}
    // {{{ rtrim()
    string StringManip::rtrim(string &strTrim, const string strString)
    {
      return trim(strTrim, strString, 2);
    }
    // }}}
    // {{{ toCase()
    string StringManip::toCase(string &strCase, const string strString, int nCase)
    {
      strCase = strString;
      for (size_t nPosition = 0; nPosition < strCase.size(); nPosition++)
      {
        if (nCase == 0)
        {
          strCase[nPosition] = tolower(strCase[nPosition]);
        }
        else
        {
          strCase[nPosition] = toupper(strCase[nPosition]);
        }
      }

      return strCase;
    }
    // }}}
    // {{{ toLower()
    string StringManip::toLower(string &strLower, const string strString)
    {
      return toCase(strLower, strString, 0);
    }
    // }}}
    // {{{ toShort()
    string StringManip::toShort(float fNumber, string &strNumber)
    {
      bool bNegative = false;
      string strSuffix;
      stringstream ssNumber;

      if (fNumber < 0)
      {
        bNegative = true;
        fNumber *= -1;
      }
      if (fNumber >= 1000)
      {
        strSuffix = "K";
        fNumber /= 1000;
        if (fNumber >= 1000)
        {
          strSuffix = "M";
          fNumber /= 1000;
          if (fNumber >= 1000)
          {
            strSuffix = "B";
            fNumber /= 1000;
            if (fNumber >= 1000)
            {
              strSuffix = "T";
              fNumber /= 1000;
            }
          }
        }
      }
      ssNumber << ((bNegative)?"-":"") << fixed << setprecision(2) << fNumber << strSuffix;
      strNumber = ssNumber.str();

      return strNumber;
    }
    // }}}
    // {{{ toShortByte()
    string StringManip::toShortByte(float fNumber, string &strNumber)
    {
      bool bNegative = false;
      string strSuffix;
      stringstream ssNumber;

      if (fNumber < 0)
      {
        bNegative = true;
        fNumber *= -1;
      }
      if (fNumber >= 1024)
      {
        strSuffix = "KB";
        fNumber /= 1024;
        if (fNumber >= 1024)
        {
          strSuffix = "MB";
          fNumber /= 1024;
          if (fNumber >= 1024)
          {
            strSuffix = "GB";
            fNumber /= 1024;
            if (fNumber >= 1024)
            {
              strSuffix = "TB";
              fNumber /= 1024;
            }
          }
        }
      }
      ssNumber << ((bNegative)?"-":"") << fixed << setprecision(2) << fNumber << strSuffix;
      strNumber = ssNumber.str();

      return strNumber;
    }
    // }}}
    // {{{ toString()
    string StringManip::toString(const string strValue, string &strResult)
    {
      return strResult = strValue;
    }
    string StringManip::toString(const unsigned int unValue, string &strResult)
    {
      ostringstream outStream;

      outStream << unValue;

      return strResult = outStream.str();
    }
    string StringManip::toString(const int nValue, string &strResult)
    {
      ostringstream outStream;

      outStream << nValue;

      return strResult = outStream.str();
    }
    string StringManip::toString(const float fValue, string &strResult)
    {
      ostringstream outStream;

      outStream << fValue;

      return strResult = outStream.str();
    }
    string StringManip::toString(const double dValue, string &strResult)
    {
      ostringstream outStream;

      outStream << dValue;

      return strResult = outStream.str();
    }
    string StringManip::toString(const unsigned long ulValue, string &strResult)
    {
      ostringstream outStream;

      outStream << ulValue;

      return strResult = outStream.str();
    }
    string StringManip::toString(const long lValue, string &strResult)
    {
      ostringstream outStream;

      outStream << lValue;

      return strResult = outStream.str();
    }
    string StringManip::toString(const unsigned long long ullValue, string &strResult)
    {
      ostringstream outStream;

      outStream << ullValue;

      return strResult = outStream.str();
    }
    string StringManip::toString(const long long llValue, string &strResult)
    {
      ostringstream outStream;

      outStream << llValue;

      return strResult = outStream.str();
    }
    string StringManip::toString(const char cValue, string &strResult)
    {
      ostringstream outStream;

      outStream << cValue;

      return strResult = outStream.str();
    }
    string StringManip::toString(const char szValue[], string &strResult)
    {
      return strResult = szValue;
    }
    // }}}
    // {{{ toUpper()
    string StringManip::toUpper(string &strUpper, const string strString)
    {
      return toCase(strUpper, strString, 1);
    }
    // }}}
    // {{{ trim()
    string StringManip::trim(string &strTrim, const string strString, const int nSide)
    {
      bool bDone = false;

      strTrim = strString;
      if (!strTrim.empty() && (nSide == 0 || nSide == 1))
      {
        for (size_t i = 0; !bDone && i < strTrim.size(); i++)
        {
          if (isspace(strTrim[i]))
          {
            strTrim.erase(i--, 1);
          }
          else
          {
            bDone = true;
          }
        }
      }
      bDone = false;
      if (!strTrim.empty() && (nSide == 0 || nSide == 2))
      {
        for (size_t i = strTrim.size() - 1; !bDone && i >= 0; i--)
        {
          if (isspace(strTrim[i]))
          {
            strTrim.erase(i, 1);
          }
          else
          {
            bDone = true;
          }
        }
      }

      return strTrim;
    }
    // }}}
    // {{{ urlDecode()
    string StringManip::urlDecode(string &strDecoded, const string strData)
    {
      unsigned char c;
      unsigned int unSize = strData.size();
      stringstream ssResult;

      for (unsigned int i = 0; i < unSize; i++)
      {
        c = strData[i];
        if (c == '+')
        {
          ssResult << ' ';
        }
        else if (c == '%')
        {
          unsigned int unPosition[2] = {0, 0};
          for (unsigned int j = 0; j < 2; j++)
          {
            char cChar = strData[i + j + 1];
            unPosition[j] = ((cChar >= '0' && cChar <= '9')?(cChar - '0'):(cChar - 'A' + 10));
          }
          ssResult << char(unPosition[0] * 16 + unPosition[1]);
          i += 2;
        }
        else
        {
          ssResult << c;
        }
      }
      strDecoded = ssResult.str();

      return strDecoded;
    }
    // }}}
    // {{{ urlEncode()
    string StringManip::urlEncode(string &strEncoded, const string strData)
    {
      unsigned char hexchars[] = "0123456789ABCDEF";
      unsigned char c;
      unsigned int unSize = strData.size();
      stringstream ssResult;

      for (unsigned int i = 0; i < unSize; i++)
      {
        c = strData[i];
        if (c == ' ')
        {
          ssResult << '+';
        }
        else if ((c < '0' && c != '-' && c != '.') || (c < 'A' && c > '9') || (c > 'Z' && c < 'a' && c != '_') || (c > 'z'))
        {
          ssResult << '%' << hexchars[c >> 4] << hexchars[c & 15];
        }
        else
        {
          ssResult << c;
        }
      }
      strEncoded = ssResult.str();

      return strEncoded;
    }
    // }}}
  }
}
