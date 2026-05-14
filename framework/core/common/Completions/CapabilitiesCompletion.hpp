#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../../Completion.hpp"
#include "../../common/MessageHeader.hpp"
#include "../../common/Completions/MessagePayload.hpp"
#include "../../CashAcceptor/CashAcceptorSchemas.hpp"
#include "../../CashManagement/CashManagementSchemas.hpp"
#include "../CommonSchemas.hpp"
// сюда подключи заголовки всех CapabilitiesClass / InterfaceClass / CapabilityPropertiesClass
// #include "../CommonSchemas.hpp"
// #include "../../CardReader/CardReaderSchemas.hpp"
// ...

namespace XFS4IoT::Common::Completions
{
    class CapabilitiesCompletionPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        CapabilitiesCompletionPayloadData(
            std::optional<std::vector<std::shared_ptr<InterfaceClass>>> interfaces = std::nullopt,
            std::shared_ptr<CapabilityPropertiesClass> common = nullptr,
            //std::shared_ptr<CardReader::CapabilitiesClass> cardReader = nullptr,
            std::shared_ptr<XFS4IoT::CashAcceptor::CapabilitiesClass> cashAcceptor = nullptr,
            //std::shared_ptr<CashDispenser::CapabilitiesClass> cashDispenser = nullptr,
            std::shared_ptr<XFS4IoT::CashManagement::CapabilitiesClass> cashManagement = nullptr
            //std::shared_ptr<Check::CapabilitiesClass> check = nullptr,
            //std::shared_ptr<MixedMedia::CapabilitiesClass> mixedMedia = nullptr,
            //std::shared_ptr<PinPad::CapabilitiesClass> pinPad = nullptr,
            //std::shared_ptr<Crypto::CapabilitiesClass> crypto = nullptr,
            //std::shared_ptr<KeyManagement::CapabilitiesClass> keyManagement = nullptr,
            //std::shared_ptr<Keyboard::CapabilitiesClass> keyboard = nullptr,
            //std::shared_ptr<TextTerminal::CapabilitiesClass> textTerminal = nullptr,
            //std::shared_ptr<Printer::CapabilitiesClass> printer = nullptr,
            //std::shared_ptr<BarcodeReader::CapabilitiesClass> barcodeReader = nullptr,
            //std::shared_ptr<Biometric::CapabilitiesClass> biometric = nullptr,
            //std::shared_ptr<Camera::CapabilitiesClass> camera = nullptr,
            //std::shared_ptr<German::CapabilitiesClass> german = nullptr,
            //std::shared_ptr<Lights::CapabilitiesClass> lights = nullptr,
            //std::shared_ptr<BanknoteNeutralization::CapabilitiesClass> banknoteNeutralization = nullptr,
            //std::shared_ptr<Auxiliaries::CapabilitiesClass> auxiliaries = nullptr,
            //std::shared_ptr<Deposit::CapabilitiesClass> deposit = nullptr,
            //std::shared_ptr<VendorApplication::CapabilitiesClass> vendorApplication = nullptr,
            //std::shared_ptr<PowerManagement::CapabilitiesClass> powerManagement = nullptr
            )
            : 
            interfaces_(std::move(interfaces))
            , common_(std::move(common))
            //, cardReader_(std::move(cardReader))
            , cashAcceptor_(std::move(cashAcceptor))
            //, cashDispenser_(std::move(cashDispenser))
            , cashManagement_(std::move(cashManagement))
            //, check_(std::move(check))
            //, mixedMedia_(std::move(mixedMedia))
            //, pinPad_(std::move(pinPad))
            //, crypto_(std::move(crypto))
            //, keyManagement_(std::move(keyManagement))
            //, keyboard_(std::move(keyboard))
            //, textTerminal_(std::move(textTerminal))
            //, printer_(std::move(printer))
            //, barcodeReader_(std::move(barcodeReader))
            //, biometric_(std::move(biometric))
            //, camera_(std::move(camera))
            //, german_(std::move(german))
            //, lights_(std::move(lights))
            //, banknoteNeutralization_(std::move(banknoteNeutralization))
            //, auxiliaries_(std::move(auxiliaries))
            //, deposit_(std::move(deposit))
            //, vendorApplication_(std::move(vendorApplication))
            //, powerManagement_(std::move(powerManagement))
        {
        }

