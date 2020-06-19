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
const std::string Constants::PRODUCT_CNC = "CNC";
const std::string Constants::PRODUCT_NRML = "NRML";

const std::string Constants::ORDER_TYPE_MARKET = "MARKET";
const std::string Constants::ORDER_TYPE_LIMIT = "LIMIT";
const std::string Constants::ORDER_TYPE_SLM = "SL-M";
const std::string Constants::ORDER_TYPE_SL = "SL";

const std::string Constants::ORDER_STATUS_COMPLETE = "COMPLETE";
const std::string Constants::ORDER_STATUS_CANCELLED = "CANCELLED";
const std::string Constants::ORDER_STATUS_REJECTED = "REJECTED";

const std::string Constants::VARIETY_REGULAR = "regular";
const std::string Constants::VARIETY_BO = "bo";
const std::string Constants::VARIETY_CO = "co";
const std::string Constants::VARIETY_AMO = "amo";

const std::string Constants::TRANSACTION_TYPE_BUY = "BUY";
const std::string Constants::TRANSACTION_TYPE_SELL = "SELL";

const std::string Constants::VALIDITY_DAY = "DAY";
const std::string Constants::VALIDITY_IOC = "IOC";

const std::string Constants::EXCHANGE_NSE = "NSE";
const std::string Constants::EXCHANGE_BSE = "BSE";
const std::string Constants::EXCHANGE_NFO = "NFO";
const std::string Constants::EXCHANGE_CDS = "CDS";
const std::string Constants::EXCHANGE_BFO = "BFO";
const std::string Constants::EXCHANGE_MCX = "MCX";

const std::string Constants::MARGIN_EQUITY = "equity";
const std::string Constants::MARGIN_COMMODITY = "commodity";

const std::string Constants::MODE_FULL = "full";
const std::string Constants::MODE_QUOTE = "quote";
const std::string Constants::MODE_LTP = "ltp";

const std::string Constants::POSITION_DAY = "day";
const std::string Constants::POSITION_OVERNIGHT = "overnight";

const std::string Constants::INTERVAL_MINUTE = "minute";
const std::string Constants::INTERVAL_3MINUTE = "3minute";
const std::string Constants::INTERVAL_5MINUTE = "5minute";
const std::string Constants::INTERVAL_10MINUTE = "10minute";
const std::string Constants::INTERVAL_15MINUTE = "15minute";
const std::string Constants::INTERVAL_30MINUTE = "30minute";
const std::string Constants::INTERVAL_60MINUTE = "60minute";
const std::string Constants::INTERVAL_DAY = "day";

const std::string Constants::GTT_ACTIVE = "active";
const std::string Constants::GTT_TRIGGERED = "triggered";
const std::string Constants::GTT_DISABLED = "disabled";
const std::string Constants::GTT_EXPIRED = "expired";
const std::string Constants::GTT_CANCELLED = "cancelled";
const std::string Constants::GTT_REJECTED = "rejected";
const std::string Constants::GTT_DELETED = "deleted";


const std::string Constants::GTT_TRIGGER_OCO = "two-leg";
const std::string Constants::GTT_TRIGGER_SINGLE = "single";
}
