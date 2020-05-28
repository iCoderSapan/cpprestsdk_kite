/*using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Collections;
using System.Reflection;
*/
#include <string>
#include <cpprest/http_client.h> 
#include <cpprest/filestream.h> 
//----- Some standard C++ headers emitted for brevity
#include "cpprest/json.h" 
#include "cpprest/http_listener.h" 
#include "cpprest/uri.h" 
#include "cpprest/asyncrt_utils.h"
#include <boost/algorithm/string.hpp>

#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include <openssl/sha.h>
#include <memory>
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
using namespace web::http::details // HTTP constants
using namespace concurrency::streams;// Asynchronous streams
using namespace boost::algorithm;
using namespace std;      // Use std c++ features


namespace KiteConnect
{
    /// <summary>
    /// The API client class. In production, you may initialize a single instance of this class per `APIKey`.
    /// </summary>
    class KiteResponse
    {
        public:
        json::value handle(http_response &response, string body) /*throws IOException, KiteException, JSONException */{
        if (response.header("Content-Type").contains("json")) {
            JSONObject jsonObject = new JSONObject(body);
            if(jsonObject.has("error_type")) {
                throw dealWithException(jsonObject, response.code());
            }
            return jsonObject;
        } else {
            throw new DataException("Unexpected content type received from server: "+ response.header("Content-Type")+" "+response.body().string(), 502);
        }
    }

    public String handle(Response response, String body, String type) throws IOException, KiteException, JSONException {
        if (response.header("Content-Type").contains("csv")) {
            return body;
        } else if(response.header("Content-Type").contains("json")){
            throw dealWithException(new JSONObject(response.body().string()), response.code());
        } else {
            throw new DataException("Unexpected content type received from server: "+ response.header("Content-Type")+" "+response.body().string(), 502);
        }
    }


