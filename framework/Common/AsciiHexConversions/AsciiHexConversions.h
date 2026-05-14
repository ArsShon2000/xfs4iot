#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <span>
#include <ranges>
#include <format>
#include <charconv>
#include <algorithm>

namespace AsciiHexConversions {

    bool _do_unit_testing(void);

    // Преобразование AsciiHex-байта в двоичное представление
    //
    // @param [in] ms - старший HEX-разряд
    // @param [in] ls - младший HEX-разряд
    // @return двоичное представление HEX-числа
    uint8_t AsciiHex2Byte(char ms, char ls);

    // Преобразование строки ASCII в массив байтов
    // @return - false, если входная строка содержит недопустимые символы
    bool AsciiStringToHex(const char* pszString, std::vector< uint8_t >& arrHex, bool bFailIfEmpty = true);
    bool AsciiStringToHex(const char* pchString, int nLen, std::vector< uint8_t >& arrHex, bool bFailIfEmpty = true);
    bool AsciiStringToHex(const std::string& strString, std::vector< uint8_t >& arrHex, bool bFailIfEmpty = true);
    bool AsciiStringToHexHTTP(const std::string& strString, std::string& strOutString, bool bFailIfEmpty = true);

    std::vector< uint8_t > AsciiStringToHex(const std::string& strString);
    std::vector< uint8_t > AsciiStringToHex(const char* pszString);
    std::vector< uint8_t > AsciiStringToHex(const char* pchString, int nLen);

    // Преобразование двоичного массива в представление ASCII
    // @param [in] bInsertSpaces - вставлять разделяющие пробелы между байтами
    void BinToAsciiString(const uint8_t* pData, size_t nLen, std::string& strHex, bool bInsertSpaces = true);
    void BinToAsciiString(const std::vector< uint8_t >& arrHex, std::string& strHex, bool bInsertSpaces = true);
    std::string BinToAsciiString(const std::string& pData, bool bInsertSpaces, std::string spaceSymbol);
    std::string BinToAsciiString(const uint8_t* pData, int nLen, bool bInsertSpaces = true);
    std::string BinToAsciiString(const std::vector< uint8_t >& arrHex, bool bInsertSpaces);
    std::string BinToAsciiString(const std::span<uint8_t>& arrHex, bool bInsertSpaces);

    template< class _InArrIter >
    std::string BinToAsciiString(const _InArrIter& _from, const _InArrIter& _to, bool bInsertSpaces = true, const std::string prefixString = std::string());


} // namespace Xfs::Utilities::AsciiHexConversions
