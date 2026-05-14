#include "AsciiHexConversions.h"
#include <algorithm> // Для std::starts_with
#include <cctype>

namespace AsciiHexConversions {

    bool AsciiStringToHex_impl(const char* pszString, size_t nSize, std::vector< uint8_t >& arrHex, bool bFailIfEmpty = true)
    {
        arrHex.clear();

        if (!pszString || 0 == nSize) {
            return !bFailIfEmpty;
        }

        bool bEvenCycle = false;

        char msc;

        const char pszDelimiters[] = { " \t:,\r\n%" };

        const char* pszCurPos = pszString;

        for (;; ++pszCurPos) {

            if (size_t(pszCurPos - pszString) >= nSize) {
                break;
            }

            // Пропускаем допустимый символ-разделитель
            if (strchr(pszDelimiters, *pszCurPos)) {
                continue;
            }

            // Проверяем: это допустимый HEX-digit
            if (!isxdigit(*pszCurPos)) {
                return false;
            }

            if (!bEvenCycle) {

                msc = toupper(*pszCurPos);
                bEvenCycle = true;

            }
            else {

                // Собираем ASCII-байт
                arrHex.push_back(AsciiHex2Byte(msc, toupper(*pszCurPos)));
                bEvenCycle = false;
            }
        }

        // Проверяем: не превались на незаконченной тетраде
        if (bEvenCycle) {
            arrHex.clear();
            return false;
        }

        return true;
    }


    bool _do_unit_testing(void)
    {
        std::vector< uint8_t > result;

        char pszStr1[] = "12 3 4 Df 67";
        char pszStr2[] = " 12 3 4 Dg 67";
        char pszStr3[] = " 12 3 4 AC 67 :";
        char pszStr4[] = " 123 ";
        char pszStr5[] = "01 23 45 67 89 AB CD ef";
        char pszStrEmpty[] = "";

        if (!AsciiStringToHex_impl(pszStr1, strlen(pszStr1), result)) {
            return false;
        }
        if (result != std::vector < uint8_t > { 0x12, 0x34, 0xdf, 0x67 }) {
            return false;
        }
        if (AsciiStringToHex_impl(pszStr2, strlen(pszStr2), result)) {
            return false;
        }
        if (!AsciiStringToHex_impl(pszStr3, strlen(pszStr3), result)) {
            return false;
        }
        if (AsciiStringToHex_impl(pszStrEmpty, strlen(pszStrEmpty), result, true)) {
            return false;
        }
        if (!AsciiStringToHex_impl(pszStrEmpty, strlen(pszStrEmpty), result, false)) {
            return false;
        }
        if (!AsciiStringToHex_impl(pszStr5, strlen(pszStr5), result)) {
            return false;
        }
        if (result != std::vector < uint8_t > { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xef }) {
            return false;
        }

        return true;
    }

    uint8_t AsciiHex2Byte(char ms, char ls)
    {
        uint8_t res = 0;
        char szPattern[] = "0123456789ABCDEF";

        if (ms >= 'a' && ms <= 'z') {
            ms &= ~0x20;
        }

        if (ls >= 'a' && ls <= 'z') {
            ls &= ~0x20;
        }

        char* pos;
        pos = strchr(szPattern, ls);
        if (pos == 0) return 0;
        res = (uint8_t)(pos - szPattern);

        pos = strchr(szPattern, ms);
        if (pos == 0) return 0;
        res |= (uint8_t)(pos - szPattern) << 4;

        return res;
    }

    bool AsciiStringToHex(const char* pszString, std::vector< uint8_t >& arrHex, bool bFailIfEmpty)
    {
        if (0 == pszString) {
            return false;
        }
        return AsciiStringToHex_impl(pszString, strlen(pszString), arrHex, bFailIfEmpty);
    }

    bool AsciiStringToHex(const char* pchString, int nLen, std::vector< uint8_t >& arrHex, bool bFailIfEmpty)
    {
        return AsciiStringToHex_impl(pchString, nLen, arrHex, bFailIfEmpty);
    }

    bool AsciiStringToHex(const std::string& strString, std::vector< uint8_t >& arrHex, bool bFailIfEmpty)
    {
        return AsciiStringToHex_impl(strString.c_str(), strString.size(), arrHex, bFailIfEmpty);
    }

