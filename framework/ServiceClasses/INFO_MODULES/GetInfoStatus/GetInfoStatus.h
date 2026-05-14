#ifndef __GETINFOSTATUS_H__
#define __GETINFOSTATUS_H__
#include <mutex>
#include "../../../../Devices/CashAcceptorSample/CashAcceptorSample.hpp"

/**
 * @file GetInfoStatus.h
 * @brief Сбор и представление статуса устройства купюроприёмника (WFSCIM STATUS).
 *
 * Этот заголовочный файл содержит определение класса `GetInfoStatus`, который
 * собирает и кэширует информацию о текущем состоянии устройства (онлайн/оффлайн,
 * состояние приёмника, стекера, сенсоров и позиций). Класс реализован как
 * ленивый синглтон и предназначен для многопоточного использования. Для
 * интеграции с низкоуровневым обработчиком используется `DorsPSHandler`.
 */

 /**
  * @enum DeviceStatusPackage
  * @brief Общие состояния устройства (отображение на WFSCIM STATUS).
  */
enum DeviceStatusPackage : uint16_t
{
	ONLINE = 0,   /**< Устройство в сети и готово */
	OFFLINE = 1,   /**< Устройство офлайн */
	POWEROFF = 2,   /**< Питание отключено */
	NODEVICE = 3,   /**< Устройство отсутствует/не обнаружено */
	HWERROR = 4,   /**< Ошибка оборудования */
	USERERROR = 5,   /**< Ошибка пользователя */
	BUSY = 6,   /**< Занято */
	FRAUDATTEMPT = 7, /**< Попытка мошенничества */
	POTENTIALFRAUD = 8, /**< Потенциальная попытка мошенничества */
	STARTING = 9  /**< Идёт запуск (XFS4) */
};

inline std::string DeviceStatusToString(uint16_t device)
{
	switch (device) {
	case DeviceStatusPackage::ONLINE: return "online";
	case DeviceStatusPackage::OFFLINE: return "offline";
	case DeviceStatusPackage::POWEROFF: return "powerOff";
	case DeviceStatusPackage::NODEVICE: return "noDevice";
	case DeviceStatusPackage::HWERROR: return "hardwareError";
	case DeviceStatusPackage::USERERROR: return "userError";
	case DeviceStatusPackage::BUSY: return "deviceBusy";
	case DeviceStatusPackage::FRAUDATTEMPT: return "fraudAttempt";
	case DeviceStatusPackage::POTENTIALFRAUD: return "potentialFraud";
	case DeviceStatusPackage::STARTING: return "starting";
	default: return "UnknownResult";
	}
}

/**
 * @enum SafeDoorPackage
 * @brief Состояние дверцы сейфа (safe door).
 */
enum SafeDoorPackage : uint16_t
{
	DOORNOTSUPPORTED = 1, /**< Дверца не поддерживается */
	DOOROPEN = 2,         /**< Дверца открыта */
	DOORCLOSED = 3,       /**< Дверца закрыта */
	DOORUNKNOWN = 4,      /**< Состояние дверцы неизвестно */
};

/**
 * @enum AcceptorPackage
 * @brief Состояние приёмника (acceptor).
 */
enum AcceptorPackage : uint16_t
{
	ACCOK = 0,        /**< Приёмник в норме */
	ACCCUSTATE = 1,   /**< Приёмник достиг порогового состояния */
	ACCCUSTOP = 2,    /**< Приёмник остановлен (например, переполнен) */
	ACCCUUNKNOWN = 3, /**< Состояние приёмника неизвестно */
};


inline std::string AcceptorToString(uint16_t acceptor)
{
	switch (acceptor) {
	case AcceptorPackage::ACCOK: return "ok";
	case AcceptorPackage::ACCCUSTATE: return "attention";
	case AcceptorPackage::ACCCUSTOP: return "stop";
	case AcceptorPackage::ACCCUUNKNOWN: return "unknown";
	default: return "unknown";
	}
}

/**
 * @enum IntermediateStackerPackage
 * @brief Состояние промежуточного стакера.
 */
enum IntermediateStackerPackage : uint16_t
{
	ISEMPTY = 0,        /**< Пуст */
	ISNOTEMPTY = 1,     /**< Не пуст */
	ISFULL = 2,         /**< Полон */
	ISUNKNOWN = 4,      /**< Неизвестно */
	ISNOTSUPPORTED = 5, /**< Не поддерживается */
};

/**
 * @enum StackerItemsPackage
 * @brief Доступ к предметам в стакере/приёмной области.
 */
enum StackerItemsPackage : uint16_t
{
	CUSTOMERACCESS = 0,   /**< Доступен клиенту */
	NOCUSTOMERACCESS = 1, /**< Нет доступа клиенту */
	ACCESSUNKNOWN = 2,    /**< Доступ неизвестен */
	NOITEMS = 4,          /**< Предметов нет */
};

/**
 * @enum BankNoteReaderPackage
 * @brief Состояние модуля распознавания банкнот.
 */
