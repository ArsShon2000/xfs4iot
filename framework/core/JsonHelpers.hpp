#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <memory>
#include <string>

namespace XFS4IoT::JsonHelpers
{
    template<typename T>
    inline void PutOptional(nlohmann::json& j, const char* key, const std::optional<T>& value)
    {
        if (value.has_value())
            j[key] = value.value();
    }

    template<typename T>
    inline void PutPtr(nlohmann::json& j, const char* key, const std::shared_ptr<T>& value)
    {
        if (value)
            j[key] = *value;
    }

    template<typename T>
    inline void PutValue(nlohmann::json& j, const char* key, const T& value)
    {
        j[key] = value;
    }
}


// using 
// например в классе CapabilitiesCompletionPayloadData

// нужно создать функцию 
// inline void to_json(
//nlohmann::json& j,
//const CapabilitiesCompletionPayloadData& p)
//{
//    using namespace XFS4IoT::JsonHelpers;
//
//    j = nlohmann::json::object();
//
//    PutOptional(j, "interfaces", p.GetInterfaces());
//
//    PutPtr(j, "common", p.GetCommon());
//    PutPtr(j, "cardReader", p.GetCardReader());
//    PutPtr(j, "cashAcceptor", p.GetCashAcceptor());
//    PutPtr(j, "cashDispenser", p.GetCashDispenser());
//    PutPtr(j, "cashManagement", p.GetCashManagement());
//    PutPtr(j, "check", p.GetCheck());
//    PutPtr(j, "mixedMedia", p.GetMixedMedia());
//    PutPtr(j, "pinPad", p.GetPinPad());
//    PutPtr(j, "crypto", p.GetCrypto());
//    PutPtr(j, "keyManagement", p.GetKeyManagement());
//    PutPtr(j, "keyboard", p.GetKeyboard());
//    PutPtr(j, "textTerminal", p.GetTextTerminal());
//    PutPtr(j, "printer", p.GetPrinter());
//    PutPtr(j, "barcodeReader", p.GetBarcodeReader());
//    PutPtr(j, "biometric", p.GetBiometric());
//    PutPtr(j, "camera", p.GetCamera());
//    PutPtr(j, "german", p.GetGerman());
//    PutPtr(j, "lights", p.GetLights());
//    PutPtr(j, "banknoteNeutralization", p.GetBanknoteNeutralization());
//    PutPtr(j, "auxiliaries", p.GetAuxiliaries());
//    PutPtr(j, "deposit", p.GetDeposit());
//    PutPtr(j, "vendorApplication", p.GetVendorApplication());
//    PutPtr(j, "powerManagement", p.GetPowerManagement());
//}