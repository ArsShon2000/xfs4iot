#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "../../Completion.hpp"
#include "../../common/Completions/MessagePayload.hpp"
#include "../ServiceClass.hpp"
#include "../../MessageBase.hpp"

namespace XFS4IoT::ServicePublisher::Completions
{
        class PayloadDataGetServicesCompletion final : public XFS4IoT::MessagePayloadBase
        {
        public:
            PayloadDataGetServicesCompletion(std::optional<std::string> vendorName = std::nullopt,
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

            friend void to_json(nlohmann::json& j, const PayloadDataGetServicesCompletion& p)
            {
                j = nlohmann::json::object();

                if (p.vendorName_.has_value()) {
                    j["vendorName"] = p.vendorName_.value();
                }
                if (p.services_.has_value()) {
                    j["services"] = p.services_.value();
                }
            }
        };

    class GetServicesCompletion final
        : public XFS4IoT::Completion<PayloadDataGetServicesCompletion>
    {

    public:
        static constexpr const char* CompletionName = "ServicePublisher.GetServices";
        static constexpr const char* Version = "1.0";

        GetServicesCompletion(int requestId,
            std::shared_ptr<PayloadDataGetServicesCompletion> payload,
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            const std::string& errorDescription)
            : XFS4IoT::Completion<PayloadDataGetServicesCompletion>(
                CompletionName,
                Version,
                requestId,
                std::move(payload),
                completionCode,
                errorDescription)
        {
        }

    private:
        static bool registered_ ;
    };

    inline bool GetServicesCompletion::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(GetServicesCompletion),
                GetServicesCompletion::CompletionName,
                GetServicesCompletion::Version);
            return true;
        }();
}