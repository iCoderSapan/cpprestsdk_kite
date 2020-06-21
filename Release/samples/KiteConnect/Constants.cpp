#include<string>
#include "Constants.h"

namespace KiteConnect
{

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
