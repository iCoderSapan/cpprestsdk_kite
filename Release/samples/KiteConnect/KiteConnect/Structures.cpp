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
using namespace web::http::details; // HTTP constants
using namespace concurrency::streams;// Asynchronous streams
using namespace boost::algorithm;
using namespace std;      // Use std c++ features

namespace KiteConnect
{
    /// <summary>
    /// Tick data structure
    /// </summary>
    struct Tick
    {
        string Mode { get; set; }
        UInt32 InstrumentToken { get; set; }
        bool Tradable { get; set; }
        double LastPrice { get; set; }
        UInt32 LastQuantity { get; set; }
        double AveragePrice { get; set; }
        UInt32 Volume { get; set; }
        UInt32 BuyQuantity { get; set; }
        UInt32 SellQuantity { get; set; }
        double Open { get; set; }
        double High { get; set; }
        double Low { get; set; }
        double Close { get; set; }
        double Change { get; set; }
        DepthItem[] Bids { get; set; }
        DepthItem[] Offers { get; set; }

        // KiteConnect 3 Fields

        DateTime? LastTradeTime { get; set; }
        UInt32 OI { get; set; }
        UInt32 OIDayHigh { get; set; }
        UInt32 OIDayLow { get; set; }
        DateTime? Timestamp { get; set; }
    }

    /// <summary>
    /// Market depth item structure
    /// </summary>
    struct DepthItem
    {
        DepthItem(Dictionary<string, dynamic> data)
        {
            Quantity = Convert.ToUInt32(data["quantity"]);
            Price = data["price"];
            Orders = Convert.ToUInt32(data["orders"]);
        }

        UInt32 Quantity { get; set; }
        double Price { get; set; }
        UInt32 Orders { get; set; }
    }

    /// <summary>
    /// Historical structure
    /// </summary>
    struct Historical
    {
        Historical(ArrayList data)
        {
            TimeStamp = Convert.ToDateTime(data[0]);
            Open = Convert.ToDecimal(data[1]);
            High = Convert.ToDecimal(data[2]);
            Low = Convert.ToDecimal(data[3]);
            Close = Convert.ToDecimal(data[4]);
            Volume = Convert.ToUInt32(data[5]);
            OI = data.Count > 6 ? Convert.ToUInt32(data[6]) : 0;
        }

        DateTime TimeStamp { get; }
        double Open { get; }
        double High { get; }
        double Low { get; }
        double Close { get; }
        UInt32 Volume { get; }
        UInt32 OI { get; }
    }

    /// <summary>
    /// Holding structure
    /// </summary>
    struct Holding
    {
        Holding(Dictionary<string, dynamic> data)
        {
            try
            {
                Product = data["product"];
                Exchange = data["exchange"];
                Price = data["price"];
                LastPrice = data["last_price"];
                CollateralQuantity = data["collateral_quantity"];
                PNL = data["pnl"];
                ClosePrice = data["close_price"];
                AveragePrice = data["average_price"];
                TradingSymbol = data["tradingsymbol"];
                CollateralType = data["collateral_type"];
                T1Quantity = data["t1_quantity"];
                InstrumentToken = Convert.ToUInt32(data["instrument_token"]);
                ISIN = data["isin"];
                RealisedQuantity = data["realised_quantity"];
                Quantity = data["quantity"];
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }

        string Product { get; set; }
        string Exchange { get; set; }
        double Price { get; set; }
        double LastPrice { get; set; }
        int CollateralQuantity { get; set; }
        double PNL { get; set; }
        double ClosePrice { get; set; }
        double AveragePrice { get; set; }
        string TradingSymbol { get; set; }
        string CollateralType { get; set; }
        int T1Quantity { get; set; }
        UInt32 InstrumentToken { get; set; }
        string ISIN { get; set; }
        int RealisedQuantity { get; set; }
        int Quantity { get; set; }
    }

