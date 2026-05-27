#include "EscrowManager.hpp"
#include "../../CashAcceptorSample.hpp"
#include "../../../../framework/core/Persistent/PersistentDatasHandler.hpp"

namespace XFS4IoTSP::CashAcceptor::Sample
{
    EscrowManager::EscrowManager(
        std::shared_ptr<ILogger> logger,
        std::shared_ptr<CashAcceptorSample> device)
        : logger_(std::move(logger))
        , device_(std::move(device))
    {
        EnsureCashInStatus();
        LoadTransactionStatus();
    }

    void EscrowManager::EnsureCashInStatus()
    {
        cashInStatus_ = device_->GetCashInStatus();

		acceptedItems_ = device_->GetAcceptedItems();

        if (!cashInStatus_)
        {
            cashInStatus_ =
                std::make_shared<XFS4IoTFramework::CashManagement::CashInStatusClass>();

            device_->SetCashInStatus(cashInStatus_);
        }
    }

    EscrowManager::StatusEnum EscrowManager::GetStatus() const
    {
        std::lock_guard lock(mutex_);
        return cashInStatus_->GetStatus();
    }

    bool EscrowManager::IsCashInActive() const
    {
        std::lock_guard lock(mutex_);

        return cashInStatus_
            && cashInStatus_->GetStatus() == StatusEnum::Active;
    }

    std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass>
        EscrowManager::GetCashInStatus() const
    {
        std::lock_guard lock(mutex_);
        return cashInStatus_;
    }

    uint16_t EscrowManager::GetNumOfRefused() const
    {
        std::lock_guard lock(mutex_);
        return cashInStatus_->GetNumOfRefusedItems();
    }

    void EscrowManager::OpenTransaction()
    {
        std::lock_guard lock(mutex_);

        cashInStatus_->SetStatus(StatusEnum::Active);
        cashInStatus_->SetNumOfRefusedItems(0);
        acceptedItems_->clear();

        SaveTransactionStatus();
    }

    void EscrowManager::CloseTransaction(StatusEnum status)
    {
        std::lock_guard lock(mutex_);

        cashInStatus_->SetStatus(status);

        SaveTransactionStatus();
    }

    void EscrowManager::AddRefusedBanknotes(uint16_t count)
    {
        std::lock_guard lock(mutex_);

        cashInStatus_->SetNumOfRefusedItems(cashInStatus_->GetNumOfRefusedItems() + 1);

        SaveTransactionStatus();
    }

    void EscrowManager::SaveTransactionStatus() const
    {
        PersistentDatasHandler::GetInstance()->setCashInTransactionStatus(
            static_cast<int>(cashInStatus_->GetStatus()));

        PersistentDatasHandler::GetInstance()->setCashInNumOfRefused(cashInStatus_->GetNumOfRefusedItems());

         nlohmann::json acceptedItemsJson = nlohmann::json::object();
         for (const auto& [cashItemId, count] : *acceptedItems_)
         {
             acceptedItemsJson[cashItemId] = count;
         }
        
         PersistentDatasHandler::GetInstance()->setCashInAcceptedItems(acceptedItemsJson);
    }

    void EscrowManager::LoadTransactionStatus()
    {
        std::lock_guard lock(mutex_);

        auto* persistent = PersistentDatasHandler::GetInstance();

        auto status = static_cast<StatusEnum>(
            persistent->getCashInTransactionStatus(true));

        if (status == StatusEnum::Active)
        {
            status = StatusEnum::Unknown;

            persistent->setCashInTransactionStatus(
                static_cast<int>(status));
        }

        cashInStatus_->SetStatus(status);

        cashInStatus_->SetNumOfRefusedItems(persistent->getCashInNumOfRefused(true));


		auto unrecognized = persistent->getCashInUnrecognized(true);
        std::map<std::string, XFS4IoTFramework::Storage::CashItemCountClass> itemCounts;
		auto itemCountsJson = persistent->getCashInCashItemCount(true);

        if (itemCountsJson.is_object())
         {
             for (const auto& [cashItemId, value] : itemCountsJson.items())
             {
                 if (!value.is_object())
                 {
                     continue;
                 }
                 itemCounts.emplace(
                     cashItemId,
                     value.get<XFS4IoTFramework::Storage::CashItemCountClass>());
             }
		}

		auto cashCounts = std::make_shared<XFS4IoTFramework::Storage::StorageCashCountClass>(unrecognized, itemCounts);
        cashInStatus_->SetCashCounts(cashCounts);

        acceptedItems_->clear();

        const auto acceptedItemsJson =
            persistent->getCashInAcceptedItems(true);

        if (acceptedItemsJson.is_object())
        {
            for (const auto& [cashItemId, value] : acceptedItemsJson.items())
            {
                if (!value.is_object())
                {
                    continue;
                }

                acceptedItems_->emplace(
                    cashItemId,
                    value.get<XFS4IoTFramework::Storage::CashItemCountClass>());
            }
        }
    }
}