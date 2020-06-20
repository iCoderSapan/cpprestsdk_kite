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

    /// <summary>
    /// Market depth item structure
    /// </summary>
    struct DepthItem
    {
        DepthItem(json::value data);
        uint32_t quantity;
        double price;
        uint32_t orders;
    };

    /// <summary>
    /// Historical structure
    /// </summary>
    struct Historical
    {
        Historical(json::value jsonValueObj);

        std::string timeStamp;
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
        HistoricalResponse(std::string jsonData);
        std::list<Historical> historicals;
    };

    /// <summary>
    /// Holding structure
    /// </summary>
    struct Holding
    {
        Holding(json::value data);
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
        HoldingsResponse(std::string jsonData);
        std::list<Holding> holdingsData;
    };

    /// <summary>
    /// ConvertPosition response structure
    /// </summary>
    struct ConvertPositionResponse
    {
        ConvertPositionResponse(std::string jsonData);
        bool status;
    };

    /// <summary>
    /// Available margin structure
    /// </summary>
    struct AvailableMargin
    {
        AvailableMargin () {}
        AvailableMargin(json::value &data);

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
        UtilisedMargin() {}
        UtilisedMargin(json::value &data);

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
        UserMargin () {}
        UserMargin(json::value &data);
        UserMargin(std::string &jsonData);
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
        UserMarginsResponse(std::string jsonData);

        UserMargin equity;
        UserMargin commodity;
    };

    /// <summary>
    /// Order structure
    /// </summary>
    struct Order
    {
        Order(json::value data);

        double averagePrice;
        int cancelledQuantity;
        int disclosedQuantity;
        std::string exchange;
        std::string exchangeOrderId;
        std::string exchangeTimestamp;
        int filledQuantity;
        uint32_t instrumentToken;
        std::string orderId;
        std::string orderTimestamp;
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
    };

    /// <summary>
    /// Place, Modify, Cancel - OrderResponse structure
    /// </summary>
    struct OrderResponse
    {
        OrderResponse(std::string jsonData);

        std::string order_id;
    };

    /// <summary>
    /// GetOrdersResponse structure
    /// </summary>
    struct GetOrdersResponse
    {
        GetOrdersResponse(std::string jsonData);

        std::list<Order> orders;
    };

    /// <summary>
    /// OrderHistoryResponse structure
    /// </summary>
    struct OrderHistoryResponse
    {
        OrderHistoryResponse(std::string jsonData);

        std::list<Order> orderHistory;
    };

    /// <summary>
    /// Trade structure
    /// </summary>
    struct Trade
    {
        Trade(json::value data);

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
        std::string fillTimestamp;
        std::string exchangeTimestamp;
    };

    /// <summary>
    /// OrderTradesResponse structure
    /// </summary>
    struct OrderTradesResponse
    {
        OrderTradesResponse(std::string jsonData);

        std::list<Trade> orderTrades;
    };
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
    /// <summary>
    /// Position structure
    /// </summary>
    struct Position
    {
        Position(json::value data);

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
        PositionResponse(std::string jsonData);

        std::list<Position> day;
        std::list<Position> net;
    };

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

    /// <summary>
    /// Instrument structure
    /// </summary>
    struct Instrument
    {
        Instrument(std::map<std::string, std::string> data);

        uint32_t instrumentToken;
        uint32_t exchangeToken;
        std::string tradingSymbol;
        std::string name;
        double lastPrice;
        double tickSize;
        std::string expiry;
        std::string instrumentType;
        std::string segment;
        std::string exchange;
        double strike;
        uint32_t lotSize;
    };

    struct GetInstrumentsResponse
    {
        GetInstrumentsResponse(std::string &csvData);

        std::list<Instrument> instruments;
    };
    
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
    /// <summary>
    /// User structure
    /// </summary>
    struct User
    {
        User(std::string jsonData);

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
        std::string loginTime;
        std::vector<std::string> exchanges;
        std::vector<std::string> orderTypes;
        std::string Email;
    };

    struct TokenSet
    {
        TokenSet(std::string jsonData);

        std::string userId;
        std::string accessToken;
        std::string refreshToken;
    };

    /// <summary>
    /// User structure
    /// </summary>
    struct Profile
    {
        Profile(std::string jsonData);

        vector<string> products;
        std::string userName;
        std::string userShortName;
        std::string avatarURL;
        std::string broker;
        std::string userType;
        vector<string> exchanges;
        vector<string> orderTypes;
        std::string email;
    };

    /// <summary>
    /// Quote structure
    /// </summary>
    struct Quote
    {
        Quote(json::value data);

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

        std::string lastTradeTime;
        uint32_t OI;
        uint32_t OIDayHigh;
        uint32_t OIDayLow;
        std::string timestamp;
    };

    /// <summary>
    /// Quote Response structure
    /// </summary>
    struct QuoteResponse
    {
        QuoteResponse(std::string jsonData, vector <std::string> instrumentIds);

        std::list <Quote> quoteList;
    };

    /// <summary>
    /// OHLC Quote structure
    /// </summary>
    struct OHLC
    {
        OHLC(json::value data);

        uint32_t instrumentToken;
        double lastPrice;
        double open;
        double close;
        double high;
        double low;
    };
    
    /// <summary>
    /// OHLC Response structure
    /// </summary>
    struct OHLCResponse
    {
        OHLCResponse(std::string jsonData, vector<std::string> instrumentIds);

        std::list<OHLC> ohlcList;
    };

    /// <summary>
    /// LTP Quote structure
    /// </summary>
    struct LTP
    {
        LTP(json::value data);

        uint32_t instrumentToken;
        double lastPrice;
    };

    /// <summary>
    /// LTP Response structure
    /// </summary>
    struct LTPResponse
    {
        LTPResponse(std::string jsonData, vector<std::string> instrumentIds);

        std::list<LTP> ltpList;
    };

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