    /// <summary>
    /// Available margin structure
    /// </summary>
    struct AvailableMargin
    {
        AvailableMargin(json::value data)
        {
            try
            {
                adHocMargin = data["adhoc_margin"].as_double();
                cash = data["cash"].as_double();
                collateral = data["collateral"].as_double();
                intradayPayin = data["intraday_payin"].as_double();
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }

        double adHocMargin;
        double cash;
        double collateral;
        double intradayPayin;
    };

    /// <summary>
    /// Utilised margin structure
    /// </summary>
    struct UtilisedMargin
    {
        UtilisedMargin(json::value data)
        {
            try
            {
                debits = data["debits"].as_double();
                exposure = data["exposure"].as_double();
                m2MRealised = data["m2m_realised"].as_double();
                m2MUnrealised = data["m2m_unrealised"].as_double();
                optionPremium = data["option_premium"].as_double();
                payout = data["payout"].as_double();
                span = data["span"].as_double();
                holdingSales = data["holding_sales"].as_double();
                turnover = data["turnover"].as_double();
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }

        double debits;
        double exposure;
        double m2MRealised;
        double m2MUnrealised;
        double optionPremium;
        double payout;
        double span;
        double holdingSales;
        double turnover;
    };

    /// <summary>
    /// UserMargin structure
    /// </summary>
    struct UserMargin
    {
        UserMargin(json::value &data)
        {
            try
            {
                enabled = data["enabled"].as_bool();
                net = data["net"].as_double();
                available = AvailableMargin(data["available"]);
                utilised = UtilisedMargin(data["utilised"]);
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }

        bool enabled;
        double net;
        AvailableMargin available;
        UtilisedMargin utilised;
    };

    /// <summary>
    /// User margins response structure
    /// </summary>
    struct UserMarginsResponse
    {
        UserMarginsResponse(std::string jsonData)
        {
            try
            {
                json::value data = json::value::parse(jsonData);
                equity = UserMargin(data["equity"]);
                commodity = UserMargin(data["commodity"]);
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }
        UserMargin equity;
        UserMargin commodity;
    };

    /// <summary>
    /// UserMargin structure
    /// </summary>
    struct InstrumentMargin
    {
        InstrumentMargin(Dictionary<string, dynamic> data)
        {
            try
            {
                Margin = data["margin"];
                COLower = data["co_lower"];
                MISMultiplier = data["mis_multiplier"];
                Tradingsymbol = data["tradingsymbol"];
                COUpper = data["co_upper"];
                NRMLMargin = data["nrml_margin"];
                MISMargin = data["mis_margin"];
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }

        string Tradingsymbol { get; set; }
        double Margin { get; set; }
        double COLower { get; set; }
        double COUpper { get; set; }
        double MISMultiplier { get; set; }
        double MISMargin { get; set; }
        double NRMLMargin { get; set; }
    }
    /// <summary>
    /// Position structure
    /// </summary>
    struct Position
    {
        Position(Dictionary<string, dynamic> data)
        {
            try
            {
                Product = data["product"];
                OvernightQuantity = data["overnight_quantity"];
                Exchange = data["exchange"];
                SellValue = data["sell_value"];
                BuyM2M = data["buy_m2m"];
                LastPrice = data["last_price"];
                TradingSymbol = data["tradingsymbol"];
                Realised = data["realised"];
                PNL = data["pnl"];
                Multiplier = data["multiplier"];
                SellQuantity = data["sell_quantity"];
                SellM2M = data["sell_m2m"];
                BuyValue = data["buy_value"];
                BuyQuantity = data["buy_quantity"];
                AveragePrice = data["average_price"];
                Unrealised = data["unrealised"];
                Value = data["value"];
                BuyPrice = data["buy_price"];
                SellPrice = data["sell_price"];
                M2M = data["m2m"];
                InstrumentToken = Convert.ToUInt32(data["instrument_token"]);
                ClosePrice = data["close_price"];
                Quantity = data["quantity"];
                DayBuyQuantity = data["day_buy_quantity"];
                DayBuyValue = data["day_buy_value"];
                DayBuyPrice = data["day_buy_price"];
                DaySellQuantity = data["day_sell_quantity"];
                DaySellValue = data["day_sell_value"];
                DaySellPrice = data["day_sell_price"];
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }

        string Product { get; }
        int OvernightQuantity { get; }
        string Exchange { get; }
        double SellValue { get; }
        double BuyM2M { get; }
        double LastPrice { get; }
        string TradingSymbol { get; }
        double Realised { get; }
        double PNL { get; }
        double Multiplier { get; }
        int SellQuantity { get; }
        double SellM2M { get; }
        double BuyValue { get; }
        int BuyQuantity { get; }
        double AveragePrice { get; }
        double Unrealised { get; }
        double Value { get; }
        double BuyPrice { get; }
        double SellPrice { get; }
        double M2M { get; }
        UInt32 InstrumentToken { get; }
        double ClosePrice { get; }
        int Quantity { get; }
        int DayBuyQuantity { get; }
        double DayBuyPrice { get; }
        double DayBuyValue { get; }
        int DaySellQuantity { get; }
        double DaySellPrice { get; }
        double DaySellValue { get; }
    }

