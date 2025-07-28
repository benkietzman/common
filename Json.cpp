/* -*- c++ -*- */
///////////////////////////////////////////
// JSON
// -------------------------------------
// file       : Json.cpp
// author     : Ben Kietzman
// begin      : 2011-07-06
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
/*! \file Json.cpp
* \brief Json Functions
*/
// {{{ includes
#include "Json"
// }}}
extern "C++"
{
  namespace common
  {
    unsigned long long gullUniqueJson = 0;
    // {{{ jsonData()
    void jsonData(void *data, const XML_Char *s, int len)
    {
      string strValue(s, len);
      Json *ptJson = (Json *)data;
      ptJson->m_ptCurrent->v.append(strValue);
    }
    // }}}
    // {{{ jsonEnd()
    void jsonEnd(void *data, const char *el)
    {
      Json *ptJson = (Json *)data;
      if (ptJson->m_ptCurrent->m_ptParent != NULL && !ptJson->m_ptCurrent->m_ptParent->l.empty())
      {
        ptJson->m_ptCurrent = ptJson->m_ptCurrent->m_ptParent->m_ptParent;
      }
      else
      {
        ptJson->m_ptCurrent = ptJson->m_ptCurrent->m_ptParent;
      }
    }
    // }}}
    // {{{ jsonStart()
    void jsonStart(void *data, const char *el, const char *attr[])
    {
      Json *ptJson = (Json *)data;
      if (ptJson->m_ptCurrent->m.find(el) != ptJson->m_ptCurrent->m.end())
      {
        Json *ptSubJson;
        if (ptJson->m_ptCurrent->m[el]->l.empty())
        {
          ptSubJson = ptJson->m_ptCurrent->m[el];
          ptJson->m_ptCurrent->m[el] = new Json;
          ptSubJson->m_ptParent = ptJson->m_ptCurrent->m[el];
          ptJson->m_ptCurrent->m[el]->m_ptParent = ptJson->m_ptCurrent;
          ptJson->m_ptCurrent->m[el]->push_back(ptSubJson);
          delete ptSubJson;
        }
        ptSubJson = new Json;
        ptSubJson->m_strTag = el;
        for (int i = 0; attr[i]; i += 2)
        {
          ptSubJson->a[attr[i]] = attr[i+1];
        }
        ptSubJson->m_ptParent = ptJson->m_ptCurrent->m[el];
        ptJson->m_ptCurrent->m[el]->push_back(ptSubJson);
        delete ptSubJson;
        ptJson->m_ptCurrent = ptJson->m_ptCurrent->m[el]->l.back();
      }
      else
      {
        Json *ptSubJson = new Json;
        ptJson->m_ptCurrent->insert(el, ptSubJson);
        delete ptSubJson;
        for (int i = 0; attr[i]; i += 2)
        {
          ptJson->m_ptCurrent->m[el]->a[attr[i]] = attr[i+1];
        }
        ptJson->m_ptCurrent->m[el]->m_ptParent = ptJson->m_ptCurrent;
        ptJson->m_ptCurrent = ptJson->m_ptCurrent->m[el];
      }
    }
    // }}}
    // {{{ parseJson()
    void parseJson(json_t *current, json_t *end, map<string, string> &jsonMap, const bool bOverwrite)
    {
      bool bParsed = false;

      while (!bParsed)
      {
        if (current->child != NULL)
        {
          if (current->child->child != NULL)
          {
            parseJson(current->child, current->child_end, jsonMap, bOverwrite);
          }
          else if ((current->type == JSON_STRING || current->type == JSON_NUMBER) && current->text != NULL && (current->child->type == JSON_STRING || current->child->type == JSON_NUMBER) && current->child->text != NULL)
          {
            char *pszValue;
            size_t nPosition = 0;
            string strValue = current->child->text;
            stringstream ssKey;
            if (!bOverwrite && jsonMap.find(current->text) != jsonMap.end())
            {
              ssKey << setw(32) << setfill('0') << gullUniqueJson++ << "#";
            }
            pszValue = json_unescape(current->text);
            ssKey << pszValue;
            free(pszValue);
            while ((nPosition = strValue.find("<newline>", 0)) != string::npos)
            {
              strValue.replace(nPosition, 9, "\n");
            }
            pszValue = json_unescape((char *)strValue.c_str());
            jsonMap[ssKey.str()] = pszValue;
            free(pszValue);
          }
        }
        if (current == end)
        {
          bParsed = true;
        }
        else
        {
          current = current->next;
        }
      }
    }
    // }}}
    // {{{ Json::Json()
    Json::Json(json_t *root) : t('s'), m_ptCurrent(this), m_ptParent(NULL)
    {
      Json();
      parse(root);
    }
    Json::Json(list<string> jsonList) : t('s'), m_ptCurrent(this), m_ptParent(NULL)
    {
      Json();
      insert(jsonList);
    }
    Json::Json(map<string, string> jsonMap) : t('s'), m_ptCurrent(this), m_ptParent(NULL)
    {
      Json();
      insert(jsonMap);
    }
    Json::Json(const string strJson) : t('s'), m_ptCurrent(this), m_ptParent(NULL)
    {
      Json();
      parse(strJson);
    }
    Json::Json(Json *ptJson) : t('s'), m_ptCurrent(this), m_ptParent(NULL)
    {
      Json();
      merge(ptJson, true, false);
    }
    // }}}
    // {{{ Json::~Json()
    Json::~Json()
    {
      for (list<Json *>::iterator i = l.begin(); i != l.end(); i++)
      {
        delete *i;
      }
      l.clear();
      for (map<string, Json *>::iterator i = m.begin(); i != m.end(); i++)
      {
        delete i->second;
      }
      m.clear();
      v.clear();
    }
    // }}}
    // {{{ Json::clear()
    void Json::clear()
    {
      clearError();
      clearList();
      clearMap();
      clearValue();
    }
    // }}}
    // {{{ Json::clearError()
    void Json::clearError()
    {
      m_strError.clear();
    }
    // }}}
    // {{{ Json::clearList()
    void Json::clearList()
    {
      if (!l.empty())
      {
        for (list<Json *>::iterator i = l.begin(); i != l.end(); i++)
        {
          delete (*i);
        }
        l.clear();
      }
    }
    // }}}
    // {{{ Json::clearMap()
    void Json::clearMap()
    {
      if (!m.empty())
      {
        for (map<string, Json *>::iterator i = m.begin(); i != m.end(); i++)
        {
          delete i->second;
        }
        m.clear();
      }
    }
    // }}}
    // {{{ Json::clearValue()
    void Json::clearValue()
    {
      t = 's';
      if (!v.empty())
      {
        v.clear();
      }
    }
    // }}}
    // {{{ Json::clearForList()
    void Json::clearForList()
    {
      clearMap();
      clearValue();
    }
    // }}}
    // {{{ Json::clearForMap()
    void Json::clearForMap(const string strKey)
    {
      clearList();
      clearValue();
      if (m.find(strKey) != m.end())
      {
        delete m[strKey];
      }
    }
    // }}}
    // {{{ Json::clearForValue()
    void Json::clearForValue()
    {
      clearList();
      clearMap();
    }
    // }}}
    // {{{ Json::fcif()
    string Json::fcif(string &strFcif, const bool bAppend)
    {
      string strEncoded;

      if (!bAppend)
      {
        string strData;
        strFcif = "*";
        fcif(strData, true);
        if (!strData.empty() && strData[0] == '<' && strData[strData.size() - 1] == '>')
        {
          strData.erase(0, 1);
          strData.erase(strData.size() - 1, 1);
        }
        strFcif += strData;
      }
      else if (!m.empty()) 
      {
        strFcif += '<';
        for (map<string, Json *>::iterator i = m.begin(); i != m.end(); i++)
        {
          if (!i->second->l.empty())
          {
            for (list<Json *>::iterator j = i->second->l.begin(); j != i->second->l.end(); j++)
            {
              strFcif += fcifEncode(strEncoded, i->first);
              (*j)->fcif(strFcif, true);
            }
          }
          else
          {
            strFcif += fcifEncode(strEncoded, i->first);
            i->second->fcif(strFcif, true);
          }
        }
        strFcif += '>';
      }
      else if (!v.empty())
      {
        strFcif += '=';
        strFcif += fcifEncode(strEncoded, v);
        strFcif += ';';
      }

      return strFcif;
    }
    // }}}
    // {{{ Json::fcifDecode()
    string Json::fcifDecode(string &strDecoded, const string strData)
    {
      list<string> item;

      item.push_back("\\$");
      item.push_back("\\%");
      item.push_back("\\*"); 
      item.push_back("\\;");
      item.push_back("\\<");
      item.push_back("\\=");
      item.push_back("\\>");
      strDecoded = strData;
      while (!item.empty())
      {
        size_t nPosition;
        while ((nPosition = strDecoded.find(item.front())) != string::npos)
        {
          strDecoded.erase(nPosition, 1);
        }
        item.pop_front();
      }

      return strDecoded;
    }
    // }}}
    // {{{ Json::fcifEncode()
    string Json::fcifEncode(string &strEncoded, const string strData)
    {
      list<string> item;

      item.push_back("$");
      item.push_back("%");
      item.push_back("*");
      item.push_back(";");
      item.push_back("<");
      item.push_back("=");
      item.push_back(">");
      strEncoded = strData;
      while (!item.empty())
      {
        size_t nPosition = 0;
        while ((nPosition = strEncoded.find(item.front(), nPosition)) != string::npos)
        {
          strEncoded.insert(nPosition, "\\");
          nPosition += 2;
        }
        item.pop_front();
      }

      return strEncoded;
    }
    // }}}
    // {{{ Json::flatten()
    void Json::flatten(list<string> &jsonList)
    {
      for (list<Json *>::iterator i = l.begin(); i != l.end(); i++)
      {
        if (!(*i)->v.empty())
        {
          jsonList.push_back((*i)->v);
        }
      }
    }
    void Json::flatten(map<string, string> &jsonMap, const bool bOverwrite, const bool bMultiLayer)
    {
      for (map<string, Json *>::iterator i = m.begin(); i != m.end(); i++)
      {
        if (i->second->l.empty() && i->second->m.empty())
        {
          stringstream ssKey;
          if (!bOverwrite && jsonMap.find(i->first) != jsonMap.end())
          {
            ssKey << setw(32) << setfill('0') << gullUniqueJson++ << "#";
          }
          ssKey << i->first;
          jsonMap[ssKey.str()] = i->second->v;
          ssKey.clear();
        }
        else if (bMultiLayer && !i->second->m.empty())
        {
          i->second->flatten(jsonMap, bOverwrite, bMultiLayer);
        }
      }
    }
    // }}}
    // {{{ Json::getError()
    string Json::getError()
    {
      return m_strError;
    }
    // }}}
    // {{{ Json::good()
    bool Json::good()
    {
      return m_strError.empty();
    }
    // }}}
    // {{{ Json::i()
    void Json::i(list<string> jsonList)
    {
      insert(jsonList);
    }
    void Json::i(map<string, string> jsonMap)
    {
      insert(jsonMap);
    }
    void Json::i(const string strKey, list<string> jsonList)
    {
      insert(strKey, jsonList);
    }
    void Json::i(const string strKey, map<string, string> jsonMap)
    {
      insert(strKey, jsonMap);
    }
    void Json::i(const string strKey, const string strValue, const char cType)
    {
      insert(strKey, strValue, cType);
    }
    void Json::i(const string strKey, Json *ptJson)
    {
      insert(strKey, ptJson);
    }
    void Json::i(const string strValue, const char cType)
    {
      insert(strValue, cType);
    }
    // }}}
    // {{{ Json::insert()
    void Json::insert(list<string> jsonList)
    {
      for (list<string>::iterator i = jsonList.begin(); i != jsonList.end(); i++)
      {
        push_back(*i);
      }
    }
    void Json::insert(map<string, string> jsonMap)
    {
      for (map<string, string>::iterator i = jsonMap.begin(); i != jsonMap.end(); i++)
      {
        insert(i->first, i->second);
      }
    }
    void Json::insert(const string strKey, list<string> jsonList)
    {
      clearForMap(strKey);
      m[strKey] = new Json(jsonList);
    }
    void Json::insert(const string strKey, map<string, string> jsonMap)
    {
      clearForMap(strKey);
      m[strKey] = new Json(jsonMap);
    }
    void Json::insert(const string strKey, const string strValue, const char cType)
    {
      Json *ptJson = new Json;
      clearForMap(strKey);
      ptJson->m_ptParent = this;
      ptJson->m_strTag = strKey;
      ptJson->value(strValue, cType);
      m[strKey] = ptJson;
    }
    void Json::insert(const string strKey, Json *ptJson)
    {
      Json *ptSubJson = new Json(ptJson);
      clearForMap(strKey);
      ptSubJson->m_ptParent = this;
      ptSubJson->m_strTag = strKey;
      m[strKey] = ptSubJson;
    }
    void Json::insert(const string strValue, const char cType)
    {
      if (!v.empty())
      {
        value(strValue, cType);
      }
      else
      {
        push_back(strValue, cType);
      }
    }
    // }}}
    // {{{ Json::j()
    string Json::j(string &strJson, const bool bAppend)
    {
      return json(strJson, bAppend);
    }
    // }}}
    // {{{ Json::json()
    string Json::json(string &strJson, const bool bAppend)
    {
      stringstream os;

      if (!bAppend)
      {
        strJson.clear();
      }
      json(os);
      strJson = os.str();

      return strJson;
    }
    ostream &Json::json(ostream &os)
    {
      char *pszValue;

      if (!l.empty())
      {
        os << "[";
        for (list<Json *>::iterator i = l.begin(); i != l.end(); i++)
        {
          if (i != l.begin())
          {
            os << ",";
          }
          os << (*i);
        }
        os << "]";
      }
      else if (!m.empty())
      {
        os << "{";
        for (map<string, Json *>::iterator i = m.begin(); i != m.end(); i++)
        {
          if (i != m.begin())
          {
            os << ",";
          }
          os << "\"";
          if (!i->first.empty())
          {
            pszValue = json_escape((char *)i->first.c_str());
            os << pszValue;
            free(pszValue);
          }
          os << "\":";
          os << i->second;
        }
        os << "}";
      }
      else
      {
        if (t == '\0')
        {
          os << "null";
        }
        else if (t == '0')
        {
          os << "false";
        }
        else if (t == '1')
        {
          os << "true";
        }
        else
        {
          if (t != 'n')
          {
            os << "\"";
          }
          if (!v.empty())
          {
            pszValue = json_escape((char *)v.c_str());
            os << pszValue;
            free(pszValue);
          }
          if (t != 'n')
          {
            os << "\"";
          }
        }
      }

      return os;
    }
    // }}}
    // {{{ Json::match()
    bool Json::match(Json *ptJson)
    {
      bool bMatch = false;
 
      if (!ptJson->m.empty() && !m.empty())
      {
        bool bFoundMap[2] = {false, false};
        for (map<string, Json *>::iterator i = m.begin(); i != m.end(); i++)
        {
          if (!i->second->m.empty())
          {
            bFoundMap[0] = true;
          }
        }
        for (map<string, Json *>::iterator i = ptJson->m.begin(); i != ptJson->m.end(); i++)
        {
          if (!i->second->m.empty())
          {
            bFoundMap[1] = true;
          }
        }
        if (bFoundMap[0] && bFoundMap[1])
        {
          for (map<string, Json *>::iterator i = ptJson->m.begin(); i != ptJson->m.end(); i++)
          {
            if (m.find(i->first) != m.end())
            {
              bMatch = m[i->first]->match(i->second);
            }
          }
        }
        else
        {
          bMatch = true;
        }
      }
      else if (ptJson->l.empty() && l.empty())
      {
        bMatch = true;
      }
 
      return bMatch;
    }
    // }}}
    // {{{ Json::merge()
    void Json::merge(Json *ptJson, const bool bOverwrite, const bool bMatch)
    {
      // {{{ list
      if (!ptJson->l.empty())
      {
        a = ptJson->a;
        if (bOverwrite)
        {
          clearForList();
        }
        if (m.empty() && v.empty())
        {
          for (list<Json *>::iterator i = ptJson->l.begin(); i != ptJson->l.end(); i++)
          {
            bool bFoundMatch = false;
            if (bMatch)
            {
              for (list<Json *>::iterator j = l.begin(); j != l.end(); j++)
              {
                if ((*j)->match((*i)))
                {
                  bFoundMatch = true;
                  (*j)->merge((*i), bOverwrite, bMatch);
                }
              }
            }
            if (!bFoundMatch)
            {
              push_back(*i);
            }
          }
        }
      }
      // }}}
      // {{{ map
      else if (!ptJson->m.empty())
      {
        a = ptJson->a;
        if (bOverwrite)
        {
          clearForMap();
        }
        if (l.empty() && v.empty())
        {
          for (map<string, Json *>::iterator i = ptJson->m.begin(); i != ptJson->m.end(); i++)
          {
            if (m.find(i->first) == m.end() || bOverwrite)
            {
              insert(i->first, i->second);
            }
            else
            {
              m[i->first]->merge(i->second, bOverwrite, bMatch);
            }
          }
        }
      }
      // }}}
      // {{{ value
      else
      {
        if (bOverwrite)
        {
          clearForValue();
        }
        if (l.empty() && m.empty())
        {
          if (v.empty() || bOverwrite)
          {
            a = ptJson->a;
            value(ptJson->v, ptJson->t);
          }
        }
      }
      // }}}
    }
    // }}}
    // {{{ Json::parse()
    void Json::parse(json_t *root)
    {
      if (root->child != NULL)
      {
        bool bParsed = false;
        json_t *current = root->child, *end = root->child_end;
        while (!bParsed)
        {
          char *pszValue;
          Json *ptJson;
          if (root->type == JSON_ARRAY)
          {
            if (current->type == JSON_ARRAY || current->type == JSON_OBJECT)
            {
              ptJson = new Json;
              ptJson->m_ptParent = this;
              ptJson->parse(current);
              push_back(ptJson);
              delete ptJson;
            }
            else if (current->type == JSON_FALSE || current->type == JSON_NULL || current->type == JSON_TRUE || ((current->type == JSON_NUMBER || current->type == JSON_STRING) && current->text != NULL))
            {
              string strValue;
              ptJson = new Json;
              ptJson->m_ptParent = this;
              ptJson->m_strTag = m_strTag;
              if (current->type == JSON_FALSE)
              {
                ptJson->value("0", '0');
              }
              else if (current->type == JSON_NULL)
              {
                ptJson->value("", '\0');
              }
              else if (current->type == JSON_NUMBER)
              {
                pszValue = json_unescape(current->text);
                ptJson->value(pszValue, 'n');
                free(pszValue);
              }
              else if (current->type == JSON_TRUE)
              {
                ptJson->value("1", '1');
              }
              else
              {
                pszValue = json_unescape(current->text);
                ptJson->value(pszValue, 's');
                free(pszValue);
              }
              push_back(ptJson);
              delete ptJson;
            }
          }
          else if ((current->type == JSON_NUMBER || current->type == JSON_STRING) && current->text != NULL && current->child != NULL)
          {
            string strKey;
            pszValue = json_unescape(current->text);
            strKey = pszValue;
            free(pszValue);
            if (current->child->type == JSON_ARRAY || current->child->type == JSON_OBJECT)
            {
              ptJson = new Json;
              ptJson->parse(current->child);
              insert(strKey, ptJson);
              delete ptJson;
            }
            else if (current->child->type == JSON_FALSE || current->child->type == JSON_NULL || current->child->type == JSON_TRUE || ((current->child->type == JSON_NUMBER || current->child->type == JSON_STRING) && current->child->text != NULL))
            {
              string strValue;
              ptJson = new Json;
              if (current->child->type == JSON_FALSE)
              {
                ptJson->value("0", '0');
              }
              else if (current->child->type == JSON_NULL)
              {
                ptJson->value("", '\0');
              }
              else if (current->child->type == JSON_NUMBER)
              {
                pszValue = json_unescape(current->child->text);
                ptJson->value(pszValue, 'n');
                free(pszValue);
              }
              else if (current->child->type == JSON_TRUE)
              {
                ptJson->value("1", '1');
              }
              else
              {
                pszValue = json_unescape(current->child->text);
                ptJson->value(pszValue, 's');
                free(pszValue);
              }
              insert(strKey, ptJson);
              delete ptJson;
            }
          }
          if (current == end)
          {
            bParsed = true;
          }
          else
          {
            current = current->next;
          }
        }
      }
    }
    bool Json::parse(const string strJson)
    {
      bool bDone, bResult = false;
      string strTrim = strJson;

      bDone = false;
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
      bDone = false;
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
      if (!strTrim.empty())
      {
        size_t nPosition[2] = {0, 0};
        if (strTrim[0] == '<')
        {
          int nNumber;
          string strLine;
          XML_Parser parser = XML_ParserCreate(NULL);
          m_ptCurrent = this;
          m_ptParent = NULL;
          XML_SetUserData(parser, this);
          XML_SetElementHandler(parser, jsonStart, jsonEnd);
          XML_SetCharacterDataHandler(parser, jsonData);
          while (nPosition[0] < strTrim.size() && (nPosition[0] = strTrim.find("&#", nPosition[0])) != string::npos)
          {
            if (strTrim[nPosition[0] + 3] == ';')
            {
              strTrim.erase(nPosition[0], 4);
              nPosition[0]--;
            }
            else if (strTrim[nPosition[0] + 4] == ';')
            {
              nNumber = atoi(strTrim.substr(nPosition[0] + 2, 2).c_str());
              if (nNumber != 10)
              {
                strTrim.erase(nPosition[0], 5);
                nPosition[0]--;
              }
            }
            else if (strTrim[nPosition[0] + 5] == ';')
            {
              nNumber = atoi(strTrim.substr(nPosition[0] + 2, 3).c_str());
              if (nNumber != 10)
              {
                strTrim.erase(nPosition[0], 6);
                nPosition[0]--;
              }
            }
            nPosition[0]++;
          }
          strLine = "<xml>";
          XML_Parse(parser, strLine.c_str(), strLine.size(), 0);
          if (XML_Parse(parser, strTrim.c_str(), strTrim.size(), 0))
          {
            bResult = true;
          }
          else
          {
            setError(XML_ErrorString(XML_GetErrorCode(parser)));
          }
          strLine = "</xml>";
          XML_Parse(parser, strLine.c_str(), strLine.size(), 0);
          XML_Parse(parser, NULL, 0, 1);
          XML_ParserFree(parser);
          if (m.find("xml") != m.end())
          {
            Json *ptJson = new Json(m["xml"]);
            clearMap();
            merge(ptJson, true, false);
            delete ptJson;
          }
        }
        else if (strTrim[0] == '*')
        {
          bResult = true;
          parseFcif(strTrim.substr(1, strTrim.size() - 1));
        }
        else
        {
          enum json_error nReturn;
          json_t *root = NULL;
          strTrim = (string)"{\"json\":" + strTrim + (string)"}";
          if ((nReturn = json_parse_document(&root, (char *)strTrim.c_str())) == JSON_OK && root != NULL)
          {
            bResult = true;
            parse(root);
            if (m.find("json") != m.end())
            {
              Json *ptJson = new Json(m["json"]);
              clearMap();
              merge(ptJson, true, false);
              delete ptJson;
            }
          }
          else
          {
            switch (nReturn)
            {
              case JSON_INCOMPLETE_DOCUMENT : setError("[JSON_INCOMPLETE_DOCUMENT] The parsed document did not end."); break;
              case JSON_WAITING_FOR_EOF     : setError("[JSON_WAITING_FOR_EOF] A complete JSON document tree was already finished but needs to get to EOF."); break;
              case JSON_MALFORMED_DOCUMENT  : setError("[JSON_MALFORMED_DOCUMENT] The JSON document which was fed to this parser is malformed."); break;
              case JSON_INCOMPATIBLE_TYPE   : setError("[JSON_INCOMPATIBLE_TYPE] The currently parsed type does not belong here."); break;
              case JSON_MEMORY              : setError("[JSON_MEMORY] An error occurred when allocating memory."); break;
              case JSON_ILLEGAL_CHARACTER   : setError("[JSON_ILLEGAL_CHARACTER] The currently parsed character does not belong here."); break;
              case JSON_BAD_TREE_STRUCTURE  : setError("[JSON_BAD_TREE_STRUCTURE] The document tree structure is malformed."); break;
              case JSON_MAXIMUM_LENGTH      : setError("[JSON_MAXIMUM_LENGTH] The parsed string reached the maximum allowed size."); break;
              case JSON_UNKNOWN_PROBLEM     : setError("[JSON_UNKNOWN_PROBLEM] Some random problem occurred."); break;
              default                       : setError("Encountered an unknown error.");
            }
          }
          if (root != NULL)
          {
            json_free_value(&root);
          }
        }
      }
      else
      {
        setError("No input provided.");
      }

      return bResult;
    }
    // }}}
    // {{{ Json::parseFcif()
    void Json::parseFcif(const string strFcif)
    {
      bool bText = false, bValue = false;
      char cPrevChar = '\0';
      unsigned int unInMap = 0;
      string strData, strKey, strMap, strValue;

      for (size_t i = 0; i < strFcif.size(); i++)
      {
        if (strFcif[i] == '<' && cPrevChar != '\\')
        {
          if (unInMap > 0)
          {
            strData += strFcif[i];
          }
          else if (unInMap == 0 && bText)
          {
            fcifDecode(strMap, strKey);
          }
          bText = bValue = false;
          strKey.clear();
          strValue.clear();
          unInMap++;
        }
        else if (unInMap > 0 && strFcif[i] == '>' && cPrevChar != '\\' && --unInMap == 0)
        {
          bText = bValue = false;
          if (m.find(strMap) != m.end())
          {
            Json *ptData;
            if (m[strMap]->l.empty())
            {
              ptData = m[strMap];
              m[strMap] = new Json;
              m[strMap]->l.push_back(ptData);
            }
            ptData = new Json;
            ptData->parseFcif(strData);
            m[strMap]->l.push_back(ptData);
          }
          else
          {
            m[strMap] = new Json;
            m[strMap]->parseFcif(strData);
          }
          strData.clear();
          strKey.clear();
          strMap.clear();
          strValue.clear();
        }
        else if (unInMap > 0)
        {
          bText = bValue = false;
          strData += strFcif[i];
        }
        else if (bText && strFcif[i] == '=' && cPrevChar != '\\')
        {
          bValue = true;
        }
        else if (bText && bValue && strFcif[i] == ';' && cPrevChar != '\\')
        {
          string strDecodedKey, strDecodedValue;
          bText = bValue = false;
          fcifDecode(strDecodedKey, strKey);
          fcifDecode(strDecodedValue, strValue);
          if (m.find(strDecodedKey) != m.end())
          {
            if (!m[strDecodedKey]->v.empty())
            {
              string strOldValue = m[strDecodedKey]->v;
              m[strDecodedKey]->clearForList();
              m[strDecodedKey]->push_back(strOldValue);
            }
            m[strDecodedKey]->push_back(strDecodedValue);
          }
          else
          {
            insert(strDecodedKey, strDecodedValue);
          }
          strKey.clear();
          strValue.clear();
        }
        else if (bText && bValue)
        {
          if (strFcif[i] == '\\')
          {
            strValue.append(1, strFcif[i++]);
          }
          strValue.append(1, strFcif[i]);
        }
        else
        {
          bText = true;
          if (strFcif[i] == '\\')
          {
            strKey.append(1, strFcif[i++]);
          }
          strKey.append(1, strFcif[i]);
        }
        cPrevChar = strFcif[i];
      }
    }
    // }}}
    // {{{ Json::pb()
    void Json::pb(list<string> jsonList)
    {
      push_back(jsonList);
    }
    void Json::pb(map<string, string> jsonMap)
    {
      push_back(jsonMap);
    }
    void Json::pb(const string strValue, const char cType)
    {
      push_back(strValue, cType);
    }
    void Json::pb(Json *ptJson)
    {
      push_back(ptJson);
    }
    // }}}
    // {{{ Json::push_back()
    void Json::push_back(list<string> jsonList)
    {
      Json *ptJson = new Json(jsonList);
      push_back(ptJson);
      delete ptJson;
    }
    void Json::push_back(map<string, string> jsonMap)
    {
      Json *ptJson = new Json(jsonMap);
      push_back(ptJson);
      delete ptJson;
    }
    void Json::push_back(const string strValue, const char cType)
    {
      Json *ptJson = new Json;
      ptJson->value(strValue, cType);
      push_back(ptJson);
      delete ptJson;
    }
    void Json::push_back(Json *ptJson)
    {
      Json *ptSubJson = new Json(ptJson);
      clearForList();
      ptSubJson->m_ptParent = this;
      ptSubJson->m_strTag = m_strTag;
      l.push_back(ptSubJson);
    }
    // }}}
    // {{{ Json::pf()
    void Json::pf(list<string> jsonList)
    {
      push_front(jsonList);
    }
    void Json::pf(map<string, string> jsonMap)
    {
      push_front(jsonMap);
    }
    void Json::pf(const string strValue, const char cType)
    {
      push_front(strValue, cType);
    }
    void Json::pf(Json *ptJson)
    {
      push_front(ptJson);
    }
    // }}}
    // {{{ Json::push_front()
    void Json::push_front(list<string> jsonList)
    {
      Json *ptJson = new Json(jsonList);
      push_front(ptJson);
      delete ptJson;
    }
    void Json::push_front(map<string, string> jsonMap)
    {
      Json *ptJson = new Json(jsonMap);
      push_front(ptJson);
      delete ptJson;
    }
    void Json::push_front(const string strValue, const char cType)
    {
      Json *ptJson = new Json;
      ptJson->value(strValue, cType);
      push_front(ptJson);
      delete ptJson;
    }
    void Json::push_front(Json *ptJson)
    {
      Json *ptSubJson = new Json(ptJson);
      clearForList();
      ptSubJson->m_ptParent = this;
      ptSubJson->m_strTag = m_strTag;
      l.push_front(ptSubJson);
    }
    // }}}
    // {{{ Json::setError()
    void Json::setError(const string strError)
    {
      m_strError = strError;
    }
    // }}}
    // {{{ Json::value()
    void Json::value(const string strValue, const char cType)
    {
      clearForValue();
      t = cType;
      v = strValue;
    }
    // }}}
    // {{{ Json::x()
    string Json::x(string &strXml, const bool bAppend)
    {
      return xml(strXml, bAppend);
    }
    // }}}
    // {{{ Json::xml()
    string Json::xml(string &strXml, const bool bAppend)
    {
      if (!bAppend)
      {
        strXml.clear();
      }
      if (!m.empty())
      {
        for (map<string, Json *>::iterator i = m.begin(); i != m.end(); i++)
        {
          string strTag;
          if (!i->first.empty())
          {
            strTag = i->first;
          }
          if (!i->second->l.empty())
          {
            i->second->xml(strXml, strTag);
          }
          else
          {
            strXml += (string)"<" + strTag;
            for (map<string, string>::iterator j = i->second->a.begin(); j != i->second->a.end(); j++)
            {
              strXml += (string)" " + j->first + (string)"=\"" + j->second + (string)"\"";
            }
            strXml += (string)">";
            i->second->xml(strXml, true);
            strXml += (string)"</" + strTag + (string)">";
          }
        }
      }
      else if (!v.empty())
      {
        string strEncoded;
        strXml += xmlEncode(strEncoded, v);
      }

      return strXml;
    }
    string Json::xml(string &strXml, const string strTag)
    {
      for (list<Json *>::iterator i = l.begin(); i != l.end(); i++)
      {
        strXml += (string)"<" + strTag;
        for (map<string, string>::iterator j = (*i)->a.begin(); j != (*i)->a.end(); j++)
        {
          strXml += (string)" " + j->first + (string)"=\"" + j->second + (string)"\"";
        }
        strXml += (string)">";
        (*i)->xml(strXml, true);
        strXml += (string)"</" + strTag + (string)">";
      }

      return strXml;
    }
    // }}}
    // {{{ Json::xmlEncode()
    string Json::xmlEncode(string &strEncoded, const string strData)
    {
      strEncoded.clear();
      for (size_t i = 0; i < strData.size(); i++)
      {
        switch (strData[i])
        {
          case '"'  : strEncoded += "&quot;";   break;
          case '&'  : strEncoded += "&amp;";    break;
          case '\'' : strEncoded += "&apos;";   break;
          case '<'  : strEncoded += "&lt;";     break;
          case '>'  : strEncoded += "&gt;";     break;
          default   : strEncoded += strData[i]; break;
        }
      }

      return strEncoded;
    }
    // }}}
    // {{{ operator<<()
    ostream &operator<<(ostream &os, Json &tJson)
    {
      return tJson.json(os);
    }
    ostream &operator<<(ostream &os, Json *ptJson)
    {
      return ptJson->json(os);
    }
    // }}}
  }
}
