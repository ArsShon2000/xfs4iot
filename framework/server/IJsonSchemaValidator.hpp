#pragma once
#include <optional>
#include <boost/asio/awaitable.hpp>


namespace XFS4IoTServer
{
    /// <summary>
    /// Interface for using JsonSchema validation library for testing incoming
    /// command messages
    /// </summary>
    class IJsonSchemaValidator
    {
    public:
        virtual ~IJsonSchemaValidator() = default;

        /// <summary>
        /// SP framework calls once the ServicePublisher object gets created to load 
        /// any of JSON schema library to validate XFS4 command message.
        /// </summary>
        virtual boost::asio::awaitable<void> LoadSchemaAsync() = 0;

        /// <summary>
        /// This method will be called when the SP framework receives incoming command messages.
        /// </summary>
        /// <param name="command">JSON command string to validate</param>
        /// <returns>Optional error message if validation failed, nullopt if successful</returns>
        virtual std::optional<std::string> Validate(const std::string& command) = 0;

        /// <summary>
        /// This property must be set if the XFS4 JSON schema is loaded successfully.
        /// If this property is set to false, Validate method won't be called from the SP framework.
        /// </summary>
        virtual bool IsSchemaLoaded() const = 0;
        virtual void SetSchemaLoaded(bool loaded) = 0;
    };
}