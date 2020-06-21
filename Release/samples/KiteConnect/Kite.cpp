﻿/*
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Collections;
using System.Reflection;
*/
#include <string>
#include <cpprest/http_client.h> 
#include <cpprest/http_headers.h>
#include <cpprest/http_msg.h>
#include <cpprest/filestream.h> 
//----- Some standard C++ headers emitted for brevity
#include "cpprest/json.h" 
#include "cpprest/http_listener.h" 
#include "cpprest/uri.h" 
//#include "cpprest/asyncrt_utils.h"
#include <boost/algorithm/string.hpp>

#include "Kite.h"

#include <iomanip>
#include <sstream>
#include <iostream>
#include <openssl/sha.h>
#include <memory>

//////////////////////////////////////////////// 
// A Simple HTTP Client to Demonstrate  
// REST SDK Client programming model 
// The Toy sample shows how one can read  
// contents of a web page 
// 
using namespace utility;  // Common utilities like std::string conversions 
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
    /// The API client class. In production, you may initialize a single instance of this class per `APIKey`.
    /// </summary>
        Kite::Kite(std::string APIKey, web::web_proxy proxy, std::string accessToken, std::string root, bool debug, int timeout, int Pool)
        {
            _accessToken = accessToken;
            _apiKey = APIKey;
            if (!root.empty())
                _root = root;

            log.SetLogging(debug);

            _proxy = proxy;

            //_timeout = Timeout;
            _timeout = std::chrono::seconds(timeout);
            http_client_config clientConfig;
            clientConfig.set_timeout(_timeout);
            // if(proxy != "") {
            //     clientConfig.set_proxy(*proxy);
            // }
            httpClient = std::make_shared<http_client>(http_client(_root, clientConfig)); //Fill in the root ur
            //ServicePointManager.DefaultConnectionLimit = Pool;
        }

        /// <summary>
        /// Enabling logging prints HTTP request and response summaries to console
        /// </summary>
        /// <param name="enableLogging">Set to true to enable logging</param>
        void Kite::EnableLogging(bool enableLogging)
        {
            log.SetLogging(enableLogging);
        }

        /// <summary>
        /// Set a callback hook for session (`TokenException` -- timeout, expiry etc.) errors.
		/// An `AccessToken` (login session) can become invalid for a number of
        /// reasons, but it doesn't make sense for the client to
		/// try and catch it during every API call.
        /// A callback method that handles session errors
        /// can be set here and when the client encounters
        /// a token error at any point, it'll be called.
        /// This callback, for instance, can log the user out of the UI,
		/// clear session cookies, or initiate a fresh login.
        /// </summary>
        /// <param name="Method">Action to be invoked when session becomes invalid.</param>
        void Kite::SetSessionExpiryHook(std::function<void(void)> method)
        {
            _sessionHook = method;
        }

        /// <summary>
        /// Set the `AccessToken` received after a successful authentication.
        /// </summary>
        /// <param name="AccessToken">Access token for the session.</param>
        void Kite::SetAccessToken(std::string accessToken)
        {
            _accessToken = accessToken;
        }

        /// <summary>
        /// Get the remote login url to which a user should be redirected to initiate the login flow.
        /// </summary>
        /// <returns>Login url to authenticate the user.</returns>
        std::string Kite::GetLoginURL()
        {
            std::stringstream url;
            url<<_login<<"?api_key="<<_apiKey<<"&v=3";
            return url.str();
        }

        /// <summary>
        /// Do the token exchange with the `RequestToken` obtained after the login flow,
		/// and retrieve the `AccessToken` required for all subsequent requests.The
        /// response contains not just the `AccessToken`, but metadata for
        /// the user who has authenticated.
        /// </summary>
        /// <param name="RequestToken">Token obtained from the GET paramers after a successful login redirect.</param>
        /// <param name="AppSecret">API secret issued with the API key.</param>
        /// <returns>User structure with tokens and profile data</returns>
        User Kite::GenerateSession(std::string RequestToken, std::string AppSecret)
        {
            std::string checksum = Utils::SHA256(_apiKey + RequestToken + AppSecret);
            logging::Write(RequestToken);
            ParamType param{
                {"api_key", _apiKey},
                {"request_token", RequestToken},
                {"checksum", checksum}
            };

            std::string jsonData = Post("api.token", param);
            return User(jsonData);
        }

        /// <summary>
        /// Kill the session by invalidating the access token
        /// </summary>
        /// <param name="AccessToken">Access token to invalidate. Default is the active access token.</param>
        /// <returns>Json response in the form of nested std::string dictionary.</returns>
        std::string Kite::InvalidateAccessToken(std::string AccessToken)
        {
            ParamType param;
            //map<std::string, std::string> param;

            Utils::AddIfNotNull(param, "api_key", _apiKey);
            Utils::AddIfNotNull(param, "access_token", AccessToken);

            return Delete("api.token", param);
        }

        /// <summary>
        /// Invalidates RefreshToken
        /// </summary>
        /// <param name="RefreshToken">RefreshToken to invalidate</param>
        /// <returns>Json response in the form of nested std::string dictionary.</returns>
        std::string Kite::InvalidateRefreshToken(std::string RefreshToken)
        {
            ParamType param;

            Utils::AddIfNotNull(param, "api_key", _apiKey);
            Utils::AddIfNotNull(param, "refresh_token", RefreshToken);
            return Delete("api.token", param);
        }

        /// <summary>
        /// Renew AccessToken using RefreshToken
        /// </summary>
        /// <param name="RefreshToken">RefreshToken to renew the AccessToken.</param>
        /// <param name="AppSecret">API secret issued with the API key.</param>
        /// <returns>TokenRenewResponse that contains new AccessToken and RefreshToken.</returns>
        TokenSet Kite::RenewAccessToken(std::string RefreshToken, std::string AppSecret)
        {
            ParamType param;

            std::string checksum = Utils::SHA256(_apiKey + RefreshToken + AppSecret);

            Utils::AddIfNotNull(param, "api_key", _apiKey);
            Utils::AddIfNotNull(param, "refresh_token", RefreshToken);
            Utils::AddIfNotNull(param, "checksum", checksum);

            return TokenSet(Post("api.refresh", param));
        }

        /// <summary>
        /// Gets currently logged in user details
        /// </summary>
        /// <returns>User profile</returns>
        Profile Kite::GetProfile()
        {
            std::string profileData = Get("user.profile");

            return Profile(profileData);
        }

        ///// <summary>
        ///// Margin data for intraday trading
        ///// </summary>
        ///// <param name="Segment">Tradingsymbols under this segment will be returned</param>
        ///// <returns>List of margins of intruments</returns>
        //public List<InstrumentMargin> GetInstrumentsMargins(std::string Segment)
        //{
        //    var instrumentsMarginsData = Get("instrument.margins", new Dictionary<std::string, dynamic> { { "segment", Segment } });

        //    List<InstrumentMargin> instrumentsMargins = new List<InstrumentMargin>();
        //    foreach (Dictionary<std::string, dynamic> item in instrumentsMarginsData["data"])
        //        instrumentsMargins.Add(new InstrumentMargin(item));

        //    return instrumentsMargins;
        //}

        /// <summary>
        /// Get account balance and cash margin details for all segments.
        /// </summary>
        /// <returns>User margin response with both equity and commodity margins.</returns>
        UserMarginsResponse Kite::GetMargins()
        {
            std::string marginsData = Get("user.margins");
            return UserMarginsResponse(marginsData);
        }

        /// <summary>
        /// Get account balance and cash margin details for a particular segment.
        /// </summary>
        /// <param name="Segment">Trading segment (eg: equity or commodity)</param>
        /// <returns>Margins for specified segment.</returns>
        UserMargin Kite::GetMargins(std::string Segment)
        {
            ParamType params {
                {"segment", Segment}
            };
            std::string userMarginData = Get("user.segment_margins", params);
            return UserMargin(userMarginData);
        }

        /// <summary>
        /// Place an order
        /// </summary>
        /// <param name="Exchange">Name of the exchange</param>
        /// <param name="TradingSymbol">Tradingsymbol of the instrument</param>
        /// <param name="TransactionType">BUY or SELL</param>
        /// <param name="Quantity">Quantity to transact</param>
        /// <param name="Price">For LIMIT orders</param>
        /// <param name="Product">Margin product applied to the order (margin is blocked based on this)</param>
        /// <param name="OrderType">Order type (MARKET, LIMIT etc.)</param>
        /// <param name="Validity">Order validity</param>
        /// <param name="DisclosedQuantity">Quantity to disclose publicly (for equity trades)</param>
        /// <param name="TriggerPrice">For SL, SL-M etc.</param>
        /// <param name="SquareOffValue">Price difference at which the order should be squared off and profit booked (eg: Order price is 100. Profit target is 102. So squareoff = 2)</param>
        /// <param name="StoplossValue">Stoploss difference at which the order should be squared off (eg: Order price is 100. Stoploss target is 98. So stoploss = 2)</param>
        /// <param name="TrailingStoploss">Incremental value by which stoploss price changes when market moves in your favor by the same incremental value from the entry price (optional)</param>
        /// <param name="Variety">You can place orders of varieties; regular orders, after market orders, cover orders etc. </param>
        /// <param name="Tag">An optional tag to apply to an order to identify it (alphanumeric, max 8 chars)</param>
        /// <returns>Json response in the form of nested std::string dictionary.</returns>
        OrderResponse Kite::PlaceOrder(
            std::string exchange,
            std::string tradingSymbol,
            std::string transactionType,
            int &quantity,
            double price,
            std::string product,
            std::string orderType,
            std::string validity,
            int disclosedQuantity,
            double triggerPrice,
            double squareOffValue,
            double stoplossValue,
            double trailingStoploss,
            std::string variety,
            std::string tag)
        {
            ParamType params;

            Utils::AddIfNotNull(params, "exchange", exchange);
            Utils::AddIfNotNull(params, "tradingsymbol", tradingSymbol);
            Utils::AddIfNotNull(params, "transaction_type", transactionType);
            Utils::AddIfNotNull(params, "quantity", std::to_string(quantity));
            Utils::AddIfNotNull(params, "price", std::to_string(price));
            Utils::AddIfNotNull(params, "product", product);
            Utils::AddIfNotNull(params, "order_type", orderType);
            Utils::AddIfNotNull(params, "validity", validity);
            Utils::AddIfNotNull(params, "disclosed_quantity", std::to_string(disclosedQuantity));
            Utils::AddIfNotNull(params, "trigger_price", std::to_string(triggerPrice));
            Utils::AddIfNotNull(params, "squareoff", std::to_string(squareOffValue));
            Utils::AddIfNotNull(params, "stoploss", std::to_string(stoplossValue));
            Utils::AddIfNotNull(params, "trailing_stoploss", std::to_string(trailingStoploss));
            Utils::AddIfNotNull(params, "variety", variety);
            Utils::AddIfNotNull(params, "tag", tag);

            std::string jsonResp = Post("orders.place", params);
            return OrderResponse(jsonResp);
        }

        /// <summary>
        /// Modify an open order.
        /// </summary>
        /// <param name="OrderId">Id of the order to be modified</param>
        /// <param name="ParentOrderId">Id of the parent order (obtained from the /orders call) as BO is a multi-legged order</param>
        /// <param name="Exchange">Name of the exchange</param>
        /// <param name="TradingSymbol">Tradingsymbol of the instrument</param>
        /// <param name="TransactionType">BUY or SELL</param>
        /// <param name="Quantity">Quantity to transact</param>
        /// <param name="Price">For LIMIT orders</param>
        /// <param name="Product">Margin product applied to the order (margin is blocked based on this)</param>
        /// <param name="OrderType">Order type (MARKET, LIMIT etc.)</param>
        /// <param name="Validity">Order validity</param>
        /// <param name="DisclosedQuantity">Quantity to disclose publicly (for equity trades)</param>
        /// <param name="TriggerPrice">For SL, SL-M etc.</param>
        /// <param name="Variety">You can place orders of varieties; regular orders, after market orders, cover orders etc. </param>
        /// <returns>Json response in the form of nested std::string dictionary.</returns>
        OrderResponse Kite::ModifyOrder(
            std::string orderId,
            std::string parentOrderId,
            std::string exchange,
            std::string tradingSymbol,
            std::string transactionType,
            std::string quantity,
            double price,
            std::string product,
            std::string orderType,
            std::string validity,
            int disclosedQuantity,
            double triggerPrice,
            std::string variety)
        {
            ParamType params;

            std::string varietyString = variety;
            std::string productString = product;

            if ((productString == "bo" || productString == "co") && varietyString != productString)
                throw std::runtime_error("Invalid variety. It should be: " + productString);

            Utils::AddIfNotNull(params, "order_id", orderId);
            Utils::AddIfNotNull(params, "parent_order_id", parentOrderId);
            Utils::AddIfNotNull(params, "trigger_price", std::to_string(triggerPrice));
            Utils::AddIfNotNull(params, "variety", variety);

            if (varietyString == "bo" && productString == "bo")
            {
                Utils::AddIfNotNull(params, "quantity", quantity);
                Utils::AddIfNotNull(params, "price", std::to_string(price));
                Utils::AddIfNotNull(params, "disclosed_quantity", std::to_string(disclosedQuantity));
            }
            else if (varietyString != "co" && productString != "co")
            {
                Utils::AddIfNotNull(params, "exchange", exchange);
                Utils::AddIfNotNull(params, "tradingsymbol", tradingSymbol);
                Utils::AddIfNotNull(params, "transaction_type", transactionType);
                Utils::AddIfNotNull(params, "quantity", quantity);
                Utils::AddIfNotNull(params, "price", std::to_string(price));
                Utils::AddIfNotNull(params, "product", product);
                Utils::AddIfNotNull(params, "order_type", orderType);
                Utils::AddIfNotNull(params, "validity", validity);
                Utils::AddIfNotNull(params, "disclosed_quantity", std::to_string(disclosedQuantity));
            }

            std::string jsonResp = Put("orders.modify", params);
            return OrderResponse(jsonResp);
        }

        /// <summary>
        /// Cancel an order
        /// </summary>
        /// <param name="OrderId">Id of the order to be cancelled</param>
        /// <param name="Variety">You can place orders of varieties; regular orders, after market orders, cover orders etc. </param>
        /// <param name="ParentOrderId">Id of the parent order (obtained from the /orders call) as BO is a multi-legged order</param>
        /// <returns>Json response in the form of nested std::string dictionary.</returns>
        OrderResponse Kite::CancelOrder(std::string orderId,
                                  std::string variety,
                                  std::string parentOrderId)
        {
            ParamType params;

            Utils::AddIfNotNull(params, "order_id", orderId);
            Utils::AddIfNotNull(params, "parent_order_id", parentOrderId);
            Utils::AddIfNotNull(params, "variety", variety);

            return OrderResponse(Delete("orders.cancel", params));
        }

        /// <summary>
        /// Gets the collection of orders from the orderbook.
        /// </summary>
        /// <returns>GetOrdersResponse which contains List of orders.</returns>
        GetOrdersResponse Kite::GetOrders()
        {
            return GetOrdersResponse(Get("orders"));
        }

        /// <summary>
        /// Gets information about given OrderId.
        /// </summary>
        /// <param name="OrderId">Unique order id</param>
        /// <returns>List of order objects.</returns>
        OrderHistoryResponse Kite::GetOrderHistory(std::string OrderId)
        {
            ParamType param {
                {"order_id", OrderId},
            };

            return OrderHistoryResponse(Get("orders.history", param));
        }

        /// <summary>
        /// Retreive the list of trades executed (all or ones under a particular order).
        /// An order can be executed in tranches based on market conditions.
        /// These trades are individually recorded under an order.
        /// </summary>
        /// <param name="OrderId">is the ID of the order (optional) whose trades are to be retrieved. If no `OrderId` is specified, all trades for the day are returned.</param>
        /// <returns>List of trades of given order.</returns>
        OrderTradesResponse Kite::GetOrderTrades(std::string orderId)
        {
            if (!orderId.empty())
            {
                ParamType param 
                {
                   {"order_id", orderId}
                };
                return(OrderTradesResponse(Get("orders.trades", param)));
            }
            return OrderTradesResponse(Get("trades"));

        }

        /// <summary>
        /// Retrieve the list of positions.
        /// </summary>
        /// <returns>Day and net positions.</returns>
        PositionResponse Kite::GetPositions()
        {
            std::string positionsdata = Get("portfolio.positions");
            return PositionResponse(positionsdata);
        }

        /// <summary>
        /// Retrieve the list of equity holdings.
        /// </summary>
        /// <returns>List of holdings.</returns>
        HoldingsResponse Kite::GetHoldings()
        {
            std::string holdingsData = Get("portfolio.holdings");
            return HoldingsResponse(holdingsData);
        }

        /// <summary>
        /// Modify an open position's product type.
        /// </summary>
        /// <param name="Exchange">Name of the exchange</param>
        /// <param name="TradingSymbol">Tradingsymbol of the instrument</param>
        /// <param name="TransactionType">BUY or SELL</param>
        /// <param name="PositionType">overnight or day</param>
        /// <param name="Quantity">Quantity to convert</param>
        /// <param name="OldProduct">Existing margin product of the position</param>
        /// <param name="NewProduct">Margin product to convert to</param>
        /// <returns>Json response in the form of nested std::string dictionary.</returns>
        ConvertPositionResponse Kite::ConvertPosition(
            std::string exchange,
            std::string tradingSymbol,
            std::string transactionType,
            std::string positionType,
            int quantity,
            std::string oldProduct,
            std::string newProduct)
        {
            ParamType params;

            Utils::AddIfNotNull(params, "exchange", exchange);
            Utils::AddIfNotNull(params, "tradingsymbol", tradingSymbol);
            Utils::AddIfNotNull(params, "transaction_type", transactionType);
            Utils::AddIfNotNull(params, "position_type", positionType);
            Utils::AddIfNotNull(params, "quantity", std::to_string(quantity));
            Utils::AddIfNotNull(params, "old_product", oldProduct);
            Utils::AddIfNotNull(params, "new_product", newProduct);

            return ConvertPositionResponse(Put("portfolio.positions.modify", params));
        }

        /// <summary>
        /// Retrieve the list of market instruments available to trade.
        /// Note that the results could be large, several hundred KBs in size,
		/// with tens of thousands of entries in the list.
        /// </summary>
        /// <param name="Exchange">Name of the exchange</param>
        /// <returns>List of instruments.</returns>
        GetInstrumentsResponse Kite::GetInstruments(std::string exchange)
        {
            ParamType param;
            std::string instrumentsData;

            if (exchange.empty())
                instrumentsData = Get("market.instruments.all", param);
            else
            {
                param[0] = std::pair<std::string, std::string>("exchange", exchange);
                instrumentsData = Get("market.instruments", param);
            }
            return GetInstrumentsResponse(instrumentsData);
        }

        /// <summary>
        /// Retrieve quote and market depth of upto 200 instruments
        /// </summary>
        /// <param name="InstrumentId">Indentification of instrument in the form of EXCHANGE:TRADINGSYMBOL (eg: NSE:INFY) or InstrumentToken (eg: 408065)</param>
        /// <returns>Dictionary of all Quote objects with keys as in InstrumentId</returns>
        QuoteResponse Kite::GetQuote(vector<std::string> instrumentIds)
        {
            ParamType param;
            for (int idx = 0; idx < instrumentIds.size(); idx++)
            {
                param[idx] = {"i", instrumentIds[idx]};
            }

            std::string quoteResponse= Get("market.quote", param);

            return QuoteResponse(quoteResponse, instrumentIds);
        }

        /// <summary>
        /// Retrieve LTP and OHLC of upto 200 instruments
        /// </summary>
        /// <param name="InstrumentId">Indentification of instrument in the form of EXCHANGE:TRADINGSYMBOL (eg: NSE:INFY) or InstrumentToken (eg: 408065)</param>
        /// <returns>Dictionary of all OHLC objects with keys as in InstrumentId</returns>
        OHLCResponse Kite::GetOHLC(vector<std::string> instrumentIds)
        {
            ParamType param;
            for (int idx = 0; idx < instrumentIds.size(); idx++)
            {
                param[idx] = {"i", instrumentIds[idx]};
            }

            std::string ohlcResponse= Get("market.ohlc", param);
            return OHLCResponse(ohlcResponse, instrumentIds);
        }

        /// <summary>
        /// Retrieve LTP of upto 200 instruments
        /// </summary>
        /// <param name="InstrumentId">Indentification of instrument in the form of EXCHANGE:TRADINGSYMBOL (eg: NSE:INFY) or InstrumentToken (eg: 408065)</param>
        /// <returns>Dictionary with InstrumentId as key and LTP as value.</returns>
        LTPResponse Kite::GetLTP(vector<std::string> instrumentIds)
        {
            ParamType param;
            for (int idx = 0; idx < instrumentIds.size(); idx++)
            {
                param[idx] = {"i", instrumentIds[idx]};
            }

            std::string ohlcResponse= Get("market.ltp", param);
            return LTPResponse(ohlcResponse, instrumentIds);
            // Dictionary<std::string, dynamic> ltpData = Get("market.ltp", param)["data"];

            // Dictionary<std::string, LTP> ltps = new Dictionary<std::string, LTP>();
            // foreach (std::string item in ltpData.Keys)
            //     ltps.Add(item, new LTP(ltpData[item]));

            // return ltps;
        }

        /// <summary>
        /// Retrieve historical data (candles) for an instrument.
        /// </summary>
        /// <param name="InstrumentToken">Identifier for the instrument whose historical records you want to fetch. This is obtained with the instrument list API.</param>
        /// <param name="FromDate">Date in format yyyy-MM-dd for fetching candles between two days. Date in format yyyy-MM-dd hh:mm:ss for fetching candles between two timestamps.</param>
        /// <param name="ToDate">Date in format yyyy-MM-dd for fetching candles between two days. Date in format yyyy-MM-dd hh:mm:ss for fetching candles between two timestamps.</param>
        /// <param name="Interval">The candle record interval. Possible values are: minute, day, 3minute, 5minute, 10minute, 15minute, 30minute, 60minute</param>
        /// <param name="Continuous">Pass true to get continous data of expired instruments.</param>
        /// <param name="OI">Pass true to get open interest data.</param>
        /// <returns>List of Historical objects.</returns>
        HistoricalResponse Kite::GetHistoricalData(
            std::string instrumentToken,
            DateTime fromDate,
            DateTime toDate,
            std::string interval,
            bool continuous,
            bool OI)
        {
            ParamType param
            {
                {"instrument_token", instrumentToken},
                {"from", fromDate},
                {"to", toDate},
                {"interval", interval},
                {"continuous", continuous ? "1" : "0"},
                {"oi", OI ? "1" : "0"}
            };

            std::string historicalData = Get("market.historical", param);
            return HistoricalResponse(historicalData);
        }

        /// <summary>
        /// Retrieve the buy/sell trigger range for Cover Orders.
        /// </summary>
        /// <param name="InstrumentId">Indentification of instrument in the form of EXCHANGE:TRADINGSYMBOL (eg: NSE:INFY) or InstrumentToken (eg: 408065)</param>
        /// <param name="TrasactionType">BUY or SELL</param>
        /// <returns>List of trigger ranges for given instrument ids for given transaction type.</returns>
        // Dictionary<std::string, TrigerRange> GetTriggerRange(string[] InstrumentId, std::string TrasactionType)
        // {
        //     var param = new Dictionary<std::string, dynamic>();

        //     param.Add("i", InstrumentId);
        //     param.Add("transaction_type", TrasactionType.ToLower());

        //     var triggerdata = Get("market.trigger_range", param)["data"];

        //     Dictionary<std::string, TrigerRange> triggerRanges = new Dictionary<std::string, TrigerRange>();
        //     foreach (std::string item in triggerdata.Keys)
        //         triggerRanges.Add(item, new TrigerRange(triggerdata[item]));

        //     return triggerRanges;
        // }

