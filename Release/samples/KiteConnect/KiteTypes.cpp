#include <string>
#include <exception>
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
#include <ctime>
#include <cstdint>

#include "KiteTypes.h"

#include "Utils.h"

#define TIME(x) x.as_string()

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
    #if 0
    /// <summary>
    /// Tick data structure
    /// </summary>
    struct Tick
    {
        std::string Mode;
        uint32_t InstrumentToken;
        bool Tradable;
        double LastPrice;
        uint32_t LastQuantity;
        double AveragePrice;
        uint32_t Volume;
        uint32_t BuyQuantity;
        uint32_t SellQuantity;
        double Open;
        double High;
        double Low;
        double Close;
        double Change;
        DepthItem[] Bids;
        DepthItem[] Offers;

        // KiteConnect 3 Fields

        std::string LastTradeTime;
        uint32_t OI;
        uint32_t OIDayHigh;
        uint32_t OIDayLow;
        std::string Timestamp;
    }
    #endif

    DepthItem::DepthItem(json::value data)
    {
        quantity = data["quantity"].as_number().to_uint32();
        price = data["price"].as_double();
        orders = data["orders"].as_number().to_uint32();
    }

    Historical::Historical(json::value jsonValueObj)
    {
        json::array jsonCandleObj = jsonValueObj.as_array();

        timeStamp = TIME(jsonCandleObj[0]);
        open = jsonCandleObj[1].as_double();
        high = jsonCandleObj[2].as_double();
        low = jsonCandleObj[3].as_double();
        close = jsonCandleObj[4].as_double();
        volume = jsonCandleObj[5].as_number().to_uint32();
        OI = jsonCandleObj.size() > 6 ? jsonCandleObj[6].as_number().to_uint32() : 0;
    }

    HistoricalResponse::HistoricalResponse(std::string jsonData)
    {
        json::value jsonObj = json::value::parse(jsonData);
        json::array candlesJsonObj = jsonObj["data"]["candles"].as_array();

        for(auto itr=candlesJsonObj.begin(); itr != candlesJsonObj.end(); itr++)
        {
            historicals.push_back(Historical(*itr));
        }
    }

        Holding::Holding(json::value data)
        {
            try
            {
                product = data["product"].as_string();
                exchange = data["exchange"].as_string();
                price = data["price"].as_double();
                lastPrice = data["last_price"].as_double();
                collateralQuantity = data["collateral_quantity"].as_integer();
                PNL = data["pnl"].as_double();
                closePrice = data["close_price"].as_double();
                averagePrice = data["average_price"].as_double();
                tradingSymbol = data["tradingsymbol"].as_string();
                collateralType = data["collateral_type"].as_string();
                T1Quantity = data["t1_quantity"].as_integer();
                instrumentToken = data["instrument_token"].as_number().to_uint32();
                ISIN = data["isin"].as_string();
                realisedQuantity = data["realised_quantity"].as_integer();
                quantity = data["quantity"].as_integer();
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + data.serialize() + e.what());
            }
        }

        HoldingsResponse::HoldingsResponse(std::string jsonData)
        {
            try
            {
                json::value jsonObj = json::value::parse(jsonData);
                json::array holdings_array_data(jsonObj["data"].as_array());
                for(auto itr=holdings_array_data.begin(); itr != holdings_array_data.end(); itr++)
                {
                    holdingsData.push_back(Holding(*itr));
                }
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }
        }

        ConvertPositionResponse::ConvertPositionResponse(std::string jsonData)
        {
            try
            {
                json::value jsonObj = json::value::parse(jsonData);
                status = jsonObj["data"].as_bool();
                
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }
        }

        AvailableMargin::AvailableMargin(json::value &data)
        {
            try
            {
                adHocMargin = data["adhoc_margin"].as_double();
                cash = data["cash"].as_double();
                collateral = data["collateral"].as_double();
                intradayPayin = data["intraday_payin"].as_double();
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to load data in AvailableMargin " + data.serialize() + e.what());
            }
        }

        UtilisedMargin::UtilisedMargin(json::value &data)
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
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to load data - UtilisedMargin " + data.serialize() + e.what());
            }
        }

        UserMargin::UserMargin(json::value &data)
        {
            try
            {
                enabled = data["enabled"].as_bool();
                net = data["net"].as_double();
                available = AvailableMargin(data["available"]);
                utilised = UtilisedMargin(data["utilised"]);
            }
            catch (std::exception &e)
            {
                throw std::runtime_error("User Margin - Json error - " + data.serialize() + e.what());
            }
        }

        UserMargin::UserMargin(std::string &jsonData)
        {
            try
            {
                json::value data = json::value::parse(jsonData)["data"];
                enabled = data["enabled"].as_bool();
                net = data["net"].as_double();
                available = AvailableMargin(data["available"]);
                utilised = UtilisedMargin(data["utilised"]);
            }
            catch (std::exception &e)
            {
                throw std::runtime_error("User Margin - Json error - " + jsonData + e.what());
            }
        }

        UserMarginsResponse::UserMarginsResponse(std::string jsonData)
        {
            try
            {
                json::value data = json::value::parse(jsonData);
                equity = UserMargin(data["equity"]);
                commodity = UserMargin(data["commodity"]);
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }
        }

        Order::Order(json::value data)
        {
            try
            {
                averagePrice = data["average_price"].as_double();
                cancelledQuantity = data["cancelled_quantity"].as_integer();
                disclosedQuantity = data["disclosed_quantity"].as_integer();
                exchange = data["exchange"].as_string();
                exchangeOrderId = data["exchange_order_id"].as_string();
                exchangeTimestamp = data["exchange_timestamp"].as_string();
                filledQuantity = data["filled_quantity"].as_integer();
                instrumentToken = data["instrument_token"].as_number().to_uint32();
                orderId = data["order_id"].as_string();
                orderTimestamp = data["order_timestamp"].as_string();
                orderType = data["order_type"].as_string();
                parentOrderId = data["parent_order_id"].as_string();
                pendingQuantity = data["pending_quantity"].as_integer();
                placedBy = data["placed_by"].as_string();
                price = data["price"].as_double();
                product = data["product"].as_string();
                quantity = data["quantity"].as_integer();
                status = data["status"].as_string();
                statusMessage = data["status_message"].as_string();
                tag = data["tag"].as_string();
                tradingSymbol = data["tradingsymbol"].as_string();
                transactionType = data["transaction_type"].as_string();
                triggerPrice = data["trigger_price"].as_double();
                validity = data["validity"].as_string();
                variety = data["variety"].as_string();
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + data.serialize() + e.what());
            }

        }

        OrderResponse::OrderResponse(std::string jsonData)
        {
            try
            {
                json::value data = json::value::parse(jsonData);
                order_id = data["data"]["order_id"].as_string();
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data - OrderResponse " + jsonData + e.what());
            }
        }

        GetOrdersResponse::GetOrdersResponse(std::string jsonData)
        {
            try
            {
                json::value data = json::value::parse(jsonData);
                json::array array_data(data["data"].as_array());
                for(auto itr=array_data.begin(); itr != array_data.end(); itr++)
                {
                    orders.push_back(Order(*itr));
                }
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }
        }

        OrderHistoryResponse::OrderHistoryResponse(std::string jsonData)
        {
            try
            {
                json::value data = json::value::parse(jsonData);
                json::array array_data(data["data"].as_array());
                for(auto itr=array_data.begin(); itr != array_data.end(); itr++)
                {
                    orderHistory.push_back(Order(*itr));
                }
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }
        }

        Trade::Trade(json::value data)
        {
            try
            {
                tradeId = data["trade_id"].as_string();
                orderId = data["order_id"].as_string();
                exchangeOrderId = data["exchange_order_id"].as_string();
                tradingSymbol = data["tradingsymbol"].as_string();
                exchange = data["exchange"].as_string();
                instrumentToken = data["instrument_token"].as_number().to_uint32();
                transactionType = data["transaction_type"].as_string();
                product = data["product"].as_string();
                averagePrice = data["average_price"].as_double();
                quantity = data["quantity"].as_integer();
                fillTimestamp = data["fill_timestamp"].as_string();
                exchangeTimestamp = data["exchange_timestamp"].as_string();
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + data.serialize() + e.what());
            }

        }

        OrderTradesResponse::OrderTradesResponse(std::string jsonData)
        {
            try
            {
                json::value data = json::value::parse(jsonData);
                json::array array_data(data["data"].as_array());
                for(auto itr=array_data.begin(); itr != array_data.end(); itr++)
                {
                    orderTrades.push_back(Trade(*itr));
                }
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }
        }
    #if 0
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
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }
        }

        std::string Tradingsymbol;
        double Margin;
        double COLower;
        double COUpper;
        double MISMultiplier;
        double MISMargin;
        double NRMLMargin;
    };
    #endif
        Position::Position(json::value data)
        {
            try
            {
                product = data["product"].as_string();
                overnightQuantity = data["overnight_quantity"].as_integer();
                exchange = data["exchange"].as_string();
                sellValue = data["sell_value"].as_double();
                buyM2M = data["buy_m2m"].as_double();
                lastPrice = data["last_price"].as_double();
                tradingSymbol = data["tradingsymbol"].as_string();
                realised = data["realised"].as_double();
                PNL = data["pnl"].as_double();
                multiplier = data["multiplier"].as_double();
                sellQuantity = data["sell_quantity"].as_integer();
                sellM2M = data["sell_m2m"].as_double();
                buyValue = data["buy_value"].as_double();
                buyQuantity = data["buy_quantity"].as_integer();
                averagePrice = data["average_price"].as_double();
                unrealised = data["unrealised"].as_double();
                value = data["value"].as_double();
                buyPrice = data["buy_price"].as_double();
                sellPrice = data["sell_price"].as_double();
                M2M = data["m2m"].as_double();
                instrumentToken = (data["instrument_token"]).as_number().to_uint32();
                closePrice = data["close_price"].as_double();
                quantity = data["quantity"].as_integer();
                dayBuyQuantity = data["day_buy_quantity"].as_integer();
                dayBuyValue = data["day_buy_value"].as_double();
                dayBuyPrice = data["day_buy_price"].as_double();
                daySellQuantity = data["day_sell_quantity"].as_integer();
                daySellValue = data["day_sell_value"].as_double();
                daySellPrice = data["day_sell_price"].as_double();
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to pas data. " + data.serialize() + e.what());
            }

        }

        PositionResponse::PositionResponse(std::string jsonData)
        {
            json::value data = json::value::parse(jsonData);

            json::array day_array_data(data["data"]["day"].as_array());
            for(auto itr=day_array_data.begin(); itr != day_array_data.end(); itr++)
            {
                day.push_back(Position(*itr));
            }
            json::array net_array_data(data["data"]["net"].as_array());
            for(auto itr=net_array_data.begin(); itr != net_array_data.end(); itr++)
            {
                day.push_back(Position(*itr));
            }
        }

