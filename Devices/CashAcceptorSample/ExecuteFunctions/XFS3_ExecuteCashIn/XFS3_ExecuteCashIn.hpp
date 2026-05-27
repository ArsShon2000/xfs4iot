#pragma once

#include "PatternInterfaces/InterfaceDorsCommands/IDorsCommands.h"
#include "ComTransport/DorsPSHandler.h"
#include <memory>
#include "XFSCommon/StateMachine.hpp"

using namespace Interface::DorsRequest;
using namespace Xfs::Cim;

/**
 * @file XFS3_ExecuteCashIn.hpp
 * @brief Реализация обработчика команды WFS_CMD_CIM_CASH_IN (Execute CashIn).
 *
 * Заголовочный файл содержит объявление класса `XFS3_ExecuteCashIn`, который
 * реализует интерфейс `IDorsCommands` и отвечает за обработку сценариев приёма
 * наличности (CASH_IN) в рамках XFS3 провайдера. Класс инкапсулирует логику
 * валидации условий, запуска основного обработчика и формирования результата
 * для клиента.
 */

/**
 * @class XFS3_ExecuteCashIn
 * @brief Обработчик Execute-команды CASH_IN для XFS3.
 *
 * Класс реализует следующие этапы обработки команды:
 * - проверка условий для начала приёма (ValidateProcessingConditions),
 * - подготовка и заполнение результата (fillResult),
 * - выполнение основной логики приёма банкнот (doBasicLogic).
 *
 * Объект хранит ссылку на `DorsPSHandler`, через который взаимодействует с
 * низкоуровневым оборудованием и вспомогательными менеджерами (escrow,
 * inhibit и пр.). Для возможности отмены ожидания событий используется
 * асинхронный терминатор `m_p_async_terminator`.
 */
class XFS3_ExecuteCashIn : public IDorsCommands
{
public:
    /**
     * @brief Конструктор обработчика.
     * @param handler Ссылка на `DorsPSHandler` — обработчик устройства (внешняя владение).
     */
    explicit XFS3_ExecuteCashIn(IHandler& handler)
    : m_iHandler(handler)
    , m_guard(dynamic_cast<DorsPSHandler&>(this->m_iHandler).m_stateMachine)
    {
    }

    /**
     * @brief Заполнить структуру результата команды (LPWFSRESULT).
     *
     * Вызывается до выполнения основной логики и должна поместить код
     * результата в `lppWfsResult` в случае ошибок валидации или подготовки.
     *
     * @param lppWfsResult Указатель на указатель структуры WFSRESULT, где
     *        нужно записать результат выполнения команды.
     */
    void fillResult(LPWFSRESULT* lppWfsResult) override;

    /**
     * @brief Выполнить основную логику команды (приём банкноты, ожидание событий).
     *
     * Этот метод содержит основную последовательность действий по приёму
     * банкноты: включение приёма, подписки на события стейт‑машины, ожидание
     * нужных переходов и формирование итогового ответа клиенту.
     *
     * @param lpWfsResult Указатель на структуру результата, которая будет
     *        заполнена в процессе выполнения.
     */
    void doBasicLogic(LPWFSRESULT lpWfsResult) override;

    /**
     * @brief Проверить условия для начала обработки CASH_IN.
     *
     * Производит базовую валидацию: инициализация устройства, отсутствие
     * конфликтующих операций (exchange, отсутствие кассет и т.д.), активность
     * транзакции escrow и лимитов приёма.
     *
     * @return HRESULT - код XFS: WFS_SUCCESS при возможности начать операцию,
     *         иначе соответствующий код ошибки.
     */
    HRESULT ValidateProcessingConditions();


    /** * @brief Прервать текущее ожидание событий стейт‑машины.
     *
     * Вызывает метод `AsyncCancel` у асинхронного терминатора ожидания,
     * что приводит к прерыванию блокирующего ожидания в `doBasicLogic`.
     */
    void InterruptRequest() override
    {
        if (auto p = m_p_async_terminator.lock())
        {
            p->AsyncCancel();
        }
    }

private:
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

    /**
     * @brief Установить соответствующий бит в маске набора поддерживаемых номиналов.
     * @param n Номер бита (идентификатор номинала).
     * @param Val Ссылка на целевую маску, в которую устанавливается бит.
     */
    void SetBit(BYTE n, uint32_t& Val);
};
