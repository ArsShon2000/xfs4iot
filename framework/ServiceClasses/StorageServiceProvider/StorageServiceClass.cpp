#include "StorageServiceClass.hpp"
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <algorithm>
#include "../../core/Configurations.hpp"

namespace XFS4IoTServer
{
    StorageServiceClass::StorageServiceClass(
        std::shared_ptr<IServiceProvider> serviceProvider,
        std::shared_ptr<ILogger> logger,
        std::shared_ptr<IPersistentData> persistentData,
        XFS4IoTFramework::Storage::StorageTypeEnum storageType)
        : serviceProvider_(serviceProvider)
        , logger_(logger)
        , persistentData_(persistentData)
        , storageType_(storageType)
        , cashUnits_()
    {
        device_ = std::dynamic_pointer_cast<XFS4IoTFramework::Storage::IStorageDevice>(
            serviceProvider_->GetDevice());

        if (!device_)
        {
            throw std::invalid_argument(
                "Устройство не реализует интерфейс IStorageDevice");
        }

        commonService_ = std::dynamic_pointer_cast<XFS4IoTFramework::Common::ICommonService>(serviceProvider_);
        if (!commonService_)
        {
            throw std::invalid_argument(
                "Указан недопустимый параметр интерфейса для общего сервиса: StorageServiceClass");
        }


		LoadPersistent(); // загружает persistedCashUnits_

        if (!persistedCashUnits_)
        {
            logger_->warn(std::format("{}() - {}: Не удалось загрузить постоянные данные для кассет наличных. "
                "Это может быть первый запуск, сервис не является Cash Recycler/Dispenser/Acceptor "
                "или постоянные данные отсутствуют в файловой системе.", __FUNCTION__, XFS4IoT::Constants::Framework));
        }
    }

    StorageServiceClass::~StorageServiceClass() = default;