#if 0        //#region GTT

        /// <summary>
        /// Retrieve the list of GTTs.
        /// </summary>
        /// <returns>List of GTTs.</returns>
        GTTResponse GetGTTs()
        {
            std::string gttsdata = Get("gtt");

            List<GTT> gtts = new List<GTT>();

            foreach (Dictionary<std::string, dynamic> item in gttsdata["data"])
                gtts.Add(new GTT(item));

            return gtts;
        }


        /// <summary>
        /// Retrieve a single GTT
        /// </summary>
        /// <param name="GTTId">Id of the GTT</param>
        /// <returns>GTT info</returns>
        GTT GetGTT(int GTTId)
        {
            var param = new Dictionary<std::string, dynamic>();
            param.Add("id", GTTId.ToString());

            var gttdata = Get("gtt.info", param);

            return new GTT(gttdata["data"]);
        }

        /// <summary>
        /// Place a GTT order
        /// </summary>
        /// <param name="gttParams">Contains the parameters for the GTT order</param>
        /// <returns>Json response in the form of nested std::string dictionary.</returns>
        Dictionary<std::string, dynamic> PlaceGTT(GTTParams gttParams)
        {
            var condition = new Dictionary<std::string, dynamic>();
            condition.Add("exchange", gttParams.Exchange);
            condition.Add("tradingsymbol", gttParams.TradingSymbol);
            condition.Add("trigger_values", gttParams.TriggerPrices);
            condition.Add("last_price", gttParams.LastPrice);
            condition.Add("instrument_token", gttParams.InstrumentToken);

            var ordersParam = new List<Dictionary<std::string, dynamic>>();
            foreach (var o in gttParams.Orders)
            {
                var order = new Dictionary<std::string, dynamic>();
                order["exchange"] = gttParams.Exchange;
                order["tradingsymbol"] = gttParams.TradingSymbol;
                order["transaction_type"] = o.TransactionType;
                order["quantity"] = o.Quantity;
                order["price"] = o.Price;
                order["order_type"] = o.OrderType;
                order["product"] = o.Product;
                ordersParam.Add(order);
            }

            var parms = new Dictionary<std::string, dynamic>();
            parms.Add("condition", Utils::JsonSerialize(condition));
            parms.Add("orders", Utils::JsonSerialize(ordersParam));
            parms.Add("type", gttParams.TriggerType);

            return Post("gtt.place", parms);
        }

        /// <summary>
        /// Modify a GTT order
        /// </summary>
        /// <param name="GTTId">Id of the GTT to be modified</param>
        /// <param name="gttParams">Contains the parameters for the GTT order</param>
        /// <returns>Json response in the form of nested std::string dictionary.</returns>
        Dictionary<std::string, dynamic> ModifyGTT(int GTTId, GTTParams gttParams)
        {
            var condition = new Dictionary<std::string, dynamic>();
            condition.Add("exchange", gttParams.Exchange);
            condition.Add("tradingsymbol", gttParams.TradingSymbol);
            condition.Add("trigger_values", gttParams.TriggerPrices);
            condition.Add("last_price", gttParams.LastPrice);
            condition.Add("instrument_token", gttParams.InstrumentToken);

            var ordersParam = new List<Dictionary<std::string, dynamic>>();
            foreach (var o in gttParams.Orders)
            {
                var order = new Dictionary<std::string, dynamic>();
                order["exchange"] = gttParams.Exchange;
                order["tradingsymbol"] = gttParams.TradingSymbol;
                order["transaction_type"] = o.TransactionType;
                order["quantity"] = o.Quantity;
                order["price"] = o.Price;
                order["order_type"] = o.OrderType;
                order["product"] = o.Product;
                ordersParam.Add(order);
            }

            var parms = new Dictionary<std::string, dynamic>();
            parms.Add("condition", Utils::JsonSerialize(condition));
            parms.Add("orders", Utils::JsonSerialize(ordersParam));
            parms.Add("type", gttParams.TriggerType);
            parms.Add("id", GTTId.ToString());

            return Put("gtt.modify", parms);
        }

        /// <summary>
        /// Cancel a GTT order
        /// </summary>
        /// <param name="GTTId">Id of the GTT to be modified</param>
        /// <returns>Json response in the form of nested std::string dictionary.</returns>
        Dictionary<std::string, dynamic> CancelGTT(int GTTId)
        {
            map<std::string, std::string> params {"id", std::string(GTTId)};

            return Delete("gtt.delete", parms);
        }

        // endregion GTT

        #region MF Calls

        /// <summary>
        /// Gets the Mutual funds Instruments.
        /// </summary>
        /// <returns>The Mutual funds Instruments.</returns>
        List<MFInstrument> GetMFInstruments()
        {
            var param = new Dictionary<std::string, dynamic>();

            List<Dictionary<std::string, dynamic>> instrumentsData;

            instrumentsData = Get("mutualfunds.instruments", param);

            List<MFInstrument> instruments = new List<MFInstrument>();

            foreach (Dictionary<std::string, dynamic> item in instrumentsData)
                instruments.Add(new MFInstrument(item));

            return instruments;
        }

        /// <summary>
        /// Gets all Mutual funds orders.
        /// </summary>
        /// <returns>The Mutual funds orders.</returns>
        List<MFOrder> GetMFOrders()
        {
            var param = new Dictionary<std::string, dynamic>();

            Dictionary<std::string, dynamic> ordersData;
            ordersData = Get("mutualfunds.orders", param);

            List<MFOrder> orderlist = new List<MFOrder>();

            foreach (Dictionary<std::string, dynamic> item in ordersData["data"])
                orderlist.Add(new MFOrder(item));

            return orderlist;
        }

        /// <summary>
        /// Gets the Mutual funds order by OrderId.
        /// </summary>
        /// <returns>The Mutual funds order.</returns>
        /// <param name="OrderId">Order id.</param>
        MFOrder GetMFOrders(String OrderId)
        {
            var param = new Dictionary<std::string, dynamic>();
            param.Add("order_id", OrderId);

            Dictionary<std::string, dynamic> orderData;
            orderData = Get("mutualfunds.order", param);

            return new MFOrder(orderData["data"]);
        }

        /// <summary>
        /// Places a Mutual funds order.
        /// </summary>
        /// <returns>JSON response as nested std::string dictionary.</returns>
        /// <param name="TradingSymbol">Tradingsymbol (ISIN) of the fund.</param>
        /// <param name="TransactionType">BUY or SELL.</param>
        /// <param name="Amount">Amount worth of units to purchase. Not applicable on SELLs.</param>
        /// <param name="Quantity">Quantity to SELL. Not applicable on BUYs. If the holding is less than minimum_redemption_quantity, all the units have to be sold.</param>
        /// <param name="Tag">An optional tag to apply to an order to identify it (alphanumeric, max 8 chars).</param>
        Dictionary<std::string, dynamic> PlaceMFOrder(
            std::string TradingSymbol,
            std::string TransactionType,
            double Amount,
            double Quantity = "",
            std::string Tag = "")
        {
            var param = new Dictionary<std::string, dynamic>();

            Utils::AddIfNotNull(param, "tradingsymbol", TradingSymbol);
            Utils::AddIfNotNull(param, "transaction_type", TransactionType);
            Utils::AddIfNotNull(param, "amount", Amount.ToString());
            Utils::AddIfNotNull(param, "quantity", Quantity.ToString());
            Utils::AddIfNotNull(param, "tag", Tag);

            return Post("mutualfunds.orders.place", param);
        }

        /// <summary>
        /// Cancels the Mutual funds order.
        /// </summary>
        /// <returns>JSON response as nested std::string dictionary.</returns>
        /// <param name="OrderId">Unique order id.</param>
        Dictionary<std::string, dynamic> CancelMFOrder(String OrderId)
        {
            var param = new Dictionary<std::string, dynamic>();

            Utils::AddIfNotNull(param, "order_id", OrderId);

            return Delete("mutualfunds.cancel_order", param);
        }

        /// <summary>
        /// Gets all Mutual funds SIPs.
        /// </summary>
        /// <returns>The list of all Mutual funds SIPs.</returns>
        List<MFSIP> GetMFSIPs()
        {
            var param = new Dictionary<std::string, dynamic>();

            Dictionary<std::string, dynamic> sipData;
            sipData = Get("mutualfunds.sips", param);

            List<MFSIP> siplist = new List<MFSIP>();

            foreach (Dictionary<std::string, dynamic> item in sipData["data"])
                siplist.Add(new MFSIP(item));

            return siplist;
        }

        /// <summary>
        /// Gets a single Mutual funds SIP by SIP id.
        /// </summary>
        /// <returns>The Mutual funds SIP.</returns>
        /// <param name="SIPID">SIP id.</param>
        MFSIP GetMFSIPs(String SIPID)
        {
            var param = new Dictionary<std::string, dynamic>();
            param.Add("sip_id", SIPID);

            Dictionary<std::string, dynamic> sipData;
            sipData = Get("mutualfunds.sip", param);

            return new MFSIP(sipData["data"]);
        }

        /// <summary>
        /// Places a Mutual funds SIP order.
        /// </summary>
        /// <returns>JSON response as nested std::string dictionary.</returns>
        /// <param name="TradingSymbol">ISIN of the fund.</param>
        /// <param name="Amount">Amount worth of units to purchase. It should be equal to or greated than minimum_additional_purchase_amount and in multiple of purchase_amount_multiplier in the instrument master.</param>
        /// <param name="InitialAmount">Amount worth of units to purchase before the SIP starts. Should be equal to or greater than minimum_purchase_amount and in multiple of purchase_amount_multiplier. This is only considered if there have been no prior investments in the target fund.</param>
        /// <param name="Frequency">weekly, monthly, or quarterly.</param>
        /// <param name="InstalmentDay">If Frequency is monthly, the day of the month (1, 5, 10, 15, 20, 25) to trigger the order on.</param>
        /// <param name="Instalments">Number of instalments to trigger. If set to -1, instalments are triggered at fixed intervals until the SIP is cancelled.</param>
        /// <param name="Tag">An optional tag to apply to an order to identify it (alphanumeric, max 8 chars).</param>
        Dictionary<std::string, dynamic> PlaceMFSIP(
            std::string TradingSymbol,
            double Amount,
            double InitialAmount,
            std::string Frequency,
            int InstalmentDay,
            int Instalments,
            std::string Tag = "")
        {
            var param = new Dictionary<std::string, dynamic>();

            Utils::AddIfNotNull(param, "tradingsymbol", TradingSymbol);
            Utils::AddIfNotNull(param, "initial_amount", InitialAmount.ToString());
            Utils::AddIfNotNull(param, "amount", Amount.ToString());
            Utils::AddIfNotNull(param, "frequency", Frequency);
            Utils::AddIfNotNull(param, "instalment_day", InstalmentDay.ToString());
            Utils::AddIfNotNull(param, "instalments", Instalments.ToString());

            return Post("mutualfunds.sips.place", param);
        }

        /// <summary>
        /// Modifies the Mutual funds SIP.
        /// </summary>
        /// <returns>JSON response as nested std::string dictionary.</returns>
        /// <param name="SIPId">SIP id.</param>
        /// <param name="Amount">Amount worth of units to purchase. It should be equal to or greated than minimum_additional_purchase_amount and in multiple of purchase_amount_multiplier in the instrument master.</param>
        /// <param name="Frequency">weekly, monthly, or quarterly.</param>
        /// <param name="InstalmentDay">If Frequency is monthly, the day of the month (1, 5, 10, 15, 20, 25) to trigger the order on.</param>
        /// <param name="Instalments">Number of instalments to trigger. If set to -1, instalments are triggered idefinitely until the SIP is cancelled.</param>
        /// <param name="Status">Pause or unpause an SIP (active or paused).</param>
        Dictionary<std::string, dynamic> ModifyMFSIP(
            std::string SIPId,
            double Amount,
            std::string Frequency,
            int InstalmentDay,
            int Instalments,
            std::string Status)
        {
            var param = new Dictionary<std::string, dynamic>();

            Utils::AddIfNotNull(param, "status", Status);
            Utils::AddIfNotNull(param, "sip_id", SIPId);
            Utils::AddIfNotNull(param, "amount", Amount.ToString());
            Utils::AddIfNotNull(param, "frequency", Frequency.ToString());
            Utils::AddIfNotNull(param, "instalment_day", InstalmentDay.ToString());
            Utils::AddIfNotNull(param, "instalments", Instalments.ToString());

            return Put("mutualfunds.sips.modify", param);
        }

        /// <summary>
        /// Cancels the Mutual funds SIP.
        /// </summary>
        /// <returns>JSON response as nested std::string dictionary.</returns>
        /// <param name="SIPId">SIP id.</param>
		Dictionary<std::string, dynamic> CancelMFSIP(String SIPId)
        {
            var param = new Dictionary<std::string, dynamic>();

            Utils::AddIfNotNull(param, "sip_id", SIPId);

            return Delete("mutualfunds.cancel_sips", param);
        }

        /// <summary>
        /// Gets the Mutual funds holdings.
        /// </summary>
        /// <returns>The list of all Mutual funds holdings.</returns>
        List<MFHolding> GetMFHoldings()
        {
            var param = new Dictionary<std::string, dynamic>();

            Dictionary<std::string, dynamic> holdingsData;
            holdingsData = Get("mutualfunds.holdings", param);

            List<MFHolding> holdingslist = new List<MFHolding>();

            foreach (Dictionary<std::string, dynamic> item in holdingsData["data"])
                holdingslist.Add(new MFHolding(item));

            return holdingslist;
        }

        #endregion
