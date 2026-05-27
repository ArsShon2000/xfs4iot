#pragma once

#include <memory>
#include <string>

#include "../../Completion.hpp"
#include "../../MessageBase.hpp"
#include "../../common/Completions/MessagePayload.hpp"

namespace XFS4IoT::Common::Completions
{
    class SetVersionsCompletion final
        : public XFS4IoT::Completion<XFS4IoT::MessagePayloadBase>
    {
    public:
        static constexpr const char* CompletionName = "Common.SetVersions";
        static constexpr const char* Version = "1.0";

        SetVersionsCompletion(
            int requestId,
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            const std::string& errorDescription)
            : XFS4IoT::Completion<XFS4IoT::MessagePayloadBase>(
                CompletionName,
                Version,
                requestId,
                nullptr,
                completionCode,
                errorDescription)
        {
        }

    private:
        static bool registered_;
    };

    inline bool SetVersionsCompletion::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(SetVersionsCompletion),
                SetVersionsCompletion::CompletionName,
                SetVersionsCompletion::Version);
            return true;
        }();
}