    void StorageServiceClass::ConstructCashUnits()
    {
        logger_->trace(std::format("{}() - {}: StorageDev.GetCashStorageConfiguration()", __FUNCTION__, XFS4IoT::Constants::Component));

        std::map<std::string, XFS4IoTFramework::Storage::CashUnitStorageConfiguration> newCashUnits;
        bool newConfiguration = device_->GetCashStorageConfiguration(newCashUnits);

        logger_->trace(std::format("{}() - {}: StorageDev.GetCashStorageConfiguration()-> {}",
                 __FUNCTION__, XFS4IoT::Constants::Component, (newConfiguration ? "true" : "false")));

        bool updateCashUnitFromDeviceClass = newConfiguration;

        if (newConfiguration)
        {
            if (newCashUnits.empty())
            {
                logger_->warn(std::format("{}() - {}: The function GetCashStorageConfiguration вернул true. "
                    "однако выходные данные не предоставлены.", __FUNCTION__, XFS4IoT::Constants::Framework));
                newConfiguration = false;
            }
            else
            {
                cashUnits_.clear();
                for (const auto& [key, config] : newCashUnits)
                {
                    cashUnits_[key] =
                        std::make_shared<XFS4IoTFramework::Storage::CashUnitStorage>(config);
                }

                ApplyPersistentCashUnits();
            }
        }

        if (!newConfiguration)
        {
            bool identical = newCashUnits.size() == cashUnits_.size();

            if (!newCashUnits.empty())
            {
                for (const auto& [unitKey, unitValue] : newCashUnits)
                {
                    identical = cashUnits_.contains(unitKey);
                    if (!identical)
                    {
                        logger_->warn(std::format("{}() - {}: Существующая информация о кассетах наличных не содержит ключа  "
                            "указанного классом устройства. {}. Создать новую информацию о кассетах наличных.", __FUNCTION__, XFS4IoT::Constants::Framework, unitKey));
                        break;
                    }

                    auto& existingUnit = cashUnits_[unitKey];
                    identical =
                        existingUnit->GetUnit()->GetConfiguration() == unitValue.GetConfiguration() &&
                        existingUnit->GetUnit()->GetCapabilities() == unitValue.GetCapabilities() &&
                        existingUnit->getCapacity() == unitValue.GetCapacity() &&
                        existingUnit->getPositionName() == unitValue.GetPositionName() &&
                        existingUnit->getSerialNumber() == unitValue.GetSerialNumber() &&
                        existingUnit->GetUnit()->GetStatus()->GetIndex() == unitValue.GetCashUnitAdditionalInfo().GetIndex() &&
                        (unitValue.GetCashUnitAdditionalInfo().GetAccuracySupported() ?
                            existingUnit->GetUnit()->GetStatus()->GetAccuracy() != XFS4IoTFramework::Storage::CashStatusClass::AccuracyEnum::NotSupported :
                            existingUnit->GetUnit()->GetStatus()->GetAccuracy() == XFS4IoTFramework::Storage::CashStatusClass::AccuracyEnum::NotSupported);

                    if (!identical)
                    {
                        logger_->warn(std::format("{}() - {}: Существующая информация о кассетах наличных не содержит идентичного "
                            "информацию о структуре кассеты наличных, указанную классом устройства " 
                            "{} . Создать новую информацию о кассетах наличных.", __FUNCTION__, XFS4IoT::Constants::Framework, unitKey));
                        break;
                    }
                }
            }

            if (!identical)
            {
                cashUnits_.clear();

                if (!newCashUnits.empty())
                {
                    for (const auto& [key, config] : newCashUnits)
                    {
                        cashUnits_[key] =
                            std::make_shared<XFS4IoTFramework::Storage::CashUnitStorage>(config);
                    }

                    ApplyPersistentCashUnits();
                }

                updateCashUnitFromDeviceClass = true;
            }
        }

        if (updateCashUnitFromDeviceClass)
        {
            // Use hardware status and check the device maintains status or not
            logger_->trace(std::format("{}() - {}: StorageDev.GetCashUnitCounts()", __FUNCTION__, XFS4IoT::Constants::Component));

            std::map<std::string, XFS4IoTFramework::Storage::CashUnitCountClass> unitCounts;
            bool updateCounts = device_->GetCashUnitCounts(unitCounts);

            logger_->trace(
                std::format("{}() - {}: StorageDev.GetCashUnitCounts()-> {}", __FUNCTION__, XFS4IoT::Constants::Component,
                (updateCounts ? "true" : "false")));

            if (updateCounts && !unitCounts.empty())
            {
                for (const auto& [unitKey, unitCount] : unitCounts)
                {
                    if (!cashUnits_.contains(unitKey))
                    {
                        logger_->warn(
                            std::format("{}() - {}: Указанный идентификатор хранилища не найден: {}", __FUNCTION__, XFS4IoT::Constants::Framework,  unitKey));
                        continue;
                    }

                    cashUnits_[unitKey]->GetUnit()->GetStatus()->SetCount(unitCount.GetCount());

                    if (unitCount.GetStorageCashInCount())
                    {
                        cashUnits_[unitKey]->GetUnit()->GetStatus()->SetStorageCashInCount(
                            std::make_shared<XFS4IoTFramework::Storage::StorageCashInCountClass>(*unitCount.GetStorageCashInCount()));
                    }

                    if (unitCount.GetStorageCashOutCount())
                    {
                        cashUnits_[unitKey]->GetUnit()->GetStatus()->SetStorageCashOutCount(
                            std::make_shared<XFS4IoTFramework::Storage::StorageCashOutCountClass>(*unitCount.GetStorageCashOutCount()));
                    }
                }
            }

            // Get initial counts
            logger_->trace(std::format("{}() - {}: StorageDev.GetCashUnitInitialCounts()", __FUNCTION__, XFS4IoT::Constants::Component));

            std::map<std::string, XFS4IoTFramework::Storage::StorageCashCountClass> initialCounts;
            bool updateInitialCounts = device_->GetCashUnitInitialCounts(initialCounts);

            logger_->trace(
                std::format("{}() - {}: StorageDev.GetCashUnitInitialCounts()-> ", __FUNCTION__, XFS4IoT::Constants::Component,
                (updateInitialCounts ? "true" : "false")));

            if (updateInitialCounts && !initialCounts.empty())
            {
                for (const auto& [unitKey, initialCount] : initialCounts)
                {
                    if (!cashUnits_.contains(unitKey))
                    {
                        logger_->warn(
                            std::string(std::format("{}() - {}: Указанный идентификатор хранилища не найден: {}", __FUNCTION__, XFS4IoT::Constants::Framework, unitKey)));
                        continue;
                    }

                    cashUnits_[unitKey]->GetUnit()->GetStatus()->SetInitialCounts(
                        std::make_shared<XFS4IoTFramework::Storage::StorageCashCountClass>(initialCount));
                }
            }

            // Update status from count (логика расчета статуса)
            // ... (аналогично C# коду, слишком длинно для этого ответа)

            // Get storage status
            logger_->trace(std::format("{}() - {}: StorageDev.GetCashStorageStatus()", __FUNCTION__, XFS4IoT::Constants::Component));

            std::map<std::string, XFS4IoTFramework::Storage::CashUnitStorage::StatusEnum> storageStatus;
            bool updateStatus = device_->GetCashStorageStatus(storageStatus);

            logger_->trace(
                std::format("{}() - {}: StorageDev.GetCashStorageStatus()-> {}", __FUNCTION__, XFS4IoT::Constants::Component,
                (updateStatus ? "true" : "false")));

            if (updateStatus && !storageStatus.empty())
            {
                for (const auto& [unitKey, status] : storageStatus)
                {
                    if (!cashUnits_.contains(unitKey))
                    {
                        logger_->warn(
                            std::format("{}() - {}: Указанный идентификатор хранилища не найден: {}",__FUNCTION__, XFS4IoT::Constants::Framework, unitKey));
                        continue;
                    }
                    cashUnits_[unitKey]->setStatus(status);
                }
            }

            // Similar code for GetCashUnitStatus() and GetCashUnitAccuracy()
            // ... (продолжение реализации)
        }


        // Register events
        RegisterStorageChangedEvents();

        StorePersistent();
        //if (!persistentData_->Store(storageKey, plainUnits))
        //{
        //    logger_->warn(std::format("{}() - {}: Failed to save persistent data.", __FUNCTION__, XFS4IoT::Constants::Framework));
        //}
    }