    /// <summary>
    /// Position response structure
    /// </summary>
    struct PositionResponse
    {
        PositionResponse(Dictionary<string, dynamic> data)
        {
            Day = new List<Position>();
            Net = new List<Position>();

            foreach (Dictionary<string, dynamic> item in data["day"])
                Day.Add(new Position(item));
            foreach (Dictionary<string, dynamic> item in data["net"])
                Net.Add(new Position(item));
        }

        List<Position> Day { get; }
        List<Position> Net { get; }
    }

    /// <summary>
    /// Order structure
    /// </summary>
    struct Order
    {
        Order(Dictionary<string, dynamic> data)
        {
            try
            {
                AveragePrice = data["average_price"];
                CancelledQuantity = data["cancelled_quantity"];
                DisclosedQuantity = data["disclosed_quantity"];
                Exchange = data["exchange"];
                ExchangeOrderId = data["exchange_order_id"];
                ExchangeTimestamp = Utils.StringToDate(data["exchange_timestamp"]);
                FilledQuantity = data["filled_quantity"];
                InstrumentToken = Convert.ToUInt32(data["instrument_token"]);
                OrderId = data["order_id"];
                OrderTimestamp = Utils.StringToDate(data["order_timestamp"]);
                OrderType = data["order_type"];
                ParentOrderId = data["parent_order_id"];
                PendingQuantity = data["pending_quantity"];
                PlacedBy = data["placed_by"];
                Price = data["price"];
                Product = data["product"];
                Quantity = data["quantity"];
                Status = data["status"];
                StatusMessage = data["status_message"];
                Tag = data["tag"];
                Tradingsymbol = data["tradingsymbol"];
                TransactionType = data["transaction_type"];
                TriggerPrice = data["trigger_price"];
                Validity = data["validity"];
                Variety = data["variety"];
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }

        double AveragePrice { get; set; }
        int CancelledQuantity { get; set; }
        int DisclosedQuantity { get; set; }
        string Exchange { get; set; }
        string ExchangeOrderId { get; set; }
        DateTime? ExchangeTimestamp { get; set; }
        int FilledQuantity { get; set; }
        UInt32 InstrumentToken { get; set; }
        string OrderId { get; set; }
        DateTime? OrderTimestamp { get; set; }
        string OrderType { get; set; }
        string ParentOrderId { get; set; }
        int PendingQuantity { get; set; }
        string PlacedBy { get; set; }
        double Price { get; set; }
        string Product { get; set; }
        int Quantity { get; set; }
        string Status { get; set; }
        string StatusMessage { get; set; }
        string Tag { get; set; }
        string Tradingsymbol { get; set; }
        string TransactionType { get; set; }
        double TriggerPrice { get; set; }
        string Validity { get; set; }
        string Variety { get; set; }
    }

