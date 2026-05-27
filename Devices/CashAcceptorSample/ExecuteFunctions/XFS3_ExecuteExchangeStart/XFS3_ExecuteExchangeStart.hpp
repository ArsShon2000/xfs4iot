/**
 * @file XFS3_ExecuteExchangeStart.hpp
 * @brief Обработчик команды начала режима обмена (Exchange Start) для CIM провайдера DORS.
 *
 * Файл содержит определение класса `XFS3_ExecuteExchangeStart`, реализующего логику
 * обработки команды начала обмена кассет. Класс взаимодействует с `DorsPSHandler`
 * для выполнения операций и формирования результата в формате XFS.
 */
#pragma once
#include "PatternInterfaces/InterfaceDorsCommands/IDorsCommands.h"
#include "ComTransport/DorsPSHandler.h"
#include "MODULE_STRUCTURES/CimStartExchange/CimStartExchange.hpp"

using namespace Interface::DorsRequest;
using namespace Xfs::Cim;

/**
 * @class XFS3_ExecuteExchangeStart
 * @brief Класс-обработчик команды WFS_CMD_CIM_EXCHANGE_START.
 *
 * Класс выполняет проверку условий выполнения команды, запускает необходимую
 * инициализацию/сбросы оборудования и формирует результат для клиента XFS.
 */
class XFS3_ExecuteExchangeStart : public IDorsCommands
{
public:
    /**
     * @brief Конструктор класса.
     * @param handler Ссылка на экземпляр `DorsPSHandler`, используемый для взаимодействия с устройством,
     *        логирования и отправки событий в систему.
     * @param params Структура `CimStartExchange` с параметрами команды (тип обмена, список кассет и др.).
     */
    explicit XFS3_ExecuteExchangeStart(DorsPSHandler& handler, const CimStartExchange& params)
        : m_psHandler(handler)
        , m_params(params)
    {
    }

    /**
     * @brief Заполнить структуру результата выполнения команды.
     * @param lppWfsResult Указатель на указатель результата `LPWFSRESULT`, который необходимо заполнить.
     *
     * Метод выполняет валидацию входных условий и в случае ошибки записывает соответствующий
     * код ошибки в `(*lppWfsResult)->hResult`.
     */
    void fillResult(LPWFSRESULT* lppWfsResult) override;

    /**
     * @brief Выполнение основной логики команды.
     * @param lpWfsResult Указатель на структуру результата `LPWFSRESULT`, которая будет заполнена в ходе выполнения.
     *
     * Метод выполняет необходимые аппаратные операции (сбросы, подготовку данных о кассетах),
     * устанавливает флаг `m_bExchangeInProgress` в обработчике и формирует буфер с информацией о кассетах
     * для возврата клиенту XFS.
     */
    void doBasicLogic(LPWFSRESULT lpWfsResult) override;


private:
    /**
     * @brief Ссылка на обработчик `DorsPSHandler`.
     *
     * Объект предоставляет доступ к устройству, логированию, менеджерам состояний
     * и другим вспомогательным методам (например, `Reset`, `ResetCassetteCounters`,
     * `SendCashUnitInfoChanged` и т.д.).
     */
    DorsPSHandler& m_psHandler;

    /**
     * @brief Параметры команды `CimStartExchange`.
     *
     * Содержит тип обмена, идентификатор кассира, список логических номеров кассет
     * и опциональные поля вывода.
     */
    CimStartExchange m_params;

    /**
     * @brief Проверяет предварительные условия для выполнения команды.
     * @return HRESULT Возвращает `WFS_SUCCESS` при успешной проверке, в противном случае
     *         код ошибки XFS (например, `WFS_ERR_DEV_NOT_READY`, `WFS_ERR_CIM_CASHINACTIVE` и т.д.).
     *
     * Проверки включают в себя:
     * - корректность конфигурации ПО;
     * - инициализацию устройства;
     * - отсутствие активной операции приема наличных;
     * - отсутствие уже активного режима обмена;
     * - валидность входных параметров `m_params` (тип обмена и список кассет).
     */
    HRESULT ValidateProcessingConditions();
};
