#pragma once

#include "../../CashManagementServiceProvider/Events/StorageErrorCommonCommandEvents.hpp"
#include "../../StorageServiceProvider/IStorageService.hpp"
#include <memory>

namespace XFS4IoTFramework::CashAcceptor
{
    /// <summary>
    /// PreparePresent Command Events
    /// Provides event handling for the PreparePresent command
    /// </summary>
    class PreparePresentCommandEvents final : public XFS4IoTFramework::CashManagement::StorageErrorCommonCommandEvents
    {
    public:
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="storage">Storage service interface</param>
        /// <param name="events">PreparePresent events interface</param>
        PreparePresentCommandEvents(
            std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage,
            std::shared_ptr<XFS4IoTFramework::CashManagement::IPreparePresentEvents> events)
            : StorageErrorCommonCommandEvents(storage, events)
        {
        }

        /// <summary>
        /// Virtual destructor
        /// </summary>
        virtual ~PreparePresentCommandEvents() = default;

        // Deleted copy/move constructors and assignment operators
        PreparePresentCommandEvents(const PreparePresentCommandEvents&) = delete;
        PreparePresentCommandEvents& operator=(const PreparePresentCommandEvents&) = delete;
        PreparePresentCommandEvents(PreparePresentCommandEvents&&) = delete;
        PreparePresentCommandEvents& operator=(PreparePresentCommandEvents&&) = delete;
    };

} // namespace XFS4IoTFramework::CashAcceptor