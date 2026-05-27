#pragma once
#include "PatternInterfaces/InterfaceDorsCommands/IDorsCommands.h"
#include "ComTransport/DorsPSHandler.h"
#include <boost/scope_exit.hpp>
#include <memory>
#include <mutex>

using namespace Interface::DorsRequest;
using namespace Xfs::Cim;
using namespace STRUCT;


class XFS3_ExecuteConjointCashIn : public IDorsCommands
{
public:
    explicit XFS3_ExecuteConjointCashIn(IHandler& handler);

    void InterruptRequest() override;

protected:
    void fillResult(LPWFSRESULT* lppWfsResult) override;
    void doBasicLogic(LPWFSRESULT lpWfsResult) override;

private:
    HRESULT ValidateProcessingConditions();
    HRESULT DoCancelation();

    void AddNote(LPWFSRESULT& lpWfsResult);

    bool CheckLimitAndMaybeSendInsertItems(
        std::weak_ptr<StateMachine::BlockedWaitTermination> asyncTerminator,
        bool isNeedOneBill,
        bool& limitReached);

    // Подписки вынесены в отдельные функции:
    void SubscribeForEscrowEvents(
        std::mutex& mtx,
        std::weak_ptr<StateMachine::BlockedWaitTermination> asyncTerminator,
        bool& isNeedOneBill,
        LPWFSRESULT& lpWfsResult);

    void SubscribeForStackedTransition(std::mutex& mtx, LPWFSRESULT& lpWfsResult, bool& bProtectionFromPreviousReturned);

    void SubscribeForReturnProtection(std::mutex& mtx, bool& protectionFlag);

    void SubscribeForInsertItemsEvents(
        std::mutex& mtx,
        std::weak_ptr<StateMachine::BlockedWaitTermination> asyncTerminator,
        bool& isNeedOneBill,
        bool& limitReached);

    void SubscribeForFeedModuleError(std::mutex& mtx);

    void SetBit(BYTE n, uint32_t& Val);

    IHandler& m_iHandler; /**< Ссылка на обработчик PS (внешне управляемый). */
    /** @brief Защитник подписок на события стейт‑машины. */
    SubscriptionGuard m_guard;
    /**
     * @brief Слабая ссылка на асинхронный терминатор ожидания событий стейт‑машины.
     *
     * Используется для отмены BlockedWait, если требуется прервать ожидание
     * (например, при отмене запроса клиентом).
     */
    std::weak_ptr<StateMachine::BlockedWaitTermination> m_p_async_terminator;
    // Уникальный номер состояния на момент выхода из очередного BlockedWait
    uint64_t m_lastProcessedStateUID{ 0 };

    CimNoteTypeList banknotesTypes;

    // Номинал последней обработанной банкноты
    uint16_t m_usNoteIDAccepted{ 0 };
    bool m_bHaveUnknownBanknoteJammed{ false };
    // Лимит по приему достигнут
    bool m_bLimitReached{ false };
    // Причина внешней отмены
    bool m_bCanceledByTimeout{ false };
    // Идёт отслеживание складирования банкноты
    bool m_bStackInProgress{ false };
};
