#include<string>
using namespace std;
namespace KiteConnect
{
    class Constants
    {
        public:

        // Products
        static const string PRODUCT_MIS;
        static const string PRODUCT_CNC;
        static const string PRODUCT_NRML;

        // Order types
        static const string ORDER_TYPE_MARKET;
        static const string ORDER_TYPE_LIMIT;
        static const string ORDER_TYPE_SLM;
        static const string ORDER_TYPE_SL;

        // Order status
        static const string ORDER_STATUS_COMPLETE;
        static const string ORDER_STATUS_CANCELLED;
        static const string ORDER_STATUS_REJECTED;

        // Varities
        static const string VARIETY_REGULAR;
        static const string VARIETY_BO;
        static const string VARIETY_CO;
        static const string VARIETY_AMO;

        // Transaction type
        static const string TRANSACTION_TYPE_BUY;
        static const string TRANSACTION_TYPE_SELL;

        // Validity
        static const string VALIDITY_DAY;
        static const string VALIDITY_IOC;

        // Exchanges
        static const string EXCHANGE_NSE;
        static const string EXCHANGE_BSE;
        static const string EXCHANGE_NFO;
        static const string EXCHANGE_CDS;
        static const string EXCHANGE_BFO;
        static const string EXCHANGE_MCX;

        // Margins segments
        static const string MARGIN_EQUITY;
        static const string MARGIN_COMMODITY;

        // Ticker modes
        static const string MODE_FULL;
        static const string MODE_QUOTE;
        static const string MODE_LTP;

        // Positions
        static const string POSITION_DAY;
        static const string POSITION_OVERNIGHT;

        // Historical interv
        static const string INTERVAL_MINUTE;
        static const string INTERVAL_3MINUTE;
        static const string INTERVAL_5MINUTE;
        static const string INTERVAL_10MINUTE;
        static const string INTERVAL_15MINUTE;
        static const string INTERVAL_30MINUTE;
        static const string INTERVAL_60MINUTE;
        static const string INTERVAL_DAY;

        // GTT status
        static const string GTT_ACTIVE;
        static const string GTT_TRIGGERED;
        static const string GTT_DISABLED;
        static const string GTT_EXPIRED;
        static const string GTT_CANCELLED;
        static const string GTT_REJECTED;
        static const string GTT_DELETED;


        // GTT trigger type
        static const string GTT_TRIGGER_OCO;
        static const string GTT_TRIGGER_SINGLE;
    };
const string Constants::PRODUCT_CNC = "CNC";
const string Constants::PRODUCT_NRML = "NRML";

const string Constants::ORDER_TYPE_MARKET = "MARKET";
const string Constants::ORDER_TYPE_LIMIT = "LIMIT";
const string Constants::ORDER_TYPE_SLM = "SL-M";
const string Constants::ORDER_TYPE_SL = "SL";

const string Constants::ORDER_STATUS_COMPLETE = "COMPLETE";
const string Constants::ORDER_STATUS_CANCELLED = "CANCELLED";
const string Constants::ORDER_STATUS_REJECTED = "REJECTED";

const string Constants::VARIETY_REGULAR = "regular";
const string Constants::VARIETY_BO = "bo";
const string Constants::VARIETY_CO = "co";
const string Constants::VARIETY_AMO = "amo";

const string Constants::TRANSACTION_TYPE_BUY = "BUY";
const string Constants::TRANSACTION_TYPE_SELL = "SELL";

const string Constants::VALIDITY_DAY = "DAY";
const string Constants::VALIDITY_IOC = "IOC";

const string Constants::EXCHANGE_NSE = "NSE";
const string Constants::EXCHANGE_BSE = "BSE";
const string Constants::EXCHANGE_NFO = "NFO";
const string Constants::EXCHANGE_CDS = "CDS";
const string Constants::EXCHANGE_BFO = "BFO";
const string Constants::EXCHANGE_MCX = "MCX";

const string Constants::MARGIN_EQUITY = "equity";
const string Constants::MARGIN_COMMODITY = "commodity";

const string Constants::MODE_FULL = "full";
const string Constants::MODE_QUOTE = "quote";
const string Constants::MODE_LTP = "ltp";

const string Constants::POSITION_DAY = "day";
const string Constants::POSITION_OVERNIGHT = "overnight";

const string Constants::INTERVAL_MINUTE = "minute";
const string Constants::INTERVAL_3MINUTE = "3minute";
const string Constants::INTERVAL_5MINUTE = "5minute";
const string Constants::INTERVAL_10MINUTE = "10minute";
const string Constants::INTERVAL_15MINUTE = "15minute";
const string Constants::INTERVAL_30MINUTE = "30minute";
const string Constants::INTERVAL_60MINUTE = "60minute";
const string Constants::INTERVAL_DAY = "day";

const string Constants::GTT_ACTIVE = "active";
const string Constants::GTT_TRIGGERED = "triggered";
const string Constants::GTT_DISABLED = "disabled";
const string Constants::GTT_EXPIRED = "expired";
const string Constants::GTT_CANCELLED = "cancelled";
const string Constants::GTT_REJECTED = "rejected";
const string Constants::GTT_DELETED = "deleted";


const string Constants::GTT_TRIGGER_OCO = "two-leg";
const string Constants::GTT_TRIGGER_SINGLE = "single";
}