        const std::optional<std::vector<std::shared_ptr<InterfaceClass>>>& GetInterfaces() const noexcept { return interfaces_; }
        const std::shared_ptr<CapabilityPropertiesClass>& GetCommon() const noexcept { return common_; }
        //const std::shared_ptr<CardReader::CapabilitiesClass>& GetCardReader() const noexcept { return cardReader_; }
        const std::shared_ptr<CashAcceptor::CapabilitiesClass>& GetCashAcceptor() const noexcept { return cashAcceptor_; }
        //const std::shared_ptr<CashDispenser::CapabilitiesClass>& GetCashDispenser() const noexcept { return cashDispenser_; }
        const std::shared_ptr<CashManagement::CapabilitiesClass>& GetCashManagement() const noexcept { return cashManagement_; }
        //const std::shared_ptr<Check::CapabilitiesClass>& GetCheck() const noexcept { return check_; }
        //const std::shared_ptr<MixedMedia::CapabilitiesClass>& GetMixedMedia() const noexcept { return mixedMedia_; }
        //const std::shared_ptr<PinPad::CapabilitiesClass>& GetPinPad() const noexcept { return pinPad_; }
        //const std::shared_ptr<Crypto::CapabilitiesClass>& GetCrypto() const noexcept { return crypto_; }
        //const std::shared_ptr<KeyManagement::CapabilitiesClass>& GetKeyManagement() const noexcept { return keyManagement_; }
        //const std::shared_ptr<Keyboard::CapabilitiesClass>& GetKeyboard() const noexcept { return keyboard_; }
        //const std::shared_ptr<TextTerminal::CapabilitiesClass>& GetTextTerminal() const noexcept { return textTerminal_; }
        //const std::shared_ptr<Printer::CapabilitiesClass>& GetPrinter() const noexcept { return printer_; }
        //const std::shared_ptr<BarcodeReader::CapabilitiesClass>& GetBarcodeReader() const noexcept { return barcodeReader_; }
        //const std::shared_ptr<Biometric::CapabilitiesClass>& GetBiometric() const noexcept { return biometric_; }
        //const std::shared_ptr<Camera::CapabilitiesClass>& GetCamera() const noexcept { return camera_; }
        //const std::shared_ptr<German::CapabilitiesClass>& GetGerman() const noexcept { return german_; }
        //const std::shared_ptr<Lights::CapabilitiesClass>& GetLights() const noexcept { return lights_; }
        //const std::shared_ptr<BanknoteNeutralization::CapabilitiesClass>& GetBanknoteNeutralization() const noexcept { return banknoteNeutralization_; }
        //const std::shared_ptr<Auxiliaries::CapabilitiesClass>& GetAuxiliaries() const noexcept { return auxiliaries_; }
        //const std::shared_ptr<Deposit::CapabilitiesClass>& GetDeposit() const noexcept { return deposit_; }
        //const std::shared_ptr<VendorApplication::CapabilitiesClass>& GetVendorApplication() const noexcept { return vendorApplication_; }
        //const std::shared_ptr<PowerManagement::CapabilitiesClass>& GetPowerManagement() const noexcept { return powerManagement_; }

