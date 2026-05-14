#ifndef __IDENTIFICATION_H__
#define __IDENTIFICATION_H__
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <format>
#include <memory>
//#include "XFSCommon/StrManip.h"
#include <span>

#ifdef _WIN32
#include <Windows.h>
#endif

/**
 * @file Identification.h
 * @brief Обработка идентификационных данных устройства DORS/CashCode.
 *
 * Заголовочный файл содержит классы и утилиты для разбора и представления
 * идентификационных данных от модуля купюроприёмника: номера изделия,
 * серийного номера, версии прошивки и версии базы банкнот.
 */

namespace FS365::HW::Dors {


	// Кроссплатформенная замена макроса _T
#ifdef _WIN32
	constexpr std::wstring_view DEFAULT_TRIM_SYMBOLS = L"\r\n\t ";
#else
	constexpr std::string_view DEFAULT_TRIM_SYMBOLS = "\r\n\t ";
#endif

#if defined(__linux__)
	// Кроссплатформенные замены Windows-макросов
/**
 * @brief Получить старший байт 32-битного числа (аналог HIBYTE на Windows).
 * @param value Входное 32-битное значение.
 * @return Старший байт (8 бит).
 */
constexpr std::uint8_t HIBYTE(std::uint32_t value)  {
    return static_cast<std::uint8_t>((value >> 24) & 0xFF);
}

/**
 * @brief Получить старшее слово 32-битного числа (аналог HIWORD).
 * @param value Входное 32-битное значение.
 * @return Старшее 16-битное слово.
 */
constexpr std::uint16_t HIWORD(std::uint32_t value)  {
    return static_cast<std::uint16_t>((value >> 16) & 0xFFFF);
}

/**
 * @brief Получить младший байт 16-битного значения (аналог LOBYTE).
 * @param value Входное 16-битное значение.
 * @return Младший байт (8 бит).
 */
constexpr std::uint8_t LOBYTE(std::uint16_t value)  {
    return static_cast<std::uint8_t>(value & 0xFF);
}

/**
 * @brief Получить младшее слово 32-битного значения (аналог LOWORD).
 * @param value Входное 32-битное значение.
 * @return Младшее 16-битное слово.
 */
constexpr std::uint16_t LOWORD(std::uint32_t value)  {
    return static_cast<std::uint16_t>(value & 0xFFFF);
}
#endif

/**
 * @class fw_version
 * @brief Представление версии прошивки устройства.
 *
 * Класс хранит численное представление версии (ulNumber) и предоставляет
 * метод форматированного вывода в привычном виде major.minor.patch.
 */
class fw_version {
public:
	fw_version() = default;
	/**
	 * @brief Конструктор из сырых байт (4 байта).
	 * @param arrData Данные версии в виде последовательности байт.
	 */
	explicit fw_version(std::span<const uint8_t> arrData);

	// Правило пяти — используем дефолтные реализации
	fw_version(const fw_version&) = default;
	fw_version& operator=(const fw_version&) = default;
	fw_version(fw_version&&) = default;
	fw_version& operator=(fw_version&&) = default;
	~fw_version() = default;

	/**
	 * @brief Числовое представление версии.
	 */
	uint32_t ulNumber{ 0 };

	/**
	 * @brief Вернуть строку с представлением версии (например, "1.2.3").
	 * @return Форматированная строка версии.
	 */
	[[nodiscard]] virtual std::string PrintVersion() const;
};

/**
 * @class nbase_version
 * @brief Версия базы банкнот (наследуется от fw_version).
 *
 * Переопределяет формат вывода, специфичный для версий нотно-базовой таблицы.
 */
class nbase_version : public fw_version {
public:
	nbase_version() = default;
	explicit nbase_version(std::span<const std::uint8_t> arrData);

	[[nodiscard]] std::string PrintVersion() const;
};

/**
 * @class CCnetIdentification
 * @brief Идентификационные данные CashCode (CCNet).
 *
 * Класс парсит блок байт, содержащий Part Number, Serial Number и Asset Number,
 * и предоставляет удобное строковое представление.
 */
class CCnetIdentification {
public:
	CCnetIdentification() = default;
	explicit CCnetIdentification(std::span<const std::uint8_t> arrData);

	// Правило пяти — используем дефолтные реализации
	CCnetIdentification(const CCnetIdentification&) = default;
	CCnetIdentification& operator=(const CCnetIdentification&) = default;
	CCnetIdentification(CCnetIdentification&&) = default;
	CCnetIdentification& operator=(CCnetIdentification&&) = default;
	~CCnetIdentification() = default;

