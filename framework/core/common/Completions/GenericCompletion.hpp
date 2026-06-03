#pragma once

#include <memory>
#include <string>

#include "../../Completion.hpp"
#include "../../MessageBase.hpp"
#include "../../common/Completions/MessagePayload.hpp"

namespace XFS4IoT::Common::Completions
{
    class GenericCompletion final
        : public XFS4IoT::Completion<XFS4IoT::MessagePayloadBase>
    {
    public:
        GenericCompletion(
            std::string completionName,
            std::string version,
            int requestId,
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            std::string errorDescription)
            : XFS4IoT::Completion<XFS4IoT::MessagePayloadBase>(
                completionName,
                version,
                requestId,
                nullptr,
                completionCode,
                std::move(errorDescription))
            , completionName_(std::move(completionName))
            , version_(std::move(version))
        {
        }

        const std::string& GetCompletionName() const noexcept
        {
            return completionName_;
        }

        const std::string& GetVersion() const noexcept
        {
            return version_;
        }

    private:
        std::string completionName_;
        std::string version_;
    };
}