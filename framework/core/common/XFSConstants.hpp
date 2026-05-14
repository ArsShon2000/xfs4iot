#pragma once

#include <vector>
#include <string_view>
#include "../SettingModule/SettingModule.h"

namespace XFS4IoT
{
    struct XFSConstants
    {
        /// XFS4IoT mandated port options
        static const std::vector<int>& PortRanges()
        {
            static const std::vector<int> ranges =
            {
                80,   // Only for HTTP
                443,  // Only for HTTPS
                5846,
                5847,
                5848,
                5849,
                5850,
                5851,
                5852,
                5853,
                5854,
                5855,
                5856
            };
            return ranges;
        }

        /// Service classes
        /// Use ToString to get service class name strings
        enum class ServiceClass
        {
            CardReader,
            Publisher,
            Printer,
            TextTerminal,
            PinPad,
            Common,
            CashDispenser,
            CashManagement,
            Crypto,
            Keyboard,
            KeyManagement,
            Storage,
            Lights,
            Auxiliaries,
            VendorApplication,
            VendorMode,
            BarcodeReader,
            Biometric,
            Camera,
            CashAcceptor,
            Check,
            MixedMedia,
            Deposit,
            German,
            BanknoteNeutralization,
            PowerManagement
        };

		/// Common.ServiceVersion Версия самой программы, которая может быть возвращена в ответ на Common.Capabilities или в других местах, где это уместно. Это не версия XFS4IoT, а версия конкретной реализации сервиса.
        static inline std::string appVersionStr = SettingModule::GetInstance()->getAppVersion();
        static inline std::string_view ServiceVersion = appVersionStr;

        /// Maximum number of requests which can be queued by the Service
        /// 0 means unlimited
        static inline constexpr int MaximumRequests = 0;
    };
}
