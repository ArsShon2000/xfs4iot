/**
 * @file XFS3_ExecuteSetCashInLimit.hpp
 * @brief Обработчик команды установки лимита приёма наличных (Set Cash-In Limit) для CIM провайдера DORS.
 *
 * Содержит определение класса `XFS3_ExecuteSetCashInLimit`, реализующего логику проверки
 * условий выполнения команды, сохранения лимитов в обработчике и подготовки результата.
 */
#pragma once
#include "PatternInterfaces/InterfaceDorsCommands/IDorsCommands.h"
#include "ComTransport/DorsPSHandler.h"

using namespace Interface::DorsRequest;
using namespace Xfs::Cim;

/**
 * @class XFS3_ExecuteSetCashInLimit
 * @brief Класс-обработчик команды WFS_CMD_CIM_SET_CASH_IN_LIMIT.
 *
 * Класс выполняет валидацию условий выполнения команды, сохраняет предоставленные
 * лимиты приёма в `DorsPSHandler` и при необходимости подготавливает данные для ответа.
 */
class XFS3_ExecuteSetCashInLimit : public IDorsCommands
{
public:
    /**
     * @brief Конструктор класса.
     * @param handler Ссылка на `DorsPSHandler`, используемый для взаимодействия с устройством и хранения состояний.
     * @param m_limit Структура `CimCashinLimit`, содержащая лимиты приёма (лимит суммы, лимит по количеству и т.д.).
     */
    explicit XFS3_ExecuteSetCashInLimit(DorsPSHandler& handler, const CimCashinLimit& m_limit)
        : m_psHandler(handler)
        , m_limit(m_limit)
    {
    }

    /**
     * @brief Заполняет структуру результата выполнения команды.
     * @param lppWfsResult Указатель на указатель на результат `LPWFSRESULT`, который необходимо заполнить.
     *
     * Метод выполняет предварительную валидацию условий вызова команды и в случае ошибки
     * записывает соответствующий код ошибки в `(*lppWfsResult)->hResult`.
     */
    void fillResult(LPWFSRESULT* lppWfsResult) override;

    /**
     * @brief Выполняет основную логику команды.
     * @param lpWfsResult Указатель на структуру результата `LPWFSRESULT`, которая будет заполнена в ходе выполнения.
     *
     * Метод копирует предоставленные лимиты в поля обработчика `DorsPSHandler` и при необходимости
     * использует информацию о валютных экспонентах для валидации/логирования.
     */
    void doBasicLogic(LPWFSRESULT lpWfsResult) override;


private:
    /**
     * @brief Ссылка на обработчик `DorsPSHandler`.
     *
     * Объект предоставляет доступ к состояниям устройства, журналированию и вспомогательным менеджарам.
     */
    DorsPSHandler& m_psHandler;

    /**
     * @brief Лимиты приёма наличных, переданные в команду.
     *
     * Поле содержит структуру `CimCashinLimit`, включающую ограничения по сумме и/или по количеству принятых
     * элементов в рамках операции приёма наличных.
     */
    CimCashinLimit m_limit;

    /**
     * @brief Проверяет предварительные условия для выполнения команды.
     * @return HRESULT Возвращает `WFS_SUCCESS` при успешной проверке, в противном случае код ошибки XFS.
     *
     * Проверки включают:
     * - корректность конфигурации ПО;
     * - инициализацию устройства;
     * - отсутствие активного режима обмена (exchange);
     * - корректную последовательность операций (наличие активной CASH_IN транзакции и пр.);
     * - доступность разрешения на установку лимита (`m_bSetCashInLimitAvailable`).
     */
    HRESULT ValidateProcessingConditions();
};