	/** @brief Part Number устройства (строка). */
	std::string strPartNumber;
	/** @brief Serial Number устройства (строка). */
	std::string strSerialNumber;
	/** @brief Asset Number — бинарный массив (7 байт обычно). */
	std::vector<uint8_t> arrAssetNumber;

	/**
	 * @brief Вернуть человеко-читаемое представление идентификации.
	 * @param nIndent Отступ (количество табуляций) для форматирования многострочного вывода.
	 * @return Строка с представлением Part и Serial.
	 */
	[[nodiscard]] std::string Print(int nIndent = 0) const;


	/** @brief Удаление символов в конце строки */
	template <typename T>
	[[nodiscard]] std::basic_string<T> trim_right(std::basic_string_view<T> str,
		std::basic_string_view<T> Value = DEFAULT_TRIM_SYMBOLS) noexcept {
		auto pos = str.find_last_not_of(Value);
		return pos == std::basic_string_view<T>::npos ? std::basic_string<T>{} : std::basic_string<T>(str.substr(0, pos + 1));
	}

	template <typename T>
	[[nodiscard]] std::basic_string<T> trim_right(std::basic_string_view<T> str, T Value) noexcept {
		return trim_right(str, std::basic_string_view<T>(&Value, 1));
	}
};

/**
 * @class CDbaIdentification
 * @brief Идентификационные данные DBA, включающие CCnetIdentification и версии.
 *
 * Парсит пакет данных от устройства, содержащий CCnet-часть, версию ПО и версию нотно-базы.
 */
class CDbaIdentification {
public:
	CDbaIdentification() = default;
	explicit CDbaIdentification(std::span<const std::uint8_t> arrData);

	CDbaIdentification(const CDbaIdentification&) = default;
	CDbaIdentification& operator=(const CDbaIdentification&) = default;
	CDbaIdentification(CDbaIdentification&&) = default;
	CDbaIdentification& operator=(CDbaIdentification&&) = default;
	~CDbaIdentification() = default;

	/** @brief CCNet-идентификация (поля Part/Serial/Asset). */
	CCnetIdentification ccnetIdn;
	/** @brief Версия программного обеспечения (4 байта). */
	fw_version softwareVersion;
	/** @brief Версия базы банкнот (4 байта). */
	nbase_version notebaseVersion;

	/**
	 * @brief Вернуть многострочную отладочную информацию о идентификации.
	 * @param nIndent Отступ (табуляции) для форматирования.
	 * @return Отформатированная строка с информацией.
	 */
	[[nodiscard]] std::string Print(int nIndent = 0) const;
};

/**
 * @class CIdentification
 * @brief Унифицированное представление идентификационных данных устройства.
 *
 * Класс может быть инициализирован из CCnetIdentification или CDbaIdentification
 * и предоставляет удобные строковые поля: PartNumber, SerialNumber, версии.
 */
class CIdentification {
public:
	CIdentification() = default;
	explicit CIdentification(const CCnetIdentification& ccnetIdn);
	explicit CIdentification(const CDbaIdentification& dbaIdn);

	CIdentification(const CIdentification&) = default;
	CIdentification& operator=(const CIdentification&) = default;
	CIdentification(CIdentification&&) = default;
	CIdentification& operator=(CIdentification&&) = default;
	~CIdentification() = default;

	/** @brief Part Number в виде строки. */
	std::string strPartNumber;
	/** @brief Serial Number в виде строки. */
	std::string strSerialNumber;
	/** @brief Asset Number в виде бинарного вектора. */
	std::vector<uint8_t> arrAssetNumber;
	/** @brief Версия программного обеспечения в виде строки. */
	std::string strSoftwareVersion;
	/** @brief Версия нотно-базы в виде строки. */
	std::string strNotebaseVersion;

	/**
	 * @brief Отладочный вывод идентификационных данных.
	 * @param nIndent Отступ для форматирования.
	 * @return Отформатированная строка с информацией о устройстве.
	 */
	[[nodiscard]] std::string Print(int nIndent = 0) const;

	/**
	 * @brief Проверить, что все поля идентификации пусты.
	 * @return true если все поля пусты.
	 */
	[[nodiscard]] bool IsEmpty() const;
};

} // namespace FS365::HW::Dors

#endif // #ifndef __IDENTIFICATION_H__
