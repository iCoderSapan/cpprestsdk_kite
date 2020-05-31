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
#include <ctime>
#include <cstdint>
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

        std::time_t LastTradeTime;
        uint32_t OI;
        uint32_t OIDayHigh;
        uint32_t OIDayLow;
        std::time_t Timestamp;
    }

    /// <summary>
    /// Market depth item structure
    /// </summary>
    struct DepthItem
    {
        DepthItem(json::value data)
        {
            Quantity = Convert.Touint32_t(data["quantity"]);
            Price = data["price"];
            Orders = Convert.Touint32_t(data["orders"]);
        }

        uint32_t Quantity;
        double Price;
        uint32_t Orders;
    }

    /// <summary>
    /// Historical structure
    /// </summary>
    struct Historical
    {
        Historical(json::value jsonValueObj)
        {
            json::array jsonCandleObj = jsonValueObj.as_array();

            for(auto itr=candlesJsonObj.begin(); itr != candlesJsonObj.end(); itr++)
            {
                historicals.push_back(Historical(*itr));
            }
            timeStamp = Utils::to_timestamp(jsonCandleObj[0].as_string());
            open = jsonCandleObj[1].as_double();
            high = jsonCandleObj[2].as_double();
            low = jsonCandleObj[3].as_double();
            close = jsonCandleObj[4].as_double();
            volume = jsonCandleObj[5].as_number().to_uint32();
            OI = candlesJsonObj.size() > 6 ? jsonCandleObj[6].as_number().to_uint32() : 0;
        }

        std::time_t timeStamp;
        double open;
        double high;
        double low;
        double close;
        uint32_t volume;
        uint32_t OI;
    };

    /// <summary>
    /// Historical structure
    /// </summary>
    struct HistoricalResponse
    {
        HistoricalResponse(std::string jsonData)
        {
            json::value jsonObj = json::value:parse(jsonData);
            json::array candlesJsonObj = jsonObj["data"]["candles"].as_array();

            for(auto itr=candlesJsonObj.begin(); itr != candlesJsonObj.end(); itr++)
            {
                historicals.push_back(Historical(*itr));
            }
        }
        std::list<Historical> historicals;
    };

    /// <summary>
    /// Holding structure
    /// </summary>
    struct Holding
    {
        Holding(json::value data)
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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }

        std::string product;
        std::string exchange;
        double price;
        double lastPrice;
        int collateralQuantity;
        double PNL;
        double closePrice;
        double averagePrice;
        std::string tradingSymbol;
        std::string collateralType;
        int T1Quantity;
        uint32_t instrumentToken;
        std::string ISIN;
        int realisedQuantity;
        int quantity;
    };

    /// <summary>
    /// Holdings response structure
    /// </summary>
    struct HoldingsResponse
    {
        HoldingsResponse(std::string jsonData)
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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }

        std::list<Holding> holdingsData;

    };

    /// <summary>
    /// ConvertPosition response structure
    /// </summary>
    struct ConvertPositionResponse
    {
        ConvertPositionResponse(std::string jsonData)
        {
            try
            {
                json::value jsonObj = json::value::parse(jsonData);
                status = jsonObj['data'].as_bool();
                
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }

        std:::bool status;
    };

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
        public:
        UserMarginsResponse() {}
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
    /// Place, Modify, Cancel - OrderResponse structure
    /// </summary>
    struct OrderResponse
    {
        OrderResponse(std::string jsonData)
        {
            try
            {
                json::value data = json::value::parse(jsonData);
                order_id = data["data"]["order_id"].as_string();
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }
        std::string order_id;
    };

    /// <summary>
    /// GetOrdersResponse structure
    /// </summary>
    struct GetOrdersResponse
    {
        GetOrdersResponse(std::string jsonData)
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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }
        std::list<Order> orders;
    };

    /// <summary>
    /// OrderHistoryResponse structure
    /// </summary>
    struct OrderHistoryResponse
    {
        OrderHistoryResponse(std::string jsonData)
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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }
        std::list<Order> orderHistory;
    };

    /// <summary>
    /// OrderTradesResponse structure
    /// </summary>
    struct OrderTradesResponse
    {
        OrderTradesResponse(std::string jsonData)
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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }
        std::list<Trade> orderTrades;
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

        std::string Tradingsymbol;
        double Margin;
        double COLower;
        double COUpper;
        double MISMultiplier;
        double MISMargin;
        double NRMLMargin;
    };
    /// <summary>
    /// Position structure
    /// </summary>
    struct Position
    {
        Position(json::value data)
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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }

        std::string product;
        int overnightQuantity;
        std::string exchange;
        double sellValue;
        double buyM2M;
        double lastPrice;
        std::string tradingSymbol;
        double realised;
        double PNL;
        double multiplier;
        int sellQuantity;
        double sellM2M;
        double buyValue;
        int buyQuantity;
        double averagePrice;
        double unrealised;
        double value;
        double buyPrice;
        double sellPrice;
        double M2M;
        uint32_t instrumentToken;
        double closePrice;
        int quantity;
        int dayBuyQuantity;
        double dayBuyPrice;
        double dayBuyValue;
        int daySellQuantity;
        double daySellPrice;
        double daySellValue;
    };

    /// <summary>
    /// Position response structure
    /// </summary>
    struct PositionResponse
    {
        PositionResponse(std::string jsonData)
        {
            json::value jsonObj = json::value::parse(jsonData);

            Day = new List<Position>();
            Net = new List<Position>();

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

        std::list<Position> day;
        std::list<Position> net;
    }

    /// <summary>
    /// Order structure
    /// </summary>
    struct Order
    {
        Order(json::value data)
        {
            try
            {
                averagePrice = data["average_price"].as_double();
                cancelledQuantity = data["cancelled_quantity"].as_integer();
                disclosedQuantity = data["disclosed_quantity"].as_integer();
                exchange = data["exchange"].as_string();
                exchangeOrderId = data["exchange_order_id"].as_string();
                exchangeTimestamp = Utils::to_timestamp(data["exchange_timestamp"]);
                filledQuantity = data["filled_quantity"].as_integer();
                instrumentToken = data["instrument_token"].as_number().to_uint32();
                orderId = data["order_id"].as_string();
                orderTimestamp = Utils::to_timestamp(data["order_timestamp"]);
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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }

        double averagePrice;
        int cancelledQuantity;
        int disclosedQuantity;
        std::string exchange;
        std::string exchangeOrderId;
        std::time_t exchangeTimestamp;
        int filledQuantity;
        uint32_t instrumentToken;
        std::string orderId;
        std::time_t orderTimestamp;
        std::string orderType;
        std::string parentOrderId;
        int pendingQuantity;
        std::string placedBy;
        double price;
        std::string product;
        int quantity;
        std::string status;
        std::string statusMessage;
        std::string tag;
        std::string tradingSymbol;
        std::string transactionType;
        double triggerPrice;
        std::string validity;
        std::string variety;
    }

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
                CreatedAt = Utils::to_timestamp(data["created_at"]);
                UpdatedAt = Utils::to_timestamp(data["updated_at"]);
                ExpiresAt = Utils::to_timestamp(data["expires_at"]);
                Meta = new GTTMeta(data["meta"]);
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }
        }

        int Id;
        GTTCondition? Condition;
        std::string TriggerType;
        List<GTTOrder> Orders;
        std::string Status;
        std::time_t CreatedAt;
        std::time_t UpdatedAt;
        std::time_t ExpiresAt;
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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
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
        std::string TradingSymbol;
        std::string Exchange;
        int InstrumentToken;
        std::string TriggerType;
        double LastPrice;
        List<GTTOrderParams> Orders;
        List<double> TriggerPrices;
    }

    /// <summary>
    /// GTTOrderParams structure
    /// </summary>
    struct GTTOrderParams
    {
        int Quantity;
        double Price;
        // Order type (LIMIT, SL, SL-M, MARKET)
        std::string OrderType;
        // Product code (NRML, MIS, CNC)
        std::string Product;
        // Transaction type (BUY, SELL)
        std::string TransactionType;
    }

    /// <summary>
    /// Instrument structure
    /// </summary>
    struct Instrument
    {
        Instrument(map<std::string, std::string> data)
        {
            try
            {
                instrumentToken = std::stoul(data["instrument_token"]);
                exchangeToken = std::stoul(data["exchange_token"]);
                tradingSymbol = data["tradingsymbol"];
                name = data["name"];
                lastPrice = std::stod(data["last_price"]);
                tickSize = std::stod(data["tick_size"]);
                expiry = Utils::to_timestamp(data["expiry"]);
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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }

        uint32_t instrumentToken;
        uint32_t exchangeToken;
        std::string tradingSymbol;
        std::string name;
        double lastPrice;
        double tickSize;
        std::time_t expiry;
        std::string instrumentType;
        std::string segment;
        std::string exchange;
        double strike;
        uint32_t lotSize;
    }

    struct GetInstrumentsResponse
    {
        GetInstrumentsResponse(std::string &csvData)
        {
            Utils::CSVObjType instrumentsData;
            Utils::ParseCSV(csvData, instrumentsData);
            foreach (auto it = instrumentsData.begin(); it != instrumentsData.end(); ++it)
                instruments.push_back(*it);
        }
        std::list<Instrument> instruments;
    };
    

    /// <summary>
    /// Trade structure
    /// </summary>
    struct Trade
    {
        Trade(json::value data)
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
                fillTimestamp = Utils::to_timestamp(data["fill_timestamp"].as_string());
                exchangeTimestamp = Utils::to_timestamp(data["exchange_timestamp"].as_string());
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }

        std::string tradeId;
        std::string orderId;
        std::string exchangeOrderId;
        std::string tradingSymbol;
        std::string exchange;
        uint32_t instrumentToken;
        std::string transactionType;
        std::string product;
        double averagePrice;
        int quantity;
        std::time_t fillTimestamp;
        std::time_t exchangeTimestamp;
    };

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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }
        uint32_t InstrumentToken;
        double Lower;
        double Upper;
        double Percentage;
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
                LoginTime = Utils::to_timestamp(data["data"]["login_time"]);
                Utils::jsonArrayDeserialise(data["data"]["exchanges"].as_array(), exchanges);
                Utils::jsonArrayDeserialise(data["data"]["order_types"].as_array(), orderTypes);
                Email = data["data"]["email"];
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }

        std::string apiKey;
        std::vector<string> products;
        std::string userName;
        std::string userShortName;
        std::string avatarURL;
        std::string broker;
        std::string accessToken;
        std::string publicToken;
        std::string refreshToken;
        std::string userType;
        std::string userId;
        std::time_t LoginTime;
        std::string[] Exchanges;
        std::string[] OrderTypes;
        std::string Email;
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
        std::string userName;
        std::string userShortName;
        std::string avatarURL;
        std::string broker;
        std::string userType;
        vector<string> exchanges;
        vector<string> OrderTypes;
        std::string email;
    }

    /// <summary>
    /// Quote structure
    /// </summary>
    struct Quote
    {
        Quote(json::value jsonData)
        {
            try
            {
                json::value data = jsonObj["data"];
                instrumentToken = data["instrument_token"].as_number().to_uint32();
                timestamp = Utils::to_timestamp(data["timestamp"].as_string());
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
                    lastTradeTime = Utils::to_timestamp(data["last_trade_time"].as_string());
                    averagePrice = data["average_price"].as_double();
                    volume = data["volume"]).as_number().to_uint32();

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
                            json::value::array buy_depth(data["buy"].as_array());
                            for(auto itr=buy_depth.begin(); itr != buy_depth.end(); itr++)
                            {
                                bids.push_back(DepthItem(*itr));
                            }
                        }
                        if (data.has_field("sell"))
                        {
                            json::value::array sell_depth = data["sell"].as_array();
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
                    lastTradeTime = 0
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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }

        uint32_t instrumentToken;
        double lastPrice;
        uint32_t lastQuantity;
        double averagePrice;
        uint32_t volume;
        uint32_t buyQuantity;
        uint32_t sellQuantity;
        double open;
        double high;
        double low;
        double close;
        double change;
        double lowerCircuitLimit;
        double upperCircuitLimit;
        std::list<DepthItem> bids;
        std::list<DepthItem> offers;

        // KiteConnect 3 Fields

        std::time_t lastTradeTime;
        uint32_t OI;
        uint32_t OIDayHigh;
        uint32_t OIDayLow;
        std::time_t timestamp;
    }

    /// <summary>
    /// Quote Response structure
    /// </summary>
    struct QuoteResponse
    {
        QuoteResponse(std::string jsonData, vector <std::string> instrumentIds)
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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }
        std::list <Quote> quoteList;
    };

    /// <summary>
    /// OHLC Quote structure
    /// </summary>
    struct OHLC
    {
        OHLC(json::value data)
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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }
        uint32_t instrumentToken;
        double lastPrice;
        double open;
        double close;
        double high;
        double low;
    }
    
    /// <summary>
    /// OHLC Response structure
    /// </summary>
    struct OHLCResponse
    {
        OHLCResponse(std::string jsonData, vector<std::string> instrumentIds)
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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }
        std::list<OHLC> ohlcList;
    }

    /// <summary>
    /// LTP Quote structure
    /// </summary>
    struct LTP
    {
        LTP(json::value data)
        {
            try
            {
                instrumentToken = data["instrument_token"].as_number().to_uint32();
                lastPrice = data["last_price"].as_double();
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }
        uint32_t instrumentToken;
        double lastPrice;
    }

    /// <summary>
    /// LTP Response structure
    /// </summary>
    struct LTPResponse
    {
        LTPResponse(std::string jsonData, vector<std::string> instrumentIds)
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
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
            }

        }
        std::list<LTP> ltpList;
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
                LastPriceDate = Utils::to_timestamp(data["last_price_date"]);
            }
            catch (Exception e)
            {
                throw new DataException("Unable to parse data. " + Utils.JsonSerialize(data), HttpStatusCode.OK, e);
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
        std::time_t LastPriceDate;
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
                OrderTimestamp = Utils::to_timestamp(data["order_timestamp"]);
                AveragePrice = data["average_price"];
                TransactionType = data["transaction_type"];
                ExchangeOrderId = data["exchange_order_id"];
                ExchangeTimestamp = Utils::to_timestamp(data["exchange_timestamp"]);
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

        std::string StatusMessage;
        std::string PurchaseType;
        std::string PlacedBy;
        double Amount;
        double Quantity;
        std::string SettlementId;
        std::time_t OrderTimestamp;
        double AveragePrice;
        std::string TransactionType;
        std::string ExchangeOrderId;
        std::time_t ExchangeTimestamp;
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
                Created = Utils::to_timestamp(data["created"]);
                LastInstalment = Utils::to_timestamp(data["last_instalment"]);
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

        std::string DividendType;
        int PendingInstalments;
        std::time_t Created;
        std::time_t LastInstalment;
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

}