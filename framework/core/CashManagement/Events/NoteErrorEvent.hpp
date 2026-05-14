#pragma once

#include <memory>
#include <optional>
#include <string>

#include "../../Events.hpp"
#include "../../Message.hpp"
#include "../../common/MessageHeader.hpp"

namespace XFS4IoT::CashManagement::Events
{
    class NoteErrorEventPayloadData final : public XFS4IoT::MessagePayloadBase
    {
    public:
        enum class ReasonEnum
        {
            DoubleNote,
            LongNote,
            SkewedNote,
            IncorrectCount,
            NotesTooClose,
            OtherNoteError,
            ShortNote
        };

        explicit NoteErrorEventPayloadData(
            std::optional<ReasonEnum> reason = std::nullopt)
            : reason_(reason)
        {
        }

        std::optional<ReasonEnum> GetReason() const noexcept
        {
            return reason_;
        }

        void SetReason(std::optional<ReasonEnum> reason)
        {
            reason_ = reason;
        }

    private:
        std::optional<ReasonEnum> reason_;
    };

    inline std::string ToString(NoteErrorEventPayloadData::ReasonEnum reason)
    {
        switch (reason)
        {
        case NoteErrorEventPayloadData::ReasonEnum::DoubleNote:
            return "doubleNote";
        case NoteErrorEventPayloadData::ReasonEnum::LongNote:
            return "longNote";
        case NoteErrorEventPayloadData::ReasonEnum::SkewedNote:
            return "skewedNote";
        case NoteErrorEventPayloadData::ReasonEnum::IncorrectCount:
            return "incorrectCount";
        case NoteErrorEventPayloadData::ReasonEnum::NotesTooClose:
            return "notesTooClose";
        case NoteErrorEventPayloadData::ReasonEnum::OtherNoteError:
            return "otherNoteError";
        case NoteErrorEventPayloadData::ReasonEnum::ShortNote:
            return "shortNote";
        default:
            throw std::invalid_argument("Unknown ReasonEnum value");
        }
    }

    inline void to_json(
        nlohmann::json& j,
        const NoteErrorEventPayloadData& p)
    {
        j = nlohmann::json::object();

        if (p.GetReason().has_value())
            j["reason"] = ToString(p.GetReason().value());
    }

    class NoteErrorEvent final
        : public XFS4IoT::Events::Event<NoteErrorEventPayloadData>
    {
    public:
        static constexpr const char* EventName = "CashManagement.NoteErrorEvent";
        static constexpr const char* Version = "1.0";

        NoteErrorEvent(
            int requestId,
            std::shared_ptr<NoteErrorEventPayloadData> payload)
            : XFS4IoT::Events::Event<NoteErrorEventPayloadData>(
                EventName,
                Version,
                requestId,
                std::move(payload))
        {
        }

    private:
        static bool registered_;
    };

    inline bool NoteErrorEvent::registered_ = []()
        {
            XFS4IoT::MessageBase::RegisterMessage(
                typeid(NoteErrorEvent),
                NoteErrorEvent::EventName,
                NoteErrorEvent::Version);
            return true;
        }();
}