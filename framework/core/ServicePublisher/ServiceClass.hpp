#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace XFS4IoT::ServicePublisher
{
    class ServiceClass final
    {
    public:
        ServiceClass() = default;
        explicit ServiceClass(std::string serviceURI)
            : serviceURI_(std::move(serviceURI))
        {
        }

        const std::string& GetServiceURI() const noexcept
        {
            return serviceURI_;
        }

    private:
        std::string serviceURI_;

        friend void to_json(nlohmann::json& j, const ServiceClass& s)
        {
            j = nlohmann::json{
                {"serviceURI", s.serviceURI_}
            };
        }
    };
}