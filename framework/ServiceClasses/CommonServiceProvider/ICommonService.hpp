#pragma once

#include <string>
#include <memory>
#include <boost/asio/awaitable.hpp>
#include "CommonCapabilitiesClass.hpp"
#include "CashManagementCapabilitiesClass.hpp"
#include "CashAcceptorCapabilitiesClass.hpp"
#include "../../Server/ServiceProvider.hpp"
#include "CommonStatusClass.hpp"
#include "CashManagementStatusClass.hpp"
#include "CashAcceptorStatusClass.hpp"
#include "../../core/Events/PropertyChangedEventArgs.hpp"

namespace XFS4IoTFramework::Common
{
    //// Forward declarations
    //class CommonCapabilitiesClass;
    //class CashManagementCapabilitiesClass;
    //class CashAcceptorCapabilitiesClass;
    //class CommonStatusClass;
    //class CashManagementStatusClass;
    //class CashAcceptorStatusClass;

    //// Для событий StatusChanged
    //class PropertyChangedEventArgs;

    //// Nested payload classes (предполагаем, что они определены где-то)
    //namespace StatusChangedEvent { class PayloadData; }
    //namespace ErrorEvent { class PayloadData; }
    //namespace NonceClearedEvent { class PayloadData; }

    //class ICommonUnsolicitedEvents
    //{
    //public:
    //    virtual ~ICommonUnsolicitedEvents() = default;

    //    virtual boost::asio::awaitable<void> statusChangedEvent(
    //        std::shared_ptr<StatusChangedEvent::PayloadData> payload) = 0;

    //    virtual boost::asio::awaitable<void> errorEvent(
    //        std::shared_ptr<ErrorEvent::PayloadData> payload) = 0;

    //    virtual boost::asio::awaitable<void> nonceClearedEvent(
    //        std::shared_ptr<NonceClearedEvent::PayloadData> payload) = 0;
    //};

    class ICommonService /*: public std::enable_shared_from_this<XFS4IoTServer::ServiceProvider>*/
    {
    public:
        virtual ~ICommonService() = default;

        /// <summary>
        /// Stores Common Capabilities
        /// </summary>
        virtual std::shared_ptr<CommonCapabilitiesClass> GetCommonCapabilities() const = 0;
        virtual void SetCommonCapabilities(std::shared_ptr<CommonCapabilitiesClass> capabilities) = 0;

        /// <summary>
        /// Stores CashManagement capabilities for an internal use
        /// </summary>
        virtual std::shared_ptr<CashManagementCapabilitiesClass> GetCashManagementCapabilities() const
        {
            return nullptr;
        }
        virtual void SetCashManagementCapabilities(std::shared_ptr<CashManagementCapabilitiesClass>)
        {
        }

        /// <summary>
        /// Stores CashAcceptor capabilities
        /// </summary>
        virtual std::shared_ptr<CashAcceptorCapabilitiesClass> GetCashAcceptorCapabilities() const
        {
            return nullptr;
        }
        virtual void SetCashAcceptorCapabilities(std::shared_ptr<CashAcceptorCapabilitiesClass>)
        {
        }

        /// <summary>
        /// Stores Commons status
        /// </summary>
        virtual std::shared_ptr<CommonStatusClass> GetCommonStatus() const = 0;
        virtual void SetCommonStatus(std::shared_ptr<CommonStatusClass> status) = 0;

        /// <summary>
        /// Stores CashManagement status
        /// </summary>
        virtual std::shared_ptr<CashManagementStatusClass> GetCashManagementStatus() const
        {
            return nullptr;
        }
        virtual void SetCashManagementStatus(std::shared_ptr<CashManagementStatusClass>)
        {
        }

        /// <summary>
        /// Stores CashAcceptor status
        /// </summary>
        virtual std::shared_ptr<CashAcceptorStatusClass> GetCashAcceptorStatus() const
        {
            return nullptr;
        }
        virtual void SetCashAcceptorStatus(std::shared_ptr<CashAcceptorStatusClass>)
        {
        }

        /// <summary>
        /// Sending status changed event.
        /// </summary>
        virtual boost::asio::awaitable<void> StatusChangedEvent(
            std::shared_ptr<void> sender,
            std::shared_ptr<XFS4IoTServer::PropertyChangedEventArgs> propertyInfo) = 0;

        /// <summary>
        /// Nonce cleared event
        /// </summary>
        virtual boost::asio::awaitable<void> NonceClearedEvent(
            const std::string& reasonDescription) = 0;

        /// <summary>
        /// Error event
        /// </summary>
        virtual boost::asio::awaitable<void> ErrorEvent(
            CommonStatusClass::ErrorEventIdEnum eventId,
            CommonStatusClass::ErrorActionEnum action,
            const std::string& vendorDescription) = 0;
    };

  /*  class ICommonServiceClass : public ICommonService, public ICommonUnsolicitedEvents
    {
    public:
        ~ICommonServiceClass() override = default;
    };*/
}