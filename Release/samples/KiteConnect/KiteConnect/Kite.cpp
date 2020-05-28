﻿/*using System;
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
using namespace web::http::details; // HTTP constants
using namespace concurrency::streams;// Asynchronous streams
using namespace boost::algorithm;
using namespace std;      // Use std c++ features

namespace KiteConnect
{
    /// <summary>
    /// The API client class. In production, you may initialize a single instance of this class per `APIKey`.
    /// </summary>
    class Kite
    {
        // Default root API endpoint. It's possible to
        // override this by passing the `Root` parameter during initialisation.
        private:  
            string _root = "https://api.kite.trade";
            string _login = "https://kite.trade/connect/login";
            String USER_AGENT = "SapanClient";

            string _apiKey;
            string _accessToken;
            bool _enableLogging;
            shared_ptr<web::web_proxy> _proxy;
            std::chrono::seconds _timeout;
            shared_ptr<http_client> httpClient;
            std::function<void(void)> _sessionHook;

            //private Cache cache = new Cache();

            const map<string, string> _routes = 
            {
                {"parameters", "/parameters"},
                {"api.token", "/session/token"},
                {"api.refresh", "/session/refresh_token"},

                {"instrument.margins", "/margins/{segment}"},

                {"user.profile", "/user/profile"},
                {"user.margins", "/user/margins"},
                {"user.segment_margins", "/user/margins/{segment}"},

                {"orders", "/orders"},
                {"trades", "/trades"},
                {"orders.history", "/orders/{order_id}"},

                {"orders.place", "/orders/{variety}"},
                {"orders.modify", "/orders/{variety}/{order_id}"},
                {"orders.cancel", "/orders/{variety}/{order_id}"},
                {"orders.trades", "/orders/{order_id}/trades"},

                {"gtt", "/gtt/triggers"},
                {"gtt.place", "/gtt/triggers"},
                {"gtt.info", "/gtt/triggers/{id}"},
                {"gtt.modify", "/gtt/triggers/{id}"},
                {"gtt.delete", "/gtt/triggers/{id}"},

                {"portfolio.positions", "/portfolio/positions"},
                {"portfolio.holdings", "/portfolio/holdings"},
                {"portfolio.positions.modify", "/portfolio/positions"},

                {"market.instruments.all", "/instruments"},
                {"market.instruments", "/instruments/{exchange}"},
                {"market.quote", "/quote"},
                {"market.ohlc", "/quote/ohlc"},
                {"market.ltp", "/quote/ltp"},
                {"market.historical", "/instruments/historical/{instrument_token}/{interval}"},
                {"market.trigger_range", "/instruments/trigger_range/{transaction_type}"},

                {"mutualfunds.orders", "/mf/orders"},
                {"mutualfunds.order", "/mf/orders/{order_id}"},
                {"mutualfunds.orders.place", "/mf/orders"},
                {"mutualfunds.cancel_order", "/mf/orders/{order_id}"},

                {"mutualfunds.sips", "/mf/sips"},
                {"mutualfunds.sips.place", "/mf/sips"},
                {"mutualfunds.cancel_sips", "/mf/sips/{sip_id}"},
                {"mutualfunds.sips.modify", "/mf/sips/{sip_id}"},
                {"mutualfunds.sip", "/mf/sips/{sip_id}"},

                {"mutualfunds.instruments", "/mf/instruments"},
                {"mutualfunds.holdings", "/mf/holdings"}
            };
        public:
        /// <summary>
        /// Initialize a new Kite Connect client instance.
        /// </summary>
        /// <param name="APIKey">API Key issued to you</param>
        /// <param name="AccessToken">The token obtained after the login flow in exchange for the `RequestToken` . 
        /// Pre-login, this will default to None,but once you have obtained it, you should persist it in a database or session to pass 
        /// to the Kite Connect class initialisation for subsequent requests.</param>
        /// <param name="Root">API end point root. Unless you explicitly want to send API requests to a non-default endpoint, this can be ignored.</param>
        /// <param name="Debug">If set to True, will serialise and print requests and responses to stdout.</param>
        /// <param name="Timeout">Time in seconds for which  the API client will wait for a request to complete before it fails</param>
        /// <param name="Proxy">To set proxy for http request. Should be an object of WebProxy.</param>
        /// <param name="Pool">Number of connections to server. Client will reuse the connections if they are alive.</param>
        Kite(string APIKey, string accessToken = null, string root = null, bool debug = false, int timeout = 7,  shared_ptr<web::web_proxy> proxy = nullptr, int Pool = 2)
        {
            _accessToken = accessToken;
            _apiKey = APIKey;
            if (!root.empty())
                _root = root;
            _enableLogging = debug;

            //_timeout = Timeout;
            _proxy = proxy;

            _timeout = std::chrono::seconds(timeout);
            http_client_config clientConfig;
            clientConfig.set_timeout(timeout);
            if(proxy != nullptr) {
                clientConfig.set_proxy(*proxy);
            }
            httpClient = make_unique<http_client>(_root, clientConfig); //Fill in the root ur
            //ServicePointManager.DefaultConnectionLimit = Pool;
        }

        /// <summary>
        /// Enabling logging prints HTTP request and response summaries to console
        /// </summary>
        /// <param name="enableLogging">Set to true to enable logging</param>
        void EnableLogging(bool enableLogging)
        {
            _enableLogging = enableLogging;
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
        void SetSessionExpiryHook(std::function<void(void)> method)
        {
            _sessionHook = method;
        }

        /// <summary>
        /// Set the `AccessToken` received after a successful authentication.
        /// </summary>
        /// <param name="AccessToken">Access token for the session.</param>
        void SetAccessToken(string accessToken)
        {
            _accessToken = accessToken;
        }

        /// <summary>
        /// Get the remote login url to which a user should be redirected to initiate the login flow.
        /// </summary>
        /// <returns>Login url to authenticate the user.</returns>
        string GetLoginURL()
        {
            stringstream url;
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
        User GenerateSession(string RequestToken, string AppSecret)
        {
            string checksum = Utils:::SHA256(_apiKey + RequestToken + AppSecret);

            map<string, string> data{
                {"api_key", _apiKey},
                {"request_token", RequestToken},
                {"checksum", checksum}
            };
            shared_ptr<map<string, string>> param = make_shared<map<string, string>>(data);

            string jsonData = Post("api.token", param);
            return User(jsonData);
        }

        /// <summary>
        /// Kill the session by invalidating the access token
        /// </summary>
        /// <param name="AccessToken">Access token to invalidate. Default is the active access token.</param>
        /// <returns>Json response in the form of nested string dictionary.</returns>
        std::string InvalidateAccessToken(string AccessToken = null)
        {
            map<string, string> param;

            Utils:::AddIfNotNull(param, "api_key", _apiKey);
            Utils:::AddIfNotNull(param, "access_token", AccessToken);

            return Delete("api.token", param);
        }

        /// <summary>
        /// Invalidates RefreshToken
        /// </summary>
        /// <param name="RefreshToken">RefreshToken to invalidate</param>
        /// <returns>Json response in the form of nested string dictionary.</returns>
        std::string InvalidateRefreshToken(string RefreshToken)
        {
            map<string, string> param;

            Utils:::AddIfNotNull(param, "api_key", _apiKey);
            Utils:::AddIfNotNull(param, "refresh_token", RefreshToken);
            return Delete("api.token", param);
        }

        /// <summary>
        /// Renew AccessToken using RefreshToken
        /// </summary>
        /// <param name="RefreshToken">RefreshToken to renew the AccessToken.</param>
        /// <param name="AppSecret">API secret issued with the API key.</param>
        /// <returns>TokenRenewResponse that contains new AccessToken and RefreshToken.</returns>
        TokenSet RenewAccessToken(string RefreshToken, string AppSecret)
        {
            map<string, string> param;

            string checksum = Utils:::SHA256(_apiKey + RefreshToken + AppSecret);

            Utils::AddIfNotNull(param, "api_key", _apiKey);
            Utils::AddIfNotNull(param, "refresh_token", RefreshToken);
            Utils::AddIfNotNull(param, "checksum", checksum);

            return TokenSet(Post("api.refresh", param));
        }

        /// <summary>
        /// Gets currently logged in user details
        /// </summary>
        /// <returns>User profile</returns>
        Profile GetProfile()
        {
            std::string profileData = Get("user.profile");

            return Profile(profileData);
        }

        ///// <summary>
        ///// Margin data for intraday trading
        ///// </summary>
        ///// <param name="Segment">Tradingsymbols under this segment will be returned</param>
        ///// <returns>List of margins of intruments</returns>
        //public List<InstrumentMargin> GetInstrumentsMargins(string Segment)
        //{
        //    var instrumentsMarginsData = Get("instrument.margins", new Dictionary<string, dynamic> { { "segment", Segment } });

        //    List<InstrumentMargin> instrumentsMargins = new List<InstrumentMargin>();
        //    foreach (Dictionary<string, dynamic> item in instrumentsMarginsData["data"])
        //        instrumentsMargins.Add(new InstrumentMargin(item));

        //    return instrumentsMargins;
        //}

        /// <summary>
        /// Get account balance and cash margin details for all segments.
        /// </summary>
        /// <returns>User margin response with both equity and commodity margins.</returns>
        UserMarginsResponse GetMargins()
        {
            std::string marginsData = Get("user.margins");
            return UserMarginsResponse(marginsData["data"]);
        }

        /// <summary>
        /// Get account balance and cash margin details for a particular segment.
        /// </summary>
        /// <param name="Segment">Trading segment (eg: equity or commodity)</param>
        /// <returns>Margins for specified segment.</returns>
        UserMargin GetMargins(string Segment)
        {
            var userMarginData = Get("user.segment_margins", new Dictionary<string, dynamic> { { "segment", Segment } });
            return new UserMargin(userMarginData["data"]);
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
        /// <returns>Json response in the form of nested string dictionary.</returns>
        Dictionary<string, dynamic> PlaceOrder(
            string Exchange,
            string TradingSymbol,
            string TransactionType,
            int Quantity,
            decimal? Price = null,
            string Product = null,
            string OrderType = null,
            string Validity = null,
            int? DisclosedQuantity = null,
            decimal? TriggerPrice = null,
            decimal? SquareOffValue = null,
            decimal? StoplossValue = null,
            decimal? TrailingStoploss = null,
            string Variety = Constants.VARIETY_REGULAR,
            string Tag = "")
        {
            var param = new Dictionary<string, dynamic>();

            Utils::AddIfNotNull(param, "exchange", Exchange);
            Utils::AddIfNotNull(param, "tradingsymbol", TradingSymbol);
            Utils::AddIfNotNull(param, "transaction_type", TransactionType);
            Utils::AddIfNotNull(param, "quantity", Quantity.ToString());
            Utils::AddIfNotNull(param, "price", Price.ToString());
            Utils::AddIfNotNull(param, "product", Product);
            Utils::AddIfNotNull(param, "order_type", OrderType);
            Utils::AddIfNotNull(param, "validity", Validity);
            Utils::AddIfNotNull(param, "disclosed_quantity", DisclosedQuantity.ToString());
            Utils::AddIfNotNull(param, "trigger_price", TriggerPrice.ToString());
            Utils::AddIfNotNull(param, "squareoff", SquareOffValue.ToString());
            Utils::AddIfNotNull(param, "stoploss", StoplossValue.ToString());
            Utils::AddIfNotNull(param, "trailing_stoploss", TrailingStoploss.ToString());
            Utils::AddIfNotNull(param, "variety", Variety);
            Utils::AddIfNotNull(param, "tag", Tag);

            return Post("orders.place", param);
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
        /// <returns>Json response in the form of nested string dictionary.</returns>
        Dictionary<string, dynamic> ModifyOrder(
            string OrderId,
            string ParentOrderId = null,
            string Exchange = null,
            string TradingSymbol = null,
            string TransactionType = null,
            string Quantity = null,
            decimal? Price = null,
            string Product = null,
            string OrderType = null,
            string Validity = Constants.VALIDITY_DAY,
            int? DisclosedQuantity = null,
            decimal? TriggerPrice = null,
            string Variety = Constants.VARIETY_REGULAR)
        {
            var param = new Dictionary<string, dynamic>();

            string VarietyString = Variety;
            string ProductString = Product;

            if ((ProductString == "bo" || ProductString == "co") && VarietyString != ProductString)
                throw new Exception(String.Format("Invalid variety. It should be: {}", ProductString));

            Utils::AddIfNotNull(param, "order_id", OrderId);
            Utils::AddIfNotNull(param, "parent_order_id", ParentOrderId);
            Utils::AddIfNotNull(param, "trigger_price", TriggerPrice.ToString());
            Utils::AddIfNotNull(param, "variety", Variety);

            if (VarietyString == "bo" && ProductString == "bo")
            {
                Utils::AddIfNotNull(param, "quantity", Quantity);
                Utils::AddIfNotNull(param, "price", Price.ToString());
                Utils::AddIfNotNull(param, "disclosed_quantity", DisclosedQuantity.ToString());
            }
            else if (VarietyString != "co" && ProductString != "co")
            {
                Utils::AddIfNotNull(param, "exchange", Exchange);
                Utils::AddIfNotNull(param, "tradingsymbol", TradingSymbol);
                Utils::AddIfNotNull(param, "transaction_type", TransactionType);
                Utils::AddIfNotNull(param, "quantity", Quantity);
                Utils::AddIfNotNull(param, "price", Price.ToString());
                Utils::AddIfNotNull(param, "product", Product);
                Utils::AddIfNotNull(param, "order_type", OrderType);
                Utils::AddIfNotNull(param, "validity", Validity);
                Utils::AddIfNotNull(param, "disclosed_quantity", DisclosedQuantity.ToString());
            }

            return Put("orders.modify", param);
        }

        /// <summary>
        /// Cancel an order
        /// </summary>
        /// <param name="OrderId">Id of the order to be cancelled</param>
        /// <param name="Variety">You can place orders of varieties; regular orders, after market orders, cover orders etc. </param>
        /// <param name="ParentOrderId">Id of the parent order (obtained from the /orders call) as BO is a multi-legged order</param>
        /// <returns>Json response in the form of nested string dictionary.</returns>
        Dictionary<string, dynamic> CancelOrder(string OrderId, string Variety = Constants.VARIETY_REGULAR, string ParentOrderId = null)
        {
            var param = new Dictionary<string, dynamic>();

            Utils::AddIfNotNull(param, "order_id", OrderId);
            Utils::AddIfNotNull(param, "parent_order_id", ParentOrderId);
            Utils::AddIfNotNull(param, "variety", Variety);

            return Delete("orders.cancel", param);
        }

        /// <summary>
        /// Gets the collection of orders from the orderbook.
        /// </summary>
        /// <returns>List of orders.</returns>
        List<Order> GetOrders()
        {
            var ordersData = Get("orders");

            List<Order> orders = new List<Order>();

            foreach (Dictionary<string, dynamic> item in ordersData["data"])
                orders.Add(new Order(item));

            return orders;
        }

        /// <summary>
        /// Gets information about given OrderId.
        /// </summary>
        /// <param name="OrderId">Unique order id</param>
        /// <returns>List of order objects.</returns>
        List<Order> GetOrderHistory(string OrderId)
        {
            var param = new Dictionary<string, dynamic>();
            param.Add("order_id", OrderId);

            var orderData = Get("orders.history", param);

            List<Order> orderhistory = new List<Order>();

            foreach (Dictionary<string, dynamic> item in orderData["data"])
                orderhistory.Add(new Order(item));

            return orderhistory;
        }

        /// <summary>
        /// Retreive the list of trades executed (all or ones under a particular order).
        /// An order can be executed in tranches based on market conditions.
        /// These trades are individually recorded under an order.
        /// </summary>
        /// <param name="OrderId">is the ID of the order (optional) whose trades are to be retrieved. If no `OrderId` is specified, all trades for the day are returned.</param>
        /// <returns>List of trades of given order.</returns>
        List<Trade> GetOrderTrades(string OrderId = null)
        {
            Dictionary<string, dynamic> tradesdata;
            if (!String.IsNullOrEmpty(OrderId))
            {
                var param = new Dictionary<string, dynamic>();
                param.Add("order_id", OrderId);
                tradesdata = Get("orders.trades", param);
            }
            else
                tradesdata = Get("trades");

            List<Trade> trades = new List<Trade>();

            foreach (Dictionary<string, dynamic> item in tradesdata["data"])
                trades.Add(new Trade(item));

            return trades;
        }

        /// <summary>
        /// Retrieve the list of positions.
        /// </summary>
        /// <returns>Day and net positions.</returns>
        PositionResponse GetPositions()
        {
            var positionsdata = Get("portfolio.positions");
            return new PositionResponse(positionsdata["data"]);
        }

        /// <summary>
        /// Retrieve the list of equity holdings.
        /// </summary>
        /// <returns>List of holdings.</returns>
        List<Holding> GetHoldings()
        {
            var holdingsData = Get("portfolio.holdings");

            List<Holding> holdings = new List<Holding>();

            foreach (Dictionary<string, dynamic> item in holdingsData["data"])
                holdings.Add(new Holding(item));

            return holdings;
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
        /// <returns>Json response in the form of nested string dictionary.</returns>
        Dictionary<string, dynamic> ConvertPosition(
            string Exchange,
            string TradingSymbol,
            string TransactionType,
            string PositionType,
            int? Quantity,
            string OldProduct,
            string NewProduct)
        {
            var param = new Dictionary<string, dynamic>();

            Utils::AddIfNotNull(param, "exchange", Exchange);
            Utils::AddIfNotNull(param, "tradingsymbol", TradingSymbol);
            Utils::AddIfNotNull(param, "transaction_type", TransactionType);
            Utils::AddIfNotNull(param, "position_type", PositionType);
            Utils::AddIfNotNull(param, "quantity", Quantity.ToString());
            Utils::AddIfNotNull(param, "old_product", OldProduct);
            Utils::AddIfNotNull(param, "new_product", NewProduct);

            return Put("portfolio.positions.modify", param);
        }

        /// <summary>
        /// Retrieve the list of market instruments available to trade.
        /// Note that the results could be large, several hundred KBs in size,
		/// with tens of thousands of entries in the list.
        /// </summary>
        /// <param name="Exchange">Name of the exchange</param>
        /// <returns>List of instruments.</returns>
        List<Instrument> GetInstruments(string Exchange = null)
        {
            var param = new Dictionary<string, dynamic>();

            List<Dictionary<string, dynamic>> instrumentsData;

            if (String.IsNullOrEmpty(Exchange))
                instrumentsData = Get("market.instruments.all", param);
            else
            {
                param.Add("exchange", Exchange);
                instrumentsData = Get("market.instruments", param);
            }

            List<Instrument> instruments = new List<Instrument>();

            foreach (Dictionary<string, dynamic> item in instrumentsData)
                instruments.Add(new Instrument(item));

            return instruments;
        }

        /// <summary>
        /// Retrieve quote and market depth of upto 200 instruments
        /// </summary>
        /// <param name="InstrumentId">Indentification of instrument in the form of EXCHANGE:TRADINGSYMBOL (eg: NSE:INFY) or InstrumentToken (eg: 408065)</param>
        /// <returns>Dictionary of all Quote objects with keys as in InstrumentId</returns>
        Dictionary<string, Quote> GetQuote(string[] InstrumentId)
        {
            var param = new Dictionary<string, dynamic>();
            param.Add("i", InstrumentId);
            Dictionary<string, dynamic> quoteData = Get("market.quote", param)["data"];

            Dictionary<string, Quote> quotes = new Dictionary<string, Quote>();
            foreach (string item in quoteData.Keys)
                quotes.Add(item, new Quote(quoteData[item]));

            return quotes;
        }

        /// <summary>
        /// Retrieve LTP and OHLC of upto 200 instruments
        /// </summary>
        /// <param name="InstrumentId">Indentification of instrument in the form of EXCHANGE:TRADINGSYMBOL (eg: NSE:INFY) or InstrumentToken (eg: 408065)</param>
        /// <returns>Dictionary of all OHLC objects with keys as in InstrumentId</returns>
        Dictionary<string, OHLC> GetOHLC(string[] InstrumentId)
        {
            var param = new Dictionary<string, dynamic>();
            param.Add("i", InstrumentId);
            Dictionary<string, dynamic> ohlcData = Get("market.ohlc", param)["data"];

            Dictionary<string, OHLC> ohlcs = new Dictionary<string, OHLC>();
            foreach (string item in ohlcData.Keys)
                ohlcs.Add(item, new OHLC(ohlcData[item]));

            return ohlcs;
        }

        /// <summary>
        /// Retrieve LTP of upto 200 instruments
        /// </summary>
        /// <param name="InstrumentId">Indentification of instrument in the form of EXCHANGE:TRADINGSYMBOL (eg: NSE:INFY) or InstrumentToken (eg: 408065)</param>
        /// <returns>Dictionary with InstrumentId as key and LTP as value.</returns>
        Dictionary<string, LTP> GetLTP(string[] InstrumentId)
        {
            var param = new Dictionary<string, dynamic>();
            param.Add("i", InstrumentId);
            Dictionary<string, dynamic> ltpData = Get("market.ltp", param)["data"];

            Dictionary<string, LTP> ltps = new Dictionary<string, LTP>();
            foreach (string item in ltpData.Keys)
                ltps.Add(item, new LTP(ltpData[item]));

            return ltps;
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
        List<Historical> GetHistoricalData(
            string InstrumentToken,
            DateTime FromDate,
            DateTime ToDate,
            string Interval,
            bool Continuous = false,
            bool OI = false)
        {
            var param = new Dictionary<string, dynamic>();

            param.Add("instrument_token", InstrumentToken);
            param.Add("from", FromDate.ToString("yyyy-MM-dd HH:mm:ss"));
            param.Add("to", ToDate.ToString("yyyy-MM-dd HH:mm:ss"));
            param.Add("interval", Interval);
            param.Add("continuous", Continuous ? "1" : "0");
            param.Add("oi", OI ? "1" : "0");

            var historicalData = Get("market.historical", param);

            List<Historical> historicals = new List<Historical>();

            foreach (ArrayList item in historicalData["data"]["candles"])
                historicals.Add(new Historical(item));

            return historicals;
        }

        /// <summary>
        /// Retrieve the buy/sell trigger range for Cover Orders.
        /// </summary>
        /// <param name="InstrumentId">Indentification of instrument in the form of EXCHANGE:TRADINGSYMBOL (eg: NSE:INFY) or InstrumentToken (eg: 408065)</param>
        /// <param name="TrasactionType">BUY or SELL</param>
        /// <returns>List of trigger ranges for given instrument ids for given transaction type.</returns>
        Dictionary<string, TrigerRange> GetTriggerRange(string[] InstrumentId, string TrasactionType)
        {
            var param = new Dictionary<string, dynamic>();

            param.Add("i", InstrumentId);
            param.Add("transaction_type", TrasactionType.ToLower());

            var triggerdata = Get("market.trigger_range", param)["data"];

            Dictionary<string, TrigerRange> triggerRanges = new Dictionary<string, TrigerRange>();
            foreach (string item in triggerdata.Keys)
                triggerRanges.Add(item, new TrigerRange(triggerdata[item]));

            return triggerRanges;
        }

        #region GTT

        /// <summary>
        /// Retrieve the list of GTTs.
        /// </summary>
        /// <returns>List of GTTs.</returns>
        List<GTT> GetGTTs()
        {
            var gttsdata = Get("gtt");

            List<GTT> gtts = new List<GTT>();

            foreach (Dictionary<string, dynamic> item in gttsdata["data"])
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
            var param = new Dictionary<string, dynamic>();
            param.Add("id", GTTId.ToString());

            var gttdata = Get("gtt.info", param);

            return new GTT(gttdata["data"]);
        }

        /// <summary>
        /// Place a GTT order
        /// </summary>
        /// <param name="gttParams">Contains the parameters for the GTT order</param>
        /// <returns>Json response in the form of nested string dictionary.</returns>
        Dictionary<string, dynamic> PlaceGTT(GTTParams gttParams)
        {
            var condition = new Dictionary<string, dynamic>();
            condition.Add("exchange", gttParams.Exchange);
            condition.Add("tradingsymbol", gttParams.TradingSymbol);
            condition.Add("trigger_values", gttParams.TriggerPrices);
            condition.Add("last_price", gttParams.LastPrice);
            condition.Add("instrument_token", gttParams.InstrumentToken);

            var ordersParam = new List<Dictionary<string, dynamic>>();
            foreach (var o in gttParams.Orders)
            {
                var order = new Dictionary<string, dynamic>();
                order["exchange"] = gttParams.Exchange;
                order["tradingsymbol"] = gttParams.TradingSymbol;
                order["transaction_type"] = o.TransactionType;
                order["quantity"] = o.Quantity;
                order["price"] = o.Price;
                order["order_type"] = o.OrderType;
                order["product"] = o.Product;
                ordersParam.Add(order);
            }

            var parms = new Dictionary<string, dynamic>();
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
        /// <returns>Json response in the form of nested string dictionary.</returns>
        Dictionary<string, dynamic> ModifyGTT(int GTTId, GTTParams gttParams)
        {
            var condition = new Dictionary<string, dynamic>();
            condition.Add("exchange", gttParams.Exchange);
            condition.Add("tradingsymbol", gttParams.TradingSymbol);
            condition.Add("trigger_values", gttParams.TriggerPrices);
            condition.Add("last_price", gttParams.LastPrice);
            condition.Add("instrument_token", gttParams.InstrumentToken);

            var ordersParam = new List<Dictionary<string, dynamic>>();
            foreach (var o in gttParams.Orders)
            {
                var order = new Dictionary<string, dynamic>();
                order["exchange"] = gttParams.Exchange;
                order["tradingsymbol"] = gttParams.TradingSymbol;
                order["transaction_type"] = o.TransactionType;
                order["quantity"] = o.Quantity;
                order["price"] = o.Price;
                order["order_type"] = o.OrderType;
                order["product"] = o.Product;
                ordersParam.Add(order);
            }

            var parms = new Dictionary<string, dynamic>();
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
        /// <returns>Json response in the form of nested string dictionary.</returns>
        Dictionary<string, dynamic> CancelGTT(int GTTId)
        {
            var parms = new Dictionary<string, dynamic>();
            parms.Add("id", GTTId.ToString());

            return Delete("gtt.delete", parms);
        }

        #endregion GTT


        #region MF Calls

        /// <summary>
        /// Gets the Mutual funds Instruments.
        /// </summary>
        /// <returns>The Mutual funds Instruments.</returns>
        List<MFInstrument> GetMFInstruments()
        {
            var param = new Dictionary<string, dynamic>();

            List<Dictionary<string, dynamic>> instrumentsData;

            instrumentsData = Get("mutualfunds.instruments", param);

            List<MFInstrument> instruments = new List<MFInstrument>();

            foreach (Dictionary<string, dynamic> item in instrumentsData)
                instruments.Add(new MFInstrument(item));

            return instruments;
        }

        /// <summary>
        /// Gets all Mutual funds orders.
        /// </summary>
        /// <returns>The Mutual funds orders.</returns>
        List<MFOrder> GetMFOrders()
        {
            var param = new Dictionary<string, dynamic>();

            Dictionary<string, dynamic> ordersData;
            ordersData = Get("mutualfunds.orders", param);

            List<MFOrder> orderlist = new List<MFOrder>();

            foreach (Dictionary<string, dynamic> item in ordersData["data"])
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
            var param = new Dictionary<string, dynamic>();
            param.Add("order_id", OrderId);

            Dictionary<string, dynamic> orderData;
            orderData = Get("mutualfunds.order", param);

            return new MFOrder(orderData["data"]);
        }

        /// <summary>
        /// Places a Mutual funds order.
        /// </summary>
        /// <returns>JSON response as nested string dictionary.</returns>
        /// <param name="TradingSymbol">Tradingsymbol (ISIN) of the fund.</param>
        /// <param name="TransactionType">BUY or SELL.</param>
        /// <param name="Amount">Amount worth of units to purchase. Not applicable on SELLs.</param>
        /// <param name="Quantity">Quantity to SELL. Not applicable on BUYs. If the holding is less than minimum_redemption_quantity, all the units have to be sold.</param>
        /// <param name="Tag">An optional tag to apply to an order to identify it (alphanumeric, max 8 chars).</param>
        Dictionary<string, dynamic> PlaceMFOrder(
            string TradingSymbol,
            string TransactionType,
            decimal? Amount,
            decimal? Quantity = null,
            string Tag = "")
        {
            var param = new Dictionary<string, dynamic>();

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
        /// <returns>JSON response as nested string dictionary.</returns>
        /// <param name="OrderId">Unique order id.</param>
        Dictionary<string, dynamic> CancelMFOrder(String OrderId)
        {
            var param = new Dictionary<string, dynamic>();

            Utils::AddIfNotNull(param, "order_id", OrderId);

            return Delete("mutualfunds.cancel_order", param);
        }

        /// <summary>
        /// Gets all Mutual funds SIPs.
        /// </summary>
        /// <returns>The list of all Mutual funds SIPs.</returns>
        List<MFSIP> GetMFSIPs()
        {
            var param = new Dictionary<string, dynamic>();

            Dictionary<string, dynamic> sipData;
            sipData = Get("mutualfunds.sips", param);

            List<MFSIP> siplist = new List<MFSIP>();

            foreach (Dictionary<string, dynamic> item in sipData["data"])
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
            var param = new Dictionary<string, dynamic>();
            param.Add("sip_id", SIPID);

            Dictionary<string, dynamic> sipData;
            sipData = Get("mutualfunds.sip", param);

            return new MFSIP(sipData["data"]);
        }

        /// <summary>
        /// Places a Mutual funds SIP order.
        /// </summary>
        /// <returns>JSON response as nested string dictionary.</returns>
        /// <param name="TradingSymbol">ISIN of the fund.</param>
        /// <param name="Amount">Amount worth of units to purchase. It should be equal to or greated than minimum_additional_purchase_amount and in multiple of purchase_amount_multiplier in the instrument master.</param>
        /// <param name="InitialAmount">Amount worth of units to purchase before the SIP starts. Should be equal to or greater than minimum_purchase_amount and in multiple of purchase_amount_multiplier. This is only considered if there have been no prior investments in the target fund.</param>
        /// <param name="Frequency">weekly, monthly, or quarterly.</param>
        /// <param name="InstalmentDay">If Frequency is monthly, the day of the month (1, 5, 10, 15, 20, 25) to trigger the order on.</param>
        /// <param name="Instalments">Number of instalments to trigger. If set to -1, instalments are triggered at fixed intervals until the SIP is cancelled.</param>
        /// <param name="Tag">An optional tag to apply to an order to identify it (alphanumeric, max 8 chars).</param>
        Dictionary<string, dynamic> PlaceMFSIP(
            string TradingSymbol,
            decimal? Amount,
            decimal? InitialAmount,
            string Frequency,
            int? InstalmentDay,
            int? Instalments,
            string Tag = "")
        {
            var param = new Dictionary<string, dynamic>();

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
        /// <returns>JSON response as nested string dictionary.</returns>
        /// <param name="SIPId">SIP id.</param>
        /// <param name="Amount">Amount worth of units to purchase. It should be equal to or greated than minimum_additional_purchase_amount and in multiple of purchase_amount_multiplier in the instrument master.</param>
        /// <param name="Frequency">weekly, monthly, or quarterly.</param>
        /// <param name="InstalmentDay">If Frequency is monthly, the day of the month (1, 5, 10, 15, 20, 25) to trigger the order on.</param>
        /// <param name="Instalments">Number of instalments to trigger. If set to -1, instalments are triggered idefinitely until the SIP is cancelled.</param>
        /// <param name="Status">Pause or unpause an SIP (active or paused).</param>
        Dictionary<string, dynamic> ModifyMFSIP(
            string SIPId,
            decimal? Amount,
            string Frequency,
            int? InstalmentDay,
            int? Instalments,
            string Status)
        {
            var param = new Dictionary<string, dynamic>();

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
        /// <returns>JSON response as nested string dictionary.</returns>
        /// <param name="SIPId">SIP id.</param>
		Dictionary<string, dynamic> CancelMFSIP(String SIPId)
        {
            var param = new Dictionary<string, dynamic>();

            Utils::AddIfNotNull(param, "sip_id", SIPId);

            return Delete("mutualfunds.cancel_sips", param);
        }

        /// <summary>
        /// Gets the Mutual funds holdings.
        /// </summary>
        /// <returns>The list of all Mutual funds holdings.</returns>
        List<MFHolding> GetMFHoldings()
        {
            var param = new Dictionary<string, dynamic>();

            Dictionary<string, dynamic> holdingsData;
            holdingsData = Get("mutualfunds.holdings", param);

            List<MFHolding> holdingslist = new List<MFHolding>();

            foreach (Dictionary<string, dynamic> item in holdingsData["data"])
                holdingslist.Add(new MFHolding(item));

            return holdingslist;
        }

        #endregion

//        #region HTTP Functions
private:
        /// <summary>
        /// Alias for sending a GET request.
        /// </summary>
        /// <param name="Route">URL route of API</param>
        /// <param name="Params">Additional paramerters</param>
        /// <returns>Varies according to API endpoint</returns>
        std::string Get(string route, Dictionary<string, dynamic> params = null)
        {
            return Request(route, methods::GET, params);
        }

        /// <summary>
        /// Alias for sending a POST request.
        /// </summary>
        /// <param name="Route">URL route of API</param>
        /// <param name="Params">Additional paramerters</param>
        /// <returns>Varies according to API endpoint</returns>
        std::string Post(string route, shared_ptr<map<string, string>> params = nullptr)
        {
            return Request(Route, methods::POST, params);
        }

        /// <summary>
        /// Alias for sending a PUT request.
        /// </summary>
        /// <param name="Route">URL route of API</param>
        /// <param name="Params">Additional paramerters</param>
        /// <returns>Varies according to API endpoint</returns>
        task<http_response> Put(string Route, Dictionary<string, dynamic> Params = null)
        {
            return Request(Route, methods::PUT, Params);
        }

        /// <summary>
        /// Alias for sending a DELETE request.
        /// </summary>
        /// <param name="Route">URL route of API</param>
        /// <param name="Params">Additional paramerters</param>
        /// <returns>Varies according to API endpoint</returns>
        task<http_response> Delete(string Route, Dictionary<string, dynamic> Params = null)
        {
            return Request(Route, methods::DEL, Params);
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
        std::string Request(string route, string method, map<string, string> params = null)
        {
            string url = _root + _routes[Route];
            http_response webResponse;

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
            //string paramString = String.Join("&", Params.Select(x => Utils::BuildParam(x.Key, x.Value)));
            http_request webRequest(method);
            AddExtraHeaders(webRequest);

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
               webRequest.set_body(paramEncoder.to_string().erase(2),
                                    mime_types::application_x_www_form_urlencoded);
            }
            else
            {
                //request = (HttpWebRequest)WebRequest.Create(url + "?" + paramString);
                //request.AllowAutoRedirect = true;
                //request.Method = Method;
                //TODO if (_enableLogging) Console.WriteLine("DEBUG: " + Method + " " + url + "?" + paramString);
                //AddExtraHeaders(ref request);
                route = route+paramEncoder.to_string();
            }

            webRequest.set_request_uri(route);

            std::string body;
            http::details::mime_types contentType;
            http::status_code status;

            // Send request and wait for the response
            pplx::task<web::http::http_response> webResponse =  httpClient.request(webRequest);
            try { 
                //-- All Taskss will get triggered here 
                webResponse.wait();
                body = webResponse.get().extract_string().get();
                contentType = http::details::mime_types::application_json;
                status = webResponse.status_code();
                std::cout<<"Response received:\n";
            }
            catch (const std::exception &e) {
                    cout<<("Error exception: "<<e.what())<<endl;
                    throw e;
            }
            
            //TODO if (_enableLogging) Console.WriteLine("DEBUG: " + (int)((HttpWebResponse)webResponse).StatusCode + " " + response + "\n");


            if (contentType == http::details::mime_types::application_json)
            {
                //Dictionary<string, dynamic> responseDictionary = Utils::JsonDeserialize(response);
                if (status != http::status_codes::OK)
                {
                    string errorType = "GeneralException";
                    string message = "";
                    web::json::value obj = web::json::value::parse(body);

                    if (obj.has_string_field("error_type"))
                        errorType = obj["error_type"];

                    if (obj.has_string_field("message"))
                        message = obj["message"];

                    switch (errorType)
                    {
                        case "GeneralException": throw new GeneralException(message, status);
                        case "TokenException":
                            {
                                _sessionHook.Invoke();
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
                return body;
            }
            else if (contentType == "text/csv")
                return body;
            else
                throw new DataException("Unexpected content type " + webResponse.ContentType + " " + body);
        }

        //#endregion
    }
}