    private:
        std::optional<std::vector<std::shared_ptr<InterfaceClass>>> interfaces_;
        std::shared_ptr<CapabilityPropertiesClass> common_;
        //std::shared_ptr<CardReader::CapabilitiesClass> cardReader_;
        std::shared_ptr<CashAcceptor::CapabilitiesClass> cashAcceptor_;
        //std::shared_ptr<CashDispenser::CapabilitiesClass> cashDispenser_;
        std::shared_ptr<CashManagement::CapabilitiesClass> cashManagement_;
        //std::shared_ptr<Check::CapabilitiesClass> check_;
        //std::shared_ptr<MixedMedia::CapabilitiesClass> mixedMedia_;
        //std::shared_ptr<PinPad::CapabilitiesClass> pinPad_;
        //std::shared_ptr<Crypto::CapabilitiesClass> crypto_;
        //std::shared_ptr<KeyManagement::CapabilitiesClass> keyManagement_;
        //std::shared_ptr<Keyboard::CapabilitiesClass> keyboard_;
        //std::shared_ptr<TextTerminal::CapabilitiesClass> textTerminal_;
        //std::shared_ptr<Printer::CapabilitiesClass> printer_;
        //std::shared_ptr<BarcodeReader::CapabilitiesClass> barcodeReader_;
        //std::shared_ptr<Biometric::CapabilitiesClass> biometric_;
        //std::shared_ptr<Camera::CapabilitiesClass> camera_;
        //std::shared_ptr<German::CapabilitiesClass> german_;
        //std::shared_ptr<Lights::CapabilitiesClass> lights_;
        //std::shared_ptr<BanknoteNeutralization::CapabilitiesClass> banknoteNeutralization_;
        //std::shared_ptr<Auxiliaries::CapabilitiesClass> auxiliaries_;
        //std::shared_ptr<Deposit::CapabilitiesClass> deposit_;
        //std::shared_ptr<VendorApplication::CapabilitiesClass> vendorApplication_;
        //std::shared_ptr<PowerManagement::CapabilitiesClass> powerManagement_;
    };

    inline void to_json(
        nlohmann::json& j,
        const CapabilitiesCompletionPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetInterfaces().has_value())
        {
            const auto& ifs = p.GetInterfaces().value();
            nlohmann::json jifs = nlohmann::json::array();
            for (const auto& ifacePtr : ifs)
            {
                if (ifacePtr)
                    jifs.push_back(*ifacePtr); // relies on to_json for InterfaceClass
                else
                    jifs.push_back(nullptr);
            }
            j["interfaces"] = std::move(jifs);
        }

        if (p.GetCommon())
            j["common"] = *p.GetCommon();

        //if (p.GetCardReader())
        //    j["cardReader"] = *p.GetCardReader();

        if (p.GetCashAcceptor())
            j["cashAcceptor"] = *p.GetCashAcceptor();

        //if (p.GetCashDispenser())
        //    j["cashDispenser"] = *p.GetCashDispenser();

        if (p.GetCashManagement())
            j["cashManagement"] = *p.GetCashManagement();

        //if (p.GetCheck())
        //    j["check"] = *p.GetCheck();

        //if (p.GetMixedMedia())
        //    j["mixedMedia"] = *p.GetMixedMedia();

        //if (p.GetPinPad())
        //    j["pinPad"] = *p.GetPinPad();

        //if (p.GetCrypto())
        //    j["crypto"] = *p.GetCrypto();

        //if (p.GetKeyManagement())
        //    j["keyManagement"] = *p.GetKeyManagement();

        //if (p.GetKeyboard())
        //    j["keyboard"] = *p.GetKeyboard();

        //if (p.GetTextTerminal())
        //    j["textTerminal"] = *p.GetTextTerminal();

        //if (p.GetPrinter())
        //    j["printer"] = *p.GetPrinter();

        //if (p.GetBarcodeReader())
        //    j["barcodeReader"] = *p.GetBarcodeReader();

        //if (p.GetBiometric())
        //    j["biometric"] = *p.GetBiometric();

        //if (p.GetCamera())
        //    j["camera"] = *p.GetCamera();

        //if (p.GetGerman())
        //    j["german"] = *p.GetGerman();

        //if (p.GetLights())
        //    j["lights"] = *p.GetLights();

        //if (p.GetBanknoteNeutralization())
        //    j["banknoteNeutralization"] = *p.GetBanknoteNeutralization();

        //if (p.GetAuxiliaries())
        //    j["auxiliaries"] = *p.GetAuxiliaries();

        //if (p.GetDeposit())
        //    j["deposit"] = *p.GetDeposit();

        //if (p.GetVendorApplication())
        //    j["vendorApplication"] = *p.GetVendorApplication();

        //if (p.GetPowerManagement())
        //    j["powerManagement"] = *p.GetPowerManagement();
    }

    class CapabilitiesCompletion final
        : public XFS4IoT::Completion<CapabilitiesCompletionPayloadData>
    {
    public:
        static constexpr const char* CompletionName = "Common.Capabilities";
        static constexpr const char* Version = "1.0";

        CapabilitiesCompletion(
            int requestId,
            std::shared_ptr<CapabilitiesCompletionPayloadData> payload,
            MessageHeader::CompletionCodeEnum completionCode,
            const std::string& errorDescription)
            : XFS4IoT::Completion<CapabilitiesCompletionPayloadData>(
                CompletionName,
                Version,
                requestId,
                std::move(payload),
                completionCode,
                errorDescription)
        {
        }

    private:
        static bool registered_;
    };

    inline bool CapabilitiesCompletion::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(CapabilitiesCompletion),
                CapabilitiesCompletion::CompletionName,
                CapabilitiesCompletion::Version);
            return true;
        }();
}