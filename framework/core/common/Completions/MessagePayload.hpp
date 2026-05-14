#pragma once

namespace XFS4IoT::Completions
{
    /// <summary>
    /// Base class for message payload
    /// </summary>
    class MessagePayload
    {
    public:
        virtual ~MessagePayload() = default;
    };

}

namespace XFS4IoT::Commands
{
    /// <summary>
    /// Base class for message payload
    /// </summary>
    class MessagePayload : public XFS4IoT::MessagePayloadBase
    {
    public:
        MessagePayload() = default;
        virtual ~MessagePayload() = default;
    };

}