#ifndef __GETINFOCAPABILITIES_H__
#define __GETINFOCAPABILITIES_H__
#include "ComTransport/DorsPSHandler.h"
#include "Network/TcpServer.h"
#include <cstdint>
#include <MODULE_STRUCTURES/CimCaps/CimCaps.hpp>

/**
 * @file GetInfoCapabilities.h
 * @brief Описывает возможности (capabilities) устройства купюроприёмника.
 *
 * Файл содержит определение класса `GetInfoCapabilities`, который
 * собирает информацию о возможностях устройства (аналог WFSCIM::GetInfo(CAPABILITIES)).
 * Данные используются при формировании ответа на соответствующий запрос
 * сервиса XFS/CIM.
 */

namespace Xfs::Cim {

    /**
     * @enum DeviceTypePackage
     * @brief Тип логического устройства (пакетный формат).
     */
    enum DeviceTypePackage : uint16_t
    {
        TELLERBILL = 0,      /**< Teller (оператор) банкнот */
        SELFSERVICEBILL = 1,  /**< Self-service банкнот */
        TELLERCOIN = 2,      /**< Teller монет */
        SELFSERVICECOIN = 3   /**< Self-service монет */
    };

    /**
     * @enum ExchangeTypePackage
     * @brief Типы обмена/инкассации (битовые флаги).
     */
    enum ExchangeTypePackage : uint16_t {
        EXBYHAND = 1 << 0,      /**< Ручной обмен */
        EXTOCASSETTES = 1 << 1,  /**< Обмен через кассеты */
        CLEARRECYCLER = 1 << 2,  /**< Очистка рециркулятора */
        DEPOSITINTO = 1 << 3,    /**< Депозит в */
    };

    /**
     * @enum PositionsPackage
     * @brief Возможные позиции входа/выхода (битовые маски).
     */
    enum PositionsPackage : uint16_t {
        POSINLEFT = 1 << 0,
        POSINRIGHT = 1 << 1,
        POSINCENTER = 1 << 2,
        POSINTOP = 1 << 3,
        POSINBOTTOM = 1 << 4,
        POSINFRONT = 1 << 5,
        POSINREAR = 1 << 6,
        POSOUTLEFT = 1 << 7,
        POSOUTRIGHT = 1 << 8,
        POSOUTCENTER = 1 << 9,
        POSOUTTOP = 1 << 10,
        POSOUTBOTTOM = 1 << 11,
        POSOUTFRONT = 1 << 12,
        POSOUTREAR = 1 << 13
    };

    ///**
    // * @enum RetractAreasPackage
    // * @brief Области для ретракта/удаления предметов (битовые флаги).
    // */
    //enum RetractAreasPackage : uint16_t {
    //    RA_RETRACT = 1 << 0,    /**< Область ретракта */
    //    RA_REJECT = 1 << 1,     /**< Область отклонения */
    //    RA_TRANSPORT = 1 << 2,  /**< Транспорт */
    //    RA_STACKER = 1 << 3,    /**< Стекер */
    //    RA_BILLCASSETTES = 1 << 4, /**< Кассеты */
    //    RA_CASHIN = 1 << 5,     /**< CashIn */
    //    RA_ITEMCASSETTE = 1 << 6 /**< Физическая кассета предметов (XFS4) */
    //};


    /* values of WFSCIMCAPS.fwRetractAreas */
    /* values of WFSCIMRETRACT.usRetractArea */
    enum RetractAreasPackage : uint16_t {
        CIM_RA_RETRACT = (0x0001),
        CIM_RA_TRANSPORT = (0x0002),
        CIM_RA_STACKER = (0x0004),
        CIM_RA_BILLCASSETTES = (0x0008),
        CIM_RA_NOTSUPP = (0x0010),
        CIM_RA_REJECT = (0x0020),
        CIM_RA_CASHIN = (0x0040)
    };

    /* values of WFSCIMCAPS.fwRetractTransportActions */
    /* values of WFSCIMCAPS.fwRetractStackerActions */
    enum RetractTransportActionsPackage : uint16_t {
        CIM_PRESENT = (0x0001),
        CIM_RETRACT = (0x0002),
        CIM_NOTSUPP = (0x0004),
        CIM_REJECT = (0x0008),
        CIM_BILLCASSETTES = (0x0010),
        CIM_CASHIN = (0x0020)
    };

