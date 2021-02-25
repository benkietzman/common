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
    list<CJson *> result;
    map<string, string> ticket;
    junction.setApplication("Test Application");
    ticket["TicketNum"] = "******";
    if (junction.cts("prod", "******", "******", "query", ticket, result, strError))
    {
      for (list<CJson *>::iterator i = result.begin(); i != result.end(); i++)
      {
        cout << *(*i) << endl;
      }
    }
    else
    {
      cerr << strError << endl;
    }
    ticket.clear();
    for (list<CJson *>::iterator i = result.begin(); i != result.end(); i++)
    {
      delete *i;
    }
    result.clear();
  }
  else
  {
    cerr << strError << endl;
  }

  return 0;
}
