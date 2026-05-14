//#pragma once
//
//#include <memory>
//#include <optional>
//#include <string>
//#include <typeinfo>
//#include "MessageBase.hpp"
//#include "common/MessageHeader.hpp"
//
//namespace XFS4IoT
//{
//    // Payload base
//    struct MessagePayloadBase
//    {
//        MessagePayloadBase() = default;
//    };
//
//    // Template Message class
//    template<typename T>
//        requires std::is_base_of<MessagePayloadBase, T>::value
//    class Message : public MessageBase
//    {
//    public:
//        std::shared_ptr<T> Payload;
//
//    protected:
//        // Internal constructor for response
//        Message(std::shared_ptr<MessageHeader> header, std::shared_ptr<T> payload)
//            : MessageBase(*header)
//            , Payload(payload)
//        {
//        }
//
//    public:
//        // Constructor for commands
//        Message(std::optional<int> requestId,
//            MessageHeader::TypeEnum type,
//            std::shared_ptr<T> payload,
//            std::optional<int> timeout)
//            : MessageBase(
//                /* name */    MessageBase::LookupMessageName(typeid(T)),
//                /* requestId */ requestId.has_value() ? requestId.value() : 0,
//                /* version */ MessageBase::LookupMessageVersion(typeid(T)),
//                /* type */    type,
//                /* timeout */ timeout,
//                /* status */  std::nullopt,
//                /* completionCode */ std::nullopt,
//                /* errorDescription */ std::nullopt)
//            , Payload(payload)
//        {
//        }
//
//        // Constructor for completion messages
//        Message(std::optional<int> requestId,
//            MessageHeader::TypeEnum type,
//            std::shared_ptr<T> payload,
//            std::optional<MessageHeader::CompletionCodeEnum> completionCode,
//            const std::string& errorDescription)
//            : MessageBase(
//                /* name */    MessageBase::LookupMessageName(typeid(T)),
//                /* requestId */ requestId.has_value() ? requestId.value() : 0,
//                /* version */ MessageBase::LookupMessageVersion(typeid(T)),
//                /* type */    type,
//                /* timeout */ std::nullopt,
//                /* status */  std::nullopt,
//                /* completionCode */ completionCode,
//                /* errorDescription */ std::make_optional(errorDescription))
//            , Payload(payload)
//        {
//        }
//    };
//}

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