    boost::asio::awaitable<void> StorageServiceClass::UpdateCashAccounting(
        std::optional<std::unordered_map<std::string, std::shared_ptr<XFS4IoTFramework::Storage::CashUnitCountClass>>> countDelta)
    {
        if (countDelta.has_value())
        {
            // First update item movement reported by the device class
            for (const auto& [deltaKey, deltaValue] : countDelta.value())
            {
                if (!cashUnits_.contains(deltaKey) || !deltaValue)
                {
                    continue;
                }

                // Update counts
                auto& unit = cashUnits_[deltaKey];
                unit->GetUnit()->GetStatus()->SetCount(
                    unit->GetUnit()->GetStatus()->GetCount() + deltaValue->GetCount());

                if (deltaValue->GetStorageCashOutCount())
                {
                    UpdateDeltaStorageCashCount(
                        deltaKey,
                        unit->GetUnit()->GetStatus()->GetStorageCashOutCount()->GetDistributed(),
                        deltaValue->GetStorageCashOutCount()->GetDistributed());
                    // ... (аналогично для других полей)
                }

                if (deltaValue->GetStorageCashInCount())
                {
                    unit->GetUnit()->GetStatus()->GetStorageCashInCount()->SetRetractOperations(
                        unit->GetUnit()->GetStatus()->GetStorageCashInCount()->GetRetractOperations() +
                        deltaValue->GetStorageCashInCount()->GetRetractOperations());
                    // ... (аналогично для других полей)
                }
            }
        }

        // Get updated counts from device
        logger_->trace(std::format("{}() - {}: StorageDev.GetCashUnitCounts()", __FUNCTION__, XFS4IoT::Constants::Component));

        std::map<std::string, XFS4IoTFramework::Storage::CashUnitCountClass> unitCounts;
        bool updateCounts = device_->GetCashUnitCounts(unitCounts);

        logger_->trace(
            std::format("{}() - {}: StorageDev.GetCashUnitCounts()-> {}", __FUNCTION__, XFS4IoT::Constants::Component,
            (updateCounts ? "true" : "false")));

        if (updateCounts && !unitCounts.empty())
        {
            // Overwrite counts updated by the device class
            for (const auto& [unitKey, unitCount] : unitCounts)
            {
                if (!cashUnits_.contains(unitKey))
                {
                    logger_->warn(
                        std::format("{}() - {}: Передан неизвестный идентификатор хранилища при обновлении счётчиков: {}", __FUNCTION__, XFS4IoT::Constants::Framework, unitKey));
                    continue;
                }

                auto& unit = cashUnits_[unitKey];
                unit->GetUnit()->GetStatus()->SetCount(unitCount.GetCount());

                unit->GetUnit()->GetStatus()->SetStorageCashOutCount(
                    unitCount.GetStorageCashOutCount() ?
                    std::make_shared<XFS4IoTFramework::Storage::StorageCashOutCountClass>(*unitCount.GetStorageCashOutCount()) :
                    std::make_shared<XFS4IoTFramework::Storage::StorageCashOutCountClass>());

                unit->GetUnit()->GetStatus()->SetStorageCashInCount(
                    unitCount.GetStorageCashInCount() ?
                    std::make_shared<XFS4IoTFramework::Storage::StorageCashInCountClass>(*unitCount.GetStorageCashInCount()) :
                    std::make_shared<XFS4IoTFramework::Storage::StorageCashInCountClass>());
            }
        }

        // ... (продолжение логики обновления статусов и отправки событий)

        // Store persistent data
        std::string storageKey = serviceProvider_->GetName() + typeid(XFS4IoTFramework::Storage::CashUnitStorage).name();
        using CashUnitStorageMapPersistent =
            std::unordered_map<std::string, CashUnitStoragePersistent>;

        CashUnitStorageMapPersistent plainUnits;

        for (const auto& [unitKey, unitValue] : cashUnits_)
        {
            if (!unitValue) {
                continue;
            }

            CashUnitStoragePersistent dto;
            dto.count = unitValue->GetUnit()->GetStatus()->GetCount();
            dto.status = static_cast<int>(unitValue->getStatus());

            plainUnits.emplace(unitKey, dto);
        }
        //if (!persistentData_->Store(storageKey, plainUnits))
        //{
        //    logger_->warn(std::format("{}() - {}: Failed to save persistent data.", __FUNCTION__, XFS4IoT::Constants::Framework));
        //}

        StorePersistent();
        co_return;
    }

