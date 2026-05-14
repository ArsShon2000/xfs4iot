// file : DORSHW/source/PS/CIM.PS/Managers/PowerUpManager/PowerUpManager.hpp

#pragma once
#include <condition_variable>
#include <mutex>
#include <memory>
#include <thread>
#include <string>
#include <boost/scope_exit.hpp>
#include "../../../core/common/StateMachine/StateMachine.hpp"
#include "../../../core/SettingModule/SettingModule.h"
#include "../../../../Devices/CashAcceptorSample/CashAcceptorSample.hpp"
#include "../../../core/Logger/ILogger.hpp"

/**
 * @file PowerUpManager.hpp
 * @brief Менеджер поведения устройства при подаче питания (PowerUp).
 *
 * Файл содержит определение класса `PowerUpManager`, который реализует логику
 * обработки ситуации, когда устройство включается и в нём уже находится купюра
 * (или купюры). Класс поддерживает различные стратегии обработки таких
 * сценариев: остановить приём, разрешить приём, разрешить приём, если купюра
 * в стекере и т.п. Также реализована поддержка асинхронного сброса/рестарта
 * устройства с ожиданием устойчивых состояний через встроенную стейт‑машину.
 */

using StateMachine = XfsCommon::StateMachine< FS365::HW::Dors::DorsHW::POLL_RES >;
/**
 * @brief Преобразовать текстовое представление стратегии в значение `PowerUpWithBillStrategy`.
 * @param strText Текстовое имя стратегии (например, "STOP_ACCEPT", "ALLOW_ACCEPT").
 * @return Значение `PowerUpWithBillStrategy`, соответствующее тексту. По умолчанию возвращает ALLOW_ACCEPT.
 */
PowerUpWithBillStrategy GetPowerUpWithBillStrategyByText(const std::string& strText);

/**
 * @class PowerUpManager
 * @brief Менеджер логики поведения при включении питания (PowerUp).
 *
 * `PowerUpManager` инкапсулирует стратегию обработки случаев, когда при
 * включении устройства обнаружена банкнота в тракте (в валидаторе или
 * стекере). В зависимости от выбранной стратегии менеджер при сбросе/рестарте
 * может разрешать или запрещать приём, ожидать устойчивых состояний или
 * инициировать дополнительные действия (добавление банкноты в учёт, сигнал
 * о детекции и т.д.).
 *
 * Менеджер хранит указатель на `DorsPSHandler` (не владеет), использует
 * `StateMachine` для ожидания последовательности состояний и предоставляет
 * синхронизированные методы для выполнения синхронного и асинхронного сброса.
 */
class PowerUpManager {
public:
	/**
	 * @brief Конструктор.
	 * @param handler Указатель на `CashAcceptorSample` (класс не владеет указателем).
	 * @param strategy Стратегия обработки ситуации PowerUpWithBill.
	 */
	PowerUpManager(std::shared_ptr < XFS4IoTSP::CashAcceptor::Sample::CashAcceptorSample> handler, PowerUpWithBillStrategy strategy)
		: m_strategy(strategy), m_pHandler(handler) {
	}

	/**
	 * @brief Деструктор — останавливает возможный асинхронный ресет.
	 */
	~PowerUpManager() {
		_stop_async_reset();
	}

	/**
	 * @brief Частичная оценка состояния Acceptor сразу после PowerUp.
	 *
	 * Возвращает частичный код состояния приёмника (CIM_ACC*), основываясь
	 * на текущем состоянии аппарата и выбранной стратегии `m_strategy`.
	 * Это значение может быть использовано при формировании WFSCIM STATUS до
	 * тех пор, пока не будет определено окончательное состояние устройства.
	 *
	 * @return Код состояния приёмника (CIM_ACCOK или CIM_ACCCUSTOP и т.д.).
	 */
	 uint16_t GetAcceptorPartialState();

	 /**
	  * @brief Синхронный сброс/рестарт устройства с опциональной учётом стратегии.
	  *
	  * Если `usePowerUpStrategy == true`, перед выполнением реального ресета
	  * проверяется `GetAcceptorPartialState()` и при несоответствии стратегиям
	  * рестарт может быть проигнорирован (возвращается SUCCESS).
	  *
	  * Функция вызывает `m_pHandler->m_pDevice->Reset()` и затем ожидает одно из
	  * ожидаемых состояний через `StateMachine::BlockedWait`. По результатам
	  * ожидаемых событий возвращается соответствующий код XFS (SUCCESS,
	  * ERR_HARDWARE_ERROR, ERR_CANCELED).
	  *
	  * Помимо очистки состояний, функция обрабатывает случаи движения банкноты
	  * (stacked/returned/detected) и посылает соответствующие уведомления через
	  * `m_pHandler`.
	  *
	  * @param usePowerUpStrategy Применять ли стратегию PowerUp перед выполнением ресета.
	  * @return XFS4IoT::MessageHeader::CompletionCodeEnum код результата операции (SUCCESS или XFS-ошибки).
	  */
	boost::asio::awaitable<XFS4IoT::MessageHeader::CompletionCodeEnum> Reset(
		std::shared_ptr<XFS4IoTFramework::CashManagement::ResetCommandEvents> events,
		bool usePowerUpStrategy = false);

	/**
	 * @brief Запустить асинхронный Reset в отдельном потоке.
	 * @param useStrategy Применять ли PowerUp стратегию при выполнении асинхронного сброса.
	 */
	void AsyncReset(
		boost::asio::any_io_executor executor,
		std::shared_ptr<XFS4IoTFramework::CashManagement::ResetCommandEvents> events,
		bool useStrategy = false);

private:
	/**
	 * @brief Остановить асинхронный ресет, дождаться завершения потока и инициировать отмену ожидания.
	 */
	void _stop_async_reset();

private:
	PowerUpWithBillStrategy m_strategy; /**< Выбранная стратегия обработки PowerUp с банкнотами */
	std::shared_ptr<XFS4IoTSP::CashAcceptor::Sample::CashAcceptorSample> m_pHandler{ nullptr }; /**< Указатель на обработчик PS (не владеет) */
	std::mutex m_mtxResetOperation; /**< Мьютекс для синхронизации операции Reset */
	std::thread m_threadAsyncReset; /**< Поток для асинхронного Reset */
	std::weak_ptr<StateMachine::BlockedWaitTermination> m_p_async_reset_terminator; /**< Токен отмены ожидания */
};

