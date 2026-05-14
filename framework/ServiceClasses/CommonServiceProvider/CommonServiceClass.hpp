#pragma once

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <stdexcept>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>

#include "ICommonService.hpp"
#include "ICommonDevice.hpp"
#include "../../server/IServiceProvider.hpp"
#include "../../core/Logger/ILogger.hpp"
#include "../../core/MessageTypeInfo.hpp"

#include "CommonCapabilitiesClass.hpp"
#include "CashManagementCapabilitiesClass.hpp"
#include "CashAcceptorCapabilitiesClass.hpp"

#include "CommonStatusClass.hpp"
#include "CashManagementStatusClass.hpp"
#include "CashAcceptorStatusClass.hpp"

#include "../../core/common/CommonSchemas.hpp"
#include "../../core/common/MessageHeader.hpp"

#include "../../core/common/Events/StatusChangedEvent.hpp"
#include "../../core/common/Events/ErrorEvent.hpp"
#include "../../core/common/Events/NonceClearedEvent.hpp"
#include "../../core/Events/PropertyChangedEventArgs.hpp"

#include "ICommonDevice.hpp"
#include "../../core/Logger/ILogger.hpp"


namespace XFS4IoTServer
{
    //class PropertyChangedEventArgs
    //{
    //public:
    //    explicit PropertyChangedEventArgs(std::string propertyName)
    //        : propertyName_(std::move(propertyName))
    //    {
    //    }

    //    const std::string& GetPropertyName() const noexcept
    //    {
    //        return propertyName_;
    //    }

    //private:
    //    std::string propertyName_;
    //};

    class CommonServiceClass : public  XFS4IoTFramework::Common::ICommonService
    {
    public:
        CommonServiceClass(
            std::shared_ptr<IServiceProvider> serviceProvider,
            std::shared_ptr<ILogger> logger,
            const std::string& serviceName);

        virtual ~CommonServiceClass() = default;


        std::shared_ptr<XFS4IoTFramework::Common::CommonCapabilitiesClass>
            GetCommonCapabilities() const noexcept override;
        void SetCommonCapabilities(std::shared_ptr<XFS4IoTFramework::Common::CommonCapabilitiesClass> capabilities)
        {
            commonCapabilities_ = capabilities;
        }


        std::shared_ptr<XFS4IoTFramework::Common::CashManagementCapabilitiesClass>
            GetCashManagementCapabilities() const noexcept override;
        void SetCashManagementCapabilities(std::shared_ptr<XFS4IoTFramework::Common::CashManagementCapabilitiesClass> capabilities)
        {
            cashManagementCapabilities_ = capabilities;
        }


        std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass>
            GetCashAcceptorCapabilities() const noexcept override;
        void SetCashAcceptorCapabilities(std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass> capabilities)
        {
            cashAcceptorCapabilities_ = capabilities;
        }


        std::shared_ptr<XFS4IoTFramework::Common::CommonStatusClass>
            GetCommonStatus() const noexcept override;
        void SetCommonStatus(std::shared_ptr<XFS4IoTFramework::Common::CommonStatusClass> status)
        {
            commonStatus_ = status;
        }


        std::shared_ptr<XFS4IoTFramework::Common::CashManagementStatusClass>
            GetCashManagementStatus() const noexcept override;
        void SetCashManagementStatus(std::shared_ptr<XFS4IoTFramework::Common::CashManagementStatusClass> status)
        {
            cashManagementStatus_ = status;
        }


        std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorStatusClass>
            GetCashAcceptorStatus() const noexcept override;
        void SetCashAcceptorStatus(std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorStatusClass> status)
        {
            cashAcceptorStatus_ = status;
        }


        boost::asio::awaitable<void> StatusChangedEvent(
            std::shared_ptr<XFS4IoT::Common::Events::StatusChangedEventPayloadData> payload);

        boost::asio::awaitable<void> ErrorEvent(
            std::shared_ptr<XFS4IoT::Common::Events::ErrorEventPayloadData> payload);

        boost::asio::awaitable<void> NonceClearedEvent(
            std::shared_ptr<XFS4IoT::Common::Events::NonceClearedEventPayloadData> payload);

        boost::asio::awaitable<void> StatusChangedEvent(
            std::shared_ptr<void> sender,
            std::shared_ptr<XFS4IoTServer::PropertyChangedEventArgs> propertyInfo);

        boost::asio::awaitable<void> NonceClearedEvent(
            const std::string& reasonDescription);

        boost::asio::awaitable<void> ErrorEvent(
            XFS4IoTFramework::Common::CommonStatusClass::ErrorEventIdEnum eventId,
            XFS4IoTFramework::Common::CommonStatusClass::ErrorActionEnum action,
            const std::string& vendorDescription);

        void StatusChangedEventForwarder(
            std::shared_ptr<void> sender,
            std::shared_ptr<XFS4IoTServer::PropertyChangedEventArgs> propertyInfo);



    protected:
        /// <summary>
        /// Status changed event handler defined in each of device status class
        /// </summary>
        /// <param name="sender">object where the property is changed</param>
        /// <param name="propertyInfo">including name of property is changed</param>
        virtual boost::asio::awaitable<void> StatusChangedEventHandler(
            std::shared_ptr<void> sender,
            std::shared_ptr<XFS4IoTServer::PropertyChangedEventArgs> propertyInfo)
        {
            // Default no-op handler. Derived service classes may override to provide specific processing.
            co_return;
        }

    protected:
        void GetCapabilities();
        void GetStatus();

        std::map<std::string, XFS4IoT::MessageTypeInfo> BuildSupportedServiceMessages() const;

        void AddInterfaceMessages(
            std::map<std::string, XFS4IoT::MessageTypeInfo>& target,
            XFS4IoT::Common::InterfaceClass::NameEnum interfaceName,
            const XFS4IoTFramework::Common::InterfaceInfo& interfaceInfo,
            const std::map<std::string, XFS4IoT::MessageTypeInfo>& frameworkMessages) const;

        static std::string InterfaceNameToPrefix(
            XFS4IoT::Common::InterfaceClass::NameEnum interfaceName);

        static XFS4IoT::Common::Events::ErrorEventPayloadData::EventIdEnum MapErrorEventId(
            XFS4IoTFramework::Common::CommonStatusClass::ErrorEventIdEnum value);

        static XFS4IoT::Common::Events::ErrorEventPayloadData::ActionEnum MapErrorAction(
            XFS4IoTFramework::Common::CommonStatusClass::ErrorActionEnum value);

    protected:
        std::shared_ptr<IServiceProvider> serviceProvider_;
        std::shared_ptr<ILogger> logger_;
        std::shared_ptr<XFS4IoTFramework::Common::ICommonDevice> device_;
        std::string serviceName_;

        std::shared_ptr<XFS4IoTFramework::Common::CommonCapabilitiesClass> commonCapabilities_;
        std::shared_ptr<XFS4IoTFramework::Common::CashManagementCapabilitiesClass> cashManagementCapabilities_;
        std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass> cashAcceptorCapabilities_;

        std::shared_ptr<XFS4IoTFramework::Common::CommonStatusClass> commonStatus_;
        std::shared_ptr<XFS4IoTFramework::Common::CashManagementStatusClass> cashManagementStatus_;
        std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorStatusClass> cashAcceptorStatus_;
    };
}