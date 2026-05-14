#pragma once

#include "../../StorageServiceProvider/Events/StorageErrorCommandEvent.hpp"
#include "../../StorageServiceProvider/IStorageService.hpp"
#include "../../StorageServiceProvider/CashUnit.hpp"
#include "../../../ServiceClasses/CashManagementServiceProvider/Events/StorageErrorCommonCommandEvents.hpp"
#include <memory>

namespace XFS4IoTFramework::CashManagement
{
    /// <summary>
    /// Reset Command Events
    /// Provides event handling for the Reset command
    /// </summary>
    class ResetCommandEvents final : public XFS4IoTFramework::CashManagement::StorageErrorCommonCommandEvents
    {
    public:
        using MediaDetectedCallback =
            std::function<boost::asio::awaitable<void>(
                XFS4IoTFramework::CashManagement::ItemTargetEnum target,
                std::optional<std::string> storageId,
                std::optional<int> index)>;
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="storage">Storage service interface</param>
        /// <param name="events">Reset events interface</param>
        ResetCommandEvents(
            std::shared_ptr<XFS4IoTFramework::Storage::IStorageService> storage,
            std::shared_ptr<IResetEvents> events,
            MediaDetectedCallback mediaDetectedCallback)
            : XFS4IoTFramework::CashManagement::StorageErrorCommonCommandEvents(std::move(storage), std::move(events))
            , mediaDetectedCallback_(std::move(mediaDetectedCallback))
        {
        }

        boost::asio::awaitable<void> MediaDetectedEvent(
            XFS4IoTFramework::CashManagement::ItemTargetEnum target,
            std::optional<std::string> storageId = std::nullopt,
            std::optional<int> index = std::nullopt)
        {
            if (!mediaDetectedCallback_)
                co_return;

            co_await mediaDetectedCallback_(target, std::move(storageId), index);
        }
        /// <summary>
        /// Virtual destructor
        /// </summary>
        virtual ~ResetCommandEvents() = default;

    private:
        MediaDetectedCallback mediaDetectedCallback_;
    };

} // namespace XFS4IoTFramework::CashManagement