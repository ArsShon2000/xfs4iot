// ICashAcceptorService.hpp
#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include "DeviceLockStatusClass.hpp"
#include "../CashManagementServiceProvider/CashInStatusClass.hpp"

namespace XFS4IoTServer
{
    /// <summary>
    /// ICashAcceptorService interface
    /// </summary>
    class ICashAcceptorService /*public std::enable_shared_from_this<ICashAcceptorService>*/
    {
    public:
        virtual ~ICashAcceptorService() = default;

        /// <summary>
        /// The information about the status of the currently active cash-in transaction or
        /// in the case where no cash-in transaction is active the status of the most recently ended cash-in transaction.
        /// </summary>
        virtual std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass> GetCashInStatus() const = 0;

        /// <summary>
        /// The physical lock/unlock status of the CashAcceptor device and storages.
        /// </summary>
        virtual XFS4IoTFramework::CashAcceptor::DeviceLockStatusClass GetDeviceLockStatus() const = 0;

        /// <summary>
        /// The deplete target and destination information
        /// Key - The storage id can be used for target of the depletion operation.
        /// Value - List of storage id can be used for source of the depletion operation
        /// </summary>
        virtual const std::map<std::string, std::vector<std::string>>& GetDepleteCashUnitSources() const = 0;

        /// <summary>
        /// Which storage units can be specified as targets for a given source storage unit with the CashAcceptor.Replenish command
        /// </summary>
        virtual const std::vector<std::string>& GetReplenishTargets() const = 0;
    };

} // namespace XFS4IoTServer