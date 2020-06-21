#include "KiteTypes.h"
#include "Kite.h"
#include <iostream>
#include <string>
#include <fstream>

#define APIKEY "utmcr52rmqeze8sc"
#define API_SECRET "443isdhujy5d45jw85b4ldr0wmbg07me"
#define ACCESS_TOKEN_FILE "access_token"

int main(int argc, char *argv[])
{
    if(argc != 2)
        return 1;
    std::cout << argv[1];
    KiteConnect::Kite mainObj(APIKEY);
    std::cout << "Debug..2\n";
    std::ifstream access_token_file(ACCESS_TOKEN_FILE);
    if(!access_token_file.good())
    {
        std::ofstream access_token;
        access_token.open(ACCESS_TOKEN_FILE, ios::out | ios::in);
        try {
            std::cout << "Debug..3\n";
            KiteConnect::User userDetails = mainObj.GenerateSession(argv[1], API_SECRET);
            std::cout<<userDetails.accessToken<<std::endl;
            std::cout << "Debug..4\n";
            mainObj.SetAccessToken(userDetails.accessToken);
            access_token << userDetails.accessToken;
            access_token.close();
            access_token_file.close();
            KiteConnect::Profile profile = mainObj.GetProfile();
            std::cout<<profile.userName<<" - "<<profile.email<<std::endl;
        }
        catch(std::exception e) {
            std::cout<<"Exception caught! Provide proper request token"<< e.what()<<std::endl;
            return 1;
        }
    }
    else
    {
        char access_token[100] = {0};
        std::ifstream access_token_file;
        access_token_file.open(ACCESS_TOKEN_FILE, ios::out | ios::in);
        access_token_file >> access_token;
        access_token_file.close();
        mainObj.SetAccessToken(access_token);
    }
    

    return 0;   
}
