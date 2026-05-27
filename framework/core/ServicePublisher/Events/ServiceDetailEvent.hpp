#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "../../Events.hpp"
#include "../../MessageBase.hpp"
#include "../ServiceClass.hpp"

namespace XFS4IoT::ServicePublisher::Events
{
    class ServiceDetailEventPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        ServiceDetailEventPayloadData(
            std::optional<std::string> vendorName = std::nullopt,
            std::optional<std::vector<XFS4IoT::ServicePublisher::ServiceClass>> services = std::nullopt)
            : vendorName_(std::move(vendorName))
            , services_(std::move(services))
        {
        }

        const std::optional<std::string>& GetVendorName() const noexcept
        {
            return vendorName_;
        }

        const std::optional<std::vector<XFS4IoT::ServicePublisher::ServiceClass>>& GetServices() const noexcept
        {
            return services_;
        }

    private:
        std::optional<std::string> vendorName_;
        std::optional<std::vector<XFS4IoT::ServicePublisher::ServiceClass>> services_;
    };

    inline void to_json(nlohmann::json& j, const ServiceDetailEventPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetVendorName().has_value())
        {
            j["vendorName"] = p.GetVendorName().value();
        }

        if (p.GetServices().has_value())
        {
            j["services"] = p.GetServices().value();
        }
    }

    class ServiceDetailEvent final
        : public XFS4IoT::Events::Event<ServiceDetailEventPayloadData>
    {
    public:
        static constexpr const char* EventName = "ServicePublisher.ServiceDetailEvent";
        static constexpr const char* Version = "1.0";

        ServiceDetailEvent(
            int requestId,
            std::shared_ptr<ServiceDetailEventPayloadData> payload)
            : XFS4IoT::Events::Event<ServiceDetailEventPayloadData>(
                EventName,
                Version,
                requestId,
                std::move(payload))
        {
        }

    private:
        static bool registered_;
    };

    inline bool ServiceDetailEvent::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(ServiceDetailEvent),
                ServiceDetailEvent::EventName,
                ServiceDetailEvent::Version);
            return true;
        }();
}