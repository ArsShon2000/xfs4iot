#pragma once

#include <memory>
#include <optional>
#include <string>

#include "../../Events.hpp"
#include "../../Message.hpp"
#include "../../common/MessageHeader.hpp"

#include "../../common/CommonSchemas.hpp"
#include "../../CashAcceptor/CashAcceptorSchemas.hpp"

namespace XFS4IoT::Common::Events
{
    class StatusChangedEventPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        StatusChangedEventPayloadData(
            std::shared_ptr<XFS4IoT::Common::StatusPropertiesChangedClass> common = nullptr,
            std::shared_ptr<XFS4IoT::CashAcceptor::StatusClass> cashAcceptor = nullptr,
            //std::shared_ptr<XFS4IoT::CashDispenser::StatusClass> cashDispenser = nullptr,
            std::shared_ptr<XFS4IoT::CashManagement::StatusClass> cashManagement = nullptr)
            : common_(std::move(common))
            , cashAcceptor_(std::move(cashAcceptor))
            //, cashDispenser_(std::move(cashDispenser))
            , cashManagement_(std::move(cashManagement))
        {
        }

        const std::shared_ptr<XFS4IoT::Common::StatusPropertiesChangedClass>&
            GetCommon() const noexcept
        {
            return common_;
        }

        void SetCommon(std::shared_ptr<XFS4IoT::Common::StatusPropertiesChangedClass> value)
        {
            common_ = std::move(value);
        }

        const std::shared_ptr<XFS4IoT::CashAcceptor::StatusClass>&
            GetCashAcceptor() const noexcept
        {
            return cashAcceptor_;
        }

        void SetCashAcceptor(std::shared_ptr<XFS4IoT::CashAcceptor::StatusClass> value)
        {
            cashAcceptor_ = std::move(value);
        }

        //const std::shared_ptr<XFS4IoT::CashDispenser::StatusClass>&
        //    GetCashDispenser() const noexcept
        //{
        //    return cashDispenser_;
        //}

        //void SetCashDispenser(std::shared_ptr<XFS4IoT::CashDispenser::StatusClass> value)
        //{
        //    cashDispenser_ = std::move(value);
        //}

        const std::shared_ptr<XFS4IoT::CashManagement::StatusClass>&
            GetCashManagement() const noexcept
        {
            return cashManagement_;
        }

        void SetCashManagement(std::shared_ptr<XFS4IoT::CashManagement::StatusClass> value)
        {
            cashManagement_ = std::move(value);
        }

    private:
        std::shared_ptr<XFS4IoT::Common::StatusPropertiesChangedClass> common_;
        std::shared_ptr<XFS4IoT::CashAcceptor::StatusClass> cashAcceptor_;
        //std::shared_ptr<XFS4IoT::CashDispenser::StatusClass> cashDispenser_;
        std::shared_ptr<XFS4IoT::CashManagement::StatusClass> cashManagement_;
    };

    inline void to_json(nlohmann::json& j, const StatusChangedEventPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetCommon())
        {
            j["common"] = *p.GetCommon();
        }

        if (p.GetCashAcceptor())
        {
            j["cashAcceptor"] = *p.GetCashAcceptor();
        }

        //if (p.GetCashDispenser())
        //{
        //    j["cashDispenser"] = *p.GetCashDispenser();
        //}

        if (p.GetCashManagement())
        {
            j["cashManagement"] = *p.GetCashManagement();
        }
    }

    class StatusChangedEvent final
        : public XFS4IoT::Events::UnsolicitedEvent<StatusChangedEventPayloadData>
    {
    public:
        static constexpr const char* EventName = "Common.StatusChangedEvent";
        static constexpr const char* Version = "1.0";

        explicit StatusChangedEvent(
            std::shared_ptr<StatusChangedEventPayloadData> payload)
            : XFS4IoT::Events::UnsolicitedEvent<StatusChangedEventPayloadData>(
                EventName,
                Version,
                std::move(payload))
        {
        }

    private:
        static bool registered_;
    };

    inline bool StatusChangedEvent::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(StatusChangedEvent),
                StatusChangedEvent::EventName,
                StatusChangedEvent::Version);
            return true;
        }();
}