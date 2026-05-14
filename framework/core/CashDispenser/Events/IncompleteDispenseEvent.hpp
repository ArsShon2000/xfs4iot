//#pragma once
//
//#include <memory>
//#include <optional>
//#include <string>
//#include <unordered_map>
//
//#include "../../Events.hpp"
//#include "../../Message.hpp"
//#include "../../common/MessageHeader.hpp"
//#include "../CashDispenserSchemas.hpp"
//
//namespace XFS4IoT::CashDispenser::Events
//{
//    class IncompleteDispenseEventPayloadData final : public XFS4IoT::MessagePayloadBase
//    {
//    public:
//        IncompleteDispenseEventPayloadData(
//            std::optional<std::unordered_map<std::string, double>> currencies = std::nullopt,
//            std::optional<std::unordered_map<std::string, int>> values = std::nullopt,
//            std::shared_ptr<DenominationCashBoxClass> cashBox = nullptr)
//            : currencies_(std::move(currencies))
//            , values_(std::move(values))
//            , cashBox_(std::move(cashBox))
//        {
//        }
//
//        const std::optional<std::unordered_map<std::string, double>>& GetCurrencies() const noexcept
//        {
//            return currencies_;
//        }
//
//        void SetCurrencies(std::optional<std::unordered_map<std::string, double>> currencies)
//        {
//            currencies_ = std::move(currencies);
//        }
//
//        const std::optional<std::unordered_map<std::string, int>>& GetValues() const noexcept
//        {
//            return values_;
//        }
//
//        void SetValues(std::optional<std::unordered_map<std::string, int>> values)
//        {
//            values_ = std::move(values);
//        }
//
//        const std::shared_ptr<DenominationCashBoxClass>& GetCashBox() const noexcept
//        {
//            return cashBox_;
//        }
//
//        void SetCashBox(std::shared_ptr<DenominationCashBoxClass> cashBox)
//        {
//            cashBox_ = std::move(cashBox);
//        }
//
//    private:
//        std::optional<std::unordered_map<std::string, double>> currencies_;
//        std::optional<std::unordered_map<std::string, int>> values_;
//        std::shared_ptr<DenominationCashBoxClass> cashBox_;
//    };
//
//    inline void to_json(
//        nlohmann::json& j,
//        const IncompleteDispenseEventPayloadData& p)
//    {
//        j = nlohmann::json::object();
//
//        if (p.GetCurrencies().has_value())
//        {
//            j["currencies"] = nlohmann::json::object();
//            for (const auto& [key, value] : p.GetCurrencies().value())
//            {
//                j["currencies"][key] = value;
//            }
//        }
//
//        if (p.GetValues().has_value())
//        {
//            j["values"] = nlohmann::json::object();
//            for (const auto& [key, value] : p.GetValues().value())
//            {
//                j["values"][key] = value;
//            }
//        }
//
//        if (p.GetCashBox())
//        {
//            j["cashBox"] = *p.GetCashBox();
//        }
//    }
//
//    class IncompleteDispenseEvent final
//        : public XFS4IoT::Events::Event<IncompleteDispenseEventPayloadData>
//    {
//    public:
//        static constexpr const char* EventName = "CashDispenser.IncompleteDispenseEvent";
//        static constexpr const char* Version = "1.0";
//
//        IncompleteDispenseEvent(
//            int requestId,
//            std::shared_ptr<IncompleteDispenseEventPayloadData> payload)
//            : XFS4IoT::Events::Event<IncompleteDispenseEventPayloadData>(
//                EventName,
//                Version,
//                requestId,
//                std::move(payload))
//        {
//        }
//
//    private:
//        static bool registered_;
//    };
//
//    inline bool IncompleteDispenseEvent::registered_ = []()
//        {
//            XFS4IoT::MessageBase::RegisterMessage(
//                typeid(IncompleteDispenseEvent),
//                IncompleteDispenseEvent::EventName,
//                IncompleteDispenseEvent::Version);
//            return true;
//        }();
//}