#pragma once
#ifndef __ESCROWMANAGER_H__
#define __ESCROWMANAGER_H__

/**
 * @file EscrowManager.h
 * @brief Менеджер состояния escrow для транзакций CashIn.
 *
 * Файл содержит определение класса `EscrowManager`, который отвечает за хранение
 * и управление данными текущей транзакции CashIn (статус, список номеров банкнот,
 * счётчики отказанных банкнот и дополнительная информация). Класс обеспечивает
 * потокобезопасный доступ к данным транзакции и интеграцию с системой персистентного
 * хранения через `PersistentDatasHandler` (вызовы производятся в реализации).
 */

#include <mutex>
#include <memory>
#include "../../../../framework/core/Logger/ILogger.hpp"
#include "../../../../framework/ServiceClasses/CashManagementServiceProvider/CashInStatusClass.hpp"
#include "../../../../framework/ServiceClasses/StorageServiceProvider/CashUnit.hpp"

/**
 * @class EscrowManager
 * @brief Потокобезопасный менеджер состояния escrow (CashIn transaction manager).
 *
 * `EscrowManager` инкапсулирует данные текущей транзакции CashIn в поле
 * `m_status` (тип `CimCashInStatus`) и предоставляет интерфейс для работы с
 * транзакцией: открытие/закрытие, добавление номеров банкнот, сохранение и
 * загрузка состояния, подсчёт частичного состояния промежуточного стакера и
 * пометка спорных банкнот. Все публичные методы безопасны для вызова из
 * нескольких потоков, т.к. используют внутренний мьютекс `m_mutex`.
 * 
 * status
 * статус текущей transaction
 * после рестарта Active -> Unknown
 * 
 * acceptedItems
 * что уже принято в escrow/current transaction
 * 
 * refusedItems
 * количество rejected/refused банкнот
 * 
 * unfitItems (если поддерживается, нужно уточнить у Мишы)
 * disputed/suspect/counterfeit items (если поддерживается)
 */

namespace XFS4IoTSP::CashAcceptor::Sample
{
    class CashAcceptorSample;

    class EscrowManager final
    {
    public:
        using StatusEnum =
            XFS4IoTFramework::CashManagement::CashInStatusClass::StatusEnum;

        EscrowManager(
            std::shared_ptr<ILogger> logger,
            std::shared_ptr<CashAcceptorSample> device);

        StatusEnum GetStatus() const;
        bool IsCashInActive() const;

        std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass>
            GetCashInStatus() const;

        uint16_t GetNumOfRefused() const;

        void OpenTransaction();
        void CloseTransaction(StatusEnum status);

        void AddRefusedBanknotes(uint16_t count);
        void AddNoteNumberList(XFS4IoTFramework::Storage::StorageCashCountClass& cashUnit);

        void LoadTransactionStatus();
        void SaveTransactionStatus() const;

    private:
        void EnsureCashInStatus();

    private:
        mutable std::mutex mutex_;

        std::shared_ptr<ILogger> logger_;
        std::shared_ptr<CashAcceptorSample> device_;

        std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass>
            cashInStatus_;

        std::shared_ptr<std::map<std::string, XFS4IoTFramework::Storage::CashItemCountClass>>
            acceptedItems_;
    };
}
#endif // __ESCROWMANAGER_H__



