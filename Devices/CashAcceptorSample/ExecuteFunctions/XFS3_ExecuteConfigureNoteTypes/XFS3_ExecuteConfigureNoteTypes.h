/**
 * @file XFS3_ExecuteConfigureNoteTypes.h
 * @brief Реализация команды конфигурирования типов банкнот для купюроприёмника DORS.
 */
#pragma once
#include "PatternInterfaces/InterfaceDorsCommands/IDorsCommands.h"
#include "CIM.PS/INFO_MODULES/GetinfoCurrencyExp/GetinfoCurrencyExp.h"
#include <ComTransport/DorsPSHandler.h>

using namespace Interface::DorsRequest;
using namespace Xfs::Cim;

/**
 * @class XFS3_ExecuteConfigureNoteTypes
 * @brief Класс для выполнения команды конфигурирования типов банкнот.
 *
 * Позволяет задать список поддерживаемых номиналов банкнот для купюроприёмника,
 * а также осуществляет валидацию и применение настроек через обработчик DorsPSHandler.
 */
class XFS3_ExecuteConfigureNoteTypes : public IDorsCommands
{
public:
    /**
     * @brief Конструктор с инициализацией обработчика.
     * @param handler Ссылка на обработчик DorsPSHandler
     */
    explicit XFS3_ExecuteConfigureNoteTypes(DorsPSHandler& handler)
        : m_psHandler(handler)
    {
    }

    /**
     * @brief Выполняет основную логику завершения режима обмена.
     * @param lpWfsResult Указатель на структуру результата WFSRESULT
     */
    void doBasicLogic(LPWFSRESULT lpWfsResult) override;
    /**
     * @brief Заполняет структуру результата выполнения команды.
     * @param lppWfsResult Указатель на структуру результата WFSRESULT
     */
    void fillResult(LPWFSRESULT* lppWfsResult) override;

    /**
     * @brief Устанавливает обработчик DorsPSHandler.
     * @param handler Ссылка на обработчик
     */
    void SetHandler(DorsPSHandler& handler) noexcept {}

    /**
     * @brief Устанавливает список идентификаторов банкнот для конфигурирования.
     * @param noteIDs Вектор идентификаторов банкнот
     */
    void SetNotes(std::vector<uint16_t>& noteIDs) noexcept
    {
        m_noteIDs = noteIDs;
    }

private:
    /**
     * @brief Ссылка на обработчик команд DORS.
     */
    DorsPSHandler& m_psHandler;
    /**
     * @brief Список идентификаторов банкнот для конфигурирования.
     */
    std::vector<uint16_t> m_noteIDs;
    /**
     * @brief Проверяет условия для выполнения команды.
     * @return HRESULT-код результата проверки
     */
    HRESULT ValidateProcessingConditions();
    /**
     * @brief Устанавливает бит в значении по номеру.
     * @param n Номер бита
     * @param Val Значение, в котором устанавливается бит
     */
    void SetBit(BYTE n, uint32_t& Val);
    /**
     * @brief Проверяет, что все идентификаторы банкнот поддерживаются устройством.
     * @return Код ошибки или WFS_SUCCESS
     */
    int CheckNoteIDs() const;
};