#if 0
    /// <summary>
    /// GTTOrder structure
    /// </summary>
    struct GTT
    {
        GTT(json::value data)
        {
            try
            {
                id = data["id"];
                Condition = new GTTCondition(data["condition"]);
                triggerType = data["type"];

                Orders = new List<GTTOrder>();
                foreach (Dictionary<string, dynamic> item in data["orders"])
                    Orders.Add(new GTTOrder(item));

                Status = data["status"];
                CreatedAt = TIME(data["created_at"]);
                UpdatedAt = TIME(data["updated_at"]);
                ExpiresAt = TIME(data["expires_at"]);
                Meta = new GTTMeta(data["meta"]);
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + data.serialize() + e.what());
            }
        }

        int Id;
        GTTCondition? Condition;
        std::string TriggerType;
        List<GTTOrder> Orders;
        std::string Status;
        std::string CreatedAt;
        std::string UpdatedAt;
        std::string ExpiresAt;
        GTTMeta? Meta;
    }

    /// <summary>
    /// GTTOrder structure
    /// </summary>
    struct GTTResponse
    {
        GTTResponse(std::string jsonData)
        {
            try
            {
                json::array gttsData = json::value::parse(jsonData)["data"].as_array();
                foreach (auto itr = gttsData.begin(); itr != gttsData.end(); itr++)
                {
                    gtts.push_back(GTT(*itr));
                }
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + data.serialize() + e.what());
            }
        }

        std::list<GTT> gtts;
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
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }
        }

        std::string RejectionReason;
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
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }
        }

        int InstrumentToken;
        std::string Exchange;
        std::string TradingSymbol;
        List<double> TriggerValues;
        double LastPrice;
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
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }
        }

        std::string TransactionType;
        std::string Product;
        std::string OrderType;
        int Quantity;
        double Price;
        GTTResult? Result;
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
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }
        }

        GTTOrderResult? OrderResult;
        std::string Timestamp;
        double TriggeredAtPrice;
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
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }
        }

        std::string OrderId;
        std::string RejectionReason;
    }

    /// <summary>
    /// GTTParams structure
    /// </summary>
    struct GTTParams
    {
        std::string tradingSymbol;
        std::string Exchange;
        int instrumentToken;
        std::string triggerType;
        double lastPrice;
        std::list<GTTOrderParams> orders;
        std::list<double> triggerPrices;
    }

    /// <summary>
    /// GTTOrderParams structure
    /// </summary>
    struct GTTOrderParams
    {
        int quantity;
        double price;
        // Order type (LIMIT, SL, SL-M, MARKET)
        std::string orderType;
        // Product code (NRML, MIS, CNC)
        std::string product;
        // Transaction type (BUY, SELL)
        std::string transactionType;
    }
