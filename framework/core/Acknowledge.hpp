#pragma once

#include <memory>
#include <optional>
#include <string>
#include "Message.hpp"

namespace XFS4IoT
{
    class Acknowledge final : public Message<MessagePayloadBase>
    {
    public:
        Acknowledge(
            int requestId,
            const std::string& commandName,
            const std::string& version,
            std::optional<MessageHeader::StatusEnum> status)
            : Message<MessagePayloadBase>(
                std::make_shared<MessageHeader>(
                    commandName,
                    requestId,
                    version,
                    MessageHeader::TypeEnum::Acknowledge,
                    std::nullopt,
                    status,
                    std::nullopt,
                    std::nullopt),
                nullptr)
        {
        }
    };
}

