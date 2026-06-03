#pragma once

#include <memory>
#include <string>
#include "MessageBase.hpp"
#include "Message.hpp"
#include "common/MessageHeader.hpp"

namespace XFS4IoT::Commands
{
    template<typename T>
        requires std::is_base_of_v<XFS4IoT::MessagePayloadBase, T>
    class Command : public XFS4IoT::Message<T>
    {
    public:
        Command(const std::string& name,
            const std::string& version,
            int requestId,
            std::shared_ptr<T> payload,
            int timeout)
            : XFS4IoT::Message<T>(
                name,
                version,
                requestId,
                XFS4IoT::MessageHeader::TypeEnum::Command,
                std::move(payload),
                timeout)
        {
        }

        Command()
            : XFS4IoT::Message<T>(
                "",                                          // name
                "",                                          // version
                0,                                           // requestId
                XFS4IoT::MessageHeader::TypeEnum::Command,   // type
                nullptr,                                     // shared_ptr<T> = nullptr (не nullopt!)
                0)                                           // timeout
        {
        }

        virtual ~Command() = default;
    };
}