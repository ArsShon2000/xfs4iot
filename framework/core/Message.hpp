
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include "MessageBase.hpp"
#include "common/MessageHeader.hpp"

namespace XFS4IoT
{
    struct MessagePayloadBase
    {
        MessagePayloadBase() = default;
        virtual ~MessagePayloadBase() = default;


        friend void to_json(nlohmann::json& j, const MessagePayloadBase&)
        {
            j = nlohmann::json::object();
        }
    };

    template<typename T>
        requires std::is_base_of_v<MessagePayloadBase, T>
    class Message : public MessageBase
    {
    public:
        std::shared_ptr<T> Payload;

    protected:
        Message(std::shared_ptr<MessageHeader> header, std::shared_ptr<T> payload)
            : MessageBase(*header)
            , Payload(std::move(payload))
        {
        }

        Message(const std::string& name,
            const std::string& version,
            std::optional<int> requestId,
            MessageHeader::TypeEnum type,
            std::shared_ptr<T> payload,
            std::optional<int> timeout)
            : MessageBase(
                name,
                requestId,
                version,
                type,
                timeout,
                std::nullopt,
                std::nullopt,
                std::nullopt)
            , Payload(std::move(payload))
        {
        }

        Message(const std::string& name,
            const std::string& version,
            std::optional<int> requestId,
            MessageHeader::TypeEnum type,
            std::shared_ptr<T> payload,
            MessageHeader::CompletionCodeEnum completionCode,
            const std::string& errorDescription)
            : MessageBase(
                name,
                requestId,
                version,
                type,
                std::nullopt,
                std::nullopt,
                completionCode,
                errorDescription)
            , Payload(std::move(payload))
        {
        }

        void SerializePayload(nlohmann::json& j) const override
        {
            if (!Payload) {
                j["payload"] = nullptr;
            }
            else {
                j["payload"] = *Payload;
            }
        }
    };
}