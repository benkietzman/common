#include <iostream>
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
    list<map<string, string> > result;
    junction.setApplication("Test Application");
    if (junction.oracleQuery("******", "******", "******", "select * from table", result, strError))
    {
      for (list<map<string, string> >::iterator i = result.begin(); i != result.end(); i++)
      {
        cout << "--- Row ---" << endl;
        for (map<string, string>::iterator j = i->begin(); j != i->end(); j++)
        {
          cout << j->first << ":  " << j->second << endl;
        }
        i->clear();
      }
    }
    else
    {
      cerr << strError << endl;
    }
    result.clear();
  }
  else
  {
    cerr << strError << endl;
  }

  return 0;
}