    /* values for WFSCIMCAPS.fwCashInLimit */
    enum CashInLimitPackage : uint16_t {
        CIM_LIMITNOTSUPP = (0x0000),
        CIM_LIMITBYTOTALITEMS = (0x0001),
        CIM_LIMITBYAMOUNT = (0x0002),
        CIM_LIMITMULTIPLE = (0x0004),
        CIM_LIMITREFUSEOTHER = (0x0008)
    };

    /* values of WFSCIMCAPS.fwCountActions */
    enum CountActionsPackage : uint16_t {
        CIM_COUNTNOTSUPP = (0x0000),
        CIM_COUNTINDIVIDUAL = (0x0001),
        CIM_COUNTALL = (0x0002)
    };

    /**
     * @enum RetractActionsPackage
     * @brief Доступные действия при ретракте/транспортировке (битовые флаги).
     */
    enum RetractActionsPackage : uint16_t {
        PRESENT = 1 << 0,
        RETRACT = 1 << 1,
        REJECT = 1 << 2,
        BILLCASSETTES = 1 << 3,
        CASHIN = 1 << 4,
        ITEMCASSETTE = 1 << 5
    };

    /**
     * @enum ItemInfoTypesPackage
     * @brief Типы информации о предмете (серийный номер, подпись, изображение).
     */
    enum ItemInfoTypesPackage : uint32_t {
        ITEM_NOTSUPP = 0x00000000,
        ITEM_SERIALNUMBER = 1 << 0,
        ITEM_SIGNATURE = 1 << 1,
        ITEM_IMAGEFILE = 1 << 2,
    };

    /**
     * @class GetInfoCapabilities
     * @brief Сбор и представление возможностей купюроприёмника.
     *
     * Класс собирает информацию о возможностях устройства, предоставляет
     * метод `fillResult()` для заполнения данных из `DorsPSHandler` и
     * `PrintFromResult()` для получения текстового представления (для логов/отладки).
     * Также реализован простой ленивый синглтон через `GetInstance()`.
     */
    class GetInfoCapabilities {
    public:
        GetInfoCapabilities() = default;
        GetInfoCapabilities(const GetInfoCapabilities&) = delete;
        GetInfoCapabilities& operator=(const GetInfoCapabilities&) = delete;

        /* Параметры устройства */
        uint16_t wClass;                   /**< Тип устройства (wClass) */
        uint16_t deviceType;               /**< Тип устройства (DeviceTypePackage) */
        uint16_t maxCashInItems;           /**< Максимальное количество предметов в CashIn */
        bool compound;                     /**< Является ли логический девайс частью составного физического девайса */
        bool shutter;                      /**< Наличие шторки */
        bool shutterControl;               /**< Поддержка управления шторкой */
        bool safeDoor;                     /**< Наличие дверцы сейфа */
        bool cashBox;                      /**< Наличие денежного бокса (для teller) */
        bool refill;                       /**< Перезаполнение (не используется) */
        uint16_t intermediateStacker;      /**< Наличие/ёмкость промежуточного стакера */
        bool itemsTakenSensor;             /**< Сенсор взятия предмета */
        bool itemsInsertedSensor;          /**< Сенсор вставки предмета */
        uint16_t positions;                /**< Битовая маска позиций (PositionsPackage) */
        uint16_t exchangeType;              /**< Тип обмена (ExchangeTypePackage) */
        uint16_t retractAreas;              /**< Области ретракта (RetractAreasPackage) */
        uint16_t retractTransportActions;   /**< Действия для транспорта (RetractActionsPackage) */
        uint16_t retractStackerActions;     /**< Действия для стакера (RetractActionsPackage) */

        XfsExtra_t extra;    /**< Дополнительные параметры, заполненные извне */
        bool isGuidLightsSupported = false;/**< Поддержка светодиодных индикаторов */
        uint32_t itemInfoTypes;             /**< Типы информации о предмете (ItemInfoTypesPackage) */
        bool compareSignatures;            /**< Поддержка сравнения подписей */
        bool powerSaveControl;             /**< Поддержка энергосбережения */

