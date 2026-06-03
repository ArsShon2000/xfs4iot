#pragma once

#include <memory>
#include <string>
#include <optional>
#include <ranges>
#include <algorithm>

#include "IConnection.hpp"
#include "ClientConnection.hpp"
#include "IServiceProvider.hpp"

namespace XFS4IoTServer
{
    class VersionNegotiationHelper final
    {
    public:
        static std::string ResolveMessageVersion(
            const std::shared_ptr<IConnection>& connection,
            const std::shared_ptr<IServiceProvider>& provider,
            const std::string& messageName)
        {
            if (auto client = std::dynamic_pointer_cast<ClientConnection>(connection))
            {
                if (auto negotiated = client->GetNegotiatedVersion(messageName))
                {
                    return *negotiated;
                }
            }

            const auto supported = provider->GetMessagesSupported();
            auto it = supported.find(messageName);

            if (it == supported.end() || it->second.Versions.empty())
            {
                throw std::runtime_error(
                    "Не найдены поддерживаемые версии для сообщения: " + messageName);
            }

            return *std::min_element(
                it->second.Versions.begin(),
                it->second.Versions.end(),
                [](const std::string& a, const std::string& b)
                {
                    return MajorVersion(a) < MajorVersion(b);
                });
        }

        static int MajorVersion(const std::string& version)
        {
            auto dot = version.find('.');
            return std::stoi(version.substr(0, dot));
        }
    };
}