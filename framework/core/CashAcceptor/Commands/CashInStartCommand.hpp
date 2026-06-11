#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "../../Command.hpp"
#include "../../MessageBase.hpp"
#include "../../CashManagement/CashManagementSchemas.hpp"

namespace XFS4IoT::CashAcceptor::Commands
{
    class CashInStartCommandPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        class AmountLimitClass final
        {
        public:
            AmountLimitClass(
                std::optional<std::string> currency = std::nullopt,
                std::optional<double> value = std::nullopt)
                : currency_(std::move(currency))
                , value_(value)
            {
            }

            const std::optional<std::string>& GetCurrency() const noexcept
            {
                return currency_;
            }

            void SetCurrency(std::optional<std::string> currency)
            {
                currency_ = std::move(currency);
            }

            const std::optional<double>& GetValue() const noexcept
            {
                return value_;
            }

            void SetValue(std::optional<double> value)
            {
                value_ = value;
            }

        private:
			std::optional<std::string> currency_; // Валюта в ISO 4217 формате
			std::optional<double> value_;           // Лимит суммы в указанных единицах валюты
        };

        CashInStartCommandPayloadData(
            std::optional<int> tellerID = std::nullopt,
            std::optional<bool> useRecycleUnits = std::nullopt,
            std::optional<XFS4IoT::CashManagement::OutputPositionEnum> outputPosition = std::nullopt,
            std::optional<XFS4IoT::CashManagement::InputPositionEnum> inputPosition = std::nullopt,
            std::optional<int> totalItemsLimit = std::nullopt,
            std::optional<std::vector<AmountLimitClass>> amountLimit = std::nullopt)
            : tellerID_(tellerID)
            , useRecycleUnits_(useRecycleUnits)
            , outputPosition_(outputPosition)
            , inputPosition_(inputPosition)
            , totalItemsLimit_(totalItemsLimit)
            , amountLimit_(std::move(amountLimit))
        {
        }

        const std::optional<int>& GetTellerID() const noexcept
        {
            return tellerID_;
        }

        void SetTellerID(std::optional<int> tellerID)
        {
            tellerID_ = tellerID;
        }

        const std::optional<bool>& GetUseRecycleUnits() const noexcept
        {
            return useRecycleUnits_;
        }

        void SetUseRecycleUnits(std::optional<bool> useRecycleUnits)
        {
            useRecycleUnits_ = useRecycleUnits;
        }

        const std::optional<XFS4IoT::CashManagement::OutputPositionEnum>& GetOutputPosition() const noexcept
        {
            return outputPosition_;
        }

        void SetOutputPosition(std::optional<XFS4IoT::CashManagement::OutputPositionEnum> outputPosition)
        {
            outputPosition_ = outputPosition;
        }

        const std::optional<XFS4IoT::CashManagement::InputPositionEnum>& GetInputPosition() const noexcept
        {
            return inputPosition_;
        }

        void SetInputPosition(std::optional<XFS4IoT::CashManagement::InputPositionEnum> inputPosition)
        {
            inputPosition_ = inputPosition;
        }

        const std::optional<int>& GetTotalItemsLimit() const noexcept
        {
            return totalItemsLimit_;
        }

        void SetTotalItemsLimit(std::optional<int> totalItemsLimit)
        {
            totalItemsLimit_ = totalItemsLimit;
        }

        const std::optional<std::vector<AmountLimitClass>>& GetAmountLimit() const noexcept
        {
            return amountLimit_;
        }

        void SetAmountLimit(std::optional<std::vector<AmountLimitClass>> amountLimit)
        {
            amountLimit_ = std::move(amountLimit);
        }