    /// <summary>
    /// GTTOrder structure
    /// </summary>
    struct GTT
    {
        GTT(Dictionary<string, dynamic> data)
        {
            try
            {
                Id = data["id"];
                Condition = new GTTCondition(data["condition"]);
                TriggerType = data["type"];

                Orders = new List<GTTOrder>();
                foreach (Dictionary<string, dynamic> item in data["orders"])
                    Orders.Add(new GTTOrder(item));

                Status = data["status"];
                CreatedAt = Utils.StringToDate(data["created_at"]);
                UpdatedAt = Utils.StringToDate(data["updated_at"]);
                ExpiresAt = Utils.StringToDate(data["expires_at"]);
                Meta = new GTTMeta(data["meta"]);
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }

        int Id { get; set; }
        GTTCondition? Condition { get; set; }
        string TriggerType { get; set; }
        List<GTTOrder> Orders { get; set; }
        string Status { get; set; }
        DateTime? CreatedAt { get; set; }
        DateTime? UpdatedAt { get; set; }
        DateTime? ExpiresAt { get; set; }
        GTTMeta? Meta { get; set; }
    }

    /// <summary>
    /// GTTMeta structure
    /// </summary>
    struct GTTMeta
    {
        GTTMeta(Dictionary<string, dynamic> data)
        {
            try
            {
                RejectionReason = data != null && data.ContainsKey("rejection_reason") ? data["rejection_reason"] : "";
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }

        string RejectionReason { get; set; }
    }

    /// <summary>
    /// GTTCondition structure
    /// </summary>
    struct GTTCondition
    {
        GTTCondition(Dictionary<string, dynamic> data)
        {
            try
            {
                InstrumentToken = data["instrument_token"];
                Exchange = data["exchange"];
                TradingSymbol = data["tradingsymbol"];
                TriggerValues = Utils.ToDecimalList(data["trigger_values"] as ArrayList);
                LastPrice = data["last_price"];
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }

        int InstrumentToken { get; set; }
        string Exchange { get; set; }
        string TradingSymbol { get; set; }
        List<double> TriggerValues { get; set; }
        double LastPrice { get; set; }
    }

    /// <summary>
    /// GTTOrder structure
    /// </summary>
    struct GTTOrder
    {
        GTTOrder(Dictionary<string, dynamic> data)
        {
            try
            {
                TransactionType = data["transaction_type"];
                Product = data["product"];
                OrderType = data["order_type"];
                Quantity = data["quantity"];
                Price = data["price"];
                Result = data["result"] == null ? null : new Nullable<GTTResult>(new GTTResult(data["result"]));
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }

        string TransactionType { get; set; }
        string Product { get; set; }
        string OrderType { get; set; }
        int Quantity { get; set; }
        double Price { get; set; }
        GTTResult? Result { get; set; }
    }

    /// <summary>
    /// GTTResult structure
    /// </summary>
    struct GTTResult
    {
        GTTResult(Dictionary<string, dynamic> data)
        {
            try
            {
                OrderResult = data["order_result"] == null ? null : new Nullable<GTTOrderResult>(new GTTOrderResult(data["order_result"]));
                Timestamp = data["timestamp"];
                TriggeredAtPrice = data["triggered_at"];
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }

        GTTOrderResult? OrderResult { get; set; }
        string Timestamp { get; set; }
        double TriggeredAtPrice { get; set; }
    }

    /// <summary>
    /// GTTOrderResult structure
    /// </summary>
    struct GTTOrderResult
    {
        GTTOrderResult(Dictionary<string, dynamic> data)
        {
            try
            {
                OrderId = data["order_id"];
                RejectionReason = data["rejection_reason"];
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }

        string OrderId { get; set; }
        string RejectionReason { get; set; }
    }

    /// <summary>
    /// GTTParams structure
    /// </summary>
    struct GTTParams
    {
        string TradingSymbol { get; set; }
        string Exchange { get; set; }
        int InstrumentToken { get; set; }
        string TriggerType { get; set; }
        double LastPrice { get; set; }
        List<GTTOrderParams> Orders { get; set; }
        List<double> TriggerPrices { get; set; }
    }

    /// <summary>
    /// GTTOrderParams structure
    /// </summary>
    struct GTTOrderParams
    {
        int Quantity { get; set; }
        double Price { get; set; }
        // Order type (LIMIT, SL, SL-M, MARKET)
        string OrderType { get; set; }
        // Product code (NRML, MIS, CNC)
        string Product { get; set; }
        // Transaction type (BUY, SELL)
        string TransactionType { get; set; }
    }

