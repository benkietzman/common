/* -*- c++ -*- */
///////////////////////////////////////////
// Terminal
// -------------------------------------
// file       : Terminal.cpp
// author     : Ben Kietzman
// begin      : 2014-03-20
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
/*! \file Terminal.cpp
* \brief Terminal Class
*/
// {{{ includes
#include "Terminal"
// }}}
extern "C++"
{
  namespace common
  {
    // {{{ Terminal()
    Terminal::Terminal()
    {
      m_bCaught = m_bDebug = m_bNumbered = m_bUncaught = m_bWrap = false;
      m_bWait = true;
      m_fdSocket = -1;
      m_unSaveRow = m_unSaveCol = 0;
      m_unRow = 0;
      m_unCol = 0;
      m_unRows = 24;
      m_unCols = 80;
      m_nSocketTimeout[0] = 500;
      m_nSocketTimeout[1] = 15000;
      m_strType = "vt220";
      rows(m_unRows);
      cols(m_unCols);
      row(0);
      col(0);
      m_posDebug = &cout;
      m_telopts.push_back("BINARY");
      m_telopts.push_back("ECHO");
      m_telopts.push_back("RCP");
      m_telopts.push_back("SUPPRESS GO AHEAD");
      m_telopts.push_back("NAME");
      m_telopts.push_back("STATUS");
      m_telopts.push_back("TIMING MARK");
      m_telopts.push_back("RCTE");
      m_telopts.push_back("NAOL");
      m_telopts.push_back("NAOP");
      m_telopts.push_back("NAOCRD");
      m_telopts.push_back("NAOHTS");
      m_telopts.push_back("NAOHTD");
      m_telopts.push_back("NAOFFD");
      m_telopts.push_back("NAOVTS");
      m_telopts.push_back("NAOVTD");
      m_telopts.push_back("NAOLFD");
      m_telopts.push_back("EXTEND ASCII");
      m_telopts.push_back("LOGOUT");
      m_telopts.push_back("BYTE MACRO");
      m_telopts.push_back("DATA ENTRY TERMINAL");
      m_telopts.push_back("SUPDUP");
      m_telopts.push_back("SUPDUP OUTPUT");
      m_telopts.push_back("SEND LOCATION");
      m_telopts.push_back("TERMINAL TYPE");
      m_telopts.push_back("END OF RECORD");
      m_telopts.push_back("TACACS UID");
      m_telopts.push_back("OUTPUT MARKING");
      m_telopts.push_back("TTYLOC");
      m_telopts.push_back("3270 REGIME");
      m_telopts.push_back("X.3 PAD");
      m_telopts.push_back("NAWS");
      m_telopts.push_back("TSPEED");
      m_telopts.push_back("LFLOW");
      m_telopts.push_back("LINEMODE");
      m_telopts.push_back("XDISPLOC");
      m_telopts.push_back("OLD-ENVIRON");
      m_telopts.push_back("AUTHENTICATION");
      m_telopts.push_back("ENCRYPT");
      m_telopts.push_back("NEW-ENVIRON");
    }
    // }}}
    // {{{ ~Terminal()
    Terminal::~Terminal()
    {
      for (size_t i = 0; i < rows(); i++)
      {
        m_screen[i].clear();
        m_tab[i].clear();
      }
      m_caught.clear();
      m_screen.clear();
      m_tab.clear();
      m_telopts.clear();
      m_uncaught.clear();
    }
    // }}}
    // {{{ caught()
    bool Terminal::caught()
    {
      return m_bCaught;
    }
    void Terminal::caught(const bool bCaught)
    {
      m_bCaught = bCaught;
    }
    // }}}
    // {{{ col()
    bool Terminal::col(const size_t unCol)
    {
      bool bResult = false;

      if (unCol < cols())
      {
        m_unCol = unCol;
      }
      else
      {
        m_unCol = 0;
      }

      return bResult;
    }
    size_t Terminal::col()
    {
      return m_unCol;
    }
    // }}}
    // {{{ cols()
    bool Terminal::cols(const size_t unCols)
    {
      bool bResult = false;

      if (unCols > 0)
      {
        bResult = true;
        m_unCols = unCols;
        resize();
      }

      return bResult;
    }
    size_t Terminal::cols()
    {
      return m_unCols;
    }
    // }}}
    // {{{ connect()
    bool Terminal::connect(const string strServer, const string strPort)
    {
      bool bResult = true;

      if (m_fdSocket == -1)
      {
        addrinfo hints, *result;
        int nReturn;
        bResult = false;
        memset(&hints, 0, sizeof(addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        if ((nReturn = getaddrinfo(strServer.c_str(), strPort.c_str(), &hints, &result)) == 0)
        {
          bool bSocket = false;
          addrinfo *rp;
          timeval tTimeVal;
          tTimeVal.tv_sec = 10;
          tTimeVal.tv_usec = 0;
          for (rp = result; m_fdSocket == -1 && rp != NULL; rp = rp->ai_next)
          {
            bSocket = false;
            if ((m_fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
            {
              bSocket = true;
              setsockopt(m_fdSocket, SOL_SOCKET, SO_RCVTIMEO, &tTimeVal, sizeof(timeval));
              if (::connect(m_fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
              {
                bResult = true;
                setServerHandle(m_fdSocket);
              }
              else
              {
                close(m_fdSocket);
                m_fdSocket = -1;
              }
            }
          }
          freeaddrinfo(result);
          if (!bResult)
          {
            stringstream ssError;
            ssError << "Terminal::connect(" << strServer << ")->" << ((!bSocket)?"socket":"connect") << "(" << errno << ") error:  " << strerror(errno);
            error(ssError.str());
          }
        }
        else
        {
          stringstream ssError;
          ssError << "Terminal::connect(" << strServer << ")->getaddrinfo(" << nReturn << ") error:  " << gai_strerror(nReturn);
          error(ssError.str());
        }
      }

      return bResult;
    }
    // }}}
    // {{{ connected()
    bool Terminal::connected()
    {
      return ((m_fdSocket != -1)?true:false);
    }
    // }}}
    // {{{ debug()
    bool Terminal::debug()
    {
      return m_bDebug;
    }
    void Terminal::debug(const bool bDebug)
    {
      m_bDebug = bDebug;
    }
    // }}}
    // {{{ disconnect()
    bool Terminal::disconnect()
    {
      bool bResult = true;
  
      if (m_fdSocket != -1)
      {
        bResult = false;
        if (close(m_fdSocket) == 0)
        {
          bResult = true;
          m_fdSocket = -1;
        }
        else
        {
          stringstream ssError;
          ssError << "Terminal::disconnect()->close(" << errno << ") error:  " << strerror(errno);
          error(ssError.str());
        }
      }

      return bResult;
    }
    // }}}
    // {{{ display()
    void Terminal::display()
    {
      if (m_bNumbered)
      {
        string strCols[2];
        (*m_posDebug) << "POSITION:  " << setfill('0') << setw(2) << row() << "," << setfill('0') << setw(2) << col() << endl;
        for (size_t i = 0; i < 8; i++)
        {
          string strChar;
          stringstream ssChar;
          ssChar << i;
          strChar = ssChar.str();
          strCols[0].append(10, strChar[0]);
          strCols[1].append("0123456789");
        }
        (*m_posDebug) << "   " << strCols[0] << endl;
        (*m_posDebug) << "   " << strCols[1] << endl;
        (*m_posDebug) << "   " << setw(cols()) << setfill('-') << '-' << endl;
      }
      for (size_t i = 0; i < m_screen.size(); i++)
      {
        if (m_bNumbered)
        {
          (*m_posDebug)<< setw(2) << setfill('0') << i << '|';
        }
        for (size_t j = 0; j < m_screen[i].size(); j++)
        {
          (*m_posDebug) << m_screen[i][j];
        }
        (*m_posDebug) << endl;
      }
      if (!m_caught.empty())
      {
        if (m_bCaught)
        {
          (*m_posDebug) << "CAUGHT:  ";
          for (list<string>::iterator i = m_caught.begin(); i != m_caught.end(); i++)
          {
            if (i != m_caught.begin())
            {
              (*m_posDebug) << ", ";
            }
            (*m_posDebug) << (*i);
          }
          (*m_posDebug) << endl;
        }
        m_caught.clear();
      }
      if (!m_uncaught.empty())
      {
        if (m_bUncaught)
        {
          (*m_posDebug) << "UNCAUGHT:  ";
          for (list<string>::iterator i = m_uncaught.begin(); i != m_uncaught.end(); i++)
          {
            if (i != m_uncaught.begin())
            {
              (*m_posDebug) << ", ";
            }
            (*m_posDebug) << (*i);
          }
          (*m_posDebug) << endl;
        }
        m_uncaught.clear();
      }
    }
    // }}}
    // {{{ down()
    void Terminal::down(const size_t unCount)
    {
      for (size_t i = 0; i < unCount; i++)
      {
        if (row() < (rows() - 1))
        {
          row(row() + 1);
        }
        else if (m_bWrap)
        {
          row(0);
        }
        else if (row() == (rows() - 1))
        {
          for (size_t j = 1; j < m_screen.size(); j++)
          {
            m_screen[j - 1] = m_screen[j];
          }
          m_screen[m_screen.size() - 1].replace(0, cols(), cols(), ' ');
        }
      }
    }
    // }}}
    // {{{ encode()
    string Terminal::encode(const string strData, string &strEncode)
    {
      size_t unPosition;

      strEncode = strData;
      while ((unPosition = strEncode.find("\033")) != string::npos)
      {
        strEncode.replace(unPosition, 1, "<ESC>");
      }
      while ((unPosition = strEncode.find("\n")) != string::npos)
      {
        strEncode.replace(unPosition, 1, "<NEWLINE>");
      }
      while ((unPosition = strEncode.find("\r")) != string::npos)
      {
        strEncode.replace(unPosition, 1, "<RETURN>");
      }
      while ((unPosition = strEncode.find("\t")) != string::npos)
      {
        strEncode.replace(unPosition, 1, "<TAB>");
      }

      return strEncode;
    }
    // }}}
    // {{{ error()
    string Terminal::error()
    {
      return prefix() + m_strError;
    }
    void Terminal::error(const string strError)
    {
      if (!strError.empty())
      {
        m_strError = strError;
      }
      else
      {
        m_strError.clear();
      }
    }
    void Terminal::error(const string strPrefix, const string strError)
    {
      prefix(strPrefix);
      error(strError);
    }
    // }}}
    // {{{ find()
    bool Terminal::find(const string strData)
    {
      return find(strData, 0, m_screen.size() - 1);
    }
    bool Terminal::find(const string strData, const size_t unStartRow)
    {
      return find(strData, unStartRow, 0, m_screen[unStartRow].size() - 1);
    }
    bool Terminal::find(const string strData, const size_t unStartRow, const size_t unEndRow)
    {
      size_t unCol = 0, unRow = 0;

      return findPos(strData, unStartRow, unEndRow, unRow, unCol);
    }
    bool Terminal::find(const string strData, const size_t unStartRow, const size_t unStartCol, const size_t unEndCol)
    {
      size_t unCol = 0, unRow = 0;

      return findPos(strData, unStartRow, unStartCol, unEndCol, unRow, unCol);
    }
    // }}}
    // {{{ findPos()
    bool Terminal::findPos(const string strData, size_t &unRow, size_t &unCol)
    {
      return findPos(strData, 0, m_screen.size() - 1, unRow, unCol);
    }
    bool Terminal::findPos(const string strData, size_t unStartRow, size_t &unRow,  size_t &unCol)
    {
      return findPos(strData, unStartRow, 0, m_screen[unStartRow].size() - 1, unRow, unCol);
    }
    bool Terminal::findPos(const string strData, const size_t unStartRow, const size_t unEndRow, size_t &unRow, size_t &unCol)
    {
      bool bResult = false;
      size_t unAttempt = 0;

      do
      {
        if (!m_screen.empty())
        {
          size_t unRealStartRow = unStartRow, unRealEndRow = unEndRow;
          if (unStartRow > unEndRow)
          {
            unRealStartRow = unEndRow;
            unRealEndRow = unStartRow;
          }
          if (unRealEndRow >= m_screen.size())
          {
            unRealEndRow = m_screen.size() - 1;
          }
          if (unRealStartRow <= unRealEndRow)
          {
            size_t unPosition;
            for (size_t i = unRealStartRow; !bResult && i <= unRealEndRow; i++)
            {
              if ((unPosition = m_screen[i].find(strData)) != string::npos)
              {
                bResult = true;
                unRow = i;
                unCol = unPosition;
              }
            }
          }
        }
        if (unAttempt < 1)
        {
          wait(false);
        }
      } while (!bResult && unAttempt++ < 1);
      if (!bResult)
      {
        string strEncode;
        error("find()", (string)"Failed to find \"" + encode(strData, strEncode) + (string)"\".");
      }

      return bResult;
    }
    bool Terminal::findPos(const string strData, const size_t unStartRow, const size_t unStartCol, const size_t unEndCol, size_t &unRow, size_t &unCol)
    {
      bool bResult = false;
      size_t unAttempt = 0;

      do
      {
        if (unStartRow < m_screen.size())
        {
          size_t unRealStartCol = unStartCol, unRealEndCol = unEndCol, unPosition;
          if (unStartCol > unEndCol)
          {
            unRealStartCol = unEndCol;
            unRealEndCol = unStartCol;
          }
          if (unRealEndCol >= m_screen[unStartRow].size())
          {
            unRealEndCol = m_screen[unStartRow].size() - 1;
          }
          if (unRealStartCol <= unRealEndCol && (unPosition = m_screen[unStartRow].substr(unRealStartCol, (unRealEndCol + 1) - unRealStartCol).find(strData)) != string::npos)
          {
            bResult = true;
            unRow = unStartRow;
            unCol = unPosition;
          }
        }
        if (unAttempt < 1)
        {
          wait(false);
        }
      } while (!bResult && unAttempt++ < 1);
      if (!bResult)
      {
        string strEncode;
        error("find()", (string)"Failed to find \"" + encode(strData, strEncode) + (string)"\".");
      }

      return bResult;
    }
    // }}}
    // {{{ get()
    bool Terminal::get(string &strData, const size_t unRow)
    {
      bool bResult = false;

      strData = "";
      if (unRow < m_screen.size())
      {
        bResult = true;
        strData = m_screen[unRow];
      }
      if (!bResult)
      {
        error("get()", "Failed to get data from provided positional arguments.");
      }

      return bResult;
    }
    bool Terminal::get(string &strData, const size_t unRow, const size_t unStartCol, const size_t unEndCol)
    {
      bool bResult = false;

      strData = "";
      if (unRow < m_screen.size())
      {
        size_t unRealStartCol = unStartCol, unRealEndCol = unEndCol;
        if (unStartCol > unEndCol)
        {
          unRealStartCol = unEndCol;
          unRealEndCol = unStartCol;
        }
        if (unRealEndCol >= m_screen[unRow].size())
        {
          unRealEndCol = m_screen[unRow].size() - 1;
        }
        if (unRealStartCol <= unRealEndCol)
        {
          bResult = true;
          strData = m_screen[unRow].substr(unRealStartCol, (unRealEndCol + 1) - unRealStartCol);
        }
      }
      if (!bResult)
      {
        error("get()", "Failed to get data from provided positional arguments.");
      }

      return bResult;
    }
    // }}}
    // {{{ getServer()
    bool Terminal::getServer(const string strSystem, string &strServer, string &strPort)
    {
      return false;
    }
    // }}}
    // {{{ getSocketTimeout()
    void Terminal::getSocketTimeout(int &nShort, int &nLong)
    {
      m_mutex.lock();
      nShort = m_nSocketTimeout[0];
      nLong = m_nSocketTimeout[1];
      m_mutex.unlock();
    }
    // }}}
    // {{{ left()
    void Terminal::left(const size_t unCount)
    {
      for (size_t i = 0; i < unCount; i++)
      {
        if (col() > 0)
        {
          col(col() - 1);
        }
        else if (m_bWrap)
        {
          up();
          col(cols() - 1);
        }
      }
    }
    // }}}
    // {{{ login()
    bool Terminal::login(const string strServer, const string strPort, const string strUser, const string strPassword)
    {
      return false; 
    }
    // }}}
    // {{{ logout()
    bool Terminal::logout()
    {
      return false; 
    }
    // }}}
    // {{{ message()
    string Terminal::message()
    {
      return "";
    }
    // }}}
    // {{{ numbered()
    bool Terminal::numbered()
    {
      return m_bNumbered;
    }
    void Terminal::numbered(const bool bNumbered)
    {
      m_bNumbered = bNumbered;
    }
    // }}}
    // {{{ parse()
    void Terminal::parse()
    {
      list<unsigned char> leftOver;
      string strLeftOver;

      while (!m_strBuffer.empty())
      {
        // {{{ terminal escape sequences
        if (m_strBuffer[0] == '\033')
        {
          bool bGood = false;
          if (m_strBuffer.size() > 1)
          {
            bool bCaught = true;
            stringstream ssCode;
            ssCode << "<ESC>";
            if (m_strBuffer[1] == '[' && m_strBuffer.size() > 2)
            {
              char cEnd = '\0';
              size_t unPosition;
              for (size_t i = 2; cEnd == '\0' && i < m_strBuffer.size(); i++)
              {
                if (isalpha(m_strBuffer[i]))
                {
                  unPosition = i;
                  cEnd = m_strBuffer[i];
                }
              }
              if (cEnd != '\0')
              {
                int nFirst = 0, nSecond = 0;
                string strArg;
                vector<int> arg;
                bGood = true;
                for (int i = 1; !m_manip.getToken(strArg, m_strBuffer.substr(2, unPosition - 2), i, ";", false).empty(); i++)
                {
                  if (!strArg.empty())
                  {
                    arg.push_back(atoi(strArg.c_str()));
                  }
                  else
                  {
                    arg.push_back(0);
                  }
                }
                if (arg.size() >= 1)
                {
                  nFirst = arg[0];
                }
                if (arg.size() >= 2)
                {
                  nSecond = arg[1];
                }
                ssCode << "[";
                for (size_t i = 0; i < arg.size(); i++)
                {
                  if (i > 0)
                  {
                    ssCode << ';';
                  }
                  ssCode << arg[i];
                }
                ssCode << cEnd;
                switch (cEnd)
                {
                  case 'A' : up((nFirst > 0)?nFirst:1); break;
                  case 'B' : down((nFirst > 0)?nFirst:1); break;
                  case 'C' : right((nFirst > 0)?nFirst:1); break;
                  case 'D' : left((nFirst > 0)?nFirst:1); break;
                  case 'H' :
                  case 'f' : row(nFirst - 1); col(nSecond - 1); break;
                  case 'J' :
                  {
                    if (nFirst == 0)
                    {
                      for (size_t i = row(); i < m_screen.size(); i++)
                      {
                        m_screen[i].replace(0, cols(), cols(), ' ');
                      }
                    }
                    else if (nFirst == 1)
                    {
                      for (size_t i = 0; i <= row(); i++)
                      {
                        m_screen[i].replace(0, cols(), cols(), ' ');
                      }
                    }
                    else if (nFirst == 2)
                    {
                      for (size_t i = 0; i < m_screen.size(); i++)
                      {
                        m_screen[i].replace(0, cols(), cols(), ' ');
                      }
                    }
                    break;
                  }
                  case 'K' :
                  {
                    if (row() < m_screen.size())
                    {
                      if (nFirst == 0)
                      {
                        if (col() < cols())
                        {
                          m_screen[row()].replace(col(), cols() - col(), cols() - col(), ' ');
                        }
                      }
                      else if (nFirst == 1)
                      {
                        if (col() < cols())
                        {
                          m_screen[row()].replace(0, col() + 1, col() + 1, ' ');
                        }
                      }
                      else if (nFirst == 2)
                      {
                        m_screen[row()].replace(0, cols(), cols(), ' ');
                      }
                    }
                    break;
                  }
                  case 'c' : write("\033[0c"); break;
                  case 'g' :
                  {
                    if (nFirst == 0)
                    {
                      tab(row(), col(), false);
                    }
                    else if (nFirst == 3)
                    {
                      for (size_t i = 0; i < m_tab.size(); i++)
                      {
                        m_tab[i].clear();
                      }
                    }
                    break;
                  }
                  case 'h' :
                  {
                    if (nFirst == 7)
                    {
                      m_bWrap = true;
                    }
                    break;
                  }
                  case 'l' :
                  {
                    if (nFirst == 7)
                    {
                      m_bWrap = false;
                    }
                    break;
                  }
                  case 'n' :
                  {
                    switch (nFirst)
                    {
                      case 5 : write("\033[0n"); break;
                      case 6 :
                      {
                        string strData;
                        stringstream ssData;
                        ssData << "\033[" << (row() + 1) << ";" << (col() + 1) << "R";
                        strData = ssData.str();
                        write(strData);
                        break;
                      }
                    }
                    break;
                  }
                  case 's' : m_unSaveRow = row(); m_unSaveCol = col(); break;
                  case 'u' : row(m_unSaveRow); col(m_unSaveCol); break;
                  default  : bCaught = false;
                }
                arg.clear();
                m_strBuffer.erase(2, (unPosition + 1) - 2);
              }
              else
              {
                bCaught = false;
                for (size_t i = 2; i < m_strBuffer.size(); i++)
                {
                  leftOver.push_back(m_strBuffer[i]);
                }
                m_strBuffer.erase(2, m_strBuffer.size() - 2);
              }
            }
            else
            {
              bGood = true;
              ssCode << m_strBuffer[1];
              switch (m_strBuffer[1])
              {
                case '7' : m_unSaveRow = row(); m_unSaveCol = col(); break;
                case '8' : row(m_unSaveRow); col(m_unSaveCol); break;
                case 'H' : tab(row(), col(), true); break;
                case 'c' : reset(); break;
                default  : bCaught = false;
              }
            }
            if (!bGood)
            {
              leftOver.push_front(m_strBuffer[1]);
            }
            if (bGood)
            {
              if (bCaught && caught())
              {
                m_caught.push_back(ssCode.str());
              }
              else if (!bCaught && uncaught())
              {
                m_uncaught.push_back(ssCode.str());
              }
            }
            m_strBuffer.erase(1, 1);
          }
          else if (!bGood)
          {
            leftOver.push_front(m_strBuffer[0]);
          }
        }
        // }}}
        // {{{ telnet commands
        else if ((unsigned char)m_strBuffer[0] == IAC && m_strBuffer.size() >= 3)
        {
          if ((unsigned char)m_strBuffer[1] == DO && (unsigned char)m_strBuffer[2] == TELOPT_NAWS)
          {
            if (!write((vector<unsigned char>){IAC, WILL, TELOPT_NAWS, IAC, SB, TELOPT_NAWS, 0, (unsigned char)cols(), 0, (unsigned char)rows(), IAC, SE}))
            {
              prefix("parse()");
            }
          }
          else if ((unsigned char)m_strBuffer[1] == SB && (unsigned char)m_strBuffer[2] == TELOPT_TTYPE && m_strBuffer.size() >= 6 && (unsigned char)m_strBuffer[3] == TELQUAL_SEND && (unsigned char)m_strBuffer[4] == IAC && (unsigned char)m_strBuffer[5] == SE)
          {
            string strType = "unknown";
            if (!m_strType.empty())
            {
              strType = m_strType;
            }
            if (!write((vector<unsigned char>){IAC, SB, TELOPT_TTYPE, TELQUAL_IS}) || !write(strType) || !write((vector<unsigned char>){IAC, SE}))
            {
              prefix("parse()");
            }
          }
          else
          {
            vector<unsigned char> buffer = {IAC, '\0', '\0'};
            for (int i = 1; i < 3; i++)
            {
              buffer[i] = (unsigned char)m_strBuffer[i];
              if (buffer[i] == DO)
              {
                buffer[i] = WONT;
              }
              else if (buffer[i] == WILL)
              {
                buffer[i] = DO;
              }
            }
            if (!write(buffer))
            {
              prefix("parse()");
            }
          }
          if (debug())
          {
            (*m_posDebug) << "TELNET:  ";
            if ((size_t)m_strBuffer[2] < m_telopts.size())
            {
              (*m_posDebug) << m_telopts[m_strBuffer[2]];
            }
            else
            {
              (*m_posDebug) << m_strBuffer[2];
            }
            (*m_posDebug) << endl;
          } 
          m_strBuffer.erase(1, 2);
        }
        // }}}
        // {{{ data
        else if (m_strBuffer[0] == '\007' && m_strBuffer.size() > 2 && m_strBuffer[2] == '}')
        {
          m_strBuffer.erase(1, 2);
        }
        else if (m_strBuffer[0] == '\n')
        {
          down();
        }
        else if (m_strBuffer[0] == '\r')
        {
          col(0);
        }
        else if (row() < rows() && col() < cols())
        {
          if (isprint(m_strBuffer[0]))
          {
            m_screen[row()][col()] = m_strBuffer[0];
          }
          right();
        }
        // }}}
        m_strBuffer.erase(0, 1);
      }
      for (list<unsigned char>::iterator i = leftOver.begin(); i != leftOver.end(); i++)
      {
        m_strBuffer.push_back(*i);
      }
      leftOver.clear();
      if (debug())
      {
        display();
      }
    }
    // }}}
    // {{{ prefix()
    string Terminal::prefix()
    {
      return ((!m_strPrefix.empty())?(string)"Terminal::" + m_strPrefix + (string)" error:  ":(string)"");
    }
    void Terminal::prefix(const string strPrefix)
    {
      if (!strPrefix.empty())
      {
        string strPrevPrefix = m_strPrefix;
        m_strPrefix = strPrefix;
        if (!strPrevPrefix.empty())
        {
          m_strPrefix += (string)"->" + strPrevPrefix;
        }
      }
      else
      {
        m_strPrefix.clear();
      }
    }
    // }}}
    // {{{ read()
    bool Terminal::read()
    {
      bool bResult = false;

      if (m_fdSocket != -1)
      {
        bool bClose = false, bExit = false, bIncoming = false;
        char szBuffer[4096];
        int nReturn;
        m_mutexRead.lock();
        while (!bExit)
        {
          pollfd fds[1];
          fds[0].fd = m_fdSocket;
          fds[0].events = POLLIN;
          if ((nReturn = poll(fds, 1, ((bIncoming)?m_nSocketTimeout[0]:m_nSocketTimeout[1]))) > 0)
          {
            if (fds[0].fd == m_fdSocket && (fds[0].revents & (POLLHUP | POLLIN)))
            {
              if ((nReturn = ::read(m_fdSocket, szBuffer, 4096)) > 0)
              {
                bIncoming = bResult = true;
                m_bWait = false;
                m_strBuffer.append(szBuffer, nReturn);
              }
              else
              {
                stringstream ssPrefix;
                bClose = bExit = true;
                ssPrefix << "read()->read(" << errno << ")";
                error(ssPrefix.str(), strerror(errno));
              }
            }
            if (fds[0].revents & POLLERR)
            {
              stringstream ssPrefix;
              bClose = bExit = true;
              ssPrefix << "read()->poll()";
              error(ssPrefix.str(), "Encountered a POLLERR.");
            }
            if (fds[0].revents & POLLNVAL)
            {
              stringstream ssPrefix;
              bClose = bExit = true;
              ssPrefix << "read()->poll()";
              error(ssPrefix.str(), "Encountered a POLLNVAL.");
            }
          }
          else if (nReturn < 0)
          {
            stringstream ssPrefix;
            bClose = bExit = true;
            ssPrefix << "read()->poll(" << errno << ")";
            error(ssPrefix.str(), strerror(errno));
          }
          else
          {
            bExit = true;
          }
        }
        if (bClose)
        {
          disconnect();
        }
        parse();
        m_mutexRead.unlock();
      }
      else
      {
        error("read()", "Server handle is not set.");
      }

      return bResult;
    }
    // }}}
    // {{{ reset()
    void Terminal::reset()
    {
      for (size_t i = 0; i < rows(); i++)
      {
        m_bWrap = false;
        m_screen[i].clear();
        m_tab[i].clear();
      }
      m_screen.clear();
      m_tab.clear();
      resize();
    }
    // }}}
    // {{{ resize()
    void Terminal::resize()
    {
      if (rows() > 0 && cols() > 0)
      {
        while (m_screen.size() > rows())
        {
          m_screen.back().clear();
          m_screen.pop_back();
        }
        while (m_tab.size() > rows())
        {
          m_tab.back().clear();
          m_tab.pop_back();
        }
        for (size_t i = 0; i < rows(); i++)
        {
          list<list<size_t>::iterator> removeIter;
          if (i >= m_screen.size())
          {
            string strData;
            strData.append(cols(), ' ');
            m_screen.push_back(strData);
          }
          if (i >= m_tab.size())
          {
            list<size_t> item;
            m_tab.push_back(item);
          }
          if (m_screen[i].size() < cols())
          {
            m_screen[i].append(cols() - m_screen[i].size(), ' ');
          }
          else if (m_screen[i].size() > cols())
          {
            m_screen[i].erase(cols(), m_screen[i].size() - cols());
          }
          for (list<size_t>::iterator j = m_tab[i].begin(); j != m_tab[i].end(); j++)
          {
            if ((*j) >= cols())
            {
              removeIter.push_back(j);
            }
          }
          for (list<list<size_t>::iterator>::iterator j = removeIter.begin(); j != removeIter.end(); j++)
          {
            m_tab[i].erase(*j);
          }
          removeIter.clear();
        }
        if (row() >= rows())
        {
          row(rows() - 1);
        }
        if (col() >= cols())
        {
          col(cols() - 1);
        }
      }
    }
    // }}}
    // {{{ right()
    void Terminal::right(const size_t unCount)
    {
      for (size_t i = 0; i < unCount; i++)
      {
        if (col() < (cols() - 1))
        {
          col(col() + 1);
        }
        else if (m_bWrap)
        {
          down();
          col(0);
        }
      }
    }
    // }}}
    // {{{ row()
    bool Terminal::row(const size_t unRow)
    {
      bool bResult = false;

      if (unRow < rows())
      {
        m_unRow = unRow;
      }
      else
      {
        m_unRow = 0;
      }

      return bResult;
    }
    size_t Terminal::row()
    {
      return m_unRow;
    }
    // }}}
    // {{{ rows()
    bool Terminal::rows(const size_t unRows)
    {
      bool bResult = false;

      if (unRows > 0)
      {
        bResult = true;
        m_unRows = unRows;
        resize();
      }

      return bResult;
    }
    size_t Terminal::rows()
    {
      return m_unRows;
    }
    // }}}
    // {{{ screen()
    void Terminal::screen(string &strScreen)
    {
      strScreen.clear();
      for (size_t i = 0; i < m_screen.size(); i++)
      {
        strScreen += m_screen[i] + "\n";
      }
    }
    void Terminal::screen(vector<string> &vecScreen)
    {
      vecScreen = m_screen;
    }
    // }}}
    // {{{ send()
    bool Terminal::send(const string strData, const size_t unCount)
    {
      bool bResult = true;
      string strNewData;

      for (size_t i = 0; i < strData.size(); i++)
      {
        char cChar = strData[i];
        if (cChar == '\n')
        {
          cChar = '\r';
        }
        strNewData.append(&cChar, 1);
        if (cChar == '\r')
        {
          cChar = '\0';
          strNewData.append(&cChar, 1);
        }
      }
      for (size_t i = 0; bResult && i < unCount; i++)
      {
        if (debug())
        {
          string strEncode;
          (*m_posDebug) << "SEND:  " << encode(strNewData, strEncode) << endl;
        }
        if (!write(strNewData))
        {
          bResult = false;
          prefix("send()");
        }
      }

      return bResult;
    }
    // }}}
    // {{{ sendCtrl()
    bool Terminal::sendCtrl(char cKey, const bool bWait)
    {
      bool bResult = false;

      if (int(cKey) >= 97 && int(cKey) <= 122) // a - z
      {
        cKey -= 32;
      }
      if (int(cKey) >= 64 && int(cKey) <= 95) // @ - _
      {
        stringstream ssData;
        cKey -= 64;
        ssData << cKey;
        if ((bWait && sendWait(ssData.str())) || (!bWait && send(ssData.str())))
        {
          bResult = true;
        }
      }

      return bResult;
    }
    // }}}
    // {{{ sendDown()
    bool Terminal::sendDown(const size_t unCount, const bool bWait)
    {
      return sendKey('B', unCount, bWait);
    }
    // }}}
    // {{{ sendEnter()
    bool Terminal::sendEnter(const bool bWait)
    {
      bool bResult = true;
      string strData = "\r";

      if ((bWait && sendWait(strData)) || (!bWait && send(strData)))
      {
        bResult = true;
      }

      return bResult; 
    }
    // }}}
    // {{{ sendEscape()
    bool Terminal::sendEscape(const bool bWait)
    {
      bool bResult = true;
      string strData = "\033";

      if ((bWait && sendWait(strData)) || (!bWait && send(strData)))
      {
        bResult = true;
      }

      return bResult; 
    }
    // }}}
    // {{{ sendFunction()
    bool Terminal::sendFunction(const int nKey)
    {
      stringstream ssData;

      ssData << "\033F" << setw(2) << setfill('0') << nKey;

      return ((nKey <= 12)?sendWait(ssData.str()):sendShiftFunction(nKey - 10));
    }
    // }}}
    // {{{ sendHome()
    bool Terminal::sendHome(const bool bWait)
    {
      return sendKey('H', 1, bWait);
    }
    // }}}
    // {{{ sendKey()
    bool Terminal::sendKey(const char cKey, const size_t unCount, const bool bWait)
    {
      bool bResult = true;
      stringstream ssData;

      ssData << "\033[";
      if (!bWait)
      {
        ssData << unCount;
      }
      ssData << cKey;
      if (bWait)
      {
        for (size_t i = 0; bResult && i < unCount; i++)
        {
          if (!sendWait(ssData.str()))
          {
            bResult = false;
          } 
        } 
      } 
      else if (!send(ssData.str()))
      {
        bResult = false;
      }

      return bResult; 
    }
    // }}}
    // {{{ sendKeypadEnter()
    bool Terminal::sendKeypadEnter(const bool bWait)
    {
      bool bResult = true;
      string strData = "\033OM";

      if ((bWait && sendWait(strData)) || (!bWait && send(strData)))
      {
        bResult = true;
      }

      return bResult; 
    }
    // }}}
    // {{{ sendLeft()
    bool Terminal::sendLeft(const size_t unCount, const bool bWait)
    {
      return sendKey('D', unCount, bWait);
    }
    bool Terminal::sendLeft(const size_t unRow, const size_t unStartCol, const size_t unEndCol, const string strValue)
    {
      bool bResult = false;
      string strTempValue;

      m_manip.toLower(strTempValue, m_manip.trim(strTempValue, strValue));
      if (unRow < m_screen.size())
      {
        if (unStartCol < m_screen[unRow].size() && unEndCol < m_screen[unRow].size() && unStartCol < unEndCol)
        {
          bool bExit = false;
          string strInitial, strPrev;
          m_manip.toLower(strInitial, m_manip.trim(strInitial, m_screen[unRow].substr(unStartCol, (unEndCol + 1) - unStartCol)));
          strPrev = strInitial;
          if (strTempValue == strInitial)
          {
            bResult = true;
          }
          while (!bExit && !bResult && sendLeft())
          {
            string strCurrent;
            m_manip.toLower(strCurrent, m_manip.trim(strCurrent, m_screen[unRow].substr(unStartCol, (unEndCol + 1) - unStartCol)));
            if (strTempValue == strCurrent)
            {
              bResult = true;
            }
            else if (strInitial == strCurrent || strPrev == strCurrent)
            {
              bExit = true;
            }
            strPrev = strCurrent;
          }
        }
      }

      return bResult;
    }
    // }}}
    // {{{ sendRight()
    bool Terminal::sendRight(const size_t unCount, const bool bWait)
    {
      return sendKey('C', unCount, bWait);
    }
    bool Terminal::sendRight(const size_t unRow, const size_t unStartCol, const size_t unEndCol, const string strValue)
    {
      bool bResult = false;
      string strTempValue;

      m_manip.toLower(strTempValue, m_manip.trim(strTempValue, strValue));
      if (unRow < m_screen.size())
      {
        if (unStartCol < m_screen[unRow].size() && unEndCol < m_screen[unRow].size() && unStartCol < unEndCol)
        {
          bool bExit = false;
          string strInitial, strPrev;
          m_manip.toLower(strInitial, m_manip.trim(strInitial, m_screen[unRow].substr(unStartCol, (unEndCol + 1) - unStartCol)));
          strPrev = strInitial;
          if (strTempValue == strInitial)
          {
            bResult = true;
          }
          while (!bExit && !bResult && sendRight())
          {
            string strCurrent;
            m_manip.toLower(strCurrent, m_manip.trim(strCurrent, m_screen[unRow].substr(unStartCol, (unEndCol + 1) - unStartCol)));
            if (strTempValue == strCurrent)
            {
              bResult = true;
            }
            else if (strInitial == strCurrent || strPrev == strCurrent)
            {
              bExit = true;
            }
            strPrev = strCurrent;
          }
        }
      }

      return bResult;
    }
    // }}}
    // {{{ sendShiftFunction()
    bool Terminal::sendShiftFunction(const int nKey)
    {
      stringstream ssData;

      ssData << "\033SF" << setw(2) << setfill('0') << nKey;

      return sendWait(ssData.str());
    }
    // }}}
    // {{{ sendTab()
    bool Terminal::sendTab(const size_t unCount, const bool bWait)
    {
      bool bResult = true;

      for (size_t i = 0; bResult && i < unCount; i++)
      {
        if ((bWait && !sendWait("\t")) || (!bWait && !send("\t")))
        {
          bResult = false;
        }
      }

      return bResult;
    }
    // }}}
    // {{{ sendUp()
    bool Terminal::sendUp(const size_t unCount, const bool bWait)
    {
      return sendKey('A', unCount, bWait);
    }
    // }}}
    // {{{ sendWait()
    bool Terminal::sendWait(const string strData, const size_t unCount)
    {
      bool bResult = false;

      if (send(strData, unCount))
      {
        bResult = wait(true);
      }
      else
      {
        prefix("sendWait()");
      }

      return bResult;
    }
    // }}}
    // {{{ setDebugStream()
    void Terminal::setDebugStream(ostream *posDebug)
    {
      m_posDebug = posDebug;
    }
    // }}}
    // {{{ setSocketTimeout()
    void Terminal::setSocketTimeout(const int nShort, const int nLong)
    {
      m_mutex.lock();
      m_nSocketTimeout[0] = nShort;
      m_nSocketTimeout[1] = nLong;
      m_mutex.unlock();
    }
    // }}}
    // {{{ tab()
    bool Terminal::tab(const size_t unRow, const size_t unCol, const bool bAdd)
    {
      bool bFound = false, bResult = false;
      list<size_t>::iterator iter;

      if (unRow >= m_tab.size())
      {
        for (size_t i = m_tab.size(); i <= unRow; i++)
        {
          list<size_t> item;
          m_tab.push_back(item);
        }
      }
      for (list<size_t>::iterator i = m_tab[unRow].begin(); i != m_tab[unRow].end(); i++)
      {
        if ((*i) == unCol)
        {
          bFound = true;
          iter = i;
        }
      }
      if (bAdd)
      {
        if (!bFound)
        {
          bResult = true;
          m_tab[unRow].push_back(unCol);
          m_tab[unRow].sort();
        }
      }
      else
      {
        if (bFound)
        {
          bResult = true;
          m_tab[unRow].erase(iter);
        }
      }

      return bResult;
    }
    // }}}
    // {{{ type()
    string Terminal::type()
    {
      return m_strType;
    }
    void Terminal::type(const string strType)
    {
      m_strType = strType;
    }
    // }}}
    // {{{ uncaught()
    bool Terminal::uncaught()
    {
      return m_bUncaught;
    }
    void Terminal::uncaught(const bool bUncaught)
    {
      m_bUncaught = bUncaught;
    }
    // }}}
    // {{{ up()
    void Terminal::up(const size_t unCount)
    {
      for (size_t i = 0; i < unCount; i++)
      {
        if (row() > 0)
        {
          row(row() - 1);
        }
        else if (m_bWrap)
        {
          row(rows() - 1);
        }
        else if (row() == 0)
        {
          if (m_screen.size() > 1)
          {
            for (size_t j = (m_screen.size() - 1); j > 0; j--)
            {
              m_screen[j] = m_screen[j - 1];
            }
          }
          m_screen[0].replace(0, cols(), cols(), ' ');
        }
      }
    }
    // }}}
    // {{{ wait()
    bool Terminal::wait(const bool bWait)
    {
      bool bResult = true;

      if (bWait)
      {
        m_bWait = true;
      }
      if (m_bWait)
      {
        if (read())
        {
          bResult = true;
        }
        else
        {
          prefix("wait()");
        }
      }

      return bResult;
    }
    // }}}
    // {{{ write()
    bool Terminal::write(string strBuffer)
    {
      bool bResult = false;

      if (strBuffer.empty())
      {
        bResult = true;
      }
      else if (m_fdSocket != -1)
      {
        bool bClose = false, bExit = false;
        int nReturn;
        m_mutexWrite.lock();
        while (!bExit)
        {
          pollfd fds[1];
          fds[0].fd = m_fdSocket;
          fds[0].events = POLLOUT;
          if ((nReturn = poll(fds, 1, 2000)) > 0)
          {
            if (fds[0].fd == m_fdSocket && (fds[0].revents & (POLLHUP | POLLIN)))
            {
              if ((nReturn = ::write(m_fdSocket, strBuffer.c_str(), strBuffer.size())) > 0)
              {
                strBuffer.erase(0, nReturn);
                if (strBuffer.empty())
                {
                  bExit = bResult = m_bWait = true;
                }
              }
              else
              {
                stringstream ssPrefix;
                bClose = bExit = true;
                ssPrefix << "write()->write(" << errno << ")";
                error(ssPrefix.str(), strerror(errno));
              }
            }
            if (fds[0].revents & POLLERR)
            {
              stringstream ssPrefix;
              bClose = bExit = true;
              ssPrefix << "write()->poll()";
              error(ssPrefix.str(), "Encountered a POLLERR.");
            }
            if (fds[0].revents & POLLNVAL)
            {
              stringstream ssPrefix;
              bClose = bExit = true;
              ssPrefix << "write()->poll()";
              error(ssPrefix.str(), "Encountered a POLLNVAL.");
            }
          }
          else if (nReturn < 0)
          {
            stringstream ssPrefix;
            bClose = bExit = true;
            ssPrefix << "write()->poll(" << errno << ")";
            error(ssPrefix.str(), strerror(errno));
          }
        }
        m_mutexWrite.unlock();
        if (bClose)
        {
          disconnect();
        }
      }
      else
      {
        error("write()", "Server handle is not set.");
      }

      return bResult;
    }
    bool Terminal::write(vector<unsigned char> buffer)
    {
      string strBuffer(buffer.begin(), buffer.end());

      return write(strBuffer);
    }
    // }}}
  }
}
