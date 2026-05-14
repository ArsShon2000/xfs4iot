#pragma once

#include <string>
#include <optional>

namespace XFS4IoTServer
{
    /// <summary>
    /// Interface for accessing service configuration values
    /// </summary>
    class IServiceConfiguration
    {
    public:
        virtual ~IServiceConfiguration() = default;

        /// <summary>
        /// Get configuration value associated with the key specified.
        /// Returns nullopt if specified value doesn't exist in the configuration.
        /// </summary>
        /// <param name="name">Name of the configuration value</param>
        /// <returns>Configuration value or nullopt if not found</returns>
        virtual std::optional<std::string> get(const std::string& name) const = 0;
    };
}