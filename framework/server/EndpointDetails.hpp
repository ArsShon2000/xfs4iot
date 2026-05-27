
#pragma once

#include <string>
#include <stdexcept>
#include <format>
#include <algorithm>
#include <cctype>

namespace XFS4IoTServer
{
    class EndpointDetails
    {
    public:
        EndpointDetails(std::string serverAddressUri,
            std::string serverAddressWUri,
            int port)
            : serverAddressUri_(std::move(serverAddressUri))
            , serverAddressWUri_(std::move(serverAddressWUri))
            , port_(port)
        {
            if (serverAddressUri_.empty() ||
                std::all_of(serverAddressUri_.begin(), serverAddressUri_.end(), ::isspace)) {
                throw std::invalid_argument(
                    std::format("Указан неожиданный адрес сервера: {}", "EndpointDetails"));
            }

            if (serverAddressWUri_.empty() ||
                std::all_of(serverAddressWUri_.begin(), serverAddressWUri_.end(), ::isspace)) {
                throw std::invalid_argument(
                    std::format("Указан неожиданный адрес сервера для веб-сокетов: {}",
                        "EndpointDetails"));
            }

			// Определяем, является ли соединение безопасным, проверяя, начинается ли URI с "http://"
            std::string lowerUri = ToLower(serverAddressUri_);
            secureConnection_ = !lowerUri.starts_with("http://");
        }

        int GetPort() const { return port_; }

        bool IsSecureConnection() const { return secureConnection_; }

        struct ServiceUriPair
        {
            std::string uri;
            std::string wsUri;
        };

        ServiceUriPair GetServiceUri(const std::string& serviceName) const
        {
            return ServiceUriPair{
                .uri = std::format("{}:{}/xfs4iot/v1.0/{}/",
                                  serverAddressUri_, port_, serviceName),
                .wsUri = std::format("{}:{}/xfs4iot/v1.0/{}/",
                                    serverAddressWUri_, port_, serviceName)
            };
        }

    private:
        static std::string ToLower(const std::string& str)
        {
            std::string result = str;
            std::ranges::transform(result, result.begin(),
                [](unsigned char c) { return std::tolower(c); });
            return result;
        }

        std::string serverAddressUri_;
        std::string serverAddressWUri_;
        int port_;
        bool secureConnection_;
    };
}