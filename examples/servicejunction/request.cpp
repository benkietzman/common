#include <iostream>
#include <list>
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
    list<string> request, response;
    junction.setApplication("Test Application");
    request.push_back("{\"Service\":\"addrInfo\",\"Server\":\"kietzman.org\"}");
    if (junction.request(request, response, strError))
    {
      CJson *ptJson = new CJson(response.front());
      if (ptJson->m_jsonMap.find("Status") != ptJson->m_jsonMap.end() && ptJson->m_jsonMap["Status"]->m_strValue == "okay")
      {
        if (ptJson->m_jsonMap.find("IPv4") != ptJson->m_jsonMap.end())
        {
          cout << "IPv4:  " << ptJson->m_jsonMap["IPv4"]->m_strValue << endl;
        }
        if (ptJson->m_jsonMap.find("IPv6") != ptJson->m_jsonMap.end())
        {
          cout << "IPv6:  " << ptJson->m_jsonMap["IPv6"]->m_strValue << endl;
        }
      }
      else if (ptJson->m_jsonMap.find("Error") != ptJson->m_jsonMap.end() && !ptJson->m_jsonMap["Error"]->m_strValue.empty())
      {
        cerr << ptJson->m_jsonMap["Error"];
      }
      else
      {
        cerr << "Unknown Error" << endl;
      }
      delete ptJson;
    }
    else
    {
      cerr << strError << endl;
    }
    response.clear();
    request.clear();
  }
  else
  {
    cerr << strError << endl;
  }

  return 0;
}
