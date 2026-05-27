#pragma once
#include "PatternInterfaces/InterfaceDorsCommands/IDorsCommands.h"
#include "ComTransport/DorsPSHandler.h"

using namespace Interface::DorsRequest;
using namespace Xfs::Cim;

/**
 * @file XFS3_ExecuteCashInEnd.hpp
 * @brief Обработчик завершения операции CASH_IN (WFS_CMD_CIM_CASH_IN_END) для XFS3 провайдера.
 *
 * В этом файле объявлен класс `XFS3_ExecuteCashInEnd`, реализующий интерфейс `IDorsCommands`.
 * Класс отвечает за проверку условий завершения операции CashIn, ожидание событий
 * стейт‑машины, выполнение действий по складированию/возврату банкноты, формирование
 * итоговых XFS структур и завершение транзакции в менеджере escrow.
 */
class XFS3_ExecuteCashInEnd : public IDorsCommands
{
public:
    /**
     * @brief Конструктор обработчика.
     * @param handler Ссылка на `DorsPSHandler` — интерфейс для взаимодействия с оборудованием.
     */
    explicit XFS3_ExecuteCashInEnd(IHandler& handler)
        : m_iHandler(handler)
    {
    }

    /**
     * @brief Подготовить структуру результата запроса.
     *
     * Выполняет предварительную проверку условий (`ValidateProcessingConditions`) и при
     * ошибке записывает соответствующий код ошибки в `lppWfsResult`.
     *
     * @param lppWfsResult Указатель на указатель структуры `WFSRESULT`, в которую нужно записать результат.
     */
    void fillResult(LPWFSRESULT* lppWfsResult) override;

    /**
     * @brief Выполнить основную логику команды CashInEnd.
     *
     * Метод реализует поведение при окончании операции приёма наличности: ожидает
     * события от стейт‑машины (складирование/возврат/ошибка), управляет состоянием
     * трансакции в `EscrowManager`, формирует и возвращает XFS структуры с информацией
     * о принятых банкнотах.
     *
     * @param lpWfsResult Указатель на структуру `WFSRESULT`, которая будет заполнена по результатам выполнения.
     */
    void doBasicLogic(LPWFSRESULT lpWfsResult) override;


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
    IHandler& m_iHandler; /**< Ссылка на обработчик PS (не владеет указателем). */
    /**
   * @brief Слабая ссылка на асинхронный терминатор ожидания событий стейт‑машины.
   *
   * Используется для отмены BlockedWait, если требуется прервать ожидание
   * (например, при отмене запроса клиентом).
   */
    std::weak_ptr<StateMachine::BlockedWaitTermination> m_p_async_terminator;
    /**
     * @brief Проверить условия для выполнения CashInEnd.
     *
     * Проверяет состояние устройства, инициализировано ли устройство, не выполняется ли
     * обмен (exchange), наличие необходимых кассет и активность транзакции escrow.
     *
     * @return HRESULT Код XFS: `WFS_SUCCESS` при возможности завершить операцию, иначе соответствующая ошибка.
     */
    HRESULT ValidateProcessingConditions();
};