    /// <summary>
    /// Instrument structure
    /// </summary>
    struct Instrument
    {
        Instrument(Dictionary<string, dynamic> data)
        {
            try
            {
                InstrumentToken = Convert.ToUInt32(data["instrument_token"]);
                ExchangeToken = Convert.ToUInt32(data["exchange_token"]);
                TradingSymbol = data["tradingsymbol"];
                Name = data["name"];
                LastPrice = Convert.ToDecimal(data["last_price"]);
                TickSize = Convert.ToDecimal(data["tick_size"]);
                Expiry = Utils.StringToDate(data["expiry"]);
                InstrumentType = data["instrument_type"];
                Segment = data["segment"];
                Exchange = data["exchange"];

                if (data["strike"].Contains("e"))
                    Strike = Decimal.Parse(data["strike"], System.Globalization.NumberStyles.Float);
                else
                    Strike = Convert.ToDecimal(data["strike"]);

                LotSize = Convert.ToUInt32(data["lot_size"]);
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }

        UInt32 InstrumentToken { get; set; }
        UInt32 ExchangeToken { get; set; }
        string TradingSymbol { get; set; }
        string Name { get; set; }
        double LastPrice { get; set; }
        double TickSize { get; set; }
        DateTime? Expiry { get; set; }
        string InstrumentType { get; set; }
        string Segment { get; set; }
        string Exchange { get; set; }
        double Strike { get; set; }
        UInt32 LotSize { get; set; }
    }

    /// <summary>
    /// Trade structure
    /// </summary>
    struct Trade
    {
        Trade(Dictionary<string, dynamic> data)
        {
            try
            {
                TradeId = data["trade_id"];
                OrderId = data["order_id"];
                ExchangeOrderId = data["exchange_order_id"];
                Tradingsymbol = data["tradingsymbol"];
                Exchange = data["exchange"];
                InstrumentToken = Convert.ToUInt32(data["instrument_token"]);
                TransactionType = data["transaction_type"];
                Product = data["product"];
                AveragePrice = data["average_price"];
                Quantity = data["quantity"];
                FillTimestamp = Utils.StringToDate(data["fill_timestamp"]);
                ExchangeTimestamp = Utils.StringToDate(data["exchange_timestamp"]);
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }

        string TradeId { get; }
        string OrderId { get; }
        string ExchangeOrderId { get; }
        string Tradingsymbol { get; }
        string Exchange { get; }
        UInt32 InstrumentToken { get; }
        string TransactionType { get; }
        string Product { get; }
        double AveragePrice { get; }
        int Quantity { get; }
        DateTime? FillTimestamp { get; }
        DateTime? ExchangeTimestamp { get; }
    }

    /// <summary>
    /// Trigger range structure
    /// </summary>
    struct TrigerRange
    {
        TrigerRange(Dictionary<string, dynamic> data)
        {
            try
            {
                InstrumentToken = Convert.ToUInt32(data["instrument_token"]);
                Lower = data["lower"];
                Upper = data["upper"];
                Percentage = data["percentage"];
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }
        UInt32 InstrumentToken { get; }
        double Lower { get; }
        double Upper { get; }
        double Percentage { get; }
    }