    std::vector< uint8_t > AsciiStringToHex(const std::string& strString)
    {
        std::vector< uint8_t > result;
        AsciiStringToHex_impl(strString.c_str(), strString.size(), result);
        return result;
    }

    std::vector< uint8_t > AsciiStringToHex(const char* pszString)
    {
        if (!pszString) {
            return {};
        }
        std::vector< uint8_t > result;
        AsciiStringToHex_impl(pszString, strlen(pszString), result);
        return result;
    }

    std::vector< uint8_t > AsciiStringToHex(const char* pchString, int nLen)
    {
        if (!pchString) {
            return{};
        }
        std::vector< uint8_t > result;
        AsciiStringToHex_impl(pchString, nLen, result);
        return result;
    }

    bool AsciiStringToHexHTTP(const std::string& strString, std::string& strOutString, bool bFailIfEmpty /*= true*/)
    {
        if (strString.empty() && bFailIfEmpty) {
            return false;
        }
        std::vector< uint8_t > result;
        if (AsciiStringToHex_impl(strString.c_str(), strString.size(), result))
        {
            strOutString.assign(result.begin(), result.end());
            return true;
        }
        return false;
    }

    void BinToAsciiString(const uint8_t* pData, size_t nLen, std::string& strHex, bool bInsertSpaces)
    {
        strHex.clear();
        strHex.reserve(nLen * (bInsertSpaces ? 3 : 2));
        for (size_t k = 0; k < nLen; ++k) {
            char _psz[5] = { 0 };
            if (bInsertSpaces) {
                sprintf_s(_psz, "%02X ", pData[k]);
            }
            else {
                sprintf_s(_psz, "%02X", pData[k]);
            }
            strHex += _psz;
        }

        // Удаляем лишний пробел
        if ((!strHex.empty()) && (bInsertSpaces)) {
            strHex.erase(strHex.size() - 1, 1);
        }
    }



    std::string BinToAsciiString(const uint8_t* pData, int nLen, bool bInsertSpaces)
    {
        std::string strResult;
        BinToAsciiString(pData, nLen, strResult, bInsertSpaces);
        return strResult;
    }

    std::string BinToAsciiString(const std::vector< uint8_t >& arrHex, bool bInsertSpaces)
    {
        return BinToAsciiString(arrHex.cbegin(), arrHex.cend(), bInsertSpaces);
    }

    std::string BinToAsciiString(const std::span< uint8_t >& arrHex, bool bInsertSpaces)
    {
        return BinToAsciiString(arrHex.cbegin(), arrHex.cend(), bInsertSpaces);
    }

    void BinToAsciiString(const std::vector< uint8_t >& arrHex, std::string& strHex, bool bInsertSpaces)
    {
        strHex = BinToAsciiString(arrHex.cbegin(), arrHex.cend(), bInsertSpaces);
    }

    std::string BinToAsciiString(const std::string& pData, bool bInsertSpaces, std::string prefixSymbol)
    {
        return BinToAsciiString(pData.cbegin(), pData.cend(), bInsertSpaces, prefixSymbol);
    }

    template< class _InArrIter >
    std::string BinToAsciiString(const _InArrIter& _from, const _InArrIter& _to, bool bInsertSpaces /*= true*/, const std::string prefixString /*= std::string()*/)
    {
        std::string strResult;
        strResult.reserve(std::distance(_from, _to) * (3 + prefixString.length()));

        _InArrIter it = _from;

        std::string strFmt = { bInsertSpaces ? prefixString + "%02X " : prefixString + "%02X" };

        std::string _psz;
        _psz.resize(strFmt.length() + strFmt.length() + 1);

        for (; it != _to; ++it) {
            std::fill(_psz.begin(), _psz.end(), 0);
            sprintf_s((char*)_psz.data(), _psz.length(), strFmt.c_str(), (unsigned char)*it);
            strResult += _psz.c_str();
        }

        // Удаляем лишний пробел
        if ((!strResult.empty()) && bInsertSpaces) {
            strResult.erase(strResult.size() - 1, 1);
        }

        return strResult;
    }

} // namespace Xfs::Utilities::AsciiHexConversions
