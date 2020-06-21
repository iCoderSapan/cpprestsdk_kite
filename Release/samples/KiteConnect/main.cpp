#include "KiteConnect/KiteTypes.h"
#include "KiteConnect/Kite.h"
#include <iostream>
#include <string>
#include <fstream>

#define APIKEY "utmcr52rmqeze8sc"
#define API_SECRET "443isdhujy5d45jw85b4ldr0wmbg07me"
#define ACCESS_TOKEN_FILE "/home/sapan/.access_token"

int main()
{
    KiteConnect::Kite mainObj(APIKEY);
    std::ifstream access_token_file(ACCESS_TOKEN_FILE);
    if(!access_token_file.good())
    {
        std::ofstream access_token_file;
        access_token_file.open(ACCESS_TOKEN_FILE, ios::out | ios::in);

        KiteConnect::User userDetails = mainObj.GenerateSession("LqCvoXxxGe6xdS7pMeP5ODRP1A45BTQe", API_SECRET);
        mainObj.SetAccessToken(userDetails.accessToken);
        access_token_file << userDetails.accessToken;
        access_token_file.close();
    }
    else
    {
        char access_token[100] = {0};
        access_token_file >> access_token;
        mainObj.SetAccessToken(access_token);
    }
    
    KiteConnect::Profile profile = mainObj.GetProfile();
    std::cout<<profile.userName<<" - "<<profile.email<<std::endl;

    return 0;   
}
