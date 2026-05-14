#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <boost/asio/awaitable.hpp>
#include "../CommonServiceProvider/CashManagementCapabilitiesClass.hpp"
#include <optional>
#include "ItemClassificationClass.hpp"
#include "CashInStatusClass.hpp"
#include "../../server/ServiceProvider.hpp"

namespace XFS4IoTFramework::CashManagement
{
    //// Forward declarations
    //class CashInStatusClass;
    //class CashManagementPresentStatus;
    //class ItemClassificationListClass;

    class ICashManagementService /*: public std::enable_shared_from_this<XFS4IoTServer::ServiceProvider>*/
    {
    public:
        virtual ~ICashManagementService() = default;

        /// <summary>
        /// The framework maintains cash-in status
        /// </summary>
        virtual const std::shared_ptr<CashInStatusClass> GetCashInStatusManaged() const = 0;

        /// <summary>
        /// The last status of the most recent attempt to present or return items to the
        /// customer.
        /// </summary>
        virtual const std::unordered_map<XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum,
            std::shared_ptr<CashManagementPresentStatus>>&
            GetLastCashManagementPresentStatus() const = 0;

        /// <summary>
        /// This list provides the functionality to blacklist notes and allows additional
        /// flexibility, for example to specify that notes can be taken out of circulation
        /// by specifying them as unfit. Any items not returned in this list will be handled
        /// according to normal classification rules.
        /// </summary>
        virtual const std::shared_ptr<ItemClassificationListClass>& GetItemClassificationList() const = 0;

        virtual boost::asio::awaitable<void> ItemsTakenEvent(
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position,
            std::optional<std::string> additionalBunches = std::nullopt) = 0;

        virtual boost::asio::awaitable<void> ItemsInsertedEvent(
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position) = 0;

        virtual boost::asio::awaitable<void> ItemsPresentedEvent(
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position,
            std::optional<std::string> additionalBunches) = 0;



        virtual boost::asio::awaitable<void> MediaDetectedEvent(
            std::optional<std::string> storageId,
            XFS4IoTFramework::CashManagement::ItemTargetEnum target,
            std::optional<int> index = std::nullopt) = 0;


        /// <summary>
        /// Store cash-in in status persistently
        /// </summary>
        virtual void StoreCashInStatus() = 0;

        /// <summary>
        /// Store present status persistently
        /// </summary>
        virtual void StoreCashManagementPresentStatus() = 0;

        /// <summary>
        /// Store classification list persistently
        /// </summary>
        virtual void StoreItemClassificationList() = 0;
    };
}