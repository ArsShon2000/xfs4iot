// CashInRollbackCommandEvents.hpp
#pragma once

#include "../../CashManagementServiceProvider/Events/StorageErrorCommonCommandEvents.hpp"
#include "../../StorageServiceProvider/IStorageService.hpp"
#include <memory>

namespace XFS4IoTFramework::CashAcceptor
{
    /// <summary>
    /// CashInRollback Command Events
    /// Provides event handling for the CashInRollback command
    /// </summary>
    class CashInRollbackCommandEvents final : public XFS4IoTFramework::CashManagement::StorageErrorCommonCommandEvents
    {
    public:
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="storage">Storage service interface</param>
        /// <param name="events">CashInRollback events interface</param>
        CashInRollbackCommandEvents(
            std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage,
            std::shared_ptr<XFS4IoTFramework::CashManagement::ICashInRollbackEvents> events)
            : StorageErrorCommonCommandEvents(storage, events)
        {
        }

        /// <summary>
        /// Virtual destructor
        /// </summary>
        virtual ~CashInRollbackCommandEvents() = default;

        // Deleted copy/move constructors and assignment operators
        CashInRollbackCommandEvents(const CashInRollbackCommandEvents&) = delete;
        CashInRollbackCommandEvents& operator=(const CashInRollbackCommandEvents&) = delete;
        CashInRollbackCommandEvents(CashInRollbackCommandEvents&&) = delete;
        CashInRollbackCommandEvents& operator=(CashInRollbackCommandEvents&&) = delete;
    };

} // namespace XFS4IoTFramework::CashAcceptor