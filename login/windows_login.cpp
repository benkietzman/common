// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2006-03-08
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////

/*! \file windows_login.cpp
* \brief The Windows Login application.
*
* Validate login to the Windows Services domain.
*/

// {{{ includes
#include <iostream>
#include <sys/utsname.h>
using namespace std;
#include <Samba>
using namespace common;
// }}}
// {{{ defines
#ifdef VERSION
#undef VERSION
#endif
/*! \def VERSION
* \brief Contains the application version number.
*/
#define VERSION "1.0.1"
/*! \def mUSAGE(A)
* \brief Prints the usage statement.
*/
#define mUSAGE(A) cout << endl << "Usage:  "<< A << " -v" << endl << "v  -Gives the current version of this software." << endl << endl
/*! \def mVER_USAGE(A,B)
* \brief Prints the version number.
*/
#define mVER_USAGE(A,B) cout << endl << A << " Version: " << B << endl << endl
// }}}
// {{{ main()
/*! \fn int main(int argc, char *argv[])
* \brief This is the main function.
* \return Exits with a return code for the operating system.
*/
int main(int argc, char *argv[])
{
  string strDomain, strIp, strShare, strUser, strPassword, strDirectory, strFile;
  Samba samba;

  // {{{ command-line arguments
  for (int i = 1; i < argc; i++)
  {
    string strTemp = argv[i];
    if (strTemp == "-h" || strTemp == "--help")
    {
      mUSAGE(argv[0]);
      return -1;
    }
    else if (strTemp == "-v" || strTemp == "--version")
    {
      mVER_USAGE(argv[0], VERSION);
      return -1;
    }
    else if (strTemp == "-d" || (strTemp.size() > 9 && strTemp.substr(0, 9) == "--domain="))
    {
      if (strTemp == "-d")
      {
        strDomain = argv[++i];
      }
      else
      {
        strDomain = strTemp.substr(9, strTemp.size() - 9);
      }
    }
    else if (strTemp == "-i" || (strTemp.size() > 5 && strTemp.substr(0, 5) == "--ip="))
    {
      if (strTemp == "-i")
      {
        strIp = argv[++i];
      }
      else
      {
        strIp = strTemp.substr(5, strTemp.size() - 5);
      }
    }
    else if (strTemp == "-s" || (strTemp.size() > 8 && strTemp.substr(0, 8) == "--share="))
    {
      if (strTemp == "-s")
      {
        strShare = argv[++i];
      }
      else
      {
        strShare = strTemp.substr(8, strTemp.size() - 8);
      }
    }
    else if (strTemp == "-u" || (strTemp.size() > 7 && strTemp.substr(0, 7) == "--user="))
    {
      if (strTemp == "-u")
      {
        strUser = argv[++i];
      }
      else
      {
        strUser = strTemp.substr(7, strTemp.size() - 7);
      }
    }
    else if (strTemp == "-p" || (strTemp.size() > 11 && strTemp.substr(0, 11) == "--password="))
    {
      if (strTemp == "-p")
      {
        strPassword = argv[++i];
      }
      else
      {
        strPassword = strTemp.substr(11, strTemp.size() - 11);
      }
    }
    else if (strTemp == "-d" || (strTemp.size() > 12 && strTemp.substr(0, 12) == "--directory="))
    {
      if (strTemp == "-d")
      {
        strDirectory = argv[++i];
      }
      else
      {
        strDirectory = strTemp.substr(12, strTemp.size() - 12);
      }
    }
    else if (strTemp == "-f" || (strTemp.size() > 7 && strTemp.substr(0, 7) == "--file="))
    {
      if (strTemp == "-f")
      {
        strFile = argv[++i];
      }
      else
      {
        strFile = strTemp.substr(7, strTemp.size() - 7);
      }
    }
    else
    {
      cout << endl << "Illegal option, '" << strTemp << "'." << endl;
      mUSAGE(argv[0]);
      return -1;
    }
  }
  if (strDomain.empty())
  {
    cout << "Invalid Domain" << endl;
    return -1;
  }
  else if (strIp.empty())
  {
    cout << "Invalid IP Address" << endl;
    return -1;
  }
  else if (strShare.empty())
  {
    cout << "Invalid Share" << endl;
    return -1;
  }
  else if (strUser.empty())
  {
    cout << "Invalid User ID" << endl;
    return -1;
  }
  else if (strPassword.empty())
  {
    cout << "Wrong Password" << endl;
    return -1;
  }
  else if (strDirectory.empty() && strFile.empty())
  {
    cout << "Wrong Directory/File" << endl;
    return -1;
  }
  // }}}
  samba.init(strIp, strShare, strDomain, strUser, strPassword);
  if (!strFile.empty())
  {
    if (samba.fileExist(strFile))
    {
      cout << "Access Granted" << endl;
      return 0;
    }
    else
    {
      cout << "Access Denied" << endl;
      return -1;
    }
  }
  else
  {
    if (samba.directoryExist(strDirectory))
    {
      cout << "Access Granted" << endl;
      return 0;
    }
    else
    {
      cout << "Access Denied" << endl;
      return -1;
    }
  }

  return -1;
}
// }}}