#endif

        Instrument::Instrument(std::map<std::string, std::string> data)
        {
            try
            {
                instrumentToken = std::stoul(data["instrument_token"]);
                exchangeToken = std::stoul(data["exchange_token"]);
                tradingSymbol = data["tradingsymbol"];
                name = data["name"];
                lastPrice = std::stod(data["last_price"]);
                tickSize = std::stod(data["tick_size"]);
                expiry = data["expiry"];
                instrumentType = data["instrument_type"];
                segment = data["segment"];
                exchange = data["exchange"];

                /* TODO: Convert it to proper double with 'e' characters 
                if (data["strike"].Contains("e"))
                    Strike = Decimal.Parse(data["strike"], System.Globalization.NumberStyles.Float);
                else
                    Strike = Convert.ToDecimal(data["strike"]);*/
                strike = std::stod(data["strike"]);

                lotSize = std::stoul(data["lot_size"]);
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Parse Error - "+ std::string(e.what()));
            }

        }

        GetInstrumentsResponse::GetInstrumentsResponse(std::string &csvData)
        {
            Utils::CSVObjType instrumentsData;
            Utils::ParseCSV(csvData, instrumentsData);
            for (auto it = instrumentsData.begin(); it != instrumentsData.end(); ++it)
                instruments.push_back(*it);
        }

