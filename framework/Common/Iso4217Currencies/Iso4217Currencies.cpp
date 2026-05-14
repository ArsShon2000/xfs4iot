#include "Iso4217Currencies.hpp"

namespace FS365 {
    namespace Utilities {

        uint16_t CurrencyNumByStr(LPCSTR pszCurrency)
        {
            // TODO: подтянуть всю таблицу ISO4217 и сделать более эффективный
            // механизм преобразования
            if (0 == _stricmp(pszCurrency, "RUB")) {
                return 643;
            }
            else if (0 == _stricmp(pszCurrency, "USD")) {
                return 840;
            }
            else if (0 == _stricmp(pszCurrency, "EUR")) {
                return 978;
            }
            else if (0 == _stricmp(pszCurrency, "KZT")) {
                return 398;
            }
            else if (0 == _stricmp(pszCurrency, "UZS")) {
                return 860;
            }
            else if (0 == _stricmp(pszCurrency, "UAH")) {
                return 980;
            }
            else if (0 == _stricmp(pszCurrency, "THB")) {
                return 764;
            }
            else if (0 == _stricmp(pszCurrency, "BYR")) {
                return 974;
            }
            else if (0 == _stricmp(pszCurrency, "BYN")) {
                return 933;
            }
            else if (0 == _stricmp(pszCurrency, "RUP")) {
                return 509;
            }
            else if (0 == _stricmp(pszCurrency, "XTS")) {
                return 963;
            } return 0;
        }

        LPCSTR CurrencyCodeByNum(uint16_t nCode)
        {
            // TODO: подтянуть всю таблицу ISO4217 и сделать более эффективный
            // механизм преобразования
            switch (nCode) {
            case 643: return "RUB";
            case 840: return "USD";
            case 978: return "EUR";
            case 398: return "KZT";
            case 860: return "UZS";
            case 980: return "UAH";
            case 764: return "THB";
            case 974: return "BYR";
            case 933: return "BYN";
            case 509: return "RUP";
            case 963: return "XTS";
            }
            return "???";
        }

        uint16_t GetCurrencyMinorUnitsSize(uint16_t nCurrencyCode)
        {
            // @note подавляющее большинство валют, имеют экспоненту 2
            // Этим и воспользуемся
            switch (nCurrencyCode) {
            case 48:
            case 368:
            case 400:
            case 414:
            case 434:
            case 512:
            case 788:
                return 3;
            case 108:
            case 974:
            case 990:
            case 152:
            case 262:
            case 324:
            case 352:
            case 392:
            case 174:
            case 410:
            case 600:
            case 646:
            case 950:
            case 952:
            case 953:
                return 0;
            default:
                return 2;
            }
        }

        uint16_t GetCurrencyMinorUnitsSize(LPCSTR pszCurrencyCode)
        {
            return GetCurrencyMinorUnitsSize(CurrencyNumByStr(pszCurrencyCode));
        }

    }
}
