#include "GetinfoBanknoteTypes.h"


GetinfoBanknoteTypes* GetinfoBanknoteTypes::pinstance_ = nullptr;
std::mutex GetinfoBanknoteTypes::m_sendMutex;
std::mutex GetinfoBanknoteTypes::m_instanceMutex;
std::mutex GetinfoBanknoteTypes::m_fillMutex;


void GetinfoBanknoteTypes::fillResult()
{
	std::lock_guard<std::mutex> lock(m_fillMutex);
	if (m_psHandler == nullptr) return;
	//notesList = m_psHandler->GetBanknoteTypes();
}

std::string GetinfoBanknoteTypes::getResult() const
{
	//if (notesList.empty()) return "EMPTY\n";

	std::ostringstream oss;

	//for (auto& note : notesList)
	//{
	//	oss << note.usNoteID << ", "
	//		<< note.strCurrencyID << ", "
	//		<< note.ulValues << ", ";
	//}
	return oss.str();
}
std::string GetinfoBanknoteTypes::PrintFromResult() const
{
	//if (notesList.empty()) return "EMPTY\n";

	std::ostringstream oss;

	//oss << "NumOfNoteRTypes: " << notesList.size();
	//oss << "\nNoteID"
	//	<< "\tCurID"
	//	<< "\tValues"
	//	<< "\tRelease"
	//	<< "\tConf\n";
	//for (auto& note : notesList)
	//{
	//	oss << note.usNoteID << "\t"
	//		<< note.strCurrencyID << "\t"
	//		<< note.ulValues << "\t"
	//		<< note.usRelease << "\t"
	//		<< note.bConfigured << "\n";
	//}
	return oss.str();
}

uint32_t GetinfoBanknoteTypes::validateProcessingConditions()
{
	if (!m_psHandler->m_pDevice->IsDeviceInitialized()) {
		return ERR_DEV_NOT_READY;
	}

	switch (m_psHandler->m_State)
	{
	case FS365::HW::Dors::DorsHW::POLL_RES::PowerUp:
	case FS365::HW::Dors::DorsHW::POLL_RES::Initialize:
	case FS365::HW::Dors::DorsHW::POLL_RES::Idling:
	case FS365::HW::Dors::DorsHW::POLL_RES::Accepting:
	case FS365::HW::Dors::DorsHW::POLL_RES::Stacking:
	case FS365::HW::Dors::DorsHW::POLL_RES::Returning:
	case FS365::HW::Dors::DorsHW::POLL_RES::Holding:
	case FS365::HW::Dors::DorsHW::POLL_RES::EscrowPos:
	case FS365::HW::Dors::DorsHW::POLL_RES::UnitDisabled:
	case FS365::HW::Dors::DorsHW::POLL_RES::DropCassetteFull:
	case FS365::HW::Dors::DorsHW::POLL_RES::DropCassetteOutOfPosition:
	case FS365::HW::Dors::DorsHW::POLL_RES::ValidatorJammed:
	case FS365::HW::Dors::DorsHW::POLL_RES::DropCassetteJammed:
	case FS365::HW::Dors::DorsHW::POLL_RES::Cheated:
	case FS365::HW::Dors::DorsHW::POLL_RES::FishingDetected:	// Новое название по протоколу об. дан. DORS 210BA
	case FS365::HW::Dors::DorsHW::POLL_RES::CassetteBracketOpen:
	case FS365::HW::Dors::DorsHW::POLL_RES::Pause:
	case FS365::HW::Dors::DorsHW::POLL_RES::StackMotorFail:
	case FS365::HW::Dors::DorsHW::POLL_RES::TransportMotorFail:
	case FS365::HW::Dors::DorsHW::POLL_RES::InitialCassetteStatusFail:
	case FS365::HW::Dors::DorsHW::POLL_RES::OpticCanalFail:
	case FS365::HW::Dors::DorsHW::POLL_RES::MagneticCanalFail:
	case FS365::HW::Dors::DorsHW::POLL_RES::StartTrayFailure:
	case FS365::HW::Dors::DorsHW::POLL_RES::Group47UnknownFailure:
		break;
	default:
		return ERR_DEV_NOT_READY;
	}

	return SUCCESS;
}

