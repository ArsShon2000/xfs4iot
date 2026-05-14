#pragma once

#include <string>
#include <memory>
#include <vector>
#include "Message.hpp"


namespace XFS4IoT
{
    /// <summary>
    /// Interface for message decoder with enumerable message types
    /// </summary>
    class IMessageDecoder
    {
    public:
        virtual ~IMessageDecoder() = default;

        /// <summary>
        /// Try to unserialize JSON string to a message object
        /// </summary>
        /// <param name="json">JSON string to deserialize</param>
        /// <returns>Deserialized message object or nullptr if failed</returns>
        virtual std::shared_ptr<MessageBase> TryUnserialise(const std::string& json) const = 0;

        /// <summary>
        /// Get all supported message types
        /// </summary>
        /// <returns>Vector of message type names</returns>
        virtual std::vector<std::string> GetSupportedMessageTypes() const = 0;

        // Iterator support for range-based for loops
        class Iterator
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = std::string;
            using difference_type = std::ptrdiff_t;
            using pointer = const std::string*;
            using reference = const std::string&;

            Iterator(const IMessageDecoder* decoder, size_t index)
                : decoder_(decoder), index_(index)
            {
                if (decoder_) {
                    types_ = decoder_->GetSupportedMessageTypes();
                }
            }

            reference operator*() const { return types_[index_]; }
            pointer operator->() const { return &types_[index_]; }

            Iterator& operator++()
            {
                ++index_;
                return *this;
            }

            Iterator operator++(int)
            {
                Iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            friend bool operator==(const Iterator& a, const Iterator& b)
            {
                return a.index_ == b.index_;
            }

            friend bool operator!=(const Iterator& a, const Iterator& b)
            {
                return !(a == b);
            }

        private:
            const IMessageDecoder* decoder_;
            size_t index_;
            std::vector<std::string> types_;
        };

        Iterator begin() const { return Iterator(this, 0); }
        Iterator end() const { return Iterator(this, GetSupportedMessageTypes().size()); }
    };
}