    /// <summary>
    /// User structure
    /// </summary>
    struct User
    {
        User(std::string jsonStr)
        {
            try
            {
                json::value data = json::value::parse(jsonStr); 
                APIKey = data["data"]["api_key"];
                json::array array_data(data["data"]["products"].as_array());
                for(auto itr=data["data"]["products"].as_array().begin(); itr != data["data"]["products"].as_array().end(); itr++)
                {
                    products.push_back(*itr);
                }
                Utils::jsonArrayDeserialise(data["data"]["products"].as_array(), products);
                UserName = data["data"]["user_name"];
                UserShortName = data["data"]["user_shortname"];
                AvatarURL = data["data"]["avatar_url"];
                Broker = data["data"]["broker"];
                AccessToken = data["data"]["access_token"];
                PublicToken = data["data"]["public_token"];
                RefreshToken = data["data"]["refresh_token"];
                UserType = data["data"]["user_type"];
                UserId = data["data"]["user_id"];
                LoginTime = Utils.StringToDate(data["data"]["login_time"]);
                Utils::jsonArrayDeserialise(data["data"]["exchanges"].as_array(), exchanges);
                Utils::jsonArrayDeserialise(data["data"]["order_types"].as_array(), orderTypes);
                Email = data["data"]["email"];
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }

        string apiKey { get; }
        std::vector<string> products;
        string userName { get; }
        string userShortName { get; }
        string avatarURL { get; }
        string broker { get; }
        string accessToken { get; }
        string publicToken { get; }
        string refreshToken { get; }
        string userType { get; }
        string userId { get; }
        DateTime? LoginTime { get; }
        string[] Exchanges { get; }
        string[] OrderTypes { get; }
        string Email { get; }
    }

    struct TokenSet
    {
        TokenSet(std::string data)
        {
            try
            {
                json::value data = json::value::parse(data);
                UserId = data["data"]["user_id"];
                AccessToken = data["data"]["access_token"];
                RefreshToken = data["data"]["refresh_token"];
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }
        std::string userId;
        std::string accessToken;
        std::string refreshToken;
    }

    /// <summary>
    /// User structure
    /// </summary>
    struct Profile
    {
        Profile(std::string data)
        {
            try
            {
                json::value data = json::value::parse()
                products = Utils::jsonArrayDeserialise(data["data"]["products"].as_array(), products);
                userName = data["data"]["user_name"];
                userShortName = data["data"]["user_shortname"];
                avatarURL = data["data"]["avatar_url"];
                broker = data["data"]["broker"];
                userType = data["data"]["user_type"];
                exchanges = Utils::jsonArrayDeserialise(data["data"]["exchanges"].as_array(), exchanges);
                orderTypes = Utils::jsonArrayDeserialise(data["data"]["order_types"].as_array(), orderTypes);
                email = data["data"]["email"];
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }

        vector<string> products;
        string userName;
        string userShortName;
        string avatarURL;
        string broker;
        string userType;
        vector<string> exchanges;
        vector<string> OrderTypes;
        string email;
    }

    /// <summary>
    /// Quote structure
    /// </summary>
    struct Quote
    {
        Quote(Dictionary<string, dynamic> data)
        {
            try
            {
                InstrumentToken = Convert.ToUInt32(data["instrument_token"]);
                Timestamp = Utils.StringToDate(data["timestamp"]);
                LastPrice = data["last_price"];

                Change = data["net_change"];

                Open = data["ohlc"]["open"];
                Close = data["ohlc"]["close"];
                Low = data["ohlc"]["low"];
                High = data["ohlc"]["high"];                

                if (data.ContainsKey("last_quantity"))
                {
                    // Non index quote
                    LastQuantity = Convert.ToUInt32(data["last_quantity"]);
                    LastTradeTime = Utils.StringToDate(data["last_trade_time"]);
                    AveragePrice = data["average_price"];
                    Volume = Convert.ToUInt32(data["volume"]);

                    BuyQuantity = Convert.ToUInt32(data["buy_quantity"]);
                    SellQuantity = Convert.ToUInt32(data["sell_quantity"]);

                    OI = Convert.ToUInt32(data["oi"]);

                    OIDayHigh = Convert.ToUInt32(data["oi_day_high"]);
                    OIDayLow = Convert.ToUInt32(data["oi_day_low"]);

                    LowerCircuitLimit = data["lower_circuit_limit"];
                    UpperCircuitLimit = data["upper_circuit_limit"];

                    Bids = new List<DepthItem>();
                    Offers = new List<DepthItem>();

                    if (data["depth"]["buy"] != null)
                    {
                        foreach (Dictionary<string, dynamic> bid in data["depth"]["buy"])
                            Bids.Add(new DepthItem(bid));
                    }

                    if (data["depth"]["sell"] != null)
                    {
                        foreach (Dictionary<string, dynamic> offer in data["depth"]["sell"])
                            Offers.Add(new DepthItem(offer));
                    }
                }
                else
                {
                    // Index quote
                    LastQuantity = 0;
                    LastTradeTime = null;
                    AveragePrice = 0;
                    Volume = 0;

                    BuyQuantity = 0;
                    SellQuantity = 0;

                    OI = 0;

                    OIDayHigh = 0;
                    OIDayLow = 0;

                    LowerCircuitLimit = 0;
                    UpperCircuitLimit = 0;

                    Bids = new List<DepthItem>();
                    Offers = new List<DepthItem>();
                }
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }

        UInt32 InstrumentToken { get; set; }
        double LastPrice { get; set; }
        UInt32 LastQuantity { get; set; }
        double AveragePrice { get; set; }
        UInt32 Volume { get; set; }
        UInt32 BuyQuantity { get; set; }
        UInt32 SellQuantity { get; set; }
        double Open { get; set; }
        double High { get; set; }
        double Low { get; set; }
        double Close { get; set; }
        double Change { get; set; }
        double LowerCircuitLimit { get; set; }
        double UpperCircuitLimit { get; set; }
        List<DepthItem> Bids { get; set; }
        List<DepthItem> Offers { get; set; }

        // KiteConnect 3 Fields

        DateTime? LastTradeTime { get; set; }
        UInt32 OI { get; set; }
        UInt32 OIDayHigh { get; set; }
        UInt32 OIDayLow { get; set; }
        DateTime? Timestamp { get; set; }
    }