/*
    /// <summary>
    /// Trigger range structure
    /// </summary>
    struct TrigerRange
    {
        TrigerRange(Dictionary<string, dynamic> data)
        {
            try
            {
                InstrumentToken = Convert.Touint32_t(data["instrument_token"]);
                Lower = data["lower"];
                Upper = data["upper"];
                Percentage = data["percentage"];
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }

        }
        uint32_t InstrumentToken;
        double Lower;
        double Upper;
        double Percentage;
    };
*/
        User::User(std::string jsonData)
        {
            try
            {
                json::value data = json::value::parse(jsonData); 
                apiKey = data["data"]["api_key"].as_string();
                json::array array_data(data["data"]["products"].as_array());
                for(auto itr=data["data"]["products"].as_array().begin(); itr != data["data"]["products"].as_array().end(); itr++)
                {
                    products.push_back((*itr).as_string());
                }
                Utils::JsonArrayDeserialize(data["data"]["products"].as_array(), products);
                userName = data["data"]["user_name"].as_string();
                userShortName = data["data"]["user_shortname"].as_string();
                avatarURL = data["data"]["avatar_url"].as_string();
                broker = data["data"]["broker"].as_string();
                accessToken = data["data"]["access_token"].as_string();
                publicToken = data["data"]["public_token"].as_string();
                refreshToken = data["data"]["refresh_token"].as_string();
                userType = data["data"]["user_type"].as_string();
                userId = data["data"]["user_id"].as_string();
                loginTime = TIME(data["data"]["login_time"]);
                Utils::JsonArrayDeserialize(data["data"]["exchanges"].as_array(), exchanges);
                Utils::JsonArrayDeserialize(data["data"]["order_types"].as_array(), orderTypes);
                Email = data["data"]["email"].as_string();
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }

        }
        TokenSet::TokenSet(std::string jsonData)
        {
            try
            {
                json::value data = json::value::parse(jsonData);
                userId = data["data"]["user_id"].as_string();
                accessToken = data["data"]["access_token"].as_string();
                refreshToken = data["data"]["refresh_token"].as_string();
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }
        }

        Profile::Profile(std::string jsonData)
        {
            try
            {
                json::value data = json::value::parse(jsonData);
                products = Utils::JsonArrayDeserialize(data["data"]["products"].as_array(), products);
                userName = data["data"]["user_name"].as_string();
                userShortName = data["data"]["user_shortname"].as_string();
                avatarURL = data["data"]["avatar_url"].as_string();
                broker = data["data"]["broker"].as_string();
                userType = data["data"]["user_type"].as_string();
                exchanges = Utils::JsonArrayDeserialize(data["data"]["exchanges"].as_array(), exchanges);
                orderTypes = Utils::JsonArrayDeserialize(data["data"]["order_types"].as_array(), orderTypes);
                email = data["data"]["email"].as_string();
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }

        }

        Quote::Quote(json::value data)
        {
            try
            {
                instrumentToken = data["instrument_token"].as_number().to_uint32();
                timestamp = data["timestamp"].as_string();
                lastPrice = data["last_price"].as_double();

                change = data["net_change"].as_double();

                open = data["ohlc"]["open"].as_double();
                close = data["ohlc"]["close"].as_double();
                low = data["ohlc"]["low"].as_double();
                high = data["ohlc"]["high"].as_double();       

                if (data.has_field("last_quantity"))
                {
                    // Non index quote
                    lastQuantity = data["last_quantity"].as_number().to_uint32();
                    lastTradeTime = data["last_trade_time"].as_string();
                    averagePrice = data["average_price"].as_double();
                    volume = data["volume"].as_number().to_uint32();

                    buyQuantity = data["buy_quantity"].as_number().to_uint32();
                    sellQuantity = data["sell_quantity"].as_number().to_uint32();

                    OI = data["oi"].as_number().to_uint32();

                    OIDayHigh = data["oi_day_high"].as_number().to_uint32();
                    OIDayLow = data["oi_day_low"].as_number().to_uint32();

                    lowerCircuitLimit = data["lower_circuit_limit"].as_double();
                    upperCircuitLimit = data["upper_circuit_limit"].as_double();

                    if (data.has_field("depth"))
                    {
                        if(data.has_field("buy"))
                        {
                            json::array buy_depth(data["buy"].as_array());
                            for(auto itr=buy_depth.begin(); itr != buy_depth.end(); itr++)
                            {
                                bids.push_back(DepthItem(*itr));
                            }
                        }
                        if (data.has_field("sell"))
                        {
                            json::array sell_depth = data["sell"].as_array();
                            for(auto itr=sell_depth.begin(); itr != sell_depth.end(); itr++)
                            {
                                offers.push_back(DepthItem(*itr));
                            }
                        }
                    }
                }
                else
                {
                    // Index quote
                    lastQuantity = 0;
                    lastTradeTime = "";
                    averagePrice = 0;
                    volume = 0;

                    buyQuantity = 0;
                    sellQuantity = 0;

                    OI = 0;

                    OIDayHigh = 0;
                    OIDayLow = 0;

                    lowerCircuitLimit = 0;
                    upperCircuitLimit = 0;
                }
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + data.serialize() + e.what());
            }

        }

        QuoteResponse::QuoteResponse(std::string jsonData, vector <std::string> instrumentIds)
        {
            try
            {
                json::value jsonObj = json::value::parse(jsonData);
                json::value data = jsonObj["data"];
                for (auto itr = instrumentIds.begin(); itr != instrumentIds.end(); itr++)
                {
                    quoteList.push_back(Quote(data[*itr]));
                }
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }

        }

        OHLC::OHLC(json::value data)
        {
            try
            {
                instrumentToken = data["instrument_token"].as_number().to_uint32();
                lastPrice = data["last_price"].as_double();

                open = data["ohlc"]["open"].as_double();
                close = data["ohlc"]["close"].as_double();
                low = data["ohlc"]["low"].as_double();
                high = data["ohlc"]["high"].as_double();
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + data.serialize() + e.what());
            }

        }

        OHLCResponse::OHLCResponse(std::string jsonData, vector<std::string> instrumentIds)
        {
            try
            {
                json::value jsonObj = json::value::parse(jsonData);
                json::value data = jsonObj["data"];
                for (auto itr = instrumentIds.begin(); itr != instrumentIds.end(); itr++)
                {
                    ohlcList.push_back(OHLC(data[*itr]));
                }
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }

        }

        LTP::LTP(json::value data)
        {
            try
            {
                instrumentToken = data["instrument_token"].as_number().to_uint32();
                lastPrice = data["last_price"].as_double();
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + data.serialize() + e.what());
            }

        }

        LTPResponse::LTPResponse(std::string jsonData, vector<std::string> instrumentIds)
        {
            try
            {
                json::value jsonObj = json::value::parse(jsonData);
                json::value data = jsonObj["data"];
                for (auto itr = instrumentIds.begin(); itr != instrumentIds.end(); itr++)
                {
                    ltpList.push_back(LTP(data[*itr]));
                }
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }

        }

