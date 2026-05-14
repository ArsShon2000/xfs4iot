//#pragma once
//
//#include <string>
//#include <optional>
//#include <memory>
//#include <typeinfo>
//
//#include "Message.hpp"
//#include "common/MessageHeader.hpp"
//
///**
// * @brief Completion
// *
// * Базовый класс XFS4IoT Completion.
// * Архитектурный аналог Completion.cs в KAL_XFS4IoT_SP-Dev.
// */
//
//namespace XFS4IoT
//{
//    template<typename TPayload>
//    class Completion : public MessageBase
//    {
//    public:
//
//		// ЭТОГО КОНСТРУКТОРОА НУЖНО УБРАТЬ ИЗ-ЗА НЕПРАВИЛЬНОГО ВЫЗОВА MessageBase
//        Completion(int requestId,
//            std::shared_ptr<TPayload> payload,
//            MessageHeader::CompletionCodeEnum completionCode,
//            std::optional<std::string> errorDescription)
//            : MessageBase(
//                /* name */    MessageBase::LookupMessageName(typeid(TPayload)),
//                /* requestId */ requestId,
//                /* version */ MessageBase::LookupMessageVersion(typeid(TPayload)),
//                /* type */    MessageHeader::TypeEnum::Completion,
//                /* timeout */ std::nullopt,
//                /* status */  std::nullopt,
//                /* completionCode */ completionCode,
//                /* errorDescription */ errorDescription)
//            , requestId_(requestId)
//            , payload_(std::move(payload))
//            , completionCode_(completionCode)
//            , errorDescription_(errorDescription.value_or(""))
//        {
//        }
//        //// А ЭТОГО Нужно ОСТАВИТЬ
//        //Completion(const std::string& name,
//        //    const std::string& version,
//        //    int requestId,
//        //    std::shared_ptr<TPayload> payload,
//        //    MessageHeader::CompletionCodeEnum completionCode,
//        //    const std::string& errorDescription)
//        //    : Message<TPayload>(
//        //        name,
//        //        version,
//        //        requestId,
//        //        MessageHeader::TypeEnum::Completion,
//        //        std::move(payload),
//        //        completionCode,
//        //        errorDescription)
//        //{
//        //}
//
//        std::shared_ptr<TPayload> GetPayload() const { return payload_; }
//
//        //std::unique_ptr<MessageBase> Clone() const override
//        //{
//        //    return std::make_unique<Completion<TPayload>>(*this);
//        //}
//
//        int getRequestId() const { return requestId_; }
//        std::shared_ptr<TPayload> getPayload() const { return payload_; }
//        MessageHeader::CompletionCodeEnum getCompletionCode() const { return completionCode_; }
//        const std::string& getErrorDescription() const { return errorDescription_; }
//
//        virtual std::string getName() const = 0;
//        virtual std::string getVersion() const = 0;
//
//    private:
//        int requestId_;
//        std::shared_ptr<TPayload> payload_;
//        MessageHeader::CompletionCodeEnum completionCode_;
//        std::string errorDescription_;
//    };
//}

#pragma once

#include <string>
#include <optional>
#include <memory>
#include <typeinfo>

#include "Message.hpp"
#include "common/MessageHeader.hpp"

/**
 * @brief Completion
 *
 * Базовый класс XFS4IoT Completion.
 * Архитектурный аналог Completion.cs в KAL_XFS4IoT_SP-Dev.
 */

namespace XFS4IoT
{
    template<typename TPayload>
    class Completion : public Message<TPayload>
    {
    public:
        Completion(const std::string& name,
            const std::string& version,
            int requestId,
            std::shared_ptr<TPayload> payload,
            MessageHeader::CompletionCodeEnum completionCode,
            const std::string& errorDescription)
            : Message<TPayload>(
                name,
                version,
                requestId,
                MessageHeader::TypeEnum::Completion,
                std::move(payload),
                completionCode,
                errorDescription)
        {
        }

        virtual ~Completion() = default;
    };
}