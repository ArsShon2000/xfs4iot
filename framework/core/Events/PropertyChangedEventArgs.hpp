#pragma once

#include <string>
#include <optional>
#include <memory>
#include <functional>
#include <vector>
#include <mutex>

namespace XFS4IoTServer
{
    class EventArgs
    {
    public:
        virtual ~EventArgs() = default;
    };

    class PropertyChangedEventArgs : public EventArgs
    {
    public:
        explicit PropertyChangedEventArgs(std::optional<std::string> propertyName = std::nullopt)
            : propertyName_(std::move(propertyName))
        {
        }

        virtual std::optional<std::string> getPropertyName() const
        {
            return propertyName_;
        }

        bool isPropertyChanged(const std::string& propertyName) const
        {
            if (!propertyName_.has_value())
                return true;
            return propertyName_.value() == propertyName;
        }

        bool allPropertiesChanged() const
        {
            return !propertyName_.has_value() || propertyName_.value().empty();
        }

    private:
        std::optional<std::string> propertyName_;
    };

    using PropertyChangedEventHandler = std::function<void(
        std::shared_ptr<void> sender,
        std::shared_ptr<PropertyChangedEventArgs> args)>;

    /// <summary>
    /// Interface for objects that support property change notifications
    /// Analog of INotifyPropertyChanged
    /// </summary>
    class INotifyPropertyChanged
    {
    public:
        virtual ~INotifyPropertyChanged() = default;

        /// <summary>
        /// Subscribe to property changed events
        /// </summary>
        virtual void onPropertyChanged(PropertyChangedEventHandler handler) = 0;

        /// <summary>
        /// Unsubscribe from property changed events (optional, for cleanup)
        /// </summary>
        virtual void removePropertyChangedHandler(const PropertyChangedEventHandler& handler) = 0;
    };

    /// <summary>
    /// Base class that implements INotifyPropertyChanged
    /// Can be inherited by classes that need to notify about property changes
    /// </summary>
    class NotifyPropertyChangedBase : public INotifyPropertyChanged, public std::enable_shared_from_this<NotifyPropertyChangedBase>
    {
    public:
        virtual ~NotifyPropertyChangedBase() = default;

        /// <summary>
        /// Subscribe to property changed events
        /// </summary>
        void onPropertyChanged(PropertyChangedEventHandler handler) override
        {
            std::lock_guard<std::mutex> lock(handlersMutex_);
            handlers_.push_back(std::move(handler));
        }

        /// <summary>
        /// Remove a specific handler (note: requires exact match by target)
        /// </summary>
        void removePropertyChangedHandler(const PropertyChangedEventHandler& handler) override
        {
            std::lock_guard<std::mutex> lock(handlersMutex_);
            // Note: comparing std::function is tricky, this is a simplified version
            // In production, you might want to use a token-based subscription system
        }

    protected:
        /// <summary>
        /// Raise property changed event
        /// Call this from property setters when a property changes
        /// </summary>
        void raisePropertyChanged(const std::string& propertyName)
        {
            auto args = std::make_shared<PropertyChangedEventArgs>(propertyName);
            notifyPropertyChanged(args);
        }

        /// <summary>
        /// Raise property changed event for all properties
        /// </summary>
        void raisePropertyChanged()
        {
            auto args = std::make_shared<PropertyChangedEventArgs>(std::nullopt);
            notifyPropertyChanged(args);
        }

    private:
        void notifyPropertyChanged(std::shared_ptr<PropertyChangedEventArgs> args)
        {
            std::vector<PropertyChangedEventHandler> handlersCopy;
            {
                std::lock_guard<std::mutex> lock(handlersMutex_);
                handlersCopy = handlers_;
            }

            // Call handlers outside the lock to avoid deadlocks
            for (const auto& handler : handlersCopy)
            {
                if (handler)
                {
                    handler(shared_from_this(), args);
                }
            }
        }

        std::vector<PropertyChangedEventHandler> handlers_;
        std::mutex handlersMutex_;
    };
}