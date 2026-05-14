#include "GetInfoStatus.h"
#include "../../Managers/PowerUpManager/PowerUpManager.hpp"

using namespace FS365::HW::Dors;

GetInfoStatus* GetInfoStatus::pinstance_ = nullptr;
std::mutex GetInfoStatus::m_sendMutex;
std::mutex GetInfoStatus::m_instanceMutex;

void GetInfoStatus::fillResult()
{
	std::lock_guard<std::mutex> lock(m_sendMutex);
	if (!extra.empty()) extra.clear();


	auto currentState = m_psHandler->m_State;

	//if (m_psHandler->m_bResetOperationInProgress)
	//{
	//    switch (currentState) {
	//    case DorsHW::POLL_RES::Initialize:
	//    case DorsHW::POLL_RES::Accepting:
	//    case DorsHW::POLL_RES::Stacking:
	//    case DorsHW::POLL_RES::Returning:
	//        currentState = m_psHandler->m_stateBeforeResetStarted;
	//        break;
	//    }
	//}


	{
		dropBox = false;
		safeDoor = DOORNOTSUPPORTED;

		guidLightsSupported = false;
		devicePosition = DEVICEPOSNOTSUPP;
		powerSaveRecoveryTime = 0;
	}


	if (!m_psHandler->m_pDevice->IsDeviceInitialized()
		|| (currentState == DorsHW::POLL_RES::PortError)
		)
	{
		// DEVNODEVICE | DEVPOWEROFF

		if (device != NODEVICE && device != POWEROFF)
		{
			m_log->trace(std::format("{}() - WFSCIMSTATUS.fwDevice: {} -> {}", __FUNCTION__
				, DeviceStatusToString(device)
				, DeviceStatusToString(m_psHandler->m_bSoftwareConfigurationFault ? NODEVICE : POWEROFF)));
		}
		device = m_psHandler->m_bSoftwareConfigurationFault ? NODEVICE : POWEROFF;

		if (acceptor != ACCCUUNKNOWN)
		{
			m_log->trace(std::format("{}() - WFSCIMSTATUS.fwAcceptor: {} -> {}", __FUNCTION__
				, AcceptorToString(acceptor)
				, AcceptorToString(ACCCUUNKNOWN)));
		}
		acceptor = ACCCUUNKNOWN;
		intermediateStacker = ISUNKNOWN;
		stackerItems = ACCESSUNKNOWN;
		banknoteReader = BNRUNKNOWN;


		//CimInPos position;
		//position.position = POSITION::POSINCENTER;
		//position.shutter = SHTNOTSUPPORTED;
		//position.transport = TPUNKNOWN;
		//position.transportStatus = TPSTATNOTEMPTY_UNK;
		//position.positionStatus = PSUNKNOWN;
		//positions.push_back(position);

		//position.position = POSOUTCENTER;
		//positions.push_back(position);

	}
	else {
		// Device, Acceptor
		switch (currentState)
		{
		case DorsHW::POLL_RES::Idling:
		case DorsHW::POLL_RES::UnitDisabled:
		case DorsHW::POLL_RES::Accepting:
		case DorsHW::POLL_RES::EscrowPos:
		case DorsHW::POLL_RES::BillReturned:
			if (device != ONLINE)
			{
				m_log->trace(std::format("{}() - WFSCIMSTATUS.fwDevice: {} -> {}", __FUNCTION__
					, DeviceStatusToString(device)
					, DeviceStatusToString(ONLINE)));
			}
			device = ONLINE;


			if (acceptor != ACCCUSTOP && acceptor != ACCOK)
			{
				m_log->trace(std::format("{}() - WFSCIMSTATUS.fwAcceptor: {} -> {}", __FUNCTION__
					, AcceptorToString(acceptor)
					, AcceptorToString(m_psHandler->m_bCassetteHasBeenReplaced ? ACCCUSTOP : ACCOK)));
			}
			if (m_psHandler->m_bCassetteHasBeenReplaced) {
				acceptor = ACCCUSTOP;
			}
			else
			{
				acceptor = ACCOK;
			}
			break;

			// в соовтетствии со спецификацией, переполненная кассета
			// означается состоянием ACCCUSTOP в поле Acceptor.
			// Дополнение: отсутствующая кассета, также не является случаем аппаратной ошибки
		case DorsHW::POLL_RES::DropCassetteFull:
		case DorsHW::POLL_RES::DropCassetteOutOfPosition:
		case DorsHW::POLL_RES::CassetteBracketOpen:
			if (device != ONLINE)
			{
				m_log->trace(std::format("{}() - WFSCIMSTATUS.fwDevice: {} -> {}", __FUNCTION__
					, DeviceStatusToString(device)
					, DeviceStatusToString(ONLINE)));
			}
			device = ONLINE;

			if (acceptor != ACCCUSTOP)
			{
				m_log->trace(std::format("{}() - WFSCIMSTATUS.fwAcceptor: {} -> {}", __FUNCTION__
					, AcceptorToString(acceptor)
					, AcceptorToString(ACCCUSTOP)));
			}
			acceptor = ACCCUSTOP;
			break;

		case DorsHW::POLL_RES::StackMotorFail:
		case DorsHW::POLL_RES::TransportMotorFail:
		case DorsHW::POLL_RES::InitialCassetteStatusFail:
		case DorsHW::POLL_RES::OpticCanalFail:
		case DorsHW::POLL_RES::MagneticCanalFail:
		case DorsHW::POLL_RES::StartTrayFailure:
		case DorsHW::POLL_RES::Group47UnknownFailure:
		case DorsHW::POLL_RES::ValidatorJammed:
		case DorsHW::POLL_RES::Unknown:
		case DorsHW::POLL_RES::DropCassetteJammed:
			if (device != HWERROR)
			{
				m_log->trace(std::format("{}() - WFSCIMSTATUS.fwDevice: {} -> {}", __FUNCTION__
					, DeviceStatusToString(device)
					, DeviceStatusToString(HWERROR)));
			}
			device = HWERROR;

			if (acceptor != ACCCUUNKNOWN)
			{
				m_log->trace(std::format("{}() - WFSCIMSTATUS.fwAcceptor: {} -> {}", __FUNCTION__
					, AcceptorToString(acceptor)
					, AcceptorToString(ACCCUUNKNOWN)));
			}
			acceptor = ACCCUUNKNOWN;
			break;
		case DorsHW::POLL_RES::Cheated:
		case DorsHW::POLL_RES::FishingDetected:
			if (device != USERERROR)
			{
				m_log->trace(std::format("{}() - WFSCIMSTATUS.fwDevice: {} -> {}", __FUNCTION__
					, DeviceStatusToString(device)
					, DeviceStatusToString(USERERROR)));
			}
			device = USERERROR;

			if (acceptor != ACCCUUNKNOWN)
			{
				m_log->trace(std::format("{}() - WFSCIMSTATUS.fwAcceptor: {} -> {}", __FUNCTION__
					, AcceptorToString(acceptor)
					, AcceptorToString(ACCCUUNKNOWN)));
			}
			acceptor = ACCCUUNKNOWN;
			break;
		case DorsHW::POLL_RES::Initialize:
		case DorsHW::POLL_RES::Holding:
		case DorsHW::POLL_RES::Stacking:
		case DorsHW::POLL_RES::Returning:
		case DorsHW::POLL_RES::Rejecting:
		case DorsHW::POLL_RES::RejInsertion:
		case DorsHW::POLL_RES::RejMagnetic:
		case DorsHW::POLL_RES::RejBillInHead:
		case DorsHW::POLL_RES::RejMultiplying:
		case DorsHW::POLL_RES::RejConveying:
		case DorsHW::POLL_RES::RejIdentification:
		case DorsHW::POLL_RES::RejNotebase:
		case DorsHW::POLL_RES::RejIAS:
		case DorsHW::POLL_RES::RejInhibit:
		case DorsHW::POLL_RES::RejDensity:
		case DorsHW::POLL_RES::RejOperation:
		case DorsHW::POLL_RES::RejLength:
		case DorsHW::POLL_RES::RejUV:
		case DorsHW::POLL_RES::RejTape:
		case DorsHW::POLL_RES::RejEntryCassetteSns:
		case DorsHW::POLL_RES::RejFastConveying:
		case DorsHW::POLL_RES::RejTrayClosed:
		case DorsHW::POLL_RES::RejTimeout:
		case DorsHW::POLL_RES::RejFastFeed:
		case DorsHW::POLL_RES::PowerUp:
		case DorsHW::POLL_RES::RejUnknown:
		case DorsHW::POLL_RES::Pause:
		case DorsHW::POLL_RES::BillStacked:
			if (device != BUSY)
			{
				m_log->trace(std::format("{}() - WFSCIMSTATUS.fwDevice: {} -> {}", __FUNCTION__
					, DeviceStatusToString(device)
					, DeviceStatusToString(BUSY)));
			}
			device = BUSY;

			if (acceptor != ACCOK)
			{
				m_log->trace(std::format("{}() - WFSCIMSTATUS.fwAcceptor: {} -> {}", __FUNCTION__
					, AcceptorToString(acceptor)
					, AcceptorToString(ACCOK)));
			}
			acceptor = ACCOK;
			break;
		case DorsHW::POLL_RES::PowerUpWithBillInValidator:
		case DorsHW::POLL_RES::PowerUpWithBillInStacker:
			if (device != ONLINE)
			{
				m_log->trace(std::format("{}() - WFSCIMSTATUS.fwDevice: {} -> {}", __FUNCTION__
					, DeviceStatusToString(device)
					, DeviceStatusToString(ONLINE)));
			}
			device = ONLINE;

			if (acceptor != m_psHandler->powerUpManager_->GetAcceptorPartialState())
			{
				m_log->trace(std::format("{}() - WFSCIMSTATUS.fwAcceptor: {} -> {}", __FUNCTION__
					, AcceptorToString(acceptor)
					, AcceptorToString(m_psHandler->powerUpManager_->GetAcceptorPartialState())));
			}
			acceptor = m_psHandler->powerUpManager_->GetAcceptorPartialState();
			// в дальнейшем здесб опеределяем стоит ли банкнота в стекере. пока что ACCOK
			break;
		}

		//// Корректировка для Acceptor:
		//if (ACCOK == acceptor)
		//{
		//	ULONG ulCount = m_psHandler->m_AllNumberList.GetTotalNotesCount();
		//	if (ulCount >= m_psHandler->m_ulCassetteCapacity) {

		//		if (acceptor != ACCCUSTOP)
		//		{
		//			m_log->trace(std::format("{}() - WFSCIMSTATUS.fwAcceptor: {} -> {}", __FUNCTION__
		//				, XfsGetCimAcceptorStatusText(acceptor)
		//				, XfsGetCimAcceptorStatusText(ACCCUSTOP)));
		//		}
		//		acceptor = ACCCUSTOP;
		//	}
		//	else if (ulCount >= m_psHandler->m_ulThreshold) {
		//		if (acceptor != ACCCUSTATE)
		//		{
		//			m_log->trace(std::format("{}() - WFSCIMSTATUS.fwAcceptor: {} -> {}", __FUNCTION__
		//				, XfsGetCimAcceptorStatusText(acceptor)
		//				, XfsGetCimAcceptorStatusText(ACCCUSTATE)));
		//		}
		//		acceptor = ACCCUSTATE;
		//	}
		//}

		// IntermediateStacker
		switch (currentState)
		{
		case DorsHW::POLL_RES::Accepting:
		case DorsHW::POLL_RES::Stacking:
		case DorsHW::POLL_RES::Returning:
		case DorsHW::POLL_RES::PowerUp:
		case DorsHW::POLL_RES::Idling:
		case DorsHW::POLL_RES::UnitDisabled:
		case DorsHW::POLL_RES::Rejecting:
		case DorsHW::POLL_RES::RejInsertion:
		case DorsHW::POLL_RES::RejMagnetic:
		case DorsHW::POLL_RES::RejBillInHead:
		case DorsHW::POLL_RES::RejMultiplying:
		case DorsHW::POLL_RES::RejConveying:
		case DorsHW::POLL_RES::RejIdentification:
		case DorsHW::POLL_RES::RejNotebase:
		case DorsHW::POLL_RES::RejIAS:
		case DorsHW::POLL_RES::RejInhibit:
		case DorsHW::POLL_RES::RejDensity:
		case DorsHW::POLL_RES::RejOperation:
		case DorsHW::POLL_RES::RejLength:
		case DorsHW::POLL_RES::RejUV:
		case DorsHW::POLL_RES::RejTape:
		case DorsHW::POLL_RES::RejEntryCassetteSns:
		case DorsHW::POLL_RES::RejFastConveying:
		case DorsHW::POLL_RES::RejTrayClosed:
		case DorsHW::POLL_RES::RejTimeout:
		case DorsHW::POLL_RES::RejFastFeed:
		case DorsHW::POLL_RES::RejUnknown:
		case DorsHW::POLL_RES::BillStacked:
		case DorsHW::POLL_RES::BillReturned:
		case DorsHW::POLL_RES::DropCassetteFull:
		case DorsHW::POLL_RES::DropCassetteOutOfPosition:
			intermediateStacker = ISEMPTY;
			break;
		case DorsHW::POLL_RES::StackMotorFail:
		case DorsHW::POLL_RES::TransportMotorFail:
		case DorsHW::POLL_RES::InitialCassetteStatusFail:
		case DorsHW::POLL_RES::OpticCanalFail:
		case DorsHW::POLL_RES::MagneticCanalFail:
		case DorsHW::POLL_RES::StartTrayFailure:
		case DorsHW::POLL_RES::Group47UnknownFailure:
		case DorsHW::POLL_RES::ValidatorJammed:
		case DorsHW::POLL_RES::DropCassetteJammed:
		case DorsHW::POLL_RES::Cheated:
		case DorsHW::POLL_RES::FishingDetected:
		case DorsHW::POLL_RES::PowerUpWithBillInValidator:
		case DorsHW::POLL_RES::PowerUpWithBillInStacker:
		case DorsHW::POLL_RES::Initialize:
		case DorsHW::POLL_RES::Pause:
		case DorsHW::POLL_RES::EscrowPos:
		case DorsHW::POLL_RES::Holding:
			intermediateStacker = ISFULL;
			break;
		}

		// BanknoteReader
		switch (currentState)
		{
		case DorsHW::POLL_RES::StackMotorFail:
		case DorsHW::POLL_RES::TransportMotorFail:
		case DorsHW::POLL_RES::OpticCanalFail:
		case DorsHW::POLL_RES::MagneticCanalFail:
		case DorsHW::POLL_RES::Group47UnknownFailure:
			banknoteReader = BNRINOP;
			break;
		default:
			banknoteReader = BNROK;
			break;
		}

		// StackerItems
		switch (currentState) {
		case DorsHW::POLL_RES::PowerUpWithBillInValidator:
		case DorsHW::POLL_RES::PowerUpWithBillInStacker:
		case DorsHW::POLL_RES::Holding:
		case DorsHW::POLL_RES::ValidatorJammed:
		case DorsHW::POLL_RES::DropCassetteJammed:
		case DorsHW::POLL_RES::Cheated:
		case DorsHW::POLL_RES::FishingDetected:
		case DorsHW::POLL_RES::Pause:
		case DorsHW::POLL_RES::EscrowPos:
			stackerItems = CUSTOMERACCESS;
			break;
		default:
			stackerItems = NOITEMS;
			break;
		}

		//// Positions
		//Xfs::Structs::CimInPos position;
		//position.position = POSINCENTER;
		//position.shutter = SHTNOTSUPPORTED;
		switch (currentState)
		{
		case DorsHW::POLL_RES::PowerUp:
		case DorsHW::POLL_RES::Idling:
		case DorsHW::POLL_RES::UnitDisabled:
		case DorsHW::POLL_RES::EscrowPos:
		case DorsHW::POLL_RES::BillStacked:
		case DorsHW::POLL_RES::Holding:
		case DorsHW::POLL_RES::Stacking:
		case DorsHW::POLL_RES::DropCassetteFull:
		case DorsHW::POLL_RES::DropCassetteOutOfPosition:
		case DorsHW::POLL_RES::OpticCanalFail:
		case DorsHW::POLL_RES::MagneticCanalFail:
		case DorsHW::POLL_RES::StartTrayFailure:
		case DorsHW::POLL_RES::Group47UnknownFailure:
		case DorsHW::POLL_RES::StackMotorFail:
		case DorsHW::POLL_RES::TransportMotorFail:
		case DorsHW::POLL_RES::InitialCassetteStatusFail:
			// Пусто, банкнот на транспорте точно нет
			//position.transport = TPOK;
			//position.transportStatus = TPSTATEMPTY;
			//position.positionStatus = PSEMPTY;
			break;

		case DorsHW::POLL_RES::ValidatorJammed:
		case DorsHW::POLL_RES::DropCassetteJammed:
			// Замятие на тракте
			//position.transport = TPINOP;
			//position.transportStatus = TPSTATNOTEMPTY_UNK;
			//position.positionStatus = PSUNKNOWN;
			break;

		case DorsHW::POLL_RES::Cheated:
		case DorsHW::POLL_RES::FishingDetected:
		case DorsHW::POLL_RES::Pause:
		case DorsHW::POLL_RES::Initialize:
		case DorsHW::POLL_RES::PowerUpWithBillInStacker:
		case DorsHW::POLL_RES::PowerUpWithBillInValidator:
			// Странные промежуточные состояния
			//position.positionStatus = PSUNKNOWN;
			//position.transport = TPUNKNOWN;
			//position.transportStatus = TPSTATNOTEMPTY_UNK;
			break;
		case DorsHW::POLL_RES::Accepting:
		case DorsHW::POLL_RES::Returning:
		case DorsHW::POLL_RES::Rejecting:
		case DorsHW::POLL_RES::RejInsertion:
		case DorsHW::POLL_RES::RejMagnetic:
		case DorsHW::POLL_RES::RejBillInHead:
		case DorsHW::POLL_RES::RejMultiplying:
		case DorsHW::POLL_RES::RejConveying:
		case DorsHW::POLL_RES::RejIdentification:
		case DorsHW::POLL_RES::RejNotebase:
		case DorsHW::POLL_RES::RejIAS:
		case DorsHW::POLL_RES::RejInhibit:
		case DorsHW::POLL_RES::RejDensity:
		case DorsHW::POLL_RES::RejOperation:
		case DorsHW::POLL_RES::RejLength:
		case DorsHW::POLL_RES::RejUV:
		case DorsHW::POLL_RES::RejTape:
		case DorsHW::POLL_RES::RejEntryCassetteSns:
		case DorsHW::POLL_RES::RejFastConveying:
		case DorsHW::POLL_RES::RejTrayClosed:
		case DorsHW::POLL_RES::RejTimeout:
		case DorsHW::POLL_RES::RejFastFeed:
		case DorsHW::POLL_RES::RejUnknown:
		case DorsHW::POLL_RES::BillReturned:
			// Позиция приёма/выдачи точно не пустая
			//position.positionStatus = PSNOTEMPTY;
			//position.transport = TPOK;
			//position.transportStatus = TPSTATNOTEMPTYCUST;
			break;
		}
		//positions.push_back(position);

		//position.position = POSOUTCENTER;
		//positions.push_back(position);
	}

	mixedMode = 0; // WFS_CIM_MIXEDMEDIANOTACTIVE
	antiFraudModule = false;
}