#endif
//        #region HTTP Functions
        /// <summary>
        /// Alias for sending a GET request.
        /// </summary>
        /// <param name="Route">URL route of API</param>
        /// <param name="Params">Additional paramerters</param>
        /// <returns>Varies according to API endpoint</returns>
        std::string Kite::Get(std::string route, ParamType params)
        {
            return Request(route, methods::GET, params);
        }

        /// <summary>
        /// Alias for sending a POST request.
        /// </summary>
        /// <param name="Route">URL route of API</param>
        /// <param name="Params">Additional paramerters</param>
        /// <returns>Varies according to API endpoint</returns>
        std::string Kite::Post(std::string route, ParamType params)
        {
            return Request(route, methods::POST, params);
        }

        /// <summary>
        /// Alias for sending a PUT request.
        /// </summary>
        /// <param name="Route">URL route of API</param>
        /// <param name="Params">Additional paramerters</param>
        /// <returns>Varies according to API endpoint</returns>
        std::string Kite::Put(std::string route, ParamType Params)
        {
            return Request(route, methods::PUT, Params);
        }

        /// <summary>
        /// Alias for sending a DELETE request.
        /// </summary>
        /// <param name="Route">URL route of API</param>
        /// <param name="Params">Additional paramerters</param>
        /// <returns>Varies according to API endpoint</returns>
        std::string Kite::Delete(std::string route, ParamType Params)
        {
            return Request(route, methods::DEL, Params);
        }

        /// <summary>
        /// Adds extra headers to request
        /// </summary>
        /// <param name="Req">Request object to add headers</param>

        void Kite::AddExtraHeaders(http_request &request)
        {
            request.headers().add("User-Agent", USER_AGENT);
            request.headers().add("X-Kite-Version", "3");
            if(!_accessToken.empty()) {
                logging::Write("AccessToken is present: " + _accessToken);
                request.headers().add(header_names::authorization, "token " + _apiKey + ":" + _accessToken);
            }
            /*
            var KiteAssembly = System.Reflection.Assembly.GetAssembly(typeof(Kite));
            if (KiteAssembly != "")
                Req.UserAgent = "KiteConnect.Net/" + KiteAssembly.GetName().Version;
            req.Headers.Add("User-Agent", USER_AGENT)            ;
            Req.Headers.Add("X-Kite-Version", "3");
            Req.Headers.Add("Authorization", "token " + _apiKey + ":" + _accessToken);

            //if(Req.Method == "GET" && cache.IsCached(Req.RequestUri.AbsoluteUri))
            //{
            //    Req.Headers.Add("If-None-Match: " + cache.GetETag(Req.RequestUri.AbsoluteUri));
            //}

            Req.Timeout = _timeout;
            if (_proxy != "") Req.Proxy = _proxy;

            if (_enableLogging)
            {
                foreach (std::string header in Req.Headers.Keys)
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
        std::string Kite::Request(std::string route, std::string method, ParamType params)
        {
            std::string url = _root + _routes.at(route);

            logging::Write(url);
            if (url.find('{') != std::string::npos)
            {
                if(params.size() == 0)
                    throw std::runtime_error("Parameters are missing");

                for (auto it=params.begin(); it!=params.end(); ++it) 
                {
                    if(size_t loc = url.find("{"+it->first+"}") != std::string::npos)
                    {
                        url.replace(loc, it->first.length()+2, it->second);
                        //params.erase(it);
                    }
                }

            }

            uri_builder paramEncoder;
            for (auto it=params.begin(); it!=params.end(); ++it) {
                    //std::cout << it->first << " => " << it->second << '\n';
                    paramEncoder.append_query(U(it->first), U(it->second));
            }

            http_request webRequest(method);
            AddExtraHeaders(webRequest);

            if (method == methods::POST || method == methods::PUT)
            {
               webRequest.set_body(paramEncoder.to_string().erase(0,2), 
                                   mime_types::application_x_www_form_urlencoded);
            }
            else
            {
                std::cout<<"url:"<<url<<"\n";
                // With Params paramEnoder: /?key=val&key1=val1
                // Without Param paramEncoder: ?/=
                if (!params.empty())
                {
                    url = url + paramEncoder.to_string().erase(0,3);
                }
            }

            logging::Write(url);
            webRequest.set_request_uri(url);

            std::string body;
            std::string contentType;
            http::status_code status;

            logging::Write(".......Debug.....");
            // Send request and wait for the response
            pplx::task<web::http::http_response> webResponse =  httpClient->request(webRequest);

            try { 
                //-- All Taskss will get triggered here 
                webResponse.wait();
                body = webResponse.get().extract_string().get();
                logging::Write(body);
                contentType = webResponse.get().headers().content_type();
                logging::Write(contentType);
                status = webResponse.get().status_code();
                std::cout<<"status: "<<status<<std::endl;
                std::cout<<"Response received:\n";
            }
            catch (const std::exception &e) {
                    cout<<"Error exception: "<<e.what()<<endl;
                    throw std::runtime_error("Process response is invalid - " + body + e.what());
            }
            
            //TODO if (_enableLogging) Console.WriteLine("DEBUG: " + (int)((HttpWebResponse)webResponse).StatusCode + " " + response + "\n");

            std::cout <<"content-types:" << contentType <<":" <<http::details::mime_types::application_json<<std::endl;
            if (contentType.compare(http::details::mime_types::application_json) == 0)
            {
                std::cout<<"Here Inside\n";
                //Dictionary<std::string, dynamic> responseDictionary = Utils::JsonDeserialize(response);
                if (status != http::status_codes::OK)
                {
                    std::string errorType = "GeneralException";
                    std::string message = "";
                    web::json::value obj = web::json::value::parse(body);

                    if (obj.has_string_field("error_type"))
                        errorType = obj["error_type"].as_string();
                    if (obj.has_string_field("message"))
                        message = obj["message"].as_string();

                    // Error handling
                    enum ErrorCodes {
                        UndefinedCode,
                        GeneralException,
                        TokenException,
                        PermissionException,
                        OrderException,
                        InputException,
                        DataException,
                        NetworkException,
                        EndCode
                    };
                    std::map<std::string, ErrorCodes> ErrorMap;
                    ErrorMap["GeneralException"] = GeneralException;
                    ErrorMap["TokenException"] = TokenException;
                    ErrorMap["PermissionException"] = PermissionException;
                    ErrorMap["OrderException"] = OrderException;
                    ErrorMap["InputException"] = InputException;
                    ErrorMap["DataException"] = DataException;
                    ErrorMap["NetworkException"] = NetworkException;

                    switch (ErrorMap[errorType])
                    {
                        case GeneralException: throw std::runtime_error("General Exception:" + message + ":" + std::to_string(status));
                        case TokenException:
                            {
                                _sessionHook(); // Invoke the callback
                                throw std::runtime_error("Token Exception:" + message + ":" + std::to_string(status));
                            }
                        case PermissionException: throw std::runtime_error("Permission Exception" + message + ":" + std::to_string(status));
                        case OrderException: throw std::runtime_error("Order Exception" + message + ":" + std::to_string(status));
                        case InputException: throw std::runtime_error("Input Exception" + message + ":" + std::to_string(status));
                        case DataException: throw std::runtime_error("Data Exception" + message + ":" + std::to_string(status));
                        case NetworkException: throw std::runtime_error("Network Exception" + message + ":" + std::to_string(status));
                        default: throw std::runtime_error("Unknown Exception:" + message + ":" + std::to_string(status));
                    }
                }
                return body;
            }
            else if (contentType == "text/csv")
                return body;
            else
                throw std::runtime_error("Unexpected content type " + 
                      webResponse.get().headers().content_type() + ">" + body);
        }

        //#endregion
}
