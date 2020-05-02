#include <cpprest/http_client.h> 
#include <cpprest/filestream.h> 
//----- Some standard C++ headers emitted for brevity
#include "cpprest/json.h" 
#include "cpprest/http_listener.h" 
#include "cpprest/uri.h" 
#include "cpprest/asyncrt_utils.h"
#include <iostream>
//////////////////////////////////////////////// 
// A Simple HTTP Client to Demonstrate  
// REST SDK Client programming model 
// The Toy sample shows how one can read  
// contents of a web page 
// 
using namespace utility;  // Common utilities like string conversions 
using namespace web;      // Common features like URIs. 
using namespace web::http;// Common HTTP functionality 
using namespace web::http::client;// HTTP client features 
using namespace concurrency::streams;// Asynchronous streams 
 
int main(int argc, char* argv[]) 
{ 
   auto fileStream = std::make_shared<ostream>(); 
   // Open stream to output file. 
   pplx::task<void> requestTask =  
              fstream::open_ostream(U("google_home.html")). 
         then([=](ostream outFile) 
   { 
         *fileStream = outFile; 
         // Create http_client to send the request. 
         http_client client(U("http://www.google.com")); 
         // Build request URI and start the request. 
          uri_builder builder(U("/"));
          std::cout<<builder.to_string()<<std::endl; 
         return client.request(methods::GET, builder.to_string()); 
 
   }).then([=](http_response response) 
   { 
         printf("Received response status code:%un",  
                                    response.status_code()); 
             return response.body(). 
                           read_to_end(fileStream->streambuf()); 
   }).then([=](size_t total){
         return fileStream->close(); 
   }); 
 
   // We have not started execution, just composed 
   // set of tasks in a Continuation Style 
   // Wait for all the outstanding I/O to complete  
   // and handle any exceptions, If any  
   try { 
         //-- All Taskss will get triggered here 
         requestTask.wait(); 
   } 
   catch (const std::exception &e) { 
         printf("Error exception:%sn", e.what()); 
   } 
   //---------------- pause for a key  
   getchar(); 
   return 0; 
} 