#if 0
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
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }

        }

        double Quantity;
        std::string Fund;
        std::string Folio;
        double AveragePrice;
        std::string TradingSymbol;
        double LastPrice;
        double PNL;
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
                LastPriceDate = TIME(data["last_price_date"]);
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }

        }

        std::string TradingSymbol;
        std::string AMC;
        std::string Name;

        bool PurchaseAllowed;
        bool RedemtpionAllowed;

        double MinimumPurchaseAmount;
        double PurchaseAmountMultiplier;
        double MinimumAdditionalPurchaseAmount;
        double MinimumRedemptionQuantity;
        double RedemptionQuantityMultiplier;
        double LastPrice;

        std::string DividendType;
        std::string SchemeType;
        std::string Plan;
        std::string SettlementType;
        std::string LastPriceDate;
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
                OrderTimestamp = TIME(data["order_timestamp"]);
                AveragePrice = data["average_price"];
                TransactionType = data["transaction_type"];
                ExchangeOrderId = data["exchange_order_id"];
                ExchangeTimestamp = TIME(data["exchange_timestamp"]);
                Fund = data["fund"];
                Variety = data["variety"];
                Folio = data["folio"];
                Tradingsymbol = data["tradingsymbol"];
                Tag = data["tag"];
                OrderId = data["order_id"];
                Status = data["status"];
                LastPrice = data["last_price"];
            }
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }

        }

        std::string StatusMessage;
        std::string PurchaseType;
        std::string PlacedBy;
        double Amount;
        double Quantity;
        std::string SettlementId;
        std::string OrderTimestamp;
        double AveragePrice;
        std::string TransactionType;
        std::string ExchangeOrderId;
        std::string ExchangeTimestamp;
        std::string Fund;
        std::string Variety;
        std::string Folio;
        std::string Tradingsymbol;
        std::string Tag;
        std::string OrderId;
        std::string Status;
        double LastPrice;
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
                Created = TIME(data["created"]);
                LastInstalment = TIME(data["last_instalment"]);
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
            catch (std::exception e)
            {
                throw std::runtime_error("Unable to parse data. " + jsonData + e.what());
            }

        }

        std::string DividendType;
        int PendingInstalments;
        std::string Created;
        std::string LastInstalment;
        std::string TransactionType;
        std::string Frequency;
        int InstalmentDate;
        std::string Fund;
        std::string SIPId;
        std::string Tradingsymbol;
        std::string Tag;
        int InstalmentAmount;
        int Instalments;
        std::string Status;
        std::string OrderId;
    }
#endif
}