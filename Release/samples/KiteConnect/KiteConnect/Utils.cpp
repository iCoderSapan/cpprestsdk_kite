/*using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Web.Script.Serialization;
using Microsoft.VisualBasic.FileIO;
using System.IO;
using System.Web;
using System.Security.Cryptography;
using System.Text.RegularExpressions;
*/
#include <string>
#include <map>
#include <list>
#include <vector>
#include <openssl/sha.h>

#include "Utils.h"
#include "cpprest/json.h"
#include "KiteTypes.h"

using namespace web;
namespace KiteConnect
{
#if 0
    /// <summary>
    /// Convert string to Date object
    /// </summary>
    /// <param name="obj">Date string.</param>
    /// <returns>Date object/</returns>
    static DateTime? StringToDate(string DateString)
    {
        try
        {
            if(DateString.Length == 10)
            {
                return DateTime.ParseExact(DateString, "yyyy-MM-dd", null);
            }else
            {
                return DateTime.ParseExact(DateString, "yyyy-MM-dd HH:mm:ss", null);
            }
        }catch (Exception)
        {
            return null;
        }
    }
    static std::time_t to_timestamp(const std::string& str,
                                            bool is_dst = false,
                                            const std::string& format = "%Y-%b-%d %H:%M:%S")
    {
        std::tm time_obj = {0};
        time_obj.tm_isdst = is_dst ? 1 : 0;
        std::istringstream ss(str);
        ss >> std::get_time(&t, format.c_str());
        return mktime(&t);
    }

    /// <summary>
    /// Serialize C# object to JSON string.
    /// </summary>
    /// <param name="obj">C# object to serialize.</param>
    /// <returns>JSON string/</returns>
    static string JsonSerialize(object obj)
    {
        var jss = new JavaScriptSerializer();
        string json = jss.Serialize(obj);
        MatchCollection mc = Regex.Matches(json, @"\\/Date\((\d*?)\)\\/");
        foreach (Match m in mc)
        {
            UInt64 unix = UInt64.Parse(m.Groups[1].Value) / 1000;
            json = json.Replace(m.Groups[0].Value, UnixToDateTime(unix).ToString());
        }
        return json;
    }

