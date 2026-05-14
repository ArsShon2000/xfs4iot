#pragma once
#include "XFSCommon/XFSAPI.H"
#include <string>
#include <cstdint>

namespace FS365 {
    namespace Utilities {

        /// Метод возвращает численный идентификатор валюты
        /// по буквенному идентификатору
        /// Например: XTS -> 963
        uint16_t CurrencyNumByStr(LPCSTR pszCurrency);

        /// Метод возвращает буквенный идентификатор
        /// по численному идентификатору валюты
        /// Например: 963 -> XTS
        LPCSTR CurrencyCodeByNum(uint16_t nCode);

        /// Два метода, позволяют получить количество знаков (размер) минорной части валюты.
        /// Пример: 2 знака - для рублей (0 - 99 копеек)
        uint16_t GetCurrencyMinorUnitsSize(uint16_t nCurrencyCode);
        uint16_t GetCurrencyMinorUnitsSize(LPCSTR pszCurrencyCode);
    }
}
