#pragma once

#include <string>
#include <vector>

namespace XFS4IoT
{
    /// <summary>
    /// Class to store supported message information
    /// </summary>
    class MessageTypeInfo
    {
    public:
        enum class MessageTypeEnum
        {
            Command,
            Event
        };

        MessageTypeInfo()
            : Type(MessageTypeEnum::Command)
            , Versions()
        {
        }

        // Constructor
        MessageTypeInfo(MessageTypeEnum type, std::vector<std::string> versions)
            : Type(type)
            , Versions(std::move(versions))
        {
        }

        // Copy constructor
        MessageTypeInfo(const MessageTypeInfo& other)
            : Type(other.Type)
            , Versions(other.Versions)
        {
        }

        //// Assignment operator
        //MessageTypeInfo& operator=(const MessageTypeInfo& other)
        //{
        //    if (this != &other)
        //    {
        //        // Type is declared as const, so it cannot be assigned to.
        //        // If Type should be mutable, remove const.
        //        // Type = other.Type; // Cannot assign to const member
        //        const_cast<MessageTypeEnum&>(Type) = other.Type;
        //        const_cast<std::vector<std::string>&>(Versions) = other.Versions;
        //    }
        //    return *this;
        //}

        // init-only properties equivalent
        const MessageTypeEnum Type;
        const std::vector<std::string> Versions;
    };
}