    private KiteException dealWithException(JSONObject jsonObject, int code) throws JSONException {
        String exception = jsonObject.getString("error_type");

        switch (exception){
            // if there is a token exception, generate a signal to logout the user.
            case "TokenException":
                if(KiteConnect.sessionExpiryHook != null) {
                    KiteConnect.sessionExpiryHook.sessionExpired();
                }
                return  new TokenException(jsonObject.getString("message"), code);

            case "DataException": return new DataException(jsonObject.getString("message"), code);

            case "GeneralException": return new GeneralException(jsonObject.getString("message"), code);

            case "InputException": return new InputException(jsonObject.getString("message"), code);

            case "OrderException": return new OrderException(jsonObject.getString("message"), code);

            case "NetworkException": return new NetworkException(jsonObject.getString("message"), code);

            case "PermissionException": return new PermissionException(jsonObject.getString("message"), code);

            default: return new KiteException(jsonObject.getString("message"), code);
        }
    }
}        

#if 0
/*
//        #region HTTP Functions

        /// <summary>
        /// Alias for sending a GET request.
        /// </summary>
        /// <param name="Route">URL route of API</param>
        /// <param name="Params">Additional paramerters</param>
        /// <returns>Varies according to API endpoint</returns>
        task<http_response> Get(string route, Dictionary<string, dynamic> params = null)
        {
            return Request(route, "GET", params);
        }

        /// <summary>
        /// Alias for sending a POST request.
        /// </summary>
        /// <param name="Route">URL route of API</param>
        /// <param name="Params">Additional paramerters</param>
        /// <returns>Varies according to API endpoint</returns>
        task<http_response> Post(string Route, shared_ptr<map<string, string>> params = nullptr)
        {
            return Request(Route, "POST", params);
        }

        /// <summary>
        /// Alias for sending a PUT request.
        /// </summary>
        /// <param name="Route">URL route of API</param>
        /// <param name="Params">Additional paramerters</param>
        /// <returns>Varies according to API endpoint</returns>
        task<http_response> Put(string Route, Dictionary<string, dynamic> Params = null)
        {
            return Request(Route, "PUT", Params);
        }

        /// <summary>
        /// Alias for sending a DELETE request.
        /// </summary>
        /// <param name="Route">URL route of API</param>
        /// <param name="Params">Additional paramerters</param>
        /// <returns>Varies according to API endpoint</returns>
        task<http_response> Delete(string Route, Dictionary<string, dynamic> Params = null)
        {
            return Request(Route, "DELETE", Params);
        }

        /// <summary>
        /// Adds extra headers to request
        /// </summary>
        /// <param name="Req">Request object to add headers</param>
        private void AddExtraHeaders(http_client &requent)
        {
            request.headers().add("User-Agent", USER_AGENT);
            request.headers().add("X-Kite-Version", "3");
            request.headers().add(header_names::authorization, "token " + _apiKey + ":" + _accessToken);
            /*
            var KiteAssembly = System.Reflection.Assembly.GetAssembly(typeof(Kite));
            if (KiteAssembly != null)
                Req.UserAgent = "KiteConnect.Net/" + KiteAssembly.GetName().Version;
            req.Headers.Add("User-Agent", USER_AGENT)            ;
            Req.Headers.Add("X-Kite-Version", "3");
            Req.Headers.Add("Authorization", "token " + _apiKey + ":" + _accessToken);

            //if(Req.Method == "GET" && cache.IsCached(Req.RequestUri.AbsoluteUri))
            //{
            //    Req.Headers.Add("If-None-Match: " + cache.GetETag(Req.RequestUri.AbsoluteUri));
            //}

            Req.Timeout = _timeout;
            if (_proxy != null) Req.Proxy = _proxy;

            if (_enableLogging)
            {
                foreach (string header in Req.Headers.Keys)
                {
                    Console.WriteLine("DEBUG: " + header + ": " + Req.Headers.GetValues(header)[0]);
                }
            }*/

        }

        /// <summary>
        /// Make an HTTP request.
        /// </summary>
        /// <param name="Route">URL route of API</param>
        /// <param name="Method">Method of HTTP request</param>
        /// <param name="Params">Additional paramerters</param>
        /// <returns>Varies according to API endpoint</returns>
        task<http_response> Request(string route, string method, map<string, string> params = null)
        {
            string url = _root + _routes[Route];

            if (Params is null)
                Params = new Dictionary<string, dynamic>();

            if (url.find('{') != string::npos)
            {
                for (std::map<string, string>::iterator it=params.begin(); it!=params.end(); ++it) 
                {
                    if((size_t loc=url.find("{"+it->first+"}")) != string::npos) 
                    {
                        url.replace(loc, it->first.length+2, it->second);
                        params.erase(it->first);
                    }
                }
                /*
                var urlparams = Params.ToDictionary(entry => entry.Key, entry => entry.Value);

                foreach (KeyValuePair<string, dynamic> item in urlparams)
                    if (url.Contains("{" + item.Key + "}"))
                    {
                        url = url.Replace("{" + item.Key + "}", (string)item.Value);
                        Params.Remove(item.Key);
                    }
                    */

            }

            //if (!Params.ContainsKey("api_key"))
            //    Params.Add("api_key", _apiKey);

            //if (!Params.ContainsKey("access_token") && !String.IsNullOrEmpty(_accessToken))
            //    Params.Add("access_token", _accessToken);
            uri_builder paramEncoder;
            for (std::map<string, string>::iterator it=params.begin(); it!=params.end(); ++it) {
                    //std::cout << it->first << " => " << it->second << '\n';
                    paramEncoder.append_query(U(it->first), U(it->second));
            }
            string paramString(dataEncoder.erase(2)); /*append_query() adds `/?` at the begining */
            //HttpWebRequest request;
            //string paramString = String.Join("&", Params.Select(x => Utils.BuildParam(x.Key, x.Value)));
            http_request httpRequest(method);
            AddExtraHeaders(httpRequest);

            if (method == methods::POST || method == methods::PUT)
            {
                /*request = (HttpWebRequest)WebRequest.Create(url);
                request.AllowAutoRedirect = true;
                request.Method = Method;
                request.ContentType = "application/x-www-form-urlencoded";
                request.ContentLength = paramString.Length;
                if (_enableLogging) Console.WriteLine("DEBUG: " + Method + " " + url + "\n" + paramString);
                AddExtraHeaders(ref request);

                using (Stream webStream = request.GetRequestStream())
                using (StreamWriter requestWriter = new StreamWriter(webStream))
                    requestWriter.Write(paramString);
                */
               httpRequest.set_body(paramEncoder.to_string().erase(2),
                                    mime_types::application_x_www_form_urlencoded);
            }
            else
            {
                request = (HttpWebRequest)WebRequest.Create(url + "?" + paramString);
                request.AllowAutoRedirect = true;
                request.Method = Method;
                if (_enableLogging) Console.WriteLine("DEBUG: " + Method + " " + url + "?" + paramString);
                //AddExtraHeaders(ref request);
                route = route+paramEncoder.to_string();
            }

            httpRequest.set_request_uri(route);

            // Send request and wait for the response
            pplx::task<web::http::http_response> response =  httpClient.request(httpRequest);
            try { 
                //-- All Taskss will get triggered here 
                resp.wait();
                resp.get().status_code();
                std::cout<<"Response received:\n";
                return response;
            } 
            catch (const std::exception &e) {
                    cout<<("Error exception: "<<e.what())<<endl;
                    return response;
            }
// Not used below            
            WebResponse webResponse;
            try
            {
                webResponse = request.GetResponse();
            }
            catch (WebException e)
            {
                if (e.Response is null)
                    throw e;

                webResponse = e.Response;
            }

            using (Stream webStream = webResponse.GetResponseStream())
            {
                using (StreamReader responseReader = new StreamReader(webStream))
                {
                    string response = responseReader.ReadToEnd();
                    if (_enableLogging) Console.WriteLine("DEBUG: " + (int)((HttpWebResponse)webResponse).StatusCode + " " + response + "\n");

                    HttpStatusCode status = ((HttpWebResponse)webResponse).StatusCode;

                    if (webResponse.ContentType == "application/json")
                    {
                        Dictionary<string, dynamic> responseDictionary = Utils.JsonDeserialize(response);

                        if (status != HttpStatusCode.OK)
                        {
                            string errorType = "GeneralException";
                            string message = "";

                            if (responseDictionary.ContainsKey("error_type"))
                                errorType = responseDictionary["error_type"];

                            if (responseDictionary.ContainsKey("message"))
                                message = responseDictionary["message"];

                            switch (errorType)
                            {
                                case "GeneralException": throw new GeneralException(message, status);
                                case "TokenException":
                                    {
                                        _sessionHook?.Invoke();
                                        throw new TokenException(message, status);
                                    }
                                case "PermissionException": throw new PermissionException(message, status);
                                case "OrderException": throw new OrderException(message, status);
                                case "InputException": throw new InputException(message, status);
                                case "DataException": throw new DataException(message, status);
                                case "NetworkException": throw new NetworkException(message, status);
                                default: throw new GeneralException(message, status);
                            }
                        }

                        return responseDictionary;
                    }
                    else if (webResponse.ContentType == "text/csv")
                        return Utils.ParseCSV(response);
                    else
                        throw new DataException("Unexpected content type " + webResponse.ContentType + " " + response);
                }
            }

        }

        //#endregion
    }
}
#endif