    /// <summary>
    /// OHLC Quote structure
    /// </summary>
    struct OHLC
    {
        OHLC(Dictionary<string, dynamic> data)
        {
            try
            {
                InstrumentToken = Convert.ToUInt32(data["instrument_token"]);
                LastPrice = data["last_price"];

                Open = data["ohlc"]["open"];
                Close = data["ohlc"]["close"];
                Low = data["ohlc"]["low"];
                High = data["ohlc"]["high"];
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }
        UInt32 InstrumentToken { get; set; }
        double LastPrice { get; }
        double Open { get; }
        double Close { get; }
        double High { get; }
        double Low { get; }
    }

    /// <summary>
    /// LTP Quote structure
    /// </summary>
    struct LTP
    {
        LTP(Dictionary<string, dynamic> data)
        {
            try
            {
                InstrumentToken = Convert.ToUInt32(data["instrument_token"]);
                LastPrice = data["last_price"];
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }
        UInt32 InstrumentToken { get; set; }
        double LastPrice { get; }
    }

    /// <summary>
    /// Mutual funds holdings structure
    /// </summary>
    struct MFHolding
    {
        MFHolding(Dictionary<string, dynamic> data)
        {
            try
            {
                Quantity = data["quantity"];
                Fund = data["fund"];
                Folio = data["folio"];
                AveragePrice = data["average_price"];
                TradingSymbol = data["tradingsymbol"];
                LastPrice = data["last_price"];
                PNL = data["pnl"];
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }

        double Quantity { get; }
        string Fund { get; }
        string Folio { get; }
        double AveragePrice { get; }
        string TradingSymbol { get; }
        double LastPrice { get; }
        double PNL { get; }
    }

