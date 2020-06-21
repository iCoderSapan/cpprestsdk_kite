#pragma once
#include<string>

namespace KiteConnect
{
    class Constants
    {
        public:

        // Products
        static const std::string PRODUCT_MIS;
        static const std::string PRODUCT_CNC;
        static const std::string PRODUCT_NRML;

        // Order types
        static const std::string ORDER_TYPE_MARKET;
        static const std::string ORDER_TYPE_LIMIT;
        static const std::string ORDER_TYPE_SLM;
        static const std::string ORDER_TYPE_SL;

        // Order status
        static const std::string ORDER_STATUS_COMPLETE;
        static const std::string ORDER_STATUS_CANCELLED;
        static const std::string ORDER_STATUS_REJECTED;

        // Varities
        static const std::string VARIETY_REGULAR;
        static const std::string VARIETY_BO;
        static const std::string VARIETY_CO;
        static const std::string VARIETY_AMO;

        // Transaction type
        static const std::string TRANSACTION_TYPE_BUY;
        static const std::string TRANSACTION_TYPE_SELL;

        // Validity
        static const std::string VALIDITY_DAY;
        static const std::string VALIDITY_IOC;

        // Exchanges
        static const std::string EXCHANGE_NSE;
        static const std::string EXCHANGE_BSE;
        static const std::string EXCHANGE_NFO;
        static const std::string EXCHANGE_CDS;
        static const std::string EXCHANGE_BFO;
        static const std::string EXCHANGE_MCX;

        // Margins segments
        static const std::string MARGIN_EQUITY;
        static const std::string MARGIN_COMMODITY;

        // Ticker modes
        static const std::string MODE_FULL;
        static const std::string MODE_QUOTE;
        static const std::string MODE_LTP;

        // Positions
        static const std::string POSITION_DAY;
        static const std::string POSITION_OVERNIGHT;

        // Historical interv
        static const std::string INTERVAL_MINUTE;
        static const std::string INTERVAL_3MINUTE;
        static const std::string INTERVAL_5MINUTE;
        static const std::string INTERVAL_10MINUTE;
        static const std::string INTERVAL_15MINUTE;
        static const std::string INTERVAL_30MINUTE;
        static const std::string INTERVAL_60MINUTE;
        static const std::string INTERVAL_DAY;

        // GTT status
        static const std::string GTT_ACTIVE;
        static const std::string GTT_TRIGGERED;
        static const std::string GTT_DISABLED;
        static const std::string GTT_EXPIRED;
        static const std::string GTT_CANCELLED;
        static const std::string GTT_REJECTED;
        static const std::string GTT_DELETED;


        // GTT trigger type
        static const std::string GTT_TRIGGER_OCO;
        static const std::string GTT_TRIGGER_SINGLE;
    };
}    