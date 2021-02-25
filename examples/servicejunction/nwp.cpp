#include <iostream>
#include <list>
#include <map>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <string>
using namespace std;
#include <ServiceJunction>
using namespace common;

int main(int argc, char *argv[])
{
  string strError;
  CServiceJunction junction(strError);

  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
  if (strError.empty())
  {
    list<map<string, string> > nwpResultArray;
    map<string, string> nwpRequestHeader, nwpRequestData, nwpResponseHeader, nwpResultHeader;
    junction.setApplication("Test Application");
    nwpRequestHeader["NwpHostName"] = "telecom";
    nwpRequestHeader["NwpLoginId"] = "******";
    nwpRequestHeader["NwpLoginPassword"] = "******";
    nwpRequestHeader["NwpAmpUserId"] = "******";
    nwpRequestData["TicketNumber"] = "ab123";
    nwpRequestData["SelectStatus"] = "unresolved";
    nwpRequestData["MaxActivitiesReturned"] = "100";
    nwpRequestData["AcknowledgeTicket"] = "no";
    if (junction.nwp("nmaTicket", "find",  nwpRequestHeader, nwpRequestData, nwpResponseHeader, nwpResultHeader, nwpResultArray, strError))
    {
      cout << "--- nwpResponseHeader ---" << endl;
      for (map<string, string>::iterator i = nwpResponseHeader.begin(); i != nwpResponseHeader.end(); i++)
      {
        cout << i->first << ":  " << i->second << endl;
      }
      cout << "--- nwpResultHeader ---" << endl;
      for (map<string, string>::iterator i = nwpResultHeader.begin(); i != nwpResultHeader.end(); i++)
      {
        cout << i->first << ":  " << i->second << endl;
      }
      cout << "--- nwpResultArray ---" << endl;
      for (list<map<string, string> >::iterator i = nwpResultArray.begin(); i != nwpResultArray.end(); i++)
      {
        cout << "--- Row ---" << endl;
        for (map<string, string>::iterator j = i->begin(); j != i->end(); j++)
        {
          cout << j->first << ":  " << j->second << endl;
        }
      }
    }
    else
    {
      cerr << strError << endl;
    }
    nwpRequestHeader.clear();
    nwpRequestData.clear();
    nwpResponseHeader.clear();
    nwpResultHeader.clear();
    for (list<map<string, string> >::iterator i = nwpResultArray.begin(); i != nwpResultArray.end(); i++)
    {
      i->clear();
    }
    nwpResultArray.clear();
  }
  else
  {
    cerr << strError << endl;
  }

  return 0;
}
