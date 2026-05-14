#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <optional>
#include "IServiceConfiguration.hpp"
#include "../core/Logger/ConsoleLogger.hpp"

namespace XFS4IoTServer
{
    /// <summary>
    /// Service configuration implementation
    /// </summary>
    class ServiceConfiguration final : public IServiceConfiguration
    {
    public:
        /// <summary>
        /// Constructor
        /// </summary>
        explicit ServiceConfiguration(std::shared_ptr<ILogger> logger);

        /// <summary>
        /// Get configuration value associated with the key specified.
        /// Returns nullopt if specified value doesn't exist in the configuration.
        /// </summary>
        /// <param name="name">Name of the configuration value</param>
        /// <returns>Configuration value or nullopt if not found</returns>
        virtual std::optional<std::string> get(const std::string& name) const override;

    private:
        /// <summary>
        /// Load configuration settings from file
        /// </summary>
        void loadSettings();

        /// <summary>
        /// Logging interface
        /// </summary>
        std::shared_ptr<ILogger> logger_;

        /// <summary>
        /// The collection of configuration values
        /// </summary>
        std::unordered_map<std::string, std::string> settings_;
    };
}