    /// <summary>
    /// Mutual funds instrument structure
    /// </summary>
    struct MFInstrument
    {
        MFInstrument(Dictionary<string, dynamic> data)
        {
            try
            {
                TradingSymbol = data["tradingsymbol"];
                AMC = data["amc"];
                Name = data["name"];

                PurchaseAllowed = data["purchase_allowed"] == "1";
                RedemtpionAllowed = data["redemption_allowed"] == "1";

                MinimumPurchaseAmount = Convert.ToDecimal(data["minimum_purchase_amount"]);
                PurchaseAmountMultiplier = Convert.ToDecimal(data["purchase_amount_multiplier"]);
                MinimumAdditionalPurchaseAmount = Convert.ToDecimal(data["minimum_additional_purchase_amount"]);
                MinimumRedemptionQuantity = Convert.ToDecimal(data["minimum_redemption_quantity"]);
                RedemptionQuantityMultiplier = Convert.ToDecimal(data["redemption_quantity_multiplier"]);
                LastPrice = Convert.ToDecimal(data["last_price"]);

                DividendType = data["dividend_type"];
                SchemeType = data["scheme_type"];
                Plan = data["plan"];
                SettlementType = data["settlement_type"];
                LastPriceDate = Utils.StringToDate(data["last_price_date"]);
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }

        string TradingSymbol { get; }
        string AMC { get; }
        string Name { get; }

        bool PurchaseAllowed { get; }
        bool RedemtpionAllowed { get; }

        double MinimumPurchaseAmount { get; }
        double PurchaseAmountMultiplier { get; }
        double MinimumAdditionalPurchaseAmount { get; }
        double MinimumRedemptionQuantity { get; }
        double RedemptionQuantityMultiplier { get; }
        double LastPrice { get; }

        string DividendType { get; }
        string SchemeType { get; }
        string Plan { get; }
        string SettlementType { get; }
        DateTime? LastPriceDate { get; }
    }

    /// <summary>
    /// Mutual funds order structure
    /// </summary>
    struct MFOrder
    {
        MFOrder(Dictionary<string, dynamic> data)
        {
            try
            {
                StatusMessage = data["status_message"];
                PurchaseType = data["purchase_type"];
                PlacedBy = data["placed_by"];
                Amount = data["amount"];
                Quantity = data["quantity"];
                SettlementId = data["settlement_id"];
                OrderTimestamp = Utils.StringToDate(data["order_timestamp"]);
                AveragePrice = data["average_price"];
                TransactionType = data["transaction_type"];
                ExchangeOrderId = data["exchange_order_id"];
                ExchangeTimestamp = Utils.StringToDate(data["exchange_timestamp"]);
                Fund = data["fund"];
                Variety = data["variety"];
                Folio = data["folio"];
                Tradingsymbol = data["tradingsymbol"];
                Tag = data["tag"];
                OrderId = data["order_id"];
                Status = data["status"];
                LastPrice = data["last_price"];
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }

        string StatusMessage { get; }
        string PurchaseType { get; }
        string PlacedBy { get; }
        double Amount { get; }
        double Quantity { get; }
        string SettlementId { get; }
        DateTime? OrderTimestamp { get; }
        double AveragePrice { get; }
        string TransactionType { get; }
        string ExchangeOrderId { get; }
        DateTime? ExchangeTimestamp { get; }
        string Fund { get; }
        string Variety { get; }
        string Folio { get; }
        string Tradingsymbol { get; }
        string Tag { get; }
        string OrderId { get; }
        string Status { get; }
        double LastPrice { get; }
    }

    /// <summary>
    /// Mutual funds SIP structure
    /// </summary>
    struct MFSIP
    {
        MFSIP(Dictionary<string, dynamic> data)
        {
            try
            {
                DividendType = data["dividend_type"];
                PendingInstalments = data["pending_instalments"];
                Created = Utils.StringToDate(data["created"]);
                LastInstalment = Utils.StringToDate(data["last_instalment"]);
                TransactionType = data["transaction_type"];
                Frequency = data["frequency"];
                InstalmentDate = data["instalment_date"];
                Fund = data["fund"];
                SIPId = data["sip_id"];
                Tradingsymbol = data["tradingsymbol"];
                Tag = data["tag"];
                InstalmentAmount = data["instalment_amount"];
                Instalments = data["instalments"];
                Status = data["status"];
                OrderId = data.ContainsKey(("order_id")) ? data["order_id"] : "";
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }

        string DividendType { get; }
        int PendingInstalments { get; }
        DateTime? Created { get; }
        DateTime? LastInstalment { get; }
        string TransactionType { get; }
        string Frequency { get; }
        int InstalmentDate { get; }
        string Fund { get; }
        string SIPId { get; }
        string Tradingsymbol { get; }
        string Tag { get; }
        int InstalmentAmount { get; }
        int Instalments { get; }
        string Status { get; }
        string OrderId { get; }
    }

}
