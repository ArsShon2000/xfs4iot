#include "PowerUpManager.hpp"
#include <boost/scope_exit.hpp>
#include <boost/algorithm/string/predicate.hpp>
//#include "../../../../Devices/CashAcceptorSample/CashAcceptorSample.hpp"
#include <format> 
#include <string_view> 
#include "../../INFO_MODULES/GetInfoStatus/GetInfoStatus.h"


using namespace FS365::HW::Dors;


uint16_t PowerUpManager::GetAcceptorPartialState() {

    switch (m_pHandler->m_State) {
    case DorsHW::POLL_RES::PowerUpWithBillInStacker:
        return (m_strategy == ALLOW_ACCEPT || m_strategy == ALLOW_ACCEPT_BILL_STACKER)
            ? AcceptorPackage::ACCOK
            : AcceptorPackage::ACCCUSTOP;

    case DorsHW::POLL_RES::PowerUpWithBillInValidator:
        return (m_strategy == ALLOW_ACCEPT)
            ? AcceptorPackage::ACCOK
            : AcceptorPackage::ACCCUSTOP;

    default:
        return AcceptorPackage::ACCOK;
    }
}


boost::asio::awaitable<XFS4IoT::MessageHeader::CompletionCodeEnum>  PowerUpManager::Reset(
	std::shared_ptr<XFS4IoTFramework::CashManagement::ResetCommandEvents> events,
	bool usePowerUpStrategy) {
	using enum DorsHW::POLL_RES;

	// Проверяем стратегию перед выполнением ресета
	//if (usePowerUpStrategy && GetAcceptorPartialState() != WFS_CIM_ACCOK) {
	//    m_pHandler->logger_->trace(std::format(__FUNCTION__ "() - согласно текущей стратегии( {} ) игнорируем Reset",
	//    GetPowerUpWithBillStrategyText(m_strategy)), LOGLEVEL1);

	//    return WFS_SUCCESS;
	//}

	std::lock_guard lock(m_mtxResetOperation);

	// Указываем, что операция ресета в процессе
	m_pHandler->m_bResetOperationInProgress = true;
	// Запоминаем состояние до начала ресета
	m_pHandler->m_stateBeforeResetStarted = m_pHandler->m_State;

	BOOST_SCOPE_EXIT(m_pHandler) {
		// Сбрасываем флаг операции ресета
		m_pHandler->m_bResetOperationInProgress = false;
	} BOOST_SCOPE_EXIT_END;

	XFS4IoT::MessageHeader::CompletionCodeEnum hResult = XFS4IoT::MessageHeader::CompletionCodeEnum::Success;
	// Создаем токен отмены ожидания
	auto pResetTerminator = std::make_shared<StateMachine::BlockedWaitTermination>();
	// Сохраняем токен в обработчике
	m_p_async_reset_terminator = pResetTerminator;

	// Ожидаемые исходы
	StateMachine::CEventsSequenceList outcomes;

	outcomes.push_back({
		StateMachine::EventTo
		{
			{
				DorsHW::POLL_RES::UnitDisabled,
				DorsHW::POLL_RES::Idling,
				DorsHW::POLL_RES::DropCassetteFull
			}
		}
		});
	outcomes.push_back({
		{
			StateMachine::EventTo
		{
			{
				DorsHW::POLL_RES::DropCassetteOutOfPosition,
				DorsHW::POLL_RES::ValidatorJammed,
				DorsHW::POLL_RES::DropCassetteJammed,
				DorsHW::POLL_RES::StackMotorFail,
				DorsHW::POLL_RES::TransportMotorFail,
				DorsHW::POLL_RES::InitialCassetteStatusFail,
				DorsHW::POLL_RES::OpticCanalFail,
				DorsHW::POLL_RES::MagneticCanalFail,
				DorsHW::POLL_RES::StartTrayFailure,
				DorsHW::POLL_RES::Group47UnknownFailure,
				DorsHW::POLL_RES::PortError
			}
		}
		}
		});
	// --- Подписки на события машины состояний ---
	// Флаг возврата банкноты
	bool noteReturning = false;
	// Флаг складирования банкноты
	bool bNoteStacked{ false };

	auto subscrReturning = m_pHandler->m_stateMachine.Subscribe(
		StateMachine::EventTo{ { DorsHW::POLL_RES::Returning, DorsHW::POLL_RES::BillReturned } },
		[&noteReturning](DorsHW::POLL_RES) { noteReturning = true; }
	);

	// Для складирования нераспознанную банкноту при замятии
	StateMachine::EventsSequence_t seq =
	{
		StateMachine::EventTo{ RejIdentification },
		StateMachine::EventTo{ ValidatorJammed },
		StateMachine::EventTo{ Initialize },
		StateMachine::EventTo{ BillStacked }
	};

	auto subscrRejIdentification = m_pHandler->m_stateMachine.Subscribe(seq,
		[&bNoteStacked](DorsHW::POLL_RES) {
			bNoteStacked = true;
		}
	);

	// Складирование распознанной банкноты, но запрещена для приема при замятии
	StateMachine::EventsSequence_t seq2 =
	{
		StateMachine::EventTo{ RejInhibit },
		StateMachine::EventTo{ ValidatorJammed },
		StateMachine::EventTo{ Initialize },
		StateMachine::EventTo{ BillStacked }
	};

	auto subscrRejRejInhibit = m_pHandler->m_stateMachine.Subscribe(seq2,
		[&bNoteStacked](DorsHW::POLL_RES) {
			bNoteStacked = true;
		}
	);

	// Складирование банкноты, при замятии после CASH_IN_END
	StateMachine::EventsSequence_t seq3 =
	{
		StateMachine::EventTo{ DropCassetteJammed },
		StateMachine::EventTo{ Initialize },
		StateMachine::EventTo{ BillStacked },
		StateMachine::EventTo{ UnitDisabled }
	};

	auto subscrRejAfterCashInEnd = m_pHandler->m_stateMachine.Subscribe(seq3,
		[&bNoteStacked](DorsHW::POLL_RES) {
			bNoteStacked = true;
		}
	);

	BOOST_SCOPE_EXIT(m_pHandler, &subscrReturning, &subscrRejIdentification, &subscrRejRejInhibit, &subscrRejAfterCashInEnd) {
		m_pHandler->m_stateMachine.Unsubscribe(subscrReturning);
		m_pHandler->m_stateMachine.Unsubscribe(subscrRejIdentification);
		m_pHandler->m_stateMachine.Unsubscribe(subscrRejRejInhibit);
		m_pHandler->m_stateMachine.Unsubscribe(subscrRejAfterCashInEnd);
	} BOOST_SCOPE_EXIT_END;

	// Запоминаем UIDs текущего состояния для пропуска в ожидании
	const auto skipUIDs = m_pHandler->m_stateMachine.GetCurrentStateUID();
	// Запоминаем ID банкноты, находящейся в процессе обработки
	USHORT usNoteID = m_pHandler->m_usCurrentNoteID;

	// Выполняем аппаратный сброс
	auto resetResult = m_pHandler->m_pDevice->Reset();
	if (resetResult != DorsHW::RESULT::Ok) {
		// Если мы попали в состояние Err_Port, скорее всего придется физический перезагружать устройство
		if (resetResult == DorsHW::RESULT::Err_Port) {
			// При ошибке порта выполняем перезагрузку устройства
			m_pHandler->m_pDevice->Reboot();
		}
		else {
			m_pHandler->logger_->warn(std::format("{}() - ошибка аппаратного сброса: Reset() = {}", __FUNCTION__, FS365::HW::Dors::DorsHW::ResultToString(resetResult)), LOGLEVEL1);
			co_return XFS4IoT::MessageHeader::CompletionCodeEnum::HardwareError;
		}
	}

	const auto waitResult = m_pHandler->m_stateMachine.BlockedWait(outcomes, std::chrono::milliseconds(30000), pResetTerminator, skipUIDs);
	switch (waitResult) {
		// --- Анализ результата ожидания ---
	case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_0:
	{
		//m_pHandler->SendDeviceStatusEvent(m_pHandler->GetLogicalServiceName().c_str(), WFS_CIM_DEVONLINE);
		// Успешное завершение ресета
		hResult = XFS4IoT::MessageHeader::CompletionCodeEnum::Success;
	}
	break;
	case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_1:
		// Ошибка устройства
		hResult = XFS4IoT::MessageHeader::CompletionCodeEnum::HardwareError; break;
	case StateMachine::BlockedWaitResult::BWR_CANCELLED:
		// Операция была отменена
		hResult = XFS4IoT::MessageHeader::CompletionCodeEnum::Canceled; break;
	default:
		hResult = XFS4IoT::MessageHeader::CompletionCodeEnum::HardwareError;
	}

	// --- Движение банкноты ---
	if (bNoteStacked) {
		// Замятая банкнота была успешно складирована
		if (hResult == XFS4IoT::MessageHeader::CompletionCodeEnum::Success) {
			//m_pHandler->AddBanknote(usNoteID);

			if (usNoteID == 0) {
				m_pHandler->logger_->trace(std::format("{}(): В КАССЕТУ СКЛАДИРОВАНА: НЕРАСПОЗНАННАЯ ЗАМЯТАЯ БАНКНОТА", __FUNCTION__), LOGLEVEL1);
			}
			else {
				m_pHandler->logger_->trace(std::format("{}(): В КАССЕТУ СКЛАДИРОВАНА: ЗАМЯТАЯ БАНКНОТА С ИНДЕКСОМ {}", __FUNCTION__, usNoteID), LOGLEVEL1);
			}
			// Рассылка MediaDetected с указанием складирования в кассету
			{
//co_await events->MediaDetectedEvent(
//XFS4IoT::CashManagement::ItemTargetEnumEnum::OutCenter,
//std::nullopt,
//std::nullopt);
			}
		}
		else {
//co_await events->MediaDetectedEvent(
//XFS4IoT::CashManagement::ItemTargetEnumEnum::OutCenter,
//std::nullopt,
//std::nullopt);
		}
	}
	else if (noteReturning) {
		if (hResult == XFS4IoT::MessageHeader::CompletionCodeEnum::Success) {
			// В ходе сброса был возврат банкноты
			m_pHandler->logger_->trace(std::format("{}(): ЗАМЯТАЯ БАНКНОТА ВОЗВРАЩЕНА ОБРАТНО", __FUNCTION__), LOGLEVEL1);
		}
//co_await events->MediaDetectedEvent(
//XFS4IoT::CashManagement::ItemTargetEnumEnum::OutCenter,
//std::nullopt,
//std::nullopt);
	}
	//else if (DorsHW::_isNoteDetected(m_pHandler->m_stateBeforeResetStarted)) {
		// Не удалось успешно обработать банкноту
	co_await events->MediaDetectedEvent(
		XFS4IoTFramework::CashManagement::ItemTargetEnum::OutCenter,
		std::nullopt,
		1);
//}

	co_return hResult;
}


void PowerUpManager::AsyncReset(boost::asio::any_io_executor executor,
	std::shared_ptr<XFS4IoTFramework::CashManagement::ResetCommandEvents> events,
	bool useStrategy)
{
	_stop_async_reset();

	boost::asio::co_spawn(
		executor,
		Reset(events, useStrategy),
		boost::asio::detached
	);
}


void PowerUpManager::_stop_async_reset() {
	if (m_threadAsyncReset.joinable()) {
		if (auto term = m_p_async_reset_terminator.lock())
			term->AsyncCancel();
		m_threadAsyncReset.join();
	}
}