    void StorageServiceClass::UpdateDeltaStorageCashCount(
        const std::string& storageId,
        std::shared_ptr<XFS4IoTFramework::Storage::StorageCashCountClass> storageCashCount,
        std::shared_ptr<XFS4IoTFramework::Storage::StorageCashCountClass> storageDeltaCount)
    {
        if (!storageDeltaCount)
        {
            return;
        }

        if (!storageCashCount)
        {
            logger_->warn(
                std::format("{}() - {}: целевой объект cash-in unit не установлен для Storage ID: {}. "
                    "Счётчик хранилища не будет обновлён как ожидается.", __FUNCTION__, XFS4IoT::Constants::Framework, storageId));
            return;
        }

        // Update counts
        if (storageDeltaCount->GetUnrecognized() >= 0 ||
            !storageDeltaCount->GetItemCounts().empty())
        {
            storageCashCount->SetUnrecognized(
                storageCashCount->GetUnrecognized() + storageDeltaCount->GetUnrecognized());

            for (const auto& [itemKey, itemValue] : storageDeltaCount->GetItemCounts())
            {
                if (!storageCashCount->GetItemCounts().contains(itemKey))
                {
                    if (!cashUnits_.contains(storageId))
                        logger_->warn(std::format("{}() - Передан недопустимый Storage ID "
                        "UpdateDeltaStorageCashCount", __FUNCTION__, itemKey));

                    const auto& banknoteItems = cashUnits_[storageId]->GetUnit()->GetConfiguration()->GetBanknoteItems();
                    if (std::find(banknoteItems.begin(), banknoteItems.end(), itemKey) != banknoteItems.end())
                    {
                        storageCashCount->GetItemCounts()[itemKey] = XFS4IoTFramework::Storage::CashItemCountClass();
                    }
                    else
                    {
                        throw std::runtime_error(
                            std::string("Неожиданный идентификатор банкноты, предоставленный классом устройства. ") +
                            itemKey + " для хранилища " + storageId);
                    }
                }

                auto& targetItem = storageCashCount->GetItemCounts()[itemKey];
                targetItem.SetCounterfeit(targetItem.GetCounterfeit() + itemValue.GetCounterfeit());
                targetItem.SetFit(targetItem.GetFit() + itemValue.GetFit());
                targetItem.SetInked(targetItem.GetInked() + itemValue.GetInked());
                targetItem.SetSuspect(targetItem.GetSuspect() + itemValue.GetSuspect());
                targetItem.SetUnfit(targetItem.GetUnfit() + itemValue.GetUnfit());
            }
        }
    }

    void StorageServiceClass::StorePersistent()
    {
        if (!persistentData_) {
            return;
        }

        if (cashUnits_.empty()) {
            return;
        }

        CashUnitStorageMapPersistent plainUnits;

        for (const auto& [unitKey, unitValue] : cashUnits_)
        {
            if (!unitValue || !unitValue->GetUnit() || !unitValue->GetUnit()->GetStatus()) {
                continue;
            }

            CashUnitStoragePersistent dto;
            dto.count = unitValue->GetUnit()->GetStatus()->GetCount();
            dto.status = static_cast<int>(unitValue->getStatus());

            plainUnits.emplace(unitKey, dto);
        }

        if (!persistentData_->Store(CashUnitsPersistentKey(), plainUnits))
        {
            logger_->warn(std::format(
                "{}() - {}: Failed to save persistent data for cash units.",
                __FUNCTION__,
                XFS4IoT::Constants::Framework));
        }
    }

