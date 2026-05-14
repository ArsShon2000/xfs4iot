#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <optional>
#include <boost/asio/awaitable.hpp>
#include "CashInStatusClass.hpp"
#include "../StorageServiceProvider/CashManagementPresentStatus.hpp"
#include "../../ServiceClasses/CommonServiceProvider/CashManagementCapabilitiesClass.hpp"
#include "../../ServiceClasses/CommonServiceProvider/ICommonService.hpp"
#include "../../ServiceClasses/StorageServiceProvider/IStorageService.hpp"
#include "../../ServiceClasses/CashManagementServiceProvider/ICashManagementDevice.hpp"
#include "../../server/IServiceProvider.hpp"
#include "../../core/Logger/ILogger.hpp"
#include "../../core/CashManagement/Events/ItemsTakenEvent.hpp"
#include "../../core/CashManagement/Events/ItemsInsertedEvent.hpp"
#include "../../core/CashManagement/Events/ItemsPresentedEvent.hpp"
#include "ItemClassificationClass.hpp"
#include "../../core/Persistent/IPersistentData.hpp"
#include "../../core/CashManagement/Events/MediaDetectedEvent.hpp"

namespace XFS4IoTServer
{
    // Forward declarations
    //class IServiceProvider;
    //class IPersistentData;
    //class ICashManagementDevice;
    //class ICommonService;
    //class IStorageService;
    //class CashInStatusClass;
    //class CashManagementPresentStatus;
    //class ItemClassificationListClass;
    //class PropertyChangedEventArgs;
    //class CashManagementStatusClass;
    //class CashManagementCapabilitiesClass;

 /*   namespace Constants
    {
        extern const std::string DeviceClass;
        extern const std::string Framework;
    }*/

    //static XFS4IoT::CashManagement::ItemTargetEnumEnum ToXfsItemTarget(
    //    XFS4IoTFramework::CashManagement::ItemTargetEnum target)
    //{
    //    using Src = XFS4IoTFramework::CashManagement::ItemTargetEnum;
    //    using Dst = XFS4IoT::CashManagement::ItemTargetEnumEnum;

    //    switch (target)
    //    {
    //    case Src::Retract: return Dst::Retract;
    //    case Src::Transport: return Dst::Transport;
    //    case Src::Stacker: return Dst::Stacker;
    //    case Src::Reject: return Dst::Reject;
    //    case Src::ItemCassette: return Dst::ItemCassette;
    //    case Src::CashIn: return Dst::CashIn;
    //    case Src::SingleUnit: return Dst::SingleUnit;

    //    case Src::OutDefault: return Dst::OutDefault;
    //    case Src::OutLeft: return Dst::OutLeft;
    //    case Src::OutRight: return Dst::OutRight;
    //    case Src::OutCenter: return Dst::OutCenter;
    //    case Src::OutTop: return Dst::OutTop;
    //    case Src::OutBottom: return Dst::OutBottom;
    //    case Src::OutFront: return Dst::OutFront;
    //    case Src::OutRear: return Dst::OutRear;
    //        default:
				//throw std::invalid_argument(std::format("Unsupported ItemTargetEnum value: {}", static_cast<int>(target)));
    //    }

    //}

    class CashManagementServiceClass
    {
    public:
        CashManagementServiceClass(
            std::shared_ptr<XFS4IoTServer::IServiceProvider> serviceProvider,
            std::shared_ptr<ILogger> logger,
            std::shared_ptr<IPersistentData> persistentData);

        virtual ~CashManagementServiceClass();

        // Disable copy and move
        //CashManagementServiceClass(const CashManagementServiceClass&) = delete;
        //CashManagementServiceClass& operator=(const CashManagementServiceClass&) = delete;
        //CashManagementServiceClass(CashManagementServiceClass&&) = delete;
        //CashManagementServiceClass& operator=(CashManagementServiceClass&&) = delete;

        /// <summary>
        /// The framework maintains cash-in status
        /// </summary>
        std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass> GetCashInStatusManaged() const
        {
            return cashInStatusManaged_;
        }

        /// <summary>
        /// Store cash-in status persistently
        /// </summary>
        void StoreCashInStatus();

