// vim: syntax=cpp
// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// BitMessage
// -------------------------------------
// file       : BitMessage.cpp
// author     : Ben Kietzman
// begin      : 2013-08-21
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

/*! \file BitMessage.cpp
* \brief BitMessage Functions
*/
// {{{ includes
#include "BitMessage"
// }}}
extern "C++"
{
  namespace common
  {
    // {{{ BitMessage::BitMessage()
    BitMessage::BitMessage()
    {
      xmlrpc_limit_set(XMLRPC_XML_SIZE_LIMIT_ID, 4194304);
      xmlrpc_env_init(&m_env);
      xmlrpc_client_setup_global_const(&m_env);
      m_pServerInfo = NULL;
    }
    // }}}
    // {{{ BitMessage::~BitMessage()
    BitMessage::~BitMessage()
    {
      if (!m_trash.empty())
      {
        trashMessages();
      }
      if (m_pServerInfo != NULL)
      {
        xmlrpc_server_info_free(m_pServerInfo);
      }
      xmlrpc_env_clean(&m_env);
      xmlrpc_client_teardown_global_const();
    }
    // }}}
    // {{{ BitMessage::add()
    bool BitMessage::add(const string strA, const string strB, string &strResult, string &strError)
    {
      bool bResult = false;
      xmlrpc_client *ptClient;

      xmlrpc_client_create(&m_env, XMLRPC_CLIENT_NO_FLAGS, "BitMessage", "1.0", NULL, 0, &ptClient);
      if (!m_env.fault_occurred)
      {
        xmlrpc_value *ptA, *ptB, *ptParam = xmlrpc_array_new(&m_env), *ptResult;
        ptA = xmlrpc_string_new(&m_env, strA.c_str());
        xmlrpc_array_append_item(&m_env, ptParam, ptA);
        xmlrpc_DECREF(ptA);
        ptB = xmlrpc_string_new(&m_env, strB.c_str());
        xmlrpc_array_append_item(&m_env, ptParam, ptB);
        xmlrpc_DECREF(ptB);
        xmlrpc_client_call2(&m_env, ptClient, m_pServerInfo, "add", ptParam, &ptResult);
        xmlrpc_DECREF(ptParam);
        if (!m_env.fault_occurred)
        {
          const char *pszResult;
          bResult = true;
          xmlrpc_read_string(&m_env, ptResult, &pszResult);
          strResult = pszResult;
          free((void *)pszResult);
          xmlrpc_DECREF(ptResult);
        }
        else
        {
          stringstream ssError;
          ssError << "XML-RPC Error: " <<  m_env.fault_string << " (" << m_env.fault_code << ")";
          strError = ssError.str();
        }
        xmlrpc_client_destroy(ptClient);
      }
      else
      {
        stringstream ssError;
        ssError << "XML-RPC Error: " <<  m_env.fault_string << " (" << m_env.fault_code << ")";
        strError = ssError.str();
      }

      return bResult;
    }
    // }}}
    // {{{ BitMessage::decodeBase64()
    void BitMessage::decodeBase64(const string strIn, string &strOut)
    {
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
      if (strOut.empty())
      {
        static const std::string b64_charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        const char *str = strIn.c_str();
        int in_len, i = 0, j = 0, in_ = 0;
        unsigned char block_4[4], block_3[3];
        strOut.clear(); 
        in_len = strlen(str);
        while (in_len-- && (str[in_] != '=') && is_base64(str[in_]))
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
      }
    }
    // }}}
    // {{{ BitMessage::encodeBase64()
    void BitMessage::encodeBase64(const string strIn, string &strOut)
    {
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
      if (strOut.empty())
      {
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
      }
    } 
    // }}} 
    // {{{ BitMessage::getAllInboxMessages()
    bool BitMessage::getAllInboxMessages(vector<map<string, string> > &message, string &strError)
    {
      return getAllInboxMessages("", message, strError);
    }
    bool BitMessage::getAllInboxMessages(const string strAccount, vector<map<string, string> > &message, string &strError)
    {
      bool bResult = false;
      xmlrpc_client *ptClient;

      for (size_t i = 0; i < message.size(); i++)
      {
        message[i].clear();
      }
      message.clear();
      xmlrpc_client_create(&m_env, XMLRPC_CLIENT_NO_FLAGS, "BitMessage", "1.0", NULL, 0, &ptClient);
      if (!m_env.fault_occurred)
      {
        xmlrpc_value *ptParam = xmlrpc_array_new(&m_env), *ptResult;
        xmlrpc_client_call2(&m_env, ptClient, m_pServerInfo, "getAllInboxMessages", ptParam, &ptResult);
        xmlrpc_DECREF(ptParam);
        if (!m_env.fault_occurred)
        {
          const char *pszResult;
          Json tJson;
          bResult = true;
          xmlrpc_read_string(&m_env, ptResult, &pszResult);
          tJson.parse(pszResult);
          free((void *)pszResult);
          if (tJson.m.find("inboxMessages") != tJson.m.end())
          {
            for (list<Json *>::iterator i = tJson.m["inboxMessages"]->l.begin(); i != tJson.m["inboxMessages"]->l.end(); i++)
            {
              map<string, string> item;
              for (map<string, Json *>::iterator j = (*i)->m.begin(); j != (*i)->m.end(); j++)
              {
                if (j->first == "subject" || j->first == "message")
                {
                  string strDecodedValue;
                  decodeBase64(j->second->v, strDecodedValue);
                  item[j->first] = strDecodedValue;
                }
                else
                {
                  item[j->first] = j->second->v;
                }
              }
              if (!strAccount.empty())
              {
                if (item["toAddress"] == "Broadcast")
                {
                  if (item["fromAddress"] == strAccount)
                  {
                    message.push_back(item);
                  }
                }
                else if (item["toAddress"] == strAccount)
                {
                  message.push_back(item);
                }
              }
              else
              {
                message.push_back(item);
              }
              item.clear();
            }
          }
          xmlrpc_DECREF(ptResult);
        }
        else
        {
          stringstream ssError;
          ssError << "XML-RPC Error: " <<  m_env.fault_string << " (" << m_env.fault_code << ")";
          strError = ssError.str();
        }
        xmlrpc_client_destroy(ptClient);
      }
      else
      {
        stringstream ssError;
        ssError << "XML-RPC Error: " <<  m_env.fault_string << " (" << m_env.fault_code << ")";
        strError = ssError.str();
      }

      return bResult;
    }
    // }}}
    // {{{ BitMessage::getAllSentMessages()
    bool BitMessage::getAllSentMessages(vector<map<string, string> > &message, string &strError)
    {
      return getAllSentMessages("", message, strError);
    }
    bool BitMessage::getAllSentMessages(const string strAccount, vector<map<string, string> > &message, string &strError)
    {
      bool bResult = false;
      xmlrpc_client *ptClient;

      for (size_t i = 0; i < message.size(); i++)
      {
        message[i].clear();
      }
      message.clear();
      xmlrpc_client_create(&m_env, XMLRPC_CLIENT_NO_FLAGS, "BitMessage", "1.0", NULL, 0, &ptClient);
      if (!m_env.fault_occurred)
      {
        xmlrpc_value *ptParam = xmlrpc_array_new(&m_env), *ptResult;
        xmlrpc_client_call2(&m_env, ptClient, m_pServerInfo, "getAllSentMessages", ptParam, &ptResult);
        xmlrpc_DECREF(ptParam);
        if (!m_env.fault_occurred)
        {
          const char *pszResult;
          Json tJson;
          bResult = true;
          xmlrpc_read_string(&m_env, ptResult, &pszResult);
          tJson.parse(pszResult);
          free((void *)pszResult);
          if (tJson.m.find("sentMessages") != tJson.m.end())
          {
            for (list<Json *>::iterator i = tJson.m["sentMessages"]->l.begin(); i != tJson.m["sentMessages"]->l.end(); i++)
            {
              map<string, string> item;
              for (map<string, Json *>::iterator j = (*i)->m.begin(); j != (*i)->m.end(); j++)
              {
                if (j->first == "subject" || j->first == "message")
                {
                  string strDecodedValue;
                  decodeBase64(j->second->v, strDecodedValue);
                  item[j->first] = strDecodedValue;
                }
                else
                {
                  item[j->first] = j->second->v;
                }
              }
              if (!strAccount.empty())
              {
                if (item["fromAddress"] == strAccount)
                {
                  message.push_back(item);
                }
              }
              else
              {
                message.push_back(item);
              }
              item.clear();
            }
          }
          xmlrpc_DECREF(ptResult);
        }
        else
        {
          stringstream ssError;
          ssError << "XML-RPC Error: " <<  m_env.fault_string << " (" << m_env.fault_code << ")";
          strError = ssError.str();
        }
        xmlrpc_client_destroy(ptClient);
      }
      else
      {
        stringstream ssError;
        ssError << "XML-RPC Error: " <<  m_env.fault_string << " (" << m_env.fault_code << ")";
        strError = ssError.str();
      }

      return bResult;
    }
    // }}}
    // {{{ BitMessage::getTrashedMessages()
    void BitMessage::getTrashedMessages(list<string> &trash)
    {
      trash = m_trash;
    }
    // }}}
    // {{{ BitMessage::helloWorld()
    bool BitMessage::helloWorld(const string strA, const string strB, string &strResult, string &strError)
    {
      bool bResult = false;
      xmlrpc_client *ptClient;

      xmlrpc_client_create(&m_env, XMLRPC_CLIENT_NO_FLAGS, "BitMessage", "1.0", NULL, 0, &ptClient);
      if (!m_env.fault_occurred)
      {
        xmlrpc_value *ptA, *ptB, *ptParam = xmlrpc_array_new(&m_env), *ptResult;
        ptA = xmlrpc_string_new(&m_env, strA.c_str());
        xmlrpc_array_append_item(&m_env, ptParam, ptA);
        xmlrpc_DECREF(ptA);
        ptB = xmlrpc_string_new(&m_env, strB.c_str());
        xmlrpc_array_append_item(&m_env, ptParam, ptB);
        xmlrpc_DECREF(ptB);
        xmlrpc_client_call2(&m_env, ptClient, m_pServerInfo, "helloWorld", ptParam, &ptResult);
        xmlrpc_DECREF(ptParam);
        if (!m_env.fault_occurred)
        {
          const char *pszResult;
          bResult = true;
          xmlrpc_read_string(&m_env, ptResult, &pszResult);
          strResult = pszResult;
          free((void *)pszResult);
          xmlrpc_DECREF(ptResult);
        }
        else
        {
          stringstream ssError;
          ssError << "XML-RPC Error: " <<  m_env.fault_string << " (" << m_env.fault_code << ")";
          strError = ssError.str();
        }
        xmlrpc_client_destroy(ptClient);
      }
      else
      {
        stringstream ssError;
        ssError << "XML-RPC Error: " <<  m_env.fault_string << " (" << m_env.fault_code << ")";
        strError = ssError.str();
      }

      return bResult;
    }
    // }}}
    // {{{ BitMessage::listAddresses()
    bool BitMessage::listAddresses(vector<map<string, string> > &address, string &strError)
    {
      bool bResult = false;
      xmlrpc_client *ptClient;

      for (size_t i = 0; i < address.size(); i++)
      {
        address[i].clear();
      }
      address.clear();
      xmlrpc_client_create(&m_env, XMLRPC_CLIENT_NO_FLAGS, "BitMessage", "1.0", NULL, 0, &ptClient);
      if (!m_env.fault_occurred)
      {
        xmlrpc_value *ptParam = xmlrpc_array_new(&m_env), *ptResult;
        xmlrpc_client_call2(&m_env, ptClient, m_pServerInfo, "listAddresses", ptParam, &ptResult);
        xmlrpc_DECREF(ptParam);
        if (!m_env.fault_occurred)
        {
          const char *pszResult;
          Json tJson;
          bResult = true;
          xmlrpc_read_string(&m_env, ptResult, &pszResult);
          tJson.parse(pszResult);
          free((void *)pszResult);
          for (list<Json *>::iterator i = tJson.m["addresses"]->l.begin(); i != tJson.m["addresses"]->l.end(); i++)
          {
            map<string, string> item;
            for (map<string, Json *>::iterator j = (*i)->m.begin(); j != (*i)->m.end(); j++)
            {
              item[j->first] = j->second->v;
            }
            address.push_back(item);
            item.clear();
          }
          xmlrpc_DECREF(ptResult);
        }
        else
        {
          stringstream ssError;
          ssError << "XML-RPC Error: " <<  m_env.fault_string << " (" << m_env.fault_code << ")";
          strError = ssError.str();
        }
        xmlrpc_client_destroy(ptClient);
      }
      else
      {
        stringstream ssError;
        ssError << "XML-RPC Error: " <<  m_env.fault_string << " (" << m_env.fault_code << ")";
        strError = ssError.str();
      }

      return bResult;
    }
    // }}}
    // {{{ BitMessage::is_base64()
    int BitMessage::is_base64(char c)
    {
      if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c == '+') || (c == '/') || (c == '='))
      {
        return true;
      }
  
      return false;
    }
    // }}}
    // {{{ BitMessage::sendBroadcast()
    bool BitMessage::sendBroadcast(const string strFrom, const string strSubject, const string strMessage, string &strError)
    {
      bool bResult = false;
      xmlrpc_client *ptClient;

      xmlrpc_client_create(&m_env, XMLRPC_CLIENT_NO_FLAGS, "BitMessage", "1.0", NULL, 0, &ptClient);
      if (!m_env.fault_occurred)
      {
        string strEncodedSubject, strEncodedMessage;
        xmlrpc_value *ptEncodedMessage, *ptEncodedSubject, *ptFrom, *ptParam = xmlrpc_array_new(&m_env), *ptResult;
        ptFrom = xmlrpc_string_new(&m_env, strFrom.c_str());
        xmlrpc_array_append_item(&m_env, ptParam, ptFrom);
        xmlrpc_DECREF(ptFrom);
        encodeBase64(strSubject, strEncodedSubject);
        ptEncodedSubject = xmlrpc_string_new(&m_env, strEncodedSubject.c_str());
        xmlrpc_array_append_item(&m_env, ptParam, ptEncodedSubject);
        xmlrpc_DECREF(ptEncodedSubject);
        encodeBase64(strMessage, strEncodedMessage);
        ptEncodedMessage = xmlrpc_string_new(&m_env, strEncodedMessage.c_str());
        xmlrpc_array_append_item(&m_env, ptParam, ptEncodedMessage);
        xmlrpc_DECREF(ptEncodedMessage);
        xmlrpc_client_call2(&m_env, ptClient, m_pServerInfo, "sendBroadcast", ptParam, &ptResult);
        xmlrpc_DECREF(ptParam);
        if (!m_env.fault_occurred)
        {
          bResult = true;
          xmlrpc_DECREF(ptResult);
        }
        else
        {
          stringstream ssError;
          ssError << "XML-RPC Error: " <<  m_env.fault_string << " (" << m_env.fault_code << ")";
          strError = ssError.str();
        }
        xmlrpc_client_destroy(ptClient);
      }
      else
      {
        stringstream ssError;
        ssError << "XML-RPC Error: " <<  m_env.fault_string << " (" << m_env.fault_code << ")";
        strError = ssError.str();
      }

      return bResult;
    }
    // }}}
    // {{{ BitMessage::sendMessage()
    bool BitMessage::sendMessage(const string strTo, const string strFrom, const string strSubject, const string strMessage, string &strError)
    {
      bool bResult = false;
      xmlrpc_client *ptClient;

      xmlrpc_client_create(&m_env, XMLRPC_CLIENT_NO_FLAGS, "BitMessage", "1.0", NULL, 0, &ptClient);
      if (!m_env.fault_occurred)
      {
        string strEncodedSubject, strEncodedMessage;
        xmlrpc_value *ptEncodedMessage, *ptEncodedSubject, *ptFrom, *ptParam = xmlrpc_array_new(&m_env), *ptResult, *ptTo;
        ptTo = xmlrpc_string_new(&m_env, strTo.c_str());
        xmlrpc_array_append_item(&m_env, ptParam, ptTo);
        xmlrpc_DECREF(ptTo);
        ptFrom = xmlrpc_string_new(&m_env, strFrom.c_str());
        xmlrpc_array_append_item(&m_env, ptParam, ptFrom);
        xmlrpc_DECREF(ptFrom);
        encodeBase64(strSubject, strEncodedSubject);
        ptEncodedSubject = xmlrpc_string_new(&m_env, strEncodedSubject.c_str());
        xmlrpc_array_append_item(&m_env, ptParam, ptEncodedSubject);
        xmlrpc_DECREF(ptEncodedSubject);
        encodeBase64(strMessage, strEncodedMessage);
        ptEncodedMessage = xmlrpc_string_new(&m_env, strEncodedMessage.c_str());
        xmlrpc_array_append_item(&m_env, ptParam, ptEncodedMessage);
        xmlrpc_DECREF(ptEncodedMessage);
        xmlrpc_client_call2(&m_env, ptClient, m_pServerInfo, "sendMessage", ptParam, &ptResult);
        xmlrpc_DECREF(ptParam);
        if (!m_env.fault_occurred)
        {
          bResult = true;
          xmlrpc_DECREF(ptResult);
        }
        else
        {
          stringstream ssError;
          ssError << "XML-RPC Error: " <<  m_env.fault_string << " (" << m_env.fault_code << ")";
          strError = ssError.str();
        }
        xmlrpc_client_destroy(ptClient);
      }
      else
      {
        stringstream ssError;
        ssError << "XML-RPC Error: " <<  m_env.fault_string << " (" << m_env.fault_code << ")";
        strError = ssError.str();
      }

      return bResult;
    }
    // }}}
    // {{{ BitMessage::setCredentials()
    void BitMessage::setCredentials(const string strUser, const string strPassword, const string strServer, const string strPort)
    {
      stringstream ssUrl;

      ssUrl << "http://" << strServer << ":" << strPort << "/";
      m_strUrl = ssUrl.str();
      m_pServerInfo = xmlrpc_server_info_new(&m_env, m_strUrl.c_str());
      xmlrpc_server_info_set_user(&m_env, m_pServerInfo, strUser.c_str(), strPassword.c_str());
      xmlrpc_server_info_set_basic_auth(&m_env, m_pServerInfo, strUser.c_str(), strPassword.c_str());
    }
    // }}}
    // {{{ BitMessage::trashMessage()
    void BitMessage::trashMessage(const string strMsgID)
    {
      m_trash.push_back(strMsgID);
    }
    // }}}
    // {{{ BitMessage::trashMessages()
    bool BitMessage::trashMessages()
    {
      bool bResult = false;
      xmlrpc_client *ptClient;

      xmlrpc_client_create(&m_env, XMLRPC_CLIENT_NO_FLAGS, "BitMessage", "1.0", NULL, 0, &ptClient);
      if (!m_env.fault_occurred)
      {
        for (list<string>::iterator i = m_trash.begin(); i != m_trash.end(); i++)
        {
          xmlrpc_value *ptA, *ptParam = xmlrpc_array_new(&m_env), *ptResult;
          ptA = xmlrpc_string_new(&m_env, i->c_str());
          xmlrpc_array_append_item(&m_env, ptParam, ptA);
          xmlrpc_DECREF(ptA);
          xmlrpc_client_call2(&m_env, ptClient, m_pServerInfo, "trashMessage", ptParam, &ptResult);
          xmlrpc_DECREF(ptParam);
          if (!m_env.fault_occurred)
          {
            xmlrpc_DECREF(ptResult);
          }
        }
        xmlrpc_client_destroy(ptClient);
      }
      m_trash.clear();

      return bResult;
    }
    // }}}
    // {{{ BitMessage::untrashMessage()
    void BitMessage::untrashMessage(const string strMsgID)
    {
      m_trash.remove(strMsgID);
    }
    // }}}
  }
}