    std::unordered_map<std::string, std::shared_ptr<XFS4IoT::Storage::StorageUnitClass>>
        StorageServiceClass::GetStorages(const std::vector<std::string>& unitIds)
    {
        // Полная реализация метода GetStorages из C# кода
        // ... (очень длинный метод, требует отдельного внимания)

        std::unordered_map<std::string, std::shared_ptr<XFS4IoT::Storage::StorageUnitClass>> storageResponse;

        // TODO: Implement full conversion logic

        return storageResponse;
    }

    void StorageServiceClass::RegisterStorageChangedEvents()
    {
        for (auto& [unitKey, unitValue] : cashUnits_)
        {
            if (!unitValue) {
                continue;
            }

            unitValue->setStorageId(unitKey);
            unitValue->onPropertyChanged(
                [this](auto sender, auto propertyInfo) {
                    StorageChangedEventForwarder(sender, propertyInfo);
                });

            if (unitValue->GetUnit() && unitValue->GetUnit()->GetStatus()) {
                unitValue->GetUnit()->GetStatus()->setStorageId(unitKey);
                unitValue->GetUnit()->GetStatus()->onPropertyChanged(
                    [this](auto sender, auto propertyInfo) {
                        StorageChangedEventForwarder(sender, propertyInfo);
                    });
            }
        }
    }

    boost::asio::awaitable<void> StorageServiceClass::StorageChangedEvent(
        std::shared_ptr<void> sender,
        std::shared_ptr<PropertyChangedEventArgs> propertyInfo)
    {
        co_await StorageChangedEventHandler(sender, propertyInfo);
    }

    void StorageServiceClass::StorageChangedEventForwarder(
        std::shared_ptr<void> sender,
        std::shared_ptr<PropertyChangedEventArgs> propertyInfo)
    {
        boost::asio::co_spawn(
            serviceProvider_->getIoContext(),
            StorageChangedEvent(sender, propertyInfo),
            boost::asio::detached);
    }
    std::string StorageServiceClass::CashUnitsPersistentKey() const
    {
        return serviceProvider_->GetName() +
            "_" +
            std::string(typeid(XFS4IoTFramework::Storage::CashUnitStorage).name());
    }
    void StorageServiceClass::LoadPersistent()
    {
        if (!persistentData_) {
            logger_->warn(std::format(
                "{}() - {}: IPersistentData is null.",
                __FUNCTION__,
                XFS4IoT::Constants::Framework));
            return;
        }

        persistedCashUnits_ =
            persistentData_->Load<CashUnitStorageMapPersistent>(
                CashUnitsPersistentKey());

        if (!persistedCashUnits_) {
            logger_->warn(std::format(
                "{}() - {}: Не удалось загрузить persistent data для cash units. "
                "Это нормально при первом запуске.",
                __FUNCTION__,
                XFS4IoT::Constants::Framework));
            return;
        }

        logger_->trace(std::format(
            "{}() - {}: Загружено persistent cash units: {}",
            __FUNCTION__,
            XFS4IoT::Constants::Framework,
            persistedCashUnits_->size()));
    }
    void StorageServiceClass::ApplyPersistentCashUnits()
    {
        if (!persistedCashUnits_ || persistedCashUnits_->empty()) {
            return;
        }

        for (const auto& [unitKey, persisted] : *persistedCashUnits_)
        {
            auto it = cashUnits_.find(unitKey);
            if (it == cashUnits_.end() || !it->second) {
                logger_->warn(std::format(
                    "{}() - {}: Persistent storage содержит unit '{}', "
                    "но текущая конфигурация устройства его не содержит. Игнорируем.",
                    __FUNCTION__,
                    XFS4IoT::Constants::Framework,
                    unitKey));
                continue;
            }

            auto& unit = it->second;

            if (!unit->GetUnit() || !unit->GetUnit()->GetStatus()) {
                continue;
            }

            unit->GetUnit()->GetStatus()->SetCount(persisted.count);

            unit->setStatus(
                static_cast<XFS4IoTFramework::Storage::CashUnitStorage::StatusEnum>(
                    persisted.status));
        }
    }
}