    /// <summary>
    /// Deserialize Json string to nested string dictionary.
    /// </summary>
    /// <param name="Json">Json string to deserialize.</param>
    /// <returns>Json in the form of nested string dictionary.</returns>
    static Dictionary<string, dynamic> JsonDeserialize(string Json)
    {
        var jss = new JavaScriptSerializer();
        Dictionary<string, dynamic> dict = jss.Deserialize<Dictionary<string, dynamic>>(Json);
        // Replace double with decimal in the map
        dict = DoubleToDecimal(dict);
        return dict;
    }
#endif
    /// <summary>
    /// Deserialize Json string to nested string dictionary.
    /// </summary>
    /// <param name="Json">Json string to deserialize.</param>
    /// <returns>Json in the form of nested string dictionary.</returns>
    static std::vector<std::string>& JsonArrayDeserialize(json::array jsonArray, std::vector<std::string> &result)
    {
        for(auto itr = jsonArray.begin(); itr != jsonArray.end(); itr++)
        {
            result.push_back((*itr).as_string());
        }
        return result;
    }
#if 0        
    /// <summary>
    /// Recursively traverses an object and converts double fields to decimal.
    /// This is used in Json deserialization. JavaScriptSerializer converts floats
    /// in exponential notation to double and everthing else to double. This function
    /// makes everything decimal. Currently supports only Dictionary and Array as input.
    /// </summary>
    /// <param name="obj">Input object.</param>
    /// <returns>Object with decimals instead of doubles</returns>
    static dynamic DoubleToDecimal(dynamic obj)
    {
        if (obj is double)
        {
            obj = Convert.ToDecimal(obj);
        }
        else if (obj is IDictionary)
        {
            var keys = new List<string>(obj.Keys);
            for (int i = 0; i < keys.Count; i++)
            {
                obj[keys[i]] = DoubleToDecimal(obj[keys[i]]);
            }
        }
        else if (obj is ICollection)
        {
            obj = new ArrayList(obj);
            for (int i = 0; i < obj.Count; i++)
            {
                obj[i] = DoubleToDecimal(obj[i]);
            }
        }
        return obj;
    }

#endif
    /// <summary>
    /// Parse instruments API's CSV response.
    /// Takes arguments as referene to avoid coping huge data
    /// </summary>
    /// <param name="data">Response of instruments API.</param>
    /// <param name="">Response of instruments API.</param>
    /// <returns>CSV data as array of nested string dictionary.</returns>
    static void ParseCSV(std::string &data, CSVObjType &csvObj)
    {
        CSVObjType instruments;
        /* TODO: Fill all the details here as below
            * [map of each row with parameters and values]->[map]->[map]
        string[] lines = Data.Split('\n');

        List<Dictionary<string, dynamic>> instruments = new List<Dictionary<string, dynamic>>();

        using (TextFieldParser parser = new TextFieldParser(StreamFromString(Data)))
        {
            // parser.CommentTokens = new string[] { "#" };
            parser.SetDelimiters(new string[] { "," });
            parser.HasFieldsEnclosedInQuotes = true;

            // Skip over header line.
            string[] headers = parser.ReadLine().Split(',');

            while (!parser.EndOfData)
            {
                string[] fields = parser.ReadFields();
                Dictionary<string, dynamic> item = new Dictionary<string, dynamic>();

                for (var i = 0; i < headers.Length; i++)
                    item.Add(headers[i], fields[i]);

                instruments.Add(item);
            }
        }
        */
    }

#if 0
    /// <summary>
    /// Wraps a string inside a stream
    /// </summary>
    /// <param name="value">string data</param>
    /// <returns>Stream that reads input string</returns>
    static MemoryStream StreamFromString(std::string value)
    {
        return new MemoryStream(Encoding.UTF8.GetBytes(value ?? ""));
    }
#endif
    /// <summary>
    /// Helper function to add parameter to the request only if it is not null or empty
    /// </summary>
    /// <param name="Params">Dictionary to add the key-value pair</param>
    /// <param name="Key">Key of the parameter</param>
    /// <param name="Value">Value of the parameter</param>
    inline void Utils::AddIfNotNull(ParamType& params, std::string Key, std::string &value)
    {
        if (!value.empty())
            params[Key] = value;
    }

    /// <summary>
    /// Generates SHA256 checksum for login.
    /// </summary>
    /// <param name="Data">Input data to generate checksum for.</param>
    /// <returns>SHA256 checksum in hex format.</returns>
    std::string Utils::SHA256(std::string data)
    {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, data.c_str(), data.size());
        SHA256_Final(hash, &sha256);
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
#if 0        
    /// <summary>
    /// Creates key=value with url encoded value
    /// </summary>
    /// <param name="Key">Key</param>
    /// <param name="Value">Value</param>
    /// <returns>Combined string</returns>
    static string BuildParam(string Key, dynamic Value)
    {
        if (Value is string)
        {
            return HttpUtility.UrlEncode(Key) + "=" + HttpUtility.UrlEncode((string)Value);
        }
        else
        {
            string[] values = (string[])Value;
            return String.Join("&", values.Select(x => HttpUtility.UrlEncode(Key) + "=" + HttpUtility.UrlEncode(x)));
        }
    }

    static DateTime UnixToDateTime(UInt64 unixTimeStamp)
    {
        // Unix timestamp is seconds past epoch
        DateTime dateTime = new DateTime(1970, 1, 1, 5, 30, 0, 0, DateTimeKind.Unspecified);
        dateTime = dateTime.AddSeconds(unixTimeStamp);
        return dateTime;
    }

    static List<decimal> ToDecimalList(ArrayList arrayList)
    {
        var res = new List<decimal>();
        foreach(var i in arrayList)
        {
            res.Add(Convert.ToDecimal(i));
        }
        return res;
    }
#endif        
};
