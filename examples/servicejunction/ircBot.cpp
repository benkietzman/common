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
    junction.setApplication("Test Application");
    if (junction.ircBot("#system", "Test Message", strError))
    {
      cout << "Posted the message." << endl;
    }
    else
    {
      cerr << strError << endl;
    }
  }
  else
  {
    cerr << strError << endl;
  }

  return 0;
}