        /// <summary>
        /// The last status of the most recent attempt to present or return items to the customer.
        /// </summary>
        const std::unordered_map<XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum,
            std::shared_ptr<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>>&
            GetLastCashManagementPresentStatus() const
        {
            return lastCashManagementPresentStatus_;
        }

        /// <summary>
        /// Store present status persistently
        /// </summary>
        void StoreCashManagementPresentStatus();

        /// <summary>
        /// This list provides the functionality to blacklist notes and allows additional flexibility
        /// </summary>
        std::shared_ptr<XFS4IoTFramework::CashManagement::ItemClassificationListClass> GetItemClassificationList() const
        {
            return itemClassificationList_;
        }

        /// <summary>
        /// Store classification list persistently
        /// </summary>
        void StoreItemClassificationList();

        boost::asio::awaitable<void> ItemsTakenEvent(
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position,
            std::optional<std::string> additionalBunches = std::nullopt);

        boost::asio::awaitable<void> ItemsInsertedEvent(
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position);

        boost::asio::awaitable<void> ItemsPresentedEvent(
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position,
            std::optional<std::string> additionalBunches);

        boost::asio::awaitable<void> MediaDetectedEvent(
            std::optional<std::string> storageId,
            XFS4IoTFramework::CashManagement::ItemTargetEnum target,
            std::optional<int> index);

    protected:
        /// <summary>
        /// Event implementations to be provided by derived class
        /// </summary>
        virtual boost::asio::awaitable<void> ItemsTakenEventImpl(
            std::shared_ptr<void> payload)
        {
            // Default no-op handler. Derived service classes may override to provide specific processing.
            co_return;
        }

        virtual boost::asio::awaitable<void> ItemsInsertedEventImpl(
            std::shared_ptr<void> payload)
        {
            // Default no-op handler. Derived service classes may override to provide specific processing.
            co_return;
        }

        virtual boost::asio::awaitable<void> ItemsPresentedEventImpl(
            std::shared_ptr<void> payload)
        {
            // Default no-op handler. Derived service classes may override to provide specific processing.
            co_return;
        }

        virtual boost::asio::awaitable<void> MediaDetectedEventImpl(
            std::shared_ptr<XFS4IoT::CashManagement::Events::MediaDetectedEventPayloadData> payload)
        {

            if (!serviceProvider_)
            {
                throw std::runtime_error("CashManagementServiceClass::MediaDetectedEventImpl: serviceProvider_ is null");
            }

            auto event =
                std::make_shared<XFS4IoT::CashManagement::Events::MediaDetectedEvent>(
                    std::move(payload));

            co_await serviceProvider_->BroadcastEvent(event);
        }

        std::shared_ptr<IServiceProvider> serviceProvider_;
        std::shared_ptr<ILogger> logger_;
        std::shared_ptr<XFS4IoTFramework::CashManagement::ICashManagementDevice> device_;
        std::shared_ptr<XFS4IoTFramework::Common::ICommonService> commonService_;
        std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storageService_;

    private:
        void getStatus();
        void GetCapabilities();

        /// <summary>
        /// Initialize default present status for all positions
        /// </summary>
        void initializeDefaultPresentStatus();

        /// <summary>
        /// Status changed event forwarder
        /// </summary>
        void statusChangedEventForwarder(
            std::shared_ptr<void> sender,
            std::shared_ptr<PropertyChangedEventArgs> propertyInfo);

        /// <summary>
        /// Convert internal position enum to XFS4IoT position enum
        /// </summary>
        static XFS4IoT::CashManagement::PositionEnum convertPositionToXFS4IoT(XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position);

        /// <summary>
        /// Convert ItemTargetEnum to XFS4IoT enum
        /// </summary>
        static XFS4IoT::CashManagement::ItemTargetEnumEnum convertTargetToXFS4IoT(XFS4IoTFramework::CashManagement::ItemTargetEnum target);

        std::shared_ptr<IPersistentData> persistentData_;
        std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass> cashInStatusManaged_;
        std::unordered_map<XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum,
            std::shared_ptr<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>> lastCashManagementPresentStatus_;
        std::shared_ptr<XFS4IoTFramework::CashManagement::ItemClassificationListClass> itemClassificationList_;
    };
}