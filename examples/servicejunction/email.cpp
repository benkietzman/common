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
    list<string> to, cc, bcc, file;
    junction.setApplication("Test Application");
    to.push_back("ben@kietzman.org");
    file.push_back("example.odt");
    if (junction.email("ben@kietzman.org", to, cc, bcc, "Test Message", "This is a plain-text message.", "<html><body><b>This is an HTML message.</b></body></html>", file, strError))
    {
      cout << "Sent the email." << endl;
    }
    else
    {
      cerr << strError << endl;
    }
    to.clear();
    file.clear();
  }
  else
  {
    cerr << strError << endl;
  }

  return 0;
}