        /* Поля, используемые для XFS3.40 и XFS4 */
        bool bReplenish;                    /**< Поддержка пополнения */
        uint16_t fwCashInLimit;              /**< Лимит на приём */
        uint16_t fwCountActions;             /**< Количество поддерживаемых действий */
        bool bDeviceLockControl;            /**< Поддержка блокировки устройства */
        uint16_t wMixedMode;                /**< Поддержка смешанного режима */
        bool bMixedDepositAndRollback;      /**< Поддержка одновременно пополнения/отката */
        bool bAntiFraudModule;              /**< Наличие антифрод-модуля */
        bool bDeplete;                      /**< Поддержка опорожнения */
        bool bBlacklist;                    /**< Поддержка чёрного списка */
        LPDWORD lpdwSynchronizableCommands; /**< Указатель на набор синхронизируемых команд (опционально) */
        bool bClassificationList;           /**< Поддержка списка классификации */
        bool bPhysicalNoteList;             /**< Поддержка физического списка номиналов */

        /* Сведения о ПО/прошивке/устройстве (XFS4) */
        std::string serviceVersion;        /**< Версия сервиса */
        std::string modelName;             /**< Модель устройства */
        std::string serialNumber;          /**< Серийный номер */
        std::string revisionNumber;        /**< Номер ревизии */
        std::string modelDescription;      /**< Описание модели */
        std::string firmwareName;          /**< Имя прошивки */
        std::string firmwareVersion;       /**< Версия прошивки */
        std::string hardwareRevision;      /**< Аппаратная ревизия */
        std::string softwareName;          /**< Имя ПО */
        std::string softwareVersion;       /**< Версия ПО */

        /**
         * @brief Получить или создать глобальный экземпляр (ленивый синглтон).
         * @return Указатель на экземпляр `GetInfoCapabilities`.
         */
        static GetInfoCapabilities* GetInstance(DorsPSHandler* handler = nullptr) {
            std::lock_guard<std::mutex> lock(m_instanceMutex);
            if (pinstance_ == nullptr)
            {
                pinstance_ = new GetInfoCapabilities;
                pinstance_->setHandler(*handler);
                pinstance_->Initialize();
            }
            return pinstance_;
        }

        /**
         * @brief Установить обработчик DorsPSHandler для получения данных об устройстве.
         * @param handler Ссылка на уже существующий `DorsPSHandler` (класс не владеет указателем).
         */
        void setHandler(DorsPSHandler& handler) noexcept {
            m_psHandler = &handler;
        }

        /**
         * @brief Установить сетевой сервер (опционально), используется для интеграции с сетью.
         * @param server Ссылка на `TcpServer` (класс не владеет указателем).
         */
        void setServer(TcpServer& server) noexcept {
            m_server = &server;
        }

        /**
         * @brief Заполнить поля класса актуальными значениями, получаемыми из `DorsPSHandler`.
         *
         * Метод собирает информацию о возможностях устройства и сохраняет её
         * в полях объекта. Используется при формировании ответа на GetInfo(Capabilities).
         */
        void Initialize();

        /**
         * @brief Заполнить поля класса в формате XFS3.40.
         *
         * Метод собирает информацию о возможностях устройства в формате,
         * совместимом с XFS3.40, и сохраняет её в полях объекта.
         * @param cashUnit Ссылка на структуру `CimCaps` для заполнения.
         */
        void fill_XFS3_Result(CimCaps& cashUnit);


    private:
        static GetInfoCapabilities* pinstance_; /**< Статический указатель на синглтон */
        static std::mutex m_sendMutex;          /**< Мьютекс для сериализации операций отправки/логики */
        static std::mutex m_instanceMutex;      /**< Мьютекс для инициализации синглтона */
        DorsPSHandler* m_psHandler = nullptr;  /**< Указатель на обработчик (не владеет) */
        TcpServer* m_server = nullptr;         /**< Указатель на сетевой сервер (не владеет) */
    };
}


#endif // #ifndef __GETINFOCAPABILITIES_H__
