#pragma once

#include <memory>
#include <stop_token>
#include <boost/asio/awaitable.hpp>

// Forward declarations
namespace XFS4IoTServer
{
    class IServiceProvider;
}

namespace XFS4IoTServer
{
    /// <summary>
    /// Interface for device implementation that integrates with hardware
    /// </summary>
    class IDevice
    {
    public:
        virtual ~IDevice() = default;

        /// <summary>
        /// Run the device asynchronously
        /// </summary>
        /// <param name="token">Cancellation token to stop the device</param>
        virtual boost::asio::awaitable<void> RunAsync(std::stop_token token) = 0;

        /// <summary>
        /// Set the service provider for this device
        /// </summary>
        virtual void SetServiceProvider(std::shared_ptr<IServiceProvider> serviceProvider) = 0;

        /// <summary>
        /// Get the service provider for this device
        /// </summary>
        virtual std::shared_ptr<IServiceProvider> GetServiceProvider() const = 0;
    };
}