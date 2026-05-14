// ------------------------------------------------------------------------------------
// Copyright © 2005, SYSTEMA LLC. All rights reserved.
// System: XFS
// Author: Арс Карибаев
//
// Description:
// Реализация классов для обработки идентификационных данных устройства
// Адаптировано для Windows/Linux и C++23
// ------------------------------------------------------------------------------------

#include "Identification.h"
#include <cstdint>
#include <string>
#include <format>
#include <span>
#include <algorithm>

namespace FS365 {
    namespace HW {
        namespace Dors {

            // CIdentification::CIdentification() = default;

            CIdentification::CIdentification(const CCnetIdentification& ccnetIdn)
                : strPartNumber(ccnetIdn.strPartNumber),
                strSerialNumber(ccnetIdn.strSerialNumber),
                arrAssetNumber(ccnetIdn.arrAssetNumber) {
            }

            CIdentification::CIdentification(const CDbaIdentification& dbaIdn)
                : strPartNumber(dbaIdn.ccnetIdn.strPartNumber),
                strSerialNumber(dbaIdn.ccnetIdn.strSerialNumber),
                arrAssetNumber(dbaIdn.ccnetIdn.arrAssetNumber),
                strSoftwareVersion(dbaIdn.softwareVersion.PrintVersion()),
                strNotebaseVersion(dbaIdn.notebaseVersion.PrintVersion()) {
            }

            std::string CIdentification::Print(int nIndent) const {
                std::string indent(nIndent, '\t');
                std::string result = std::format("{}Part Number: {};\n", indent, strPartNumber);
                result += std::format("{}Serial Number: {}", indent, strSerialNumber);

                if (!strSoftwareVersion.empty()) {
                    result += std::format(";\n{}Software Version: {}", indent, strSoftwareVersion);
                }
                if (!strNotebaseVersion.empty()) {
                    result += std::format(";\n{}Notebase Version: {}", indent, strNotebaseVersion);
                }
                return result;
            }

            bool CIdentification::IsEmpty() const {
                return strPartNumber.empty() && strSerialNumber.empty() && arrAssetNumber.empty() &&
                    strSoftwareVersion.empty() && strNotebaseVersion.empty();
            }

            // CCnetIdentification::CCnetIdentification()  = default;

            CCnetIdentification::CCnetIdentification(std::span<const std::uint8_t> arrData) {
                if (arrData.size() < 34) {
                    return;
                }

                // Part Number (15 bytes)
                strPartNumber.assign(arrData.subspan(0, 15).begin(), arrData.subspan(0, 15).end());
                std::ranges::replace(strPartNumber, '\0', ' ');
                strPartNumber = trim_right(std::string_view(strPartNumber), std::string_view(" "));

                // Serial Number (12 bytes)
                strSerialNumber.assign(arrData.subspan(15, 12).begin(), arrData.subspan(15, 12).end());
                std::ranges::replace(strSerialNumber, '\0', ' ');
                strSerialNumber = trim_right(std::string_view(strSerialNumber), std::string_view(" "));

                // Asset Number (7 bytes)
                arrAssetNumber.assign(arrData.subspan(27, 7).begin(), arrData.subspan(27, 7).end());
            }

            std::string CCnetIdentification::Print(int nIndent) const {
                std::string indent(nIndent, '\t');
                return std::format("{}Part Number: {};\n{}Serial Number: {}", indent, strPartNumber, indent, strSerialNumber);
            }

            // CDbaIdentification::CDbaIdentification()  = default;

            CDbaIdentification::CDbaIdentification(std::span<const std::uint8_t> arrData) {
                if (arrData.size() < 42) {
                    return;
                }

                // Cash Code Identification (34 bytes)
                ccnetIdn = CCnetIdentification(arrData.subspan(0, 34));

                // Software Version (4 bytes)
                softwareVersion = fw_version(arrData.subspan(34, 4));

                // Note Base Version (4 bytes)
                notebaseVersion = nbase_version(arrData.subspan(38, 4));
            }

            std::string CDbaIdentification::Print(int nIndent) const {
                std::string indent(nIndent, '\t');
                return std::format("{};\n{}Software Version: {};\n{}Notebase Version: {}",
                    ccnetIdn.Print(nIndent), indent, softwareVersion.PrintVersion(), indent, notebaseVersion.PrintVersion());
            }

            // fw_version::fw_version()  : ulNumber(0) {}

            fw_version::fw_version(std::span<const std::uint8_t> arrData) {
                if (arrData.size() >= 4) {
                    ulNumber = (static_cast<std::uint32_t>(arrData[0]) << 24) |
                        (static_cast<std::uint32_t>(arrData[1]) << 16) |
                        (static_cast<std::uint32_t>(arrData[2]) << 8) |
                        static_cast<std::uint32_t>(arrData[3]);
                }
            }

            std::string fw_version::PrintVersion() const {
                return std::format("{}.{}.{}", static_cast<int>(HIBYTE(HIWORD(ulNumber))),
                    static_cast<int>(LOBYTE(HIWORD(ulNumber))),
                    static_cast<int>(LOWORD(ulNumber)));
            }

            // nbase_version::nbase_version()  = default;

            nbase_version::nbase_version(std::span<const std::uint8_t> arrData) : fw_version(arrData) {}

            std::string nbase_version::PrintVersion() const {
                return std::format("{}.{}.{:03}", static_cast<int>(HIWORD(ulNumber)),
                    static_cast<int>(HIBYTE(LOWORD(ulNumber))),
                    static_cast<int>(LOBYTE(LOWORD(ulNumber))));
            }

        } // namespace Dors
    } // namespace HW
} // namespace FS365
