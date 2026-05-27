/**
 * @file XFS3_ExecuteExchangeEnd.hpp
 * @brief Реализация команды завершения режима обмена для купюроприёмника DORS.
 */
#pragma once
#include "PatternInterfaces/InterfaceDorsCommands/IDorsCommands.h"
#include "ComTransport/DorsPSHandler.h"
#include "MODULE_STRUCTURES/CimCashUnit/CimCashUnit.hpp"

using namespace Interface::DorsRequest;
using namespace Xfs::Cim;

/**
 * @class XFS3_ExecuteExchangeEnd
 * @brief Класс для выполнения команды завершения режима обмена (Exchange End).
 *
 * Осуществляет завершение режима обмена, сброс внутренних счетчиков,
 * обновление информации о кассетах и отправку соответствующих событий.
 */
class XFS3_ExecuteExchangeEnd : public IDorsCommands
{
public:
    /**
     * @brief Конструктор с инициализацией обработчика и параметров кассет.
     * @param handler Ссылка на обработчик DorsPSHandler
     * @param params Информация о кассетах (CimCashUnitInfo)
     */
    explicit XFS3_ExecuteExchangeEnd(DorsPSHandler& handler, const CimCashUnitInfo& params)
        : m_psHandler(handler)
        , m_params(params)
    {
    }

    /**
     * @brief Заполняет структуру результата выполнения команды.
     * @param lppWfsResult Указатель на структуру результата WFSRESULT
     */
    void fillResult(LPWFSRESULT* lppWfsResult) override;

    /**
     * @brief Выполняет основную логику завершения режима обмена.
     * @param lpWfsResult Указатель на структуру результата WFSRESULT
     */
    void doBasicLogic(LPWFSRESULT lpWfsResult) override;

private:
    /**
     * @brief Ссылка на обработчик команд DORS.
     */
    DorsPSHandler& m_psHandler;

    /**
     * @brief Информация о кассетах, переданная в команду.
     */
    CimCashUnitInfo m_params;

    /**
     * @brief Проверяет условия для выполнения команды.
     * @return HRESULT-код результата проверки
     */
    HRESULT ValidateProcessingConditions();
};