    private:
        std::optional<int> tellerID_;
        std::optional<bool> useRecycleUnits_;
        std::optional<XFS4IoT::CashManagement::OutputPositionEnum> outputPosition_;
        std::optional<XFS4IoT::CashManagement::InputPositionEnum> inputPosition_;
        std::optional<int> totalItemsLimit_;
        std::optional<std::vector<AmountLimitClass>> amountLimit_;
    };

    inline void to_json(
        nlohmann::json& j,
        const CashInStartCommandPayloadData::AmountLimitClass& p)
    {
        j = nlohmann::json::object();

        if (p.GetCurrency().has_value())
        {
            j["currency"] = p.GetCurrency().value();
        }

        if (p.GetValue().has_value())
        {
            j["value"] = p.GetValue().value();
        }
    }

    inline void from_json(
        const nlohmann::json& j,
        CashInStartCommandPayloadData::AmountLimitClass& p)
    {
        std::optional<std::string> currency = std::nullopt;
        std::optional<double> value = std::nullopt;

        if (j.contains("currency") && !j.at("currency").is_null())
        {
            currency = j.at("currency").get<std::string>();
        }

        if (j.contains("value") && !j.at("value").is_null())
        {
            value = j.at("value").get<double>();
        }

        p = CashInStartCommandPayloadData::AmountLimitClass(
            std::move(currency),
            value);
    }

    inline void to_json(
        nlohmann::json& j,
        const CashInStartCommandPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetTellerID().has_value())
        {
            j["tellerID"] = p.GetTellerID().value();
        }

        if (p.GetUseRecycleUnits().has_value())
        {
            j["useRecycleUnits"] = p.GetUseRecycleUnits().value();
        }

        if (p.GetOutputPosition().has_value())
        {
            j["outputPosition"] = p.GetOutputPosition().value();
        }

        if (p.GetInputPosition().has_value())
        {
            j["inputPosition"] = p.GetInputPosition().value();
        }

        if (p.GetTotalItemsLimit().has_value())
        {
            j["totalItemsLimit"] = p.GetTotalItemsLimit().value();
        }

        if (p.GetAmountLimit().has_value())
        {
            j["amountLimit"] = p.GetAmountLimit().value();
        }
    }

    class CashInStartCommand final
        : public XFS4IoT::Commands::Command<CashInStartCommandPayloadData>
    {
    public:
        static constexpr const char* CommandName = "CashAcceptor.CashInStart";
        static constexpr const char* Version = "1.0";

        CashInStartCommand(
            int requestId,
            std::shared_ptr<CashInStartCommandPayloadData> payload,
            int timeout)
            : XFS4IoT::Commands::Command<CashInStartCommandPayloadData>(
                CommandName,
                Version,
                requestId,
                std::move(payload),
                timeout)
        {
        }

        static std::shared_ptr<XFS4IoT::MessageBase> FromJson(const nlohmann::json& j)
        {
            if (!j.contains("header"))
            {
                return nullptr;
            }

            const auto& h = j.at("header");
            const int requestId = h.value("requestId", 0);
            const int timeout = h.value("timeout", 0);

            auto payload = std::make_shared<CashInStartCommandPayloadData>();

            if (j.contains("payload") && j.at("payload").is_object())
            {
                const auto& p = j.at("payload");

                if (p.contains("tellerID") && !p.at("tellerID").is_null())
                {
                    payload->SetTellerID(p.at("tellerID").get<int>());
                }

                if (p.contains("useRecycleUnits") && !p.at("useRecycleUnits").is_null())
                {
                    payload->SetUseRecycleUnits(p.at("useRecycleUnits").get<bool>());
                }

                if (p.contains("outputPosition") && !p.at("outputPosition").is_null())
                {
					std::string outputPositionStr = p.at("outputPosition").get<std::string>();
                    auto outputPosition = XFS4IoT::CashManagement::fromStringToOutputPositionEnum(outputPositionStr);
					payload->SetOutputPosition(outputPosition);
                }

                if (p.contains("inputPosition") && !p.at("inputPosition").is_null())
                {
					std::string inputPositionStr = p.at("inputPosition").get<std::string>();
                    auto inputPosition = XFS4IoT::CashManagement::fromStringToInputPositionEnum(inputPositionStr);
					payload->SetInputPosition(inputPosition);
                }

                if (p.contains("totalItemsLimit") && !p.at("totalItemsLimit").is_null())
                {
                    payload->SetTotalItemsLimit(p.at("totalItemsLimit").get<int>());
                }

                if (p.contains("amountLimit") && p.at("amountLimit").is_array())
                {
                    std::vector<CashInStartCommandPayloadData::AmountLimitClass> amountLimit;

                    for (const auto& itemJson : p.at("amountLimit"))
                    {
                        amountLimit.emplace_back(
                            itemJson.get<CashInStartCommandPayloadData::AmountLimitClass>());
                    }

                    payload->SetAmountLimit(std::move(amountLimit));
                }
            }

            return std::make_shared<CashInStartCommand>(
                requestId,
                payload,
                timeout);
        }

    private:
        static bool registered_;
    };

    inline bool CashInStartCommand::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(CashInStartCommand),
                CashInStartCommand::CommandName,
                CashInStartCommand::Version);
            return true;
        }();
}