enum BankNoteReaderPackage : uint16_t
{
	BNROK = 0,           /**< Модуль работает */
	BNRINOP = 1,         /**< Модуль не в работе */
	BNRUNKNOWN = 2,      /**< Состояние неизвестно */
	BNRNOTSUPPORTED = 3, /**< Модуль не поддерживается */
};

/**
 * @enum AntiFraudModulePackage
 * @brief Состояние антифрод-модуля.
 */
enum AntiFraudModulePackage : uint16_t
{
	AFMNOTSUPP = 0,       /**< Не поддерживается */
	AFMOK = 1,            /**< OK */
	AFMINOP = 2,          /**< Не в работе */
	AFMDEVICEDETECTED = 3,/**< Обнаружено устройство */
	AFMUNKNOWN = 4,       /**< Неизвестно */
};

/**
 * @enum DevicePositionsPackage
 * @brief Статусы положения устройства (позиции вход/выход).
 */
enum DevicePositionsPackage : uint16_t
{
	DEVICEINPOSITION = 0, /**< В позиции */
	DEVICENOTINPOSITION = 1, /**< Не в позиции */
	DEVICEPOSUNKNOWN = 2,  /**< Неизвестно */
	DEVICEPOSNOTSUPP = 3, /**< Позиции не поддерживаются */
};

/**
 * @enum PositionTypePackage
 * @brief Тип позиции: вход/выход/некорректная.
 */
enum PositionTypePackage : uint16_t
{
	POS_IN = 0x1,    /**< Вход */
	POS_OUT = 0x2,   /**< Выход */
	POS_INVALID = 0x3/**< Некорректная позиция */
};

/**
 * @class GetInfoStatus
 * @brief Сервис сбора статуса устройства (WFSCIM STATUS).
 *
 * Класс собирает состояние устройства (онлайн/офлайн/ошибки), состоянии
 * приемника, промежуточного стакера, модуля распознавания банкнот и
 * списка позиций транспорта. Реализован как ленивый синглтон и
 * предназначен для многопоточного использования — инициализация и доступ
 * защищены мьютексом.
 */
class GetInfoStatus {
public:
	GetInfoStatus() = default;
	GetInfoStatus(const GetInfoStatus&) = delete;
	GetInfoStatus& operator=(const GetInfoStatus&) = delete;

	/**
	 * @brief Обновить внутренний кэш статуса устройства.
	 *
	 * Метод использует `DorsPSHandler` для получения текущего состояния
	 * устройства и формирует поля класса, используемые при ответе на
	 * GetInfo(STATUS).
	 */
	void fillResult();

	/* --- Публичные поля, соответствующие структуре WFSCIM STATUS --- */
	uint16_t device;                          /**< Общее состояние устройства (DeviceStatusPackage) */
	uint16_t devicePosition;                  /**< Положение устройства */
	uint16_t powerSaveRecoveryTime;            /**< Время восстановления из режима энергосбережения */
	std::vector<std::string> extra;           /**< Дополнительные параметры (extra)
												 * Обычно заполняются через FillByExtra */

	uint16_t safeDoor;                        /**< Состояние дверцы сейфа (SafeDoorPackage) */
	uint16_t acceptor;                        /**< Состояние приёмника (AcceptorPackage) */
	uint16_t intermediateStacker;             /**< Состояние промежуточного стакера (IntermediateStackerPackage) */
	uint16_t stackerItems;                    /**< Состояние доступа к предметам (StackerItemsPackage) */
	uint16_t banknoteReader;                  /**< Состояние модуля распознавания банкнот (BankNoteReaderPackage) */
	bool dropBox;                             /**< Наличие/состояние drop box */

	bool guidLightsSupported = false;         /**< Поддержка световых индикаторов */
	uint16_t mixedMode;                       /**< Режим mixedMode */
	bool antiFraudModule;                 /**< Состояние антифрод-модуля (AntiFraudModulePackage) */

	/**
	 * @brief Получить или создать глобальный экземпляр (ленивый синглтон).
	 *
	 * Инициализация защищена `m_instanceMutex`.
	 * @return Указатель на экземпляр `GetInfoStatus`.
	 */
	static GetInfoStatus* GetInstance() {
		std::lock_guard<std::mutex> lock(m_instanceMutex);
		if (pinstance_ == nullptr)
		{
			pinstance_ = new GetInfoStatus;
		}
		return pinstance_;
	}


	/**
	* @brief Установить логгер для записи ошибок и отладки.
	*/
	void setLogger(std::shared_ptr<ILogger> pLogger) {
		m_log = pLogger;
	}

	/**
	* @brief Установить логгер (не владеет указателем).
	* @param logger Умный указатель на `ILogger`.
	*/
	std::shared_ptr<ILogger> m_log;


private:
	static GetInfoStatus* pinstance_; /**< Указатель на синглтон */
	static std::mutex m_sendMutex;    /**< Мьютекс для сериализации операций */
	static std::mutex m_instanceMutex;/**< Мьютекс для инициализации синглтона */
	std::shared_ptr<XFS4IoTSP::CashAcceptor::Sample::CashAcceptorSample> m_psHandler = nullptr; /**< Указатель на обработчик (не владеет) */

};

#endif // #ifndef __GETINFOSTATUS_H__
