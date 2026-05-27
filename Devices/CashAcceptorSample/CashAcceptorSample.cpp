//CashAcceptorSample.cpp

#include "CashAcceptorSample.hpp"
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
//#include <boost/asio/awaitable.hpp>
#include <chrono>
#include "../../framework/core/CashAcceptor/CashInEndCompletion.hpp"
#include "../../framework/core/CashAcceptor/CashInRollbackCompletion.hpp"
#include "../../framework/core/SettingModule/SettingModule.h"
#include "./Managers/PowerUpManager/PowerUpManager.hpp"
#include "./Managers/EscrowManager/EscrowManager.hpp"
#include "../../framework/ServiceClasses/INFO_MODULES/GetInfoStatus/GetInfoStatus.h"
#include "../../framework/Common/AsciiHexConversions/AsciiHexConversions.h"
//#include "../../framework/server/CommandDispatcher.hpp"

namespace XFS4IoTSP::CashAcceptor::Sample
{
	using namespace std::chrono_literals;

	////Static member initialization
	//const std::map<std::string, XFS4IoTFramework::Common::CashManagementCapabilitiesClass::BanknoteItem>
	//	CashAcceptorSample::allBanknoteIDs_ = {
	//		{
	//			"typeRUB5",
	//			XFS4IoTFramework::Common::CashManagementCapabilitiesClass::BanknoteItem(1, "RUB", 5.0, 1, true)
	//		},
	//		{
	//			"typeRUB10",
	//			XFS4IoTFramework::Common::CashManagementCapabilitiesClass::BanknoteItem(2, "RUB", 10.0, 1, true)
	//		},
	//		{
	//			"typeRUB20",
	//			XFS4IoTFramework::Common::CashManagementCapabilitiesClass::BanknoteItem(3, "RUB", 20.0, 1, true)
	//		},
	//		{
	//			"typeRUB50",
	//			XFS4IoTFramework::Common::CashManagementCapabilitiesClass::BanknoteItem(4, "RUB", 50.0, 1, true)
	//		},
	//		{
	//			"typeRUB100",
	//			XFS4IoTFramework::Common::CashManagementCapabilitiesClass::BanknoteItem(5, "RUB", 100.0, 1, true)
	//		},
	//		{
	//			"typeRUB200",
	//			XFS4IoTFramework::Common::CashManagementCapabilitiesClass::BanknoteItem(5, "RUB", 200.0, 1, true)
	//		},
	//		{
	//			"typeRUB500",
	//			XFS4IoTFramework::Common::CashManagementCapabilitiesClass::BanknoteItem(6, "RUB", 500.0, 1, true)
	//		}
	//};

	using Pos = XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum;
	using PS = XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusClass;
	using Sh = XFS4IoTFramework::Common::CashManagementStatusClass::ShutterEnum;
	using PSt = XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum;
	using Tr = XFS4IoTFramework::Common::CashManagementStatusClass::TransportEnum;
	using TSt = XFS4IoTFramework::Common::CashManagementStatusClass::TransportStatusEnum;

	static std::map<Pos, PS> makeLocal()
	{
		std::map<Pos, PS> m;
		m.try_emplace(Pos::InCenter, Sh::NotSupported, PSt::Empty, Tr::Ok, TSt::Empty);
		//m.try_emplace(Pos::InDefault, Sh::Closed, PSt::Empty, Tr::Ok, TSt::Empty);
		m.try_emplace(Pos::OutCenter, Sh::NotSupported, PSt::Empty, Tr::Ok, TSt::Empty);
		//m.try_emplace(Pos::OutDefault, Sh::Closed, PSt::Empty, Tr::Ok, TSt::Empty);
		return m; // (NRVO)
	}
	//<summary>
	//Constructor
	//</summary>
	CashAcceptorSample::CashAcceptorSample(
		std::unique_ptr<FS365::HW::Dors::DorsHW> device
		, FS365::HW::Dors::CIdentification& idn
		, std::shared_ptr<ILogger> logger
		, std::map<std::string, XFS4IoTFramework::Common::CashManagementCapabilitiesClass::BanknoteItem> &allBanknoteIDs
		, std::shared_ptr<XFS4IoTFramework::CashManagement::CashInStatusClass> cashInStatus
		)
		: logger_(logger)
		, m_pDevice(std::move(device))
		, m_idn(idn)
		, setServiceProvider_(nullptr)
		, cashAcceptorService_(nullptr)
		, allBanknoteIDs_(allBanknoteIDs) 

		, commonStatus_(std::make_shared<XFS4IoTFramework::Common::CommonStatusClass>(
			XFS4IoTFramework::Common::CommonStatusClass::DeviceEnum::Online,
			XFS4IoTFramework::Common::CommonStatusClass::PositionStatusEnum::InPosition,
			0,
			XFS4IoTFramework::Common::CommonStatusClass::AntiFraudModuleEnum::NotSupported,
			XFS4IoTFramework::Common::CommonStatusClass::ExchangeEnum::Inactive,
			XFS4IoTFramework::Common::CommonStatusClass::EndToEndSecurityEnum::NotEnforced))

		, cashAcceptorStatus_(std::make_shared<XFS4IoTFramework::Common::CashAcceptorStatusClass>(
			XFS4IoTFramework::Common::CashAcceptorStatusClass::IntermediateStackerEnum::Empty,
			XFS4IoTFramework::Common::CashAcceptorStatusClass::StackerItemsEnum::NoItems,
			XFS4IoTFramework::Common::CashAcceptorStatusClass::BanknoteReaderEnum::Ok,
			false,
			makeLocal()
		))

		, cashManagementStatus_(std::make_shared<XFS4IoTFramework::Common::CashManagementStatusClass>(
			XFS4IoTFramework::Common::CashManagementStatusClass::DispenserEnum::NotSupported,
			XFS4IoTFramework::Common::CashManagementStatusClass::AcceptorEnum::Ok))

		, positionStatus_(
			XFS4IoTFramework::Common::CashManagementStatusClass::ShutterEnum::NotSupported,
			XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::Empty,
			XFS4IoTFramework::Common::CashManagementStatusClass::TransportEnum::Ok,
			XFS4IoTFramework::Common::CashManagementStatusClass::TransportStatusEnum::Empty)

		, cashAcceptorCapabilities_(
			XFS4IoTFramework::Common::CashManagementCapabilitiesClass::TypeEnum::SelfServiceBill,
			1, // MaxCashInItems
			false, // Shutter
			true, // ShutterControl
			1, // IntermediateStacker
			true, // ItemsTakenSensor
			std::map<XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum, XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass>{
				//{ XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::InDefault,
				//	XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass(
				//		XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::UsageEnum::In,
				//		true, // ShutterControl
				//		true,  // ItemsTakenSensor
				//		true,  // ItemsInsertedSensor
				//		false, // PresentControl
				//		false, // PreparePresent
				//		XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum::Retract) },
				{ XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::InCenter,
					XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass(
						static_cast<XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::UsageEnum>(
							static_cast<int>(XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::UsageEnum::In)),
						true, // ShutterControl
						false,  // ItemsTakenSensor
						true,  // ItemsInsertedSensor
						false, // PresentControl
						false, // PreparePresent
						XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum::NotSupported) },
				{ XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::OutCenter,
					XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass(
						static_cast<XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::UsageEnum>(
							static_cast<int>(XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::UsageEnum::Refuse) |
							static_cast<int>(XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::UsageEnum::Rollback)),
						true, // ShutterControl
						true,  // ItemsTakenSensor
						false,  // ItemsInsertedSensor
						false, // PresentControl
						false, // PreparePresent
						XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::PositionClass::RetractAreaEnum::NotSupported) }
	},
			XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractAreaEnum::NotSupported, //RetractAreaEnum::NotSupported
			XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractTransportActionEnum::NotSupported,
			XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractStackerActionEnum::NotSupported,
			XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::CashInLimitEnum::NotSupported, // byTotalItems
			XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::CountActionEnum::NotSupported,
			XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass::RetainCounterfeitActionEnum::NotSupported)

		, cashManagementCapabilities_(std::make_shared<XFS4IoTFramework::Common::CashManagementCapabilitiesClass>(
			static_cast<XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum>(
				static_cast<int>(XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::OutCenter)),
			true, // ShutterControl
			XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractAreaEnum::Retract, // Retract to cash in position
			XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractTransportActionEnum::Retract, // Retract to cash in position
			XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractStackerActionEnum::Retract, // Retract to cash in position
			XFS4IoTFramework::Common::CashManagementCapabilitiesClass::ExchangeTypesEnum::ByHand,	// ExchangeTypes
			XFS4IoTFramework::Common::CashManagementCapabilitiesClass::ItemInfoTypesEnum::NotSupported, // ItemInfoTypes
			false, // CashBox
			false// ClassificationList
			//, allBanknoteIDs
		))

		, commonCapabilities_(std::make_shared<XFS4IoTFramework::Common::CommonCapabilitiesClass>
			//CommonInterface (required)
			(XFS4IoTFramework::Common::CommonCapabilitiesClass::CommonInterfaceClass(
				std::vector<XFS4IoTFramework::Common::CommonCapabilitiesClass::CommonInterfaceClass::CommandEnum>{
		XFS4IoTFramework::Common::CommonCapabilitiesClass::CommonInterfaceClass::CommandEnum::Capabilities,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CommonInterfaceClass::CommandEnum::Status
	},
				std::vector<XFS4IoTFramework::Common::CommonCapabilitiesClass::CommonInterfaceClass::EventEnum>{
		XFS4IoTFramework::Common::CommonCapabilitiesClass::CommonInterfaceClass::EventEnum::StatusChangedEvent,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CommonInterfaceClass::EventEnum::ErrorEvent
	}),
				//CashAcceptorInterface (optional)
				XFS4IoTFramework::Common::CommonCapabilitiesClass::CashAcceptorInterfaceClass(
					std::vector<XFS4IoTFramework::Common::CommonCapabilitiesClass::CashAcceptorInterfaceClass::CommandEnum>{
		XFS4IoTFramework::Common::CommonCapabilitiesClass::CashAcceptorInterfaceClass::CommandEnum::CashInStart,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashAcceptorInterfaceClass::CommandEnum::CashIn,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashAcceptorInterfaceClass::CommandEnum::CashInEnd,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashAcceptorInterfaceClass::CommandEnum::CashInRollback,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashAcceptorInterfaceClass::CommandEnum::CashUnitCount,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashAcceptorInterfaceClass::CommandEnum::ConfigureNoteReader,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashAcceptorInterfaceClass::CommandEnum::ConfigureNoteTypes,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashAcceptorInterfaceClass::CommandEnum::GetCashInStatus,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashAcceptorInterfaceClass::CommandEnum::GetPositionCapabilities,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashAcceptorInterfaceClass::CommandEnum::GetPresentStatus
	},
					std::vector<XFS4IoTFramework::Common::CommonCapabilitiesClass::CashAcceptorInterfaceClass::EventEnum>{
		XFS4IoTFramework::Common::CommonCapabilitiesClass::CashAcceptorInterfaceClass::EventEnum::InputRefuseEvent,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashAcceptorInterfaceClass::EventEnum::InsertItemsEvent
	}),
				//CashManagementInterface (optional)
				XFS4IoTFramework::Common::CommonCapabilitiesClass::CashManagementInterfaceClass(
					std::vector<XFS4IoTFramework::Common::CommonCapabilitiesClass::CashManagementInterfaceClass::CommandEnum>{
		XFS4IoTFramework::Common::CommonCapabilitiesClass::CashManagementInterfaceClass::CommandEnum::Retract,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashManagementInterfaceClass::CommandEnum::OpenShutter,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashManagementInterfaceClass::CommandEnum::CloseShutter,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashManagementInterfaceClass::CommandEnum::CalibrateCashUnit,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashManagementInterfaceClass::CommandEnum::Reset,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashManagementInterfaceClass::CommandEnum::GetTellerInfo,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashManagementInterfaceClass::CommandEnum::SetTellerInfo
	},
					std::vector<XFS4IoTFramework::Common::CommonCapabilitiesClass::CashManagementInterfaceClass::EventEnum>{
		XFS4IoTFramework::Common::CommonCapabilitiesClass::CashManagementInterfaceClass::EventEnum::ShutterStatusChangedEvent,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashManagementInterfaceClass::EventEnum::NoteErrorEvent,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashManagementInterfaceClass::EventEnum::ItemsTakenEvent,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashManagementInterfaceClass::EventEnum::ItemsPresentedEvent,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashManagementInterfaceClass::EventEnum::ItemsInsertedEvent,
			XFS4IoTFramework::Common::CommonCapabilitiesClass::CashManagementInterfaceClass::EventEnum::IncompleteRetractEvent
	}),

				//DepositInterface (optional - not used, so pass nullopt)
				std::nullopt,
				//DeviceInformation
				std::vector<XFS4IoTFramework::Common::CommonCapabilitiesClass::DeviceInformationClass>{
		XFS4IoTFramework::Common::CommonCapabilitiesClass::DeviceInformationClass(
			idn.strPartNumber,        // ModelName
			idn.strSerialNumber,  // SerialNumber
			"",              // revision Version
			"DORS-DBA 210",    // ModelDescription
			std::vector<XFS4IoTFramework::Common::CommonCapabilitiesClass::FirmwareClass>
		{
			XFS4IoTFramework::Common::CommonCapabilitiesClass::FirmwareClass(
				"Acme Software", // FirmwareName
				idn.strSoftwareVersion,     // FirmwareVersion
				""),     // HardwareRevision
			XFS4IoTFramework::Common::CommonCapabilitiesClass::FirmwareClass(
				"Acme NoteBase", // FirmwareName
				idn.strNotebaseVersion,     // FirmwareVersion
				"")     // HardwareRevision
		},
			std::vector<XFS4IoTFramework::Common::CommonCapabilitiesClass::SoftwareClass>{
			XFS4IoTFramework::Common::CommonCapabilitiesClass::SoftwareClass(
				"", // SoftwareName
				"")     // SoftwareVersion
		})
	},
				//PowerSaveControl (obsolete)
				false,
				//AntiFraudModule
				false,
				//EndToEndSecurity
				XFS4IoTFramework::Common::CommonCapabilitiesClass::EndToEndSecurityClass(
					XFS4IoTFramework::Common::CommonCapabilitiesClass::EndToEndSecurityClass::RequiredEnum::Always,
					false, // HardwareSecurityElement (Sample is software)
					XFS4IoTFramework::Common::CommonCapabilitiesClass::EndToEndSecurityClass::ResponseSecurityEnabledEnum::NotSupported,
					std::nullopt // CommandNonceTimeout
				)))
		, cashInStatus_(cashInStatus)
		, deviceLockStatus_()
		, cashTakenSignal_(0)
		, hwServiceBytes({ 0 })
		, m_bAdditionalRes(false)
		, m_State(FS365::HW::Dors::DorsHW::POLL_RES::Unknown)
		, m_PreviousState{ FS365::HW::Dors::DorsHW::POLL_RES::Unknown }
		, m_stateMachine(FS365::HW::Dors::DorsHW::POLL_RES::Unknown)
		, bConjointCashInIsActive(false)
		, m_bCassetteHasBeenReplaced{ false }
		, acceptedItems_(std::make_shared<std::map<std::string, XFS4IoTFramework::Storage::CashItemCountClass>>())
	{
		if (!logger_)
		{
			throw std::invalid_argument("В конструкторе CashAcceptorSample получен недопустимый параметр. logger");
		}
	}

	//<summary>
	//Destructor
	//</summary>
	CashAcceptorSample::~CashAcceptorSample()
	{
		//Cleanup if needed
	}


	void CashAcceptorSample::Initialize()
	{
		powerUpManager_ = std::make_unique<PowerUpManager>(shared_from_this(), PowerUpWithBillStrategy::ALLOW_ACCEPT);
		escrowManager_ = std::make_unique<XFS4IoTSP::CashAcceptor::Sample::EscrowManager>(logger_, shared_from_this());


		if (!allBanknoteIDs_.empty()) m_bHasBillTableCache = true;

		// =========================================================================
		// Инициализация storage information
		// =========================================================================
		{
			auto cassetteConfig = MakeCashInCassetteConfig();

			cashUnitInfo_.try_emplace(
				cassetteConfig.GetId(),
				cassetteConfig);

			// Начальные значения счетчиков
			auto& storageInfo = cashUnitInfo_.at(cassetteConfig.GetId());
			//PersistentDatasHandler::GetInstance()->;
			// TOFO: Нужно достать данные из персистентного хранилища, если оно есть, или из устройства, если оно поддерживает счетчики, вместо того, чтобы просто выставлять 0. И не забыть обновлять эти счетчики при каждой операции приема наличных и при каждой загрузке кассет.
			storageInfo.unitCount_ =
				XFS4IoTFramework::Storage::CashUnitCountClass(
					nullptr,
					std::make_shared<XFS4IoTFramework::Storage::StorageCashInCountClass>(),
					0); // общее количество купюр в кассете

			// пример:
			// storageInfo.unitCount_.GetCashItemCount()->SetCount(0);

			storageInfo.storageStatus_ =
				XFS4IoTFramework::Storage::CashUnitStorage::StatusEnum::Good;

			storageInfo.unitStatus_ =
				XFS4IoTFramework::Storage::CashStatusClass::ReplenishmentStatusEnum::Healthy;
		}


		if (m_pDevice->IsDeviceInitialized()) {
			logger_->trace(std::format("{}() - Инициализация сервис-провайдера CIM прошла успешно", __FUNCTION__), LOGLEVEL1);
			m_pollingThread = std::make_unique<std::jthread>([this]() {
				fnDevicePolling();
				});
		}
	}

	void CashAcceptorSample::fnDevicePolling()
	{
		using enum FS365::HW::Dors::DorsHW::POLL_RES;
		bool _bFirstIteration{ true };

		// Первая загрузка
		bool bFirstOnline = true;

		// Подача команды HOLD не чаще чем раз в 5 сек
		// Команда HOLD удерживает банкноту в Escrow еще на 10 сек
		auto lastHoldSend = std::chrono::steady_clock::now();

		// ведение журнала во время простоя устройства (для выявления проблем с коммуникацией, которые проявляются именно при простое)
		// проверка проводиться каждые 5 мин во время простоя
		auto loggingDuringIdleTime = std::chrono::steady_clock::now();

		// Сообщаем устройству, что достигли/не достигли уровня HIGH(кассета почти заполнена):
		//		1. При запуске ПО
		//		2. При обнаружении потери связи
		bool bRefreshCassetteHighLevel = true;


		for (;;)
		{
			if (_bFirstIteration) {
				_bFirstIteration = false;
				m_pDevice->states_stack_transfer_enable(true);

			}

			// Опрос устройства
			auto _newState = m_pDevice->Poll(m_bAdditionalRes, hwServiceBytes);
			//std::cout << "Worker THREAD: " << std::this_thread::get_id() << " POLLING: CURRENT STATE -> " << FS365::HW::Dors::GetPOLL_RESText(_newState) << "\t" << getCurrentTimeFormatted() << std::endl;

			// Если получаем проблему с портом, то делаем 10 попыток восстановления, потом только отправляем плохой статус, если не восстановились.
			if (_newState ==FS365::HW::Dors::DorsHW::POLL_RES::PortError) {

				{
					if (GetInfoStatus::GetInstance()->device != DeviceStatusPackage::NODEVICE && GetInfoStatus::GetInstance()->device != DeviceStatusPackage::POWEROFF)
					{
						logger_->trace(std::format("{}() - common/device: {} -> {}", __FUNCTION__
							, DeviceStatusToString(GetInfoStatus::GetInstance()->device)
							, DeviceStatusToString(m_pDevice->GetSoftwareConfigurationFault() ? DeviceStatusPackage::NODEVICE : DeviceStatusPackage::POWEROFF)));
					}
					GetInfoStatus::GetInstance()->device = m_pDevice->GetSoftwareConfigurationFault() ? DeviceStatusPackage::NODEVICE : DeviceStatusPackage::POWEROFF;

					if (GetInfoStatus::GetInstance()->acceptor != AcceptorPackage::ACCCUUNKNOWN)
					{
						logger_->trace(std::format("{}() - cashManagement/acceptor: {} -> {}", __FUNCTION__
							, AcceptorToString(GetInfoStatus::GetInstance()->acceptor)
							, AcceptorToString(AcceptorPackage::ACCCUUNKNOWN)));
					}
					GetInfoStatus::GetInstance()->acceptor = AcceptorPackage::ACCCUUNKNOWN;
				}

				if (m_AttemptsToRestorePort <= 10) {
					std::this_thread::sleep_for(std::chrono::seconds(1));
					m_AttemptsToRestorePort++;
					continue;
				}
				else {
					m_AttemptsToRestorePort = 0;
					if (!m_SetAsReadyToServe) {
						m_SetAsReadyToServe = true;
					}
				}
			}
			else {
				if (!m_SetAsReadyToServe) {
					m_SetAsReadyToServe = true;
				}
			}

			// Действия, требующие повтора в условиях пребывания в одном и том же состоянии
			if (_newState == m_State) {

				if (!bConjointCashInIsActive && (_newState == Holding || _newState == EscrowPos)) // Нужно спросить у Мишы подробности, нужно ли оставлять данный код
				{
					if (std::chrono::steady_clock::now() - lastHoldSend >= 5000ms)
					{
						if (m_pDevice->Hold() == FS365::HW::Dors::DorsHW::RESULT::Ok)
						{
							lastHoldSend = std::chrono::steady_clock::now();
						}
					}
				}

				if (std::chrono::steady_clock::now() - loggingDuringIdleTime >= 5min)
				{
					if (!hwServiceBytes.empty()) {
						loggingDuringIdleTime = std::chrono::steady_clock::now();
						logger_->trace(std::format("{}() SERVICE BYTES во время простоя: {}", __FUNCTION__, AsciiHexConversions::BinToAsciiString(hwServiceBytes, true).c_str()), LOGLEVEL1);
					}
				}

				// Возвращаемся на опрос
				continue;
			}

			// Фиксируем новое состояние и производим его обработку
			m_PreviousState = m_State;
			m_State = _newState;

			// Онулируем loggingDuringIdleTime при каждом изменении состояния, чтобы не засорять лог сообщениями о SERVICE BYTES, если устройство активно взаимодействует с банкнотами, а не простаивает
			loggingDuringIdleTime = std::chrono::steady_clock::now();

			loggingChangeStatus(_newState);

			if (!hwServiceBytes.empty()) {
				logger_->trace(std::format("{}() SERVICE BYTES: {}", __FUNCTION__, AsciiHexConversions::BinToAsciiString(hwServiceBytes, true).c_str()), LOGLEVEL1);
			}

			if (bFirstOnline
				&&FS365::HW::Dors::DorsHW::_isOnline(_newState)
				//&& _newState !=FS365::HW::Dors::DorsHW::POLL_RES::Initialize
				) {
				bFirstOnline = false;
				//FillCashInfo(m_cashUnitInfo);
				//SendDeviceStatusEvent(GetLogicalServiceName().c_str(), DEVONLINE);
				//m_executorThread = std::make_unique<std::jthread>([this]() {
				//	ExecutorThread();
				//	});

				//if (!::SetThreadPriority(m_executorThread->native_handle(), THREAD_PRIORITY_TIME_CRITICAL)) {
				//	unsigned long dwResult = ::GetLastError();
				//	logger_->warn(std::format("{}() - НЕВОЗМОЖНО ИЗМЕНИТЬ ПРИОРИТЕТ ПРОЦЕССА: {}", __FUNCTION__, dwResult), LOGLEVEL1);
				//	std::cout << __FUNCTION__ << "() - НЕВОЗМОЖНО ИЗМЕНИТЬ ПРИОРИТЕТ ПРОЦЕССА: " << dwResult;
				//}

				// Перезагрузка ПО устройства
				// @note: В состоянии PowerUp устройство не требует перезагрузки
				if (!FS365::HW::Dors::DorsHW::_isPowerUp(_newState)) {
					m_pDevice->Reboot();
					continue;
				}
			}

			logger_->trace(std::format("POLLING. NEW STATE {} -> {}", FS365::HW::Dors::DorsHW::PollResToString(m_PreviousState), FS365::HW::Dors::DorsHW::PollResToString(_newState)));
			{
				std::lock_guard<std::mutex> lock{ m_mtx };
				m_pDevice->println(std::format("New state ------------------- {}", FS365::HW::Dors::DorsHW::PollResToString(_newState)));
			}

//
//
//				// Для удобства потребителей, фиксируем номинал текущей обрабатываемой банкноты централизованно
//			{
//				// 1. Сброс номинала
//				if (DorsHW::_isRejCode(m_State) ||
//					DorsHW::POLL_RES::Accepting == m_State ||
//					DorsHW::POLL_RES::Idling == m_State ||
//					DorsHW::POLL_RES::UnitDisabled == m_State) {
//					m_usCurrentNoteID = 0;
//				}
//
//				// 2. Установка номинала
//				if (m_State ==FS365::HW::Dors::DorsHW::POLL_RES::EscrowPos
//					|| ((m_State ==FS365::HW::Dors::DorsHW::POLL_RES::RejInhibit)
//						&& (m_bAdditionalRes != 0xFE)))
//				{
//					auto it = m_denominationsPhysicalToLogical.find(m_bAdditionalRes);
//					if (it != m_denominationsPhysicalToLogical.cend()) {
//						if (m_State ==FS365::HW::Dors::DorsHW::POLL_RES::RejInhibit)
//						{
//							m_usCurrentNoteID = 0;
//							logger_->trace(std::format("{}() - НОМИНАЛ БАНКНОТЫ РАСПОЗНАН: BYTE2 = {}, НО НЕРАЗРЕШЕНА ДЛЯ ПРИЕМА", __FUNCTION__, m_bAdditionalRes), LOGLEVEL1);
//							// notify("НОМИНАЛ БАНКНОТЫ НЕ РАСПОЗНАН: BYTE2 =  " + std::to_string(m_bAdditionalRes), 0x00000002);
//							m_pDevice->println("НОМИНАЛ БАНКНОТЫ НЕ РАСПОЗНАН: BYTE2 = " + std::to_string(m_bAdditionalRes));
//						}
//						else {
//							m_usCurrentNoteID = it->second;
//							logger_->trace(std::format("{}() - НОМИНАЛ РАСПОЗНАН: БАНКНОТА С ИНДЕКСОМ {}", __FUNCTION__, m_usCurrentNoteID), LOGLEVEL1);
//							// notify(" РАСПОЗНАННАЯ БАНКНОТА: Id = " + std::to_string(m_usCurrentNoteID), 0x00000002);
//						}
//					}
//				}
//
//				//if (m_State ==FS365::HW::Dors::DorsHW::POLL_RES::PowerUp
//				//    || m_State ==FS365::HW::Dors::DorsHW::POLL_RES::PowerUpWithBillInValidator
//				//    || m_State ==FS365::HW::Dors::DorsHW::POLL_RES::PowerUpWithBillInStacker)
//				//{
//				//    m_pDevice->CassetteHighLevel(false);
//				//    m_pDevice->Reset();
//				//    //m_pDevice->states_stack_transfer_enable(true);
//				//}
//			}
//
			m_stateMachine.ChangeState(m_State);
//
//			switch (m_State)
//			{
//				// 
//			case FS365::HW::Dors::DorsHW::POLL_RES::DropCassetteOutOfPosition:
//				GetInfoStatus::GetInstance()->fillResult();
//				SetManipulated();
//				SendCashUnitInfoChanged();
//				m_bCassetteMissing = true;
//				break;
//			case FS365::HW::Dors::DorsHW::POLL_RES::Initialize:
//			{
//				if (m_bCassetteMissing) {
//					// Если кассету вернули обратно, а мы не в цикле инкасации то считаем это манипуляцией
//					if (!m_bExchangeInProgress) {
//						SetManipulated();
//					}
//					m_bCassetteMissing = false;
//					SendCashUnitInfoChanged();
//				}
//
//				m_pDevice->states_stack_transfer_enable(true);
//
//				auto idRefresResult = m_pDevice->Identification(m_idn);
//				if (DorsHW::RESULT::Ok == idRefresResult) {
//					m_bRefreshIdData = false;
//				}
//
//				if (m_PreviousState ==FS365::HW::Dors::DorsHW::POLL_RES::Accepting) {
//					SendItemsPresented();
//				}
//			}
//			break;
//			case FS365::HW::Dors::DorsHW::POLL_RES::DropCassetteFull:
//			{
//				if (DorsHW::POLL_RES::Returning == m_PreviousState
//					||FS365::HW::Dors::DorsHW::POLL_RES::BillReturned == m_PreviousState
//					||FS365::HW::Dors::DorsHW::_isRejCode(m_PreviousState)
//					) {
//					SendItemsTaken();
//				}
//				SendCashUnitInfoChanged();
//			}
//			break;
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejDensity:
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejBillInHead:
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejConveying:
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejIdentification:
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejInhibit:
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejFastFeed:
//			{
//				SendInputRefuse();
//			}
//			break;
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejInsertion:
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejLength:
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejMagnetic:
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejMultiplying:
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejOperation:
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejIAS:
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejNotebase:
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejUV:
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejTape:
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejEntryCassetteSns:
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejFastConveying:
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejTrayClosed:
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejTimeout:
//			case FS365::HW::Dors::DorsHW::POLL_RES::RejUnknown:
//			{
//				SendInputRefuse();
//			}
//			break;
//			case FS365::HW::Dors::DorsHW::POLL_RES::Accepting:
//			{
//				SendItemInserted();
//			}
//			break;
//			case FS365::HW::Dors::DorsHW::POLL_RES::Idling:
//				if (!m_pEscrowManager->IsCashInActive())
//				{
//					if (DorsHW::POLL_RES::Returning == m_PreviousState
//						||FS365::HW::Dors::DorsHW::POLL_RES::BillReturned == m_PreviousState
//						||FS365::HW::Dors::DorsHW::_isRejCode(m_PreviousState))
//					{
//						SendItemsTaken();
//					}
//				}
//				EnsureBillTableIsAvailable();
//				break;
//			case FS365::HW::Dors::DorsHW::POLL_RES::UnitDisabled:
//				if (m_bNotesArePresented                                        // Если банкноты были представлены
//					&&
//					(                                                           // И предыдущее состояние было одним из:
//						DorsHW::POLL_RES::Returning == m_PreviousState
//						||FS365::HW::Dors::DorsHW::POLL_RES::BillReturned == m_PreviousState
//						||FS365::HW::Dors::DorsHW::_isRejCode(m_PreviousState)
//						)
//					) {
//					// Внимание! Если произошла инициализация устройства во время приёма банкноты и было сгенерировано событие 
//					// "банкнота предъявлена", то попав в UnitDisabled мы должны сгенерировать ItemsTaken.
//					SendItemsTaken();
//				}
//
//				// Если требуется обновить ID данных (после замены кассеты)
//				if (m_idn.IsEmpty() || m_bRefreshIdData) {                                          // Если ID данных нет, пробуем получить 
//					auto idRefresResult = m_pDevice->Identification(m_idn);     // Пробуем получить ID данные
//					if (DorsHW::RESULT::Ok == idRefresResult) {
//						m_bRefreshIdData = false;                           // Флаг обновления ID данных сброшен
//					}
//				}
//
//
//				if (bRefreshCassetteHighLevel) {    // При старте ПО и при восстановлении связи с устройством 
//					if (m_AllNumberList.GetTotalNotesCount() >= m_ulThreshold) {    // Если кол-во купюр >= THRESHOLD, сообщить устройству, что достигли уровня HIGH
//						m_pDevice->CassetteHighLevel(true);                         // Инициируем установку HIGH флага в устройстве
//					}
//					else {
//						m_pDevice->CassetteHighLevel(false);
//					}
//					bRefreshCassetteHighLevel = false;                              // 
//				}
//
//				EnsureBillTableIsAvailable();
//				break;
//			case FS365::HW::Dors::DorsHW::POLL_RES::BillStacked:
//				if (m_PreviousState ==FS365::HW::Dors::DorsHW::POLL_RES::Stacking)
//				{
//					AddBanknote(m_usCurrentNoteID);
//					FillCashInfo(m_cashUnitInfo);
//					if (m_usCurrentNoteID == 0) {
//						logger_->trace(std::format("{}() - В КАССЕТУ СКЛАДИРОВАНА: НЕРАСПОЗНАННАЯ БАНКНОТА", __FUNCTION__), LOGLEVEL1);
//					}
//					else {
//						logger_->trace(std::format("{}() - В КАССЕТУ СКЛАДИРОВАНА: БАНКНОТА С ИНДЕКСОМ {}", __FUNCTION__, m_usCurrentNoteID), LOGLEVEL1);
//					}
//				}
//				break;
//			case FS365::HW::Dors::DorsHW::POLL_RES::Returning:
//			{
//				if (!m_curRq
//					) {
//					// Банкнота вернулась в не рамках отработки XFS запроса:
//					// 1. В результате автоматического аппаратного сброса
//					// 2. По escrow таймауту
//////////////////////////// if (GetinfoCashinStatus::GetInstance()->IsCashInActive()) {
//						//    GetinfoCashinStatus::GetInstance()->ResetNoteNumberList();
//						//}
//
//					if (m_pEscrowManager->IsCashInActive()) {
//						m_pEscrowManager->ResetNoteNumberList();
//					}
//				}
//			}
//			break;
//			case FS365::HW::Dors::DorsHW::POLL_RES::BillReturned:
//				// Для DBA отправка события только когда банкнота физически предъявлена.
//				// В состояние BillReturned попадаем в следующих случаях:
//				// 1. Rejecting -> BillReturned (предъявление банкноты после отбраковки)
//				// 2. Returning -> BillReturned (предъявление банкноты после возврата)
//				SendItemsPresented();
//				break;
//			case FS365::HW::Dors::DorsHW::POLL_RES::Cheated:
//			{
//				// Производим единовременную рассылку событий USER_ERROR с предупреждением о попытке мошенничества
//				char pszMessage[] = "            CIM: CHEATED";
//				std::vector< BYTE > arrMessage(pszMessage, pszMessage + sizeof(pszMessage));
//				SendUserErrorEvent(GetLogicalServiceName().c_str(), WFS_ERR_ACT_NOACTION, arrMessage);
//
//			} break;
//
//			case FS365::HW::Dors::DorsHW::POLL_RES::FishingDetected:	// Новое название по протоколу об. дан. DORS 210BA
//			{
//				if (DorsHW::POLL_RES::FishingDetected != m_PreviousState) {
//
//					AddFraudAttempt();
//
//					// Производим единовременную рассылку событий USER_ERROR с предупреждением о попытке мошенничества
//					char pszMessage[] = "            CIM: FRAUD DETECTED";
//					std::vector< BYTE > arrMessage(pszMessage, pszMessage + sizeof(pszMessage));
//					SendUserErrorEvent(GetLogicalServiceName().c_str(), WFS_ERR_ACT_NOACTION, arrMessage);
//				}
//			} break;
//			case FS365::HW::Dors::DorsHW::POLL_RES::ValidatorJammed:
//			case FS365::HW::Dors::DorsHW::POLL_RES::DropCassetteJammed:
//			case FS365::HW::Dors::DorsHW::POLL_RES::StackMotorFail:
//			case FS365::HW::Dors::DorsHW::POLL_RES::TransportMotorFail:
//			case FS365::HW::Dors::DorsHW::POLL_RES::InitialCassetteStatusFail:
//			case FS365::HW::Dors::DorsHW::POLL_RES::OpticCanalFail:
//			case FS365::HW::Dors::DorsHW::POLL_RES::MagneticCanalFail:
//			case FS365::HW::Dors::DorsHW::POLL_RES::StartTrayFailure:
//			case FS365::HW::Dors::DorsHW::POLL_RES::Group47UnknownFailure:
//				// Любая ошибка оборудования в процессе приёма наличных - взводим ивент, извещающий об ошибке операции
//				SendHardwareErrorEvent(GetLogicalServiceName().c_str(), WFS_ERR_ACT_HWMAINT);
//				SendDeviceStatusEvent(GetLogicalServiceName().c_str(), WFS_CIM_DEVHWERROR);
//				break;
//			case FS365::HW::Dors::DorsHW::POLL_RES::PowerUp:
//			case FS365::HW::Dors::DorsHW::POLL_RES::PowerUpWithBillInValidator:
//			case FS365::HW::Dors::DorsHW::POLL_RES::PowerUpWithBillInStacker:
//			{
//				// Питание было сброшено
//				// Заказываем обновление флага HIGH
//				bRefreshCassetteHighLevel = true;
//
//				// Подача Reset с учетом стратегии
//				m_pPowerUpManager->AsyncReset(true);
//				//m_pDevice->Reset();
//				m_pDevice->states_stack_transfer_enable(true);
//
//			} break;
//			}
//
//			// Если зафиксирован переход из ErrPort
//			if ((m_PreviousState ==FS365::HW::Dors::DorsHW::POLL_RES::PortError && !DorsHW::_isHWError(m_State)) || (DorsHW::_isHWError(m_PreviousState) && m_State !=FS365::HW::Dors::DorsHW::POLL_RES::PortError && m_State !=FS365::HW::Dors::DorsHW::POLL_RES::Initialize)) {
//
//				// если фиксируем переход из PortError - отправляем событие DEVICE STATUS CHANGED (DEV_ONLINE)
//				logger_->warn(std::format("{}() - ЗАФИКСИРОВАН ПЕРЕХОД ИЗ СОСТОЯНИЯ PORT_ERROR. ИНИЦИИРУЕМ РАССЫЛКУ СОБЫТИЙ DEVICE_STATUS_CHANGED (DEV_ONLINE)", __FUNCTION__), LOGLEVEL1);
//				SendDeviceStatusEvent(GetLogicalServiceName().c_str(), WFS_CIM_DEVONLINE);
//
//				// Требуем обновление версии прошивки и SN
//				m_bRefreshIdData = true;
//			}
//
//
//			// Команда ресет завершилось
//			if (m_PreviousState ==FS365::HW::Dors::DorsHW::POLL_RES::Initialize && m_State ==FS365::HW::Dors::DorsHW::POLL_RES::UnitDisabled) {
//				m_pDevice->m_bResetOperationInProgress = false;
//			}
//
//			// Если зафиксирован переход из ErrPort
//			if ((m_PreviousState ==FS365::HW::Dors::DorsHW::POLL_RES::PortError && !DorsHW::_isHWError(m_State)) ||
//				(DorsHW::_isHWError(m_PreviousState) && m_State !=FS365::HW::Dors::DorsHW::POLL_RES::PortError && m_State !=FS365::HW::Dors::DorsHW::POLL_RES::Initialize)) {
//				// Если фиксируем переход из PortError - отправляем событие DEVICE STATUS CHANGED (DEV_ONLINE)
//
//			}
//
//			// Если фиксируем переход в PortError - отправляем событие DEVICE STATUS CHANGED (DEV_POWEROFF)
//			if (DorsHW::POLL_RES::PortError == m_State) {
//				std::lock_guard<std::mutex> lock{ m_mtx };
//				if (m_pDevice->GetSoftwareConfigurationFault())
//					SendDeviceStatusEvent(GetLogicalServiceName().c_str(), WFS_CIM_DEVNODEVICE);
//				else
//				{
//					logger_->warn(std::format("{}() - ЗАФИКСИРОВАН ПЕРЕХОД В СОСТОЯНИЕ PORTERROR. ИНИЦИИРУЕМ РАССЫЛКУ СОБЫТИЙ DEVICE_STATUS_CHANGED (DEV_POWEROFF)", __FUNCTION__), LOGLEVEL1);
//					m_pDevice->println(__FUNCTION__, "() - ЗАФИКСИРОВАН ПЕРЕХОД В СОСТОЯНИЕ PORTERROR. ИНИЦИИРУЕМ РАССЫЛКУ СОБЫТИЙ DEVICE_STATUS_CHANGED (DEV_POWEROFF)");
//					SendDeviceStatusEvent(GetLogicalServiceName().c_str(), WFS_CIM_DEVPOWEROFF);
//				}
//			}
		}

	}
	//<summary>
	//RunAsync
	//Handle unsolicited events
	//Here is an example of handling ItemsTakenEvent after cash is returned and taken by customer.
	//</summary>
	boost::asio::awaitable<void> CashAcceptorSample::RunAsync(std::stop_token cancel)
	{
		cashAcceptorService_ =
			std::dynamic_pointer_cast<XFS4IoTFramework::Common::CashAcceptorServiceProvider>(
				setServiceProvider_);

		if (!cashAcceptorService_) {
			throw std::runtime_error("Для параметра Cash Acceptor Service установлено значение null.");
		}

		auto executor = co_await boost::asio::this_coro::executor;
		boost::asio::steady_timer timer(executor);

		while (!cancel.stop_requested())
		{
			timer.expires_after(500ms);

			boost::system::error_code ec;
			co_await timer.async_wait(
				boost::asio::redirect_error(boost::asio::use_awaitable, ec));

			if (cancel.stop_requested()) {
				break;
			}

			if (positionStatus_.GetPositionStatus() !=
				XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::NotEmpty)
			{
				continue;
			}

			positionStatus_.SetPositionStatus(
				XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::Empty);

			positionStatus_.SetShutter(
				XFS4IoTFramework::Common::CashManagementStatusClass::ShutterEnum::Closed);

			co_await cashAcceptorService_->ItemsTakenEvent(
				XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::OutCenter);
		}

		co_return;
	}


	void CashAcceptorSample::SetServiceProvider(std::shared_ptr<XFS4IoTServer::IServiceProvider> serviceProvider)
	{
		setServiceProvider_ = serviceProvider;
	}

	void CashAcceptorSample::loggingChangeStatus(FS365::HW::Dors::DorsHW::POLL_RES currentState)
	{
		using enum FS365::HW::Dors::DorsHW::POLL_RES;
		switch (currentState)
		{
		case Idling:
		case UnitDisabled:
		case Accepting:
		case EscrowPos:
		case BillReturned:
			if (GetInfoStatus::GetInstance()->device != ONLINE)
			{
				logger_->trace(std::format("{}() - WFSCIMSTATUS.fwDevice: {} -> {}", __FUNCTION__
					, DeviceStatusToString(GetInfoStatus::GetInstance()->device)
					, DeviceStatusToString(ONLINE)));
			}
			GetInfoStatus::GetInstance()->device = ONLINE;


			if (GetInfoStatus::GetInstance()->acceptor != ACCCUSTOP && GetInfoStatus::GetInstance()->acceptor != ACCOK)
			{
				logger_->trace(std::format("{}() - WFSCIMSTATUS.fwAcceptor: {} -> {}", __FUNCTION__
					, AcceptorToString(GetInfoStatus::GetInstance()->acceptor)
					, AcceptorToString(m_bCassetteHasBeenReplaced ? ACCCUSTOP : ACCOK)));
			}
			if (m_bCassetteHasBeenReplaced) {
				GetInfoStatus::GetInstance()->acceptor = ACCCUSTOP;
			}
			else
			{
				GetInfoStatus::GetInstance()->acceptor = ACCOK;
			}
			break;

			// в соовтетствии со спецификацией, переполненная кассета
			// означается состоянием ACCCUSTOP в поле Acceptor.
			// Дополнение: отсутствующая кассета, также не является случаем аппаратной ошибки
		case DropCassetteFull:
		case DropCassetteOutOfPosition:
		case CassetteBracketOpen:
			if (GetInfoStatus::GetInstance()->device != ONLINE)
			{
				logger_->trace(std::format("{}() - WFSCIMSTATUS.fwDevice: {} -> {}", __FUNCTION__
					, DeviceStatusToString(GetInfoStatus::GetInstance()->device)
					, DeviceStatusToString(ONLINE)));
			}
			GetInfoStatus::GetInstance()->device = ONLINE;

			if (GetInfoStatus::GetInstance()->acceptor != ACCCUSTOP)
			{
				logger_->trace(std::format("{}() - WFSCIMSTATUS.fwAcceptor: {} -> {}", __FUNCTION__
					, AcceptorToString(GetInfoStatus::GetInstance()->acceptor)
					, AcceptorToString(ACCCUSTOP)));
			}
			GetInfoStatus::GetInstance()->acceptor = ACCCUSTOP;
			break;

		case StackMotorFail:
		case TransportMotorFail:
		case InitialCassetteStatusFail:
		case OpticCanalFail:
		case MagneticCanalFail:
		case StartTrayFailure:
		case Group47UnknownFailure:
		case ValidatorJammed:
		case Unknown:
		case DropCassetteJammed:
			if (GetInfoStatus::GetInstance()->device != HWERROR)
			{
				logger_->trace(std::format("{}() - WFSCIMSTATUS.fwDevice: {} -> {}", __FUNCTION__
					, DeviceStatusToString(GetInfoStatus::GetInstance()->device)
					, DeviceStatusToString(HWERROR)));
			}
			GetInfoStatus::GetInstance()->device = HWERROR;

			if (GetInfoStatus::GetInstance()->acceptor != ACCCUUNKNOWN)
			{
				logger_->trace(std::format("{}() - WFSCIMSTATUS.fwAcceptor: {} -> {}", __FUNCTION__
					, AcceptorToString(GetInfoStatus::GetInstance()->acceptor)
					, AcceptorToString(ACCCUUNKNOWN)));
			}
			GetInfoStatus::GetInstance()->acceptor = ACCCUUNKNOWN;
			break;
		case Cheated:
		case FishingDetected:
			if (GetInfoStatus::GetInstance()->device != USERERROR)
			{
				logger_->trace(std::format("{}() - WFSCIMSTATUS.fwDevice: {} -> {}", __FUNCTION__
					, DeviceStatusToString(GetInfoStatus::GetInstance()->device)
					, DeviceStatusToString(USERERROR)));
			}
			GetInfoStatus::GetInstance()->device = USERERROR;

			if (GetInfoStatus::GetInstance()->acceptor != ACCCUUNKNOWN)
			{
				logger_->trace(std::format("{}() - WFSCIMSTATUS.fwAcceptor: {} -> {}", __FUNCTION__
					, AcceptorToString(GetInfoStatus::GetInstance()->acceptor)
					, AcceptorToString(ACCCUUNKNOWN)));
			}
			GetInfoStatus::GetInstance()->acceptor = ACCCUUNKNOWN;
			break;
		case Initialize:
		case Holding:
		case Stacking:
		case Returning:
		case Rejecting:
		case RejInsertion:
		case RejMagnetic:
		case RejBillInHead:
		case RejMultiplying:
		case RejConveying:
		case RejIdentification:
		case RejNotebase:
		case RejIAS:
		case RejInhibit:
		case RejDensity:
		case RejOperation:
		case RejLength:
		case RejUV:
		case RejTape:
		case RejEntryCassetteSns:
		case RejFastConveying:
		case RejTrayClosed:
		case RejTimeout:
		case RejFastFeed:
		case PowerUp:
		case RejUnknown:
		case Pause:
		case BillStacked:
			if (GetInfoStatus::GetInstance()->device != BUSY)
			{
				logger_->trace(std::format("{}() - WFSCIMSTATUS.fwDevice: {} -> {}", __FUNCTION__
					, DeviceStatusToString(GetInfoStatus::GetInstance()->device)
					, DeviceStatusToString(BUSY)));
			}
			GetInfoStatus::GetInstance()->device = BUSY;

			if (GetInfoStatus::GetInstance()->acceptor != ACCOK)
			{
				logger_->trace(std::format("{}() - WFSCIMSTATUS.fwAcceptor: {} -> {}", __FUNCTION__
					, AcceptorToString(GetInfoStatus::GetInstance()->acceptor)
					, AcceptorToString(ACCOK)));
			}
			GetInfoStatus::GetInstance()->acceptor = ACCOK;
			break;
		case PowerUpWithBillInValidator:
		case PowerUpWithBillInStacker:
			if (GetInfoStatus::GetInstance()->device != ONLINE)
			{
				logger_->trace(std::format("{}() - WFSCIMSTATUS.fwDevice: {} -> {}", __FUNCTION__
					, DeviceStatusToString(GetInfoStatus::GetInstance()->device)
					, DeviceStatusToString(ONLINE)));
			}
			GetInfoStatus::GetInstance()->device = ONLINE;

			if (GetInfoStatus::GetInstance()->acceptor != powerUpManager_->GetAcceptorPartialState())
			{
				logger_->trace(std::format("{}() - WFSCIMSTATUS.fwAcceptor: {} -> {}", __FUNCTION__
					, AcceptorToString(GetInfoStatus::GetInstance()->acceptor)
					, AcceptorToString(powerUpManager_->GetAcceptorPartialState())));
			}
			GetInfoStatus::GetInstance()->acceptor = powerUpManager_->GetAcceptorPartialState();
			// в дальнейшем здесб опеределяем стоит ли банкнота в стекере. пока что ACCOK
			break;
		}

		//// Корректировка для Acceptor:
		//if (ACCOK == GetInfoStatus::GetInstance()->acceptor)
		//{
		//	ULONG ulCount = m_AllNumberList.GetTotalNotesCount();
		//	if (ulCount >= m_ulCassetteCapacity) {

		//		if (GetInfoStatus::GetInstance()->acceptor != ACCCUSTOP)
		//		{
		//			logger_->trace(std::format("{}() - WFSCIMSTATUS.fwAcceptor: {} -> {}", __FUNCTION__
		//				, AcceptorToString(GetInfoStatus::GetInstance()->acceptor)
		//				, AcceptorToString(ACCCUSTOP)));
		//		}
		//		GetInfoStatus::GetInstance()->acceptor = ACCCUSTOP;
		//	}
		//	else if (ulCount >= m_ulThreshold) {
		//		if (GetInfoStatus::GetInstance()->acceptor != ACCCUSTATE)
		//		{
		//			logger_->trace(std::format("{}() - WFSCIMSTATUS.fwAcceptor: {} -> {}", __FUNCTION__
		//				, AcceptorToString(GetInfoStatus::GetInstance()->acceptor)
		//				, AcceptorToString(ACCCUSTATE)));
		//		}
		//		GetInfoStatus::GetInstance()->acceptor = ACCCUSTATE;
		//	}
		//}
	}


	// ============================================================================
	// Доступность Cash In
	// ============================================================================
	bool CashAcceptorSample::IsReadyForCashIn()
	{
		using namespace FS365::HW::Dors;
		using namespace std::chrono;

		constexpr auto kRejectWaitTimeout = 60s;
		constexpr auto kRejectPollInterval = 10ms;

		// 1️ Проверяем очевидные стоп-флаги
		if (m_bCassetteHasBeenReplaced)                                     // Кассета была заменена
		{
			logger_->warn(std::format("{}() — Кассета была заменена", __FUNCTION__), LOGLEVEL1);
			return false;
		}

		const int count = cashUnitInfo_.begin()->second.unitCount_.GetCount();
		const int capacity = cashUnitInfo_.begin()->second.cashUnitStorageConfig_.GetCapacity();
		if (count >= capacity)   // Кассета полна
		{
			logger_->warn(std::format("{}() — Кассета заполнена", __FUNCTION__), LOGLEVEL1);
			return false;
		}

		if (DorsHW::_isInoperative(m_State))                                // Устройство в неисправном состоянии
		{
			logger_->warn(std::format("{}() — Устройство в неисправном состоянии", __FUNCTION__), LOGLEVEL1);
			return false;
		}

		// 2️ Обработка "возврата" или "отвержения"
		if (DorsHW::_isRejCode(m_State) || m_State == DorsHW::POLL_RES::Returning)
		{
			const auto startTime = steady_clock::now();
			logger_->trace(std::format("{}() — ожидание выхода из состояния возврата/отвержения...", __FUNCTION__), LOGLEVEL1);

			while (true)
			{
				std::this_thread::sleep_for(kRejectPollInterval);

				const auto elapsed = steady_clock::now() - startTime;
				if (elapsed > kRejectWaitTimeout)
				{
					logger_->warn(std::format("{}() — не дождались выхода из состояния возврата банкноты", __FUNCTION__), LOGLEVEL1);
					return false;
				}

				switch (m_State)
				{
				case DorsHW::POLL_RES::UnitDisabled:
				case DorsHW::POLL_RES::Idling:
					// Нормальное состояние
					return true;

				case DorsHW::POLL_RES::EscrowPos:
				case DorsHW::POLL_RES::Holding:
				case DorsHW::POLL_RES::Accepting:
					// Возможный переход в приём
					return true;

				default:
					break;
				}

				/* if (DorsHW::_isInoperative(m_State))
				return false;*/
			}

			// unreachable, но для формальности
			return false;
		}

		// 3️ Обработка нормальных состояний
		switch (m_State)
		{
		case DorsHW::POLL_RES::BillReturned:
		case DorsHW::POLL_RES::BillStacked:
			return true;

		case DorsHW::POLL_RES::UnitDisabled:
		case DorsHW::POLL_RES::Idling:
		case DorsHW::POLL_RES::EscrowPos:
		case DorsHW::POLL_RES::Holding:
		case DorsHW::POLL_RES::Accepting:
			return true;

		case DorsHW::POLL_RES::Initialize:
		case DorsHW::POLL_RES::Stacking:
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(OPERATION_INTERVAL));
			switch (m_State)
			{
			case DorsHW::POLL_RES::UnitDisabled:
				return true;
			case DorsHW::POLL_RES::Initialize:
			case DorsHW::POLL_RES::Returning:
			case DorsHW::POLL_RES::Stacking:
			{
				logger_->warn(std::format("{}() — Одна из этих собыйтии в процессе [Initialize, Returning, Stacking]", __FUNCTION__), LOGLEVEL1);
				return false;
			}
			}
			break;
		}

		default:
			return false;
		}

		return false;
	}

	boost::asio::awaitable<XFS4IoTServer::DeviceResult> CashAcceptorSample::PowerSaveControl(int maxPowerSaveRecoveryTime, std::stop_token cancel)
	{
		return boost::asio::awaitable<XFS4IoTServer::DeviceResult>();
	}

	boost::asio::awaitable<XFS4IoTServer::DeviceResult> CashAcceptorSample::SetTransactionState(const XFS4IoTFramework::Common::SetTransactionStateRequest& request)
	{
		return boost::asio::awaitable<XFS4IoTServer::DeviceResult>();
	}

	boost::asio::awaitable<XFS4IoTFramework::Common::GetTransactionStateResult> CashAcceptorSample::GetTransactionState()
	{
		return boost::asio::awaitable<XFS4IoTFramework::Common::GetTransactionStateResult>();
	}

	boost::asio::awaitable<XFS4IoTFramework::Common::GetCommandNonceResult> CashAcceptorSample::GetCommandNonce()
	{
		return boost::asio::awaitable<XFS4IoTFramework::Common::GetCommandNonceResult>();
	}

	boost::asio::awaitable<XFS4IoTServer::DeviceResult> CashAcceptorSample::ClearCommandNonce()
	{
		return boost::asio::awaitable<XFS4IoTServer::DeviceResult>();
	}

	// ======= Device property setters / service provider accessor =======
	std::shared_ptr<XFS4IoTServer::IServiceProvider> CashAcceptorSample::GetServiceProvider() const
	{
		return setServiceProvider_;
	}

	//<summary>
	 //Thread for simulate cash taken event to be fired
	 //</summary>
	void CashAcceptorSample::CashTakenThread()
	{
		std::this_thread::sleep_for(5s);
		cashTakenSignal_.release();
	}

	XFS4IoTFramework::Storage::CashUnitStorageConfiguration CashAcceptorSample::MakeCashInCassetteConfig() const
	{
		using namespace XFS4IoTFramework::Storage;

		std::vector<std::string> banknoteItems;

		for (const auto& [itemId, banknote] : allBanknoteIDs_)
		{
			banknoteItems.push_back(itemId);
		}

		auto capabilities = std::make_shared<CashCapabilitiesClass>(
			CashCapabilitiesClass::TypesEnum::CashIn,
			CashCapabilitiesClass::ItemsEnum::Fit |
			CashCapabilitiesClass::ItemsEnum::Unfit |
			CashCapabilitiesClass::ItemsEnum::Suspect |
			CashCapabilitiesClass::ItemsEnum::Counterfeit,
			true,   // hardwareSensors
			0,      // retractAreas
			banknoteItems
		);

		auto configuration = std::make_shared<CashConfigurationClass>(
			CashCapabilitiesClass::TypesEnum::CashIn,
			CashCapabilitiesClass::ItemsEnum::Fit |
			CashCapabilitiesClass::ItemsEnum::Unfit |
			CashCapabilitiesClass::ItemsEnum::Suspect |
			CashCapabilitiesClass::ItemsEnum::Counterfeit,
			"RUB",  // currency
			0.0,    // Абсолютная стоимость единицы хранения
			1800,   // highThreshold
			0,      // lowThreshold
			true,  // appLockIn
			false,   // appLockOut
			banknoteItems
		);

		CashUnitAdditionalInfoClass additionalInfo(
			1,      // index, аналог usNumber из XFS3
			false    // accuracySupported
		);

		return CashUnitStorageConfiguration(
			"1",                    // id, можно связать с XFS3 cUnitID
			"CashInCassette1",       // positionName
			2000,                    // capacity
			"CIM-CASSETTE-001", // serialNumber
			capabilities,
			configuration,
			additionalInfo
		);
	}


	//<summary>
	// эта команда инициирует операцию по внесению наличных. 
	// Устройство должно проверить, что оно находится в состоянии, 
	// которое позволяет начать операцию по внесению наличных, 
	// и если это так, то подготовиться к приему наличных.
	//</summary>
	boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CashInStartResult> CashAcceptorSample::CashInStart(
		const XFS4IoTFramework::CashAcceptor::CashInStartRequest& request,
		std::stop_token cancellation)
	{
		if (cashAcceptorStatus_->GetIntermediateStacker() != XFS4IoTFramework::Common::CashAcceptorStatusClass::IntermediateStackerEnum::Empty)
		{
			co_return XFS4IoTFramework::CashAcceptor::CashInStartResult(
				XFS4IoT::MessageHeader::CompletionCodeEnum::HardwareError,
				"Складское устройство не находится в пустом состоянии. " +
				std::to_string(static_cast<int>(cashAcceptorStatus_->GetIntermediateStacker())));
		}

		if (positionStatus_.GetPositionStatus() != XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::Empty)
		{
			co_return XFS4IoTFramework::CashAcceptor::CashInStartResult(
				XFS4IoT::MessageHeader::CompletionCodeEnum::HardwareError,
				"Положение позиции не позволяет начать операцию по внесению наличных. " +
				std::to_string(static_cast<int>(positionStatus_.GetPositionStatus())));
		}

		if (positionStatus_.GetShutter() != XFS4IoTFramework::Common::CashManagementStatusClass::ShutterEnum::Closed)
		{
			co_return XFS4IoTFramework::CashAcceptor::CashInStartResult(
				XFS4IoT::MessageHeader::CompletionCodeEnum::HardwareError,
				"Положение затвора не является подходящим для начала операции по ввозу наличных. " +
				std::to_string(static_cast<int>(positionStatus_.GetShutter())));
		}


		escrowManager_->OpenTransaction();

		// Очищаем счетчик принятых предметов в начале каждой операции по внесению наличных, чтобы отслеживать только текущую операцию.
		acceptedItems_->clear();

		co_return XFS4IoTFramework::CashAcceptor::CashInStartResult(XFS4IoT::MessageHeader::CompletionCodeEnum::Success);
	}

	//<summary>
	//эта команда выполняет операцию по внесению наличных. 
	// В ответ на эту команду устройство должно идентифицировать и классифицировать внесенные предметы, 
	// а также переместить их в складское устройство, если это необходимо. Результат операции по 
	// внесению наличных возвращается в ответе на эту команду, а также может быть возвращен 
	// в виде события ItemsInsertedEvent, если операция по внесению наличных была выполнена успешно.
	//</summary>
	boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CashInResult> CashAcceptorSample::CashIn(
		std::shared_ptr<XFS4IoTFramework::CashAcceptor::CashInCommandEvents> events,
		const XFS4IoTFramework::CashAcceptor::CashInRequest& request,
		std::stop_token cancellation)
	{
		currentCashInItems_.clear();
		if (positionStatus_.GetPositionStatus() != XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::Empty &&
			positionStatus_.GetPositionStatus() != XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::NotEmpty)
		{
			co_return XFS4IoTFramework::CashAcceptor::CashInResult(
				XFS4IoT::MessageHeader::CompletionCodeEnum::HardwareError,
				"Устройство имеет некорректное положение. " +
				std::to_string(static_cast<int>(positionStatus_.GetPositionStatus())));
		}

		if (positionStatus_.GetPositionStatus() == XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::Empty)
		{
			// Ждем, пока предметы не будут вставлены в позицию
			co_await events->InsertItemsEvent();

			// Симулируем время, необходимое для вставки предметов и их идентификации
			co_await boost::asio::steady_timer(
				co_await boost::asio::this_coro::executor,
				2000ms
			).async_wait(boost::asio::use_awaitable);

			co_await boost::asio::steady_timer(
				co_await boost::asio::this_coro::executor,
				100ms
			).async_wait(boost::asio::use_awaitable);
		}

		// Банкноты вставлены, идентифицированы и классифицированы. Теперь перемещаем их в складское устройство (стекер) и обновляем статус.
		co_await cashAcceptorService_->ItemsInsertedEvent(
			XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::InCenter);

		co_await boost::asio::steady_timer(
			co_await boost::asio::this_coro::executor,
			100ms
		).async_wait(boost::asio::use_awaitable);


		if (acceptedItems_->size() > 0)
		{
			currentCashInItems_["typeRUB50"] = XFS4IoTFramework::Storage::CashItemCountClass(1, 0, 0, 0, 0);
		}
		else
		{
			currentCashInItems_["typeRUB10"] = XFS4IoTFramework::Storage::CashItemCountClass(1, 0, 0, 0, 0);
		}

		// Добавляем текущие принятые предметы к общему счетчику принятых предметов
		for (const auto& [key, value] : currentCashInItems_)
		{
			if (acceptedItems_->contains(key))
			{
				(*acceptedItems_)[key].SetFit((*acceptedItems_)[key].GetFit() + value.GetFit());
				(*acceptedItems_)[key].SetUnfit((*acceptedItems_)[key].GetUnfit() + value.GetUnfit());
				(*acceptedItems_)[key].SetCounterfeit((*acceptedItems_)[key].GetCounterfeit() + value.GetCounterfeit());
				(*acceptedItems_)[key].SetSuspect((*acceptedItems_)[key].GetSuspect() + value.GetSuspect());
				(*acceptedItems_)[key].SetInked((*acceptedItems_)[key].GetInked() + value.GetInked());
			}
			else
			{
				(*acceptedItems_)[key] = value;
			}
		}

		// Обновляем статус устройства
		positionStatus_.SetPositionStatus(XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::Empty);
		cashAcceptorStatus_->SetIntermediateStacker(XFS4IoTFramework::Common::CashAcceptorStatusClass::IntermediateStackerEnum::NotEmpty);
		cashAcceptorStatus_->SetStackerItems(XFS4IoTFramework::Common::CashAcceptorStatusClass::StackerItemsEnum::NoCustomerAccess);

		co_return XFS4IoTFramework::CashAcceptor::CashInResult(
			XFS4IoT::MessageHeader::CompletionCodeEnum::Success,
			currentCashInItems_,
			std::unordered_map<std::string, std::shared_ptr<XFS4IoTFramework::Storage::CashUnitCountClass>>{}, // empty movement result
			0); // unrecognized count
	}

	//<summary>
	//Эта команда выполняет операцию по завершению внесения наличных. 
	// Она может быть вызвана только после успешного выполнения команды CashInStart 
	// и может быть вызвана только один раз для каждой транзакции по внесению наличных.
	//</summary>
	boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CashInEndResult> CashAcceptorSample::CashInEnd(
		std::shared_ptr<XFS4IoTFramework::CashAcceptor::CashInEndCommandEvents> events,
		std::stop_token cancellation)
	{
		if (positionStatus_.GetPositionStatus() != XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::Empty)
		{
			co_return XFS4IoTFramework::CashAcceptor::CashInEndResult(
				XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
				"Предметы на позиции.",
				XFS4IoT::CashAcceptor::Completions::CashInEndPayloadData::ErrorCodeEnum::PositionNotEmpty);
		}

		if (cashAcceptorStatus_->GetIntermediateStacker() == XFS4IoTFramework::Common::CashAcceptorStatusClass::IntermediateStackerEnum::Empty)
		{
			cashAcceptorStatus_->SetStackerItems(XFS4IoTFramework::Common::CashAcceptorStatusClass::StackerItemsEnum::NoItems);
			co_return XFS4IoTFramework::CashAcceptor::CashInEndResult(
				XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
				"Наличные не принимаются.",
				XFS4IoT::CashAcceptor::Completions::CashInEndPayloadData::ErrorCodeEnum::NoItems);
		}

		//Simulate moving items from stacker to storage
		co_await boost::asio::steady_timer(
			co_await boost::asio::this_coro::executor,
			1000ms
		).async_wait(boost::asio::use_awaitable);

		//Build cash movement result
		std::map<std::string, XFS4IoTFramework::Storage::CashUnitCountClass> cashMovement;

		auto cashInCount = std::make_shared<XFS4IoTFramework::Storage::StorageCashInCountClass>();
		cashInCount->SetDeposited(std::make_shared<XFS4IoTFramework::Storage::StorageCashCountClass>(0, *acceptedItems_));

		cashMovement["unit3"] = XFS4IoTFramework::Storage::CashUnitCountClass(
			nullptr, // StorageCashOutCount
			cashInCount,
			cashInCount->GetDeposited()->GetTotal());

		//Clear accepted items
		acceptedItems_->clear();

		//Update status
		cashAcceptorStatus_->SetIntermediateStacker(XFS4IoTFramework::Common::CashAcceptorStatusClass::IntermediateStackerEnum::Empty);
		cashAcceptorStatus_->SetStackerItems(XFS4IoTFramework::Common::CashAcceptorStatusClass::StackerItemsEnum::CustomerAccess);
		cashAcceptorStatus_->SetStackerItems(XFS4IoTFramework::Common::CashAcceptorStatusClass::StackerItemsEnum::NoItems);

		co_return XFS4IoTFramework::CashAcceptor::CashInEndResult(
			XFS4IoT::MessageHeader::CompletionCodeEnum::Success,
			cashMovement);
	}

	//<summary>
	//This command is used to roll back a cash-in transaction. It causes all the cash items cashed in since the last
	//CashAcceptor.CashInStart command to be returned to the customer.
	//This command ends the current cash-in transaction. The cash-in transaction is ended even if this command does not
	//complete successfully.
	//</summary>
	boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CashInRollbackResult> CashAcceptorSample::CashInRollback(
		std::shared_ptr<XFS4IoTFramework::CashAcceptor::CashInRollbackCommandEvents> events,
		std::stop_token cancellation)
	{
		if (cashAcceptorStatus_->GetIntermediateStacker() == XFS4IoTFramework::Common::CashAcceptorStatusClass::IntermediateStackerEnum::Empty)
		{
			cashAcceptorStatus_->SetStackerItems(XFS4IoTFramework::Common::CashAcceptorStatusClass::StackerItemsEnum::NoItems);
			co_return XFS4IoTFramework::CashAcceptor::CashInRollbackResult(
				XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
				"В стеллаже нет товаров. " +
				std::to_string(static_cast<int>(cashAcceptorStatus_->GetIntermediateStacker())),
				XFS4IoT::CashAcceptor::Completions::CashInRollbackPayloadData::ErrorCodeEnum::NoItems);
		}

		//Simulate moving items from stacker to output position
		co_await boost::asio::steady_timer(
			co_await boost::asio::this_coro::executor,
			1000ms
		).async_wait(boost::asio::use_awaitable);

		//Present items to customer
		positionStatus_.SetPositionStatus(XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::NotEmpty);
		cashAcceptorStatus_->SetIntermediateStacker(XFS4IoTFramework::Common::CashAcceptorStatusClass::IntermediateStackerEnum::Empty);

		co_await boost::asio::steady_timer(
			co_await boost::asio::this_coro::executor,
			1000ms
		).async_wait(boost::asio::use_awaitable);

		//Clear accepted items
		acceptedItems_->clear();

		//Start thread to simulate cash taken by customer
		std::jthread takenThread(&CashAcceptorSample::CashTakenThread, this);
		takenThread.detach();

		co_return XFS4IoTFramework::CashAcceptor::CashInRollbackResult(XFS4IoT::MessageHeader::CompletionCodeEnum::Success);
	}

	//    <summary>
	// Эта команда используется для изменения типов банкнот, которые банкнотоприемник должен принимать в процессе внесения наличных.
	// В теле команды необходимо указывать только те типы банкнот, которые подлежат изменению.
	// Если указан неизвестный тип банкноты, будет возвращена ошибка UnsupportedData.
	// Значения, заданные этой командой, сохраняются постоянно.
	//</summary>
	boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::ConfigureNoteTypesResult> CashAcceptorSample::ConfigureNoteTypes(
		const XFS4IoTFramework::CashAcceptor::ConfigureNoteTypesRequest& request,
		std::stop_token cancellation)
	{
		//Simulate processing delay
		co_await boost::asio::steady_timer(
			co_await boost::asio::this_coro::executor,
			500ms
		).async_wait(boost::asio::use_awaitable);

		// Обновить конфигурацию банкнот, если она предоставлена.
		const auto& banknoteItems = request.banknoteItems;
		if (!banknoteItems.empty())
		{
			for (const auto& [itemKey, configure] : banknoteItems)
			{
				// Проверка, есть ли эта банкнота в нашем списке поддерживаемых банкнот.
				auto allIt = allBanknoteIDs_.find(itemKey);
				if (allIt == allBanknoteIDs_.end())
				{
					// Неизвестный тип банкноты — в реальной реализации это считалось бы ошибкой.
					// Пока что мы просто пропускаем её.
					continue;
				}

				// ищем в разрешенных банкнотах чтобы менять конфигурацию
				auto configIt = cashManagementCapabilities_->GetAllBanknoteItems()->find(itemKey);
				if (configIt != cashManagementCapabilities_->GetAllBanknoteItems()->end())
				{
					// Обновить статус активности
					//configIt->second.SetEnabled(enabled);
					const_cast<XFS4IoTFramework::Common::CashManagementCapabilitiesClass::BanknoteItem&>(configIt->second).SetEnabled(configure);

					//В реальной реализации также следовало бы:
					//- Обновить конфигурацию оборудования
					//- Сохранить данные в постоянное хранилище
					//- Зарегистрировать изменение
					// но пока не буду созранять данные в персистент, потом доделаю
					logger_->trace("ConfigureNoteTypes: " + itemKey + " установлен " +
						(configure ? "enabled" : "disabled"));
				}
			}
			cashManagementCapabilities_->saveBanknotes();
		}

		co_return XFS4IoTFramework::CashAcceptor::ConfigureNoteTypesResult(XFS4IoT::MessageHeader::CompletionCodeEnum::Success);
	}

	//<summary>
	//This command is used to configure the currency description configuration data into the banknote reader module.
	//The format and location of the configuration data is vendor and/or hardware dependent.
	//</summary>
	boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::ConfigureNoteReaderResult> CashAcceptorSample::ConfigureNoteReader(
		const XFS4IoTFramework::CashAcceptor::ConfigureNoteReaderRequest& request,
		std::stop_token cancellation)
	{
		//Simulate processing delay
		co_await boost::asio::steady_timer(
			co_await boost::asio::this_coro::executor,
			500ms
		).async_wait(boost::asio::use_awaitable);

		//In a real implementation, this would:
		//1. Load currency description data from a vendor-specific location
		//2. Configure the banknote reader hardware with this data
		//3. Optionally force reload if request.GetLoadAlways() is true

		//For this sample, we just simulate success
		co_return XFS4IoTFramework::CashAcceptor::ConfigureNoteReaderResult(XFS4IoT::MessageHeader::CompletionCodeEnum::Success);
	}

	//<summary>
	//This command counts the items in the storage unit(s). If it is necessary to move items internally to count them,
	//the items should be returned to the unit from which they originated before completion of the command.
	//During the execution of this command one Storage.StorageChangedEvent will be
	//generated for each storage unit that has been counted successfully, or if the counts have changed, even if the overall
	//command fails.
	//Upon successful completion, for those storage units that have been counted, the counts are accurately reported with 
	//the Storage.GetStorage command.
	//</summary>
	boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CashUnitCountResult> CashAcceptorSample::CashUnitCount(
		std::shared_ptr<XFS4IoTFramework::CashAcceptor::CashUnitCountCommandEvents> events,
		const XFS4IoTFramework::CashAcceptor::CashUnitCountRequest& request,
		std::stop_token cancellation)
	{
		//Simulate counting delay
		co_await boost::asio::steady_timer(
			co_await boost::asio::this_coro::executor,
			500ms
		).async_wait(boost::asio::use_awaitable);

		//Get the service provider to access cash units
		if (!cashAcceptorService_)
		{
			co_return XFS4IoTFramework::CashAcceptor::CashUnitCountResult(
				XFS4IoT::MessageHeader::CompletionCodeEnum::InternalError,
				"CashAcceptorService не инициализирован");
		}

		//Get storage IDs to count
		const auto& storageIds = request.storageIds;

		//Update accuracy for requested storage units
		for (const auto& storageId : storageIds)
		{
			//Check if this storage unit exists
			auto unitIt = cashUnitInfo_.find(storageId);
			if (unitIt == cashUnitInfo_.end())
			{
				//Storage unit not found - could return error or continue
				logger_->trace("CashUnitCount: Единица хранения не найдена: " + storageId);
				continue;
			}

			//Update accuracy to Accurate (simulating successful count)
			unitIt->second.accuracy_ = XFS4IoTFramework::Storage::CashStatusClass::AccuracyEnum::Accurate;

			//In a real implementation, you would:
			//1. Move items to counting position
			//2. Actually count the items
			//3. Update the counts
			//4. Return items to original position
			//5. Fire Storage.StorageChangedEvent for this unit

			logger_->trace("CashUnitCount: Счетная единица хранения: " + storageId);
		}

		co_return XFS4IoTFramework::CashAcceptor::CashUnitCountResult(XFS4IoT::MessageHeader::CompletionCodeEnum::Success);
	}

	//<summary>
	//This command can be used to lock or unlock a CashAcceptor device or one or more storage units.
	//CashAcceptor.GetDeviceLockStatus can be used to obtain the current lock state of any items which support locking.
	//During normal device operation the device and storage units will be locked and removal will not be possible.
	//If supported, the device or storage units can be unlocked, ready for removal. In this situation the device will 
	//still remain online and cash-in operations will be possible, as long as the device or storage units are not 
	//physically removed from their normal operating position.
	//</summary>
	boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::DeviceLockResult> CashAcceptorSample::DeviceLockControl(
		const XFS4IoTFramework::CashAcceptor::DeviceLockRequest& request,
		std::stop_token cancellation)
	{
		//Simulate processing delay
		co_await boost::asio::steady_timer(
			co_await boost::asio::this_coro::executor,
			500ms
		).async_wait(boost::asio::use_awaitable);

		//Get lock control parameters
		auto deviceAction = request.deviceAction;
		auto cashUnitAction = request.cashUnitAction;
		const auto& unitLockControl = request.unitLockControl;

		//Process device lock/unlock
		switch (deviceAction)
		{
		case XFS4IoTFramework::CashAcceptor::DeviceLockRequest::DeviceActionEnum::Lock:
			logger_->trace("DeviceLockControl: Запирающее устройство");
			//In real implementation: lock the physical device
			break;

		case XFS4IoTFramework::CashAcceptor::DeviceLockRequest::DeviceActionEnum::Unlock:
			logger_->trace("DeviceLockControl: Разблокировка устройства");
			//In real implementation: unlock the physical device
			break;

		case XFS4IoTFramework::CashAcceptor::DeviceLockRequest::DeviceActionEnum::NoLockAction:
			//No action on device
			break;
		}

		//Process cash unit lock/unlock
		switch (cashUnitAction)
		{
		case XFS4IoTFramework::CashAcceptor::DeviceLockRequest::CashUnitActionEnum::LockAll:
			logger_->trace("DeviceLockControl: Запирание всех складских помещений.");
			//In real implementation: lock all storage units
			for (auto& [storageId, unitInfo] : cashUnitInfo_)
			{
				//Lock this unit
				logger_->debug("  Locked: " + storageId);
			}
			break;

		case XFS4IoTFramework::CashAcceptor::DeviceLockRequest::CashUnitActionEnum::UnlockAll:
			logger_->trace("DeviceLockControl: Разблокировка всех складских помещений");
			//In real implementation: unlock all storage units
			for (auto& [storageId, unitInfo] : cashUnitInfo_)
			{
				//Unlock this unit
				logger_->debug("  Unlocked: " + storageId);
			}
			break;

		case XFS4IoTFramework::CashAcceptor::DeviceLockRequest::CashUnitActionEnum::LockIndividual:
			logger_->trace("DeviceLockControl: Блокировка/разблокировка отдельных блоков");
			//Process individual unit lock control
			for (const auto& [storageId, action] : unitLockControl)
			{
				//Check if storage unit exists
				auto unitIt = cashUnitInfo_.find(storageId);
				if (unitIt == cashUnitInfo_.end())
				{
					logger_->warn("DeviceLockControl: Единица хранения не найдена: " + storageId);
					continue;
				}

				//Perform lock/unlock action
				switch (action)
				{
				case XFS4IoTFramework::CashAcceptor::DeviceLockRequest::UnitActionEnum::Lock:
					logger_->debug("  Locked: " + storageId);
					//In real implementation: lock this unit
					break;

				case XFS4IoTFramework::CashAcceptor::DeviceLockRequest::UnitActionEnum::Unlock:
					logger_->debug("  Unlocked: " + storageId);
					//In real implementation: unlock this unit
					break;
				}
			}
			break;

		case XFS4IoTFramework::CashAcceptor::DeviceLockRequest::CashUnitActionEnum::NoLockAction:
			//No action on storage units
			break;
		}

		//In a real implementation, you would:
		//1. Actually lock/unlock the physical device
		//2. Actually lock/unlock the physical storage units
		//3. Update the DeviceLockStatus
		//4. Handle errors if locking/unlocking fails
		//5. Return appropriate error codes if needed

		co_return XFS4IoTFramework::CashAcceptor::DeviceLockResult(XFS4IoT::MessageHeader::CompletionCodeEnum::Success);
	}

	//<summary>
	//In cases where multiple bunches are to be returned under explicit shutter control,
	//this command is used for the purpose of moving a remaining bunch to the output position explicitly 
	//before using the following commands: CashManagement.OpenShutter or CashAcceptor.PresentMedia
	//The application can tell whether the additional items were left by using the CashAcceptor.GetPresentStatus command.
	//This command does not affect the status of the current cash-in transaction.
	//</summary>
	boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::PreparePresentResult> CashAcceptorSample::PreparePresent(
		std::shared_ptr<XFS4IoTFramework::CashAcceptor::PreparePresentCommandEvents> events,
		const XFS4IoTFramework::CashAcceptor::PreparePresentRequest& request,
		std::stop_token cancellation)
	{
		//Get output position
		auto outputPosition = request.outputPosition;

		//Check if the position is valid
		if (outputPosition != XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::OutCenter)
		{
			co_return XFS4IoTFramework::CashAcceptor::PreparePresentResult(
				XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
				"Неподдерживаемое положение выхода.",
				XFS4IoT::CashAcceptor::Completions::PreparePresentPayloadData::ErrorCodeEnum::UnsupportedPosition);
		}

		//Check if position is empty
		if (positionStatus_.GetPositionStatus() != XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::Empty)
		{
			co_return XFS4IoTFramework::CashAcceptor::PreparePresentResult(
				XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
				"Выходная позиция не пуста.",
				XFS4IoT::CashAcceptor::Completions::PreparePresentPayloadData::ErrorCodeEnum::PositionNotEmpty);
		}

		//Check if there are items to present
		if (cashAcceptorStatus_->GetIntermediateStacker() == XFS4IoTFramework::Common::CashAcceptorStatusClass::IntermediateStackerEnum::Empty)
		{
			co_return XFS4IoTFramework::CashAcceptor::PreparePresentResult(
				XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
				"Нет товаров для представления.",
				XFS4IoT::CashAcceptor::Completions::PreparePresentPayloadData::ErrorCodeEnum::NoItems);
		}

		//Simulate moving items to output position
		co_await boost::asio::steady_timer(
			co_await boost::asio::this_coro::executor,
			1000ms
		).async_wait(boost::asio::use_awaitable);

		//In a real implementation, you would:
		//1. Move the next bunch from stacker to output position
		//2. Update position status
		//3. Keep track of remaining bunches
		//4. Fire events if needed

		logger_->trace("PreparePresent: Переместили группу в выходное положение.");

		co_return XFS4IoTFramework::CashAcceptor::PreparePresentResult(XFS4IoT::MessageHeader::CompletionCodeEnum::Success);
	}

	//<summary>
	//This command opens the shutter and presents items to be taken by the customer. The shutter is automatically closed after
	//the media is taken. The command can be called after a CashAcceptor.CashIn, CashAcceptor.CashInRollback, 
	//CashManagement.Reset or CashAcceptor.CreateSignature command and can be used with explicit and implicit shutter control.
	//The command is only valid on positions where Usage reported by the CashAcceptor.GetPositionCapabilities command 
	//is Rollback or Refuse and where PresentControl reported by the CashAcceptor.GetPositionCapabilities command is false.
	//</summary>
	boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::PresentMediaResult> CashAcceptorSample::PresentMedia(
		const XFS4IoTFramework::CashAcceptor::PresentMediaRequest& request,
		std::stop_token cancellation)
	{
		//Get position and current present status
		auto position = request.position;
		auto currentPresentStatus = request.currentPresentStatus;

		//Check if the position is valid for present
		if (position != XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum::OutCenter)
		{
			co_return XFS4IoTFramework::CashAcceptor::PresentMediaResult(
				XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
				"В настоящее время позиция необоснованна..",
				XFS4IoT::CashAcceptor::Completions::PresentMediaPayloadData::ErrorCodeEnum::UnsupportedPosition,
				currentPresentStatus);
		}

		//Check if shutter is closed
		if (positionStatus_.GetShutter() != XFS4IoTFramework::Common::CashManagementStatusClass::ShutterEnum::Closed)
		{
			co_return XFS4IoTFramework::CashAcceptor::PresentMediaResult(
				XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
				"Стойка не закрыта.",
				XFS4IoT::CashAcceptor::Completions::PresentMediaPayloadData::ErrorCodeEnum::ShutterNotOpen,
				currentPresentStatus);
		}

		//Check if there are items to present
		if (positionStatus_.GetPositionStatus() != XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::NotEmpty)
		{
			co_return XFS4IoTFramework::CashAcceptor::PresentMediaResult(
				XFS4IoT::MessageHeader::CompletionCodeEnum::CommandErrorCode,
				"Нет товаров для представления.",
				XFS4IoT::CashAcceptor::Completions::PresentMediaPayloadData::ErrorCodeEnum::NoItems,
				currentPresentStatus);
		}

		//Open shutter
		positionStatus_.SetShutter(XFS4IoTFramework::Common::CashManagementStatusClass::ShutterEnum::Open);

		logger_->trace("PresentMedia: Shutter открылись, товары были выставлены на продажу.");

		//Simulate presentation time
		co_await boost::asio::steady_timer(
			co_await boost::asio::this_coro::executor,
			1000ms
		).async_wait(boost::asio::use_awaitable);

		//Fire ItemsPresentedEvent
		if (cashAcceptorService_)
		{
			co_await cashAcceptorService_->ItemsPresentedEvent(position, std::nullopt);
		}

		//Wait for items to be taken (in real implementation, this would be hardware detection)
		co_await boost::asio::steady_timer(
			co_await boost::asio::this_coro::executor,
			2000ms
		).async_wait(boost::asio::use_awaitable);

		//Simulate items taken
		positionStatus_.SetPositionStatus(XFS4IoTFramework::Common::CashManagementStatusClass::PositionStatusEnum::Empty);
		positionStatus_.SetShutter(XFS4IoTFramework::Common::CashManagementStatusClass::ShutterEnum::Closed);

		//Fire ItemsTakenEvent
		if (cashAcceptorService_)
		{
			co_await cashAcceptorService_->ItemsTakenEvent(position, std::nullopt);
		}

		logger_->trace("PresentMedia: Вещи украдены, Shutter закрыты.");

		//Create updated present status
		auto lastPresentStatus = std::make_shared<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>(
			XFS4IoTFramework::CashManagement::CashManagementPresentStatus::PresentStateEnum::Presented,
			XFS4IoTFramework::CashManagement::CashManagementPresentStatus::AdditionalBunchesEnum::None,
			0,
			XFS4IoTFramework::Storage::StorageCashCountClass(), // returnedItems
			XFS4IoTFramework::Storage::StorageCashCountClass(), // totalReturnedItems
			XFS4IoTFramework::Storage::StorageCashCountClass()  // remainingItems
		);

		//In a real implementation, you would:
		//1. Actually open the physical shutter
		//2. Detect when items are taken using sensors
		//3. Handle timeout if items are not taken
		//4. Handle multiple bunches if needed
		//5. Update all relevant status fields
		//6. Fire appropriate events at the right time

		co_return XFS4IoTFramework::CashAcceptor::PresentMediaResult(
			XFS4IoT::MessageHeader::CompletionCodeEnum::Success,
			lastPresentStatus);
	}

	// //<summary>
	// //CreateSignature - Not implemented in this sample
	// //</summary>
	//boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CreateSignatureResult> CashAcceptorSample::CreateSignature(
	//    std::shared_ptr<CreateSignatureCommandEvents> events,
	//    const XFS4IoTFramework::CashAcceptor::CreateSignatureRequest& request,
	//    std::stop_token cancellation)
	//{
	//    throw std::runtime_error("CreateSignature is not implemented in this sample");
	//    co_return CreateSignatureResult(XFS4IoT::MessageHeader::CompletionCodeEnum::UnsupportedCommand);
	//}

	// //<summary>
	// //CompareSignature - Not implemented in this sample
	// //</summary>
	//boost::asio::awaitable<CompareSignatureResult> CashAcceptorSample::CompareSignature(
	//    const CompareSignatureRequest& request,
	//    std::stop_token cancellation)
	//{
	//    throw std::runtime_error("CompareSignature is not implemented in this sample");
	//    co_return CompareSignatureResult(XFS4IoT::MessageHeader::CompletionCodeEnum::UnsupportedCommand);
	//}

	// //<summary>
	// //Replenish - Not implemented in this sample
	// //</summary>
	//boost::asio::awaitable<ReplenishResult> CashAcceptorSample::Replenish(
	//    std::shared_ptr<ReplenishCommandEvents> events,
	//    const ReplenishRequest& request,
	//    std::stop_token cancellation)
	//{
	//    throw std::runtime_error("Replenish is not implemented in this sample");
	//    co_return ReplenishResult(XFS4IoT::MessageHeader::CompletionCodeEnum::UnsupportedCommand);
	//}

	// //<summary>
	// //Deplete - Not implemented in this sample
	// //</summary>
	//boost::asio::awaitable<DepleteResult> CashAcceptorSample::Deplete(
	//    std::shared_ptr<DepleteCommandEvents> events,
	//    const DepleteRequest& request,
	//    std::stop_token cancellation)
	//{
	//    throw std::runtime_error("Deplete is not implemented in this sample");
	//    co_return DepleteResult(XFS4IoT::MessageHeader::CompletionCodeEnum::UnsupportedCommand);
	//}

	 //<summary>
	 //GetDepleteCashUnitSources
	 //</summary>
	std::map<std::string, std::vector<std::string>> CashAcceptorSample::GetDepleteCashUnitSources()
	{
		return std::map<std::string, std::vector<std::string>>{};
	}

	//<summary>
	//ReplenishTargets
	//</summary>
	std::vector<std::string> CashAcceptorSample::GetReplenishTargets()
	{
		return std::vector<std::string>{};
	}

	boost::asio::awaitable<XFS4IoTFramework::CashManagement::RetractResult> CashAcceptorSample::RetractAsync(std::shared_ptr<XFS4IoTFramework::CashManagement::RetractCommandEvents> events, const XFS4IoTFramework::CashManagement::RetractRequest& retractInfo, std::stop_token cancellation)
	{
		return boost::asio::awaitable<XFS4IoTFramework::CashManagement::RetractResult>();
	}

	boost::asio::awaitable<XFS4IoTFramework::CashManagement::OpenCloseShutterResult> CashAcceptorSample::OpenCloseShutterAsync(const XFS4IoTFramework::CashManagement::OpenCloseShutterRequest& shutterInfo, std::stop_token cancellation)
	{
		return boost::asio::awaitable<XFS4IoTFramework::CashManagement::OpenCloseShutterResult>();
	}

	boost::asio::awaitable<XFS4IoTFramework::CashManagement::ResetDeviceResult> CashAcceptorSample::ResetDeviceAsync(
		std::shared_ptr<XFS4IoTFramework::CashManagement::ResetCommandEvents> events,
		const XFS4IoTFramework::CashManagement::ResetDeviceRequest& resetDeviceInfo,
		std::stop_token cancellation)
	{
		if (!m_pDevice->IsDeviceInitialized())
		{
			co_return XFS4IoTFramework::CashManagement::ResetDeviceResult{
				XFS4IoT::MessageHeader::CompletionCodeEnum::HardwareError,
				"Устройство не инициализировано."
			};
		}
		//(void)events;
		(void)resetDeviceInfo;
		//(void)cancellation;

		boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor);
		timer.expires_after(std::chrono::seconds(1));

		boost::system::error_code ec;
		co_await timer.async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, ec));

		if (cancellation.stop_requested())
		{
			co_return XFS4IoTFramework::CashManagement::ResetDeviceResult{
				XFS4IoT::MessageHeader::CompletionCodeEnum::TimeOut,
				"Reset был отменен."
			};
		}

		acceptedItems_->clear();

		positionStatus_.SetTransport(XFS4IoTFramework::Common::CashManagementStatusClass::TransportEnum::Ok);
		positionStatus_.SetShutter(XFS4IoTFramework::Common::CashManagementStatusClass::ShutterEnum::Closed);
		cashAcceptorStatus_->SetIntermediateStacker(XFS4IoTFramework::Common::CashAcceptorStatusClass::IntermediateStackerEnum::Empty);
		cashAcceptorStatus_->SetStackerItems(XFS4IoTFramework::Common::CashAcceptorStatusClass::StackerItemsEnum::NoItems);

		auto completionCode = co_await powerUpManager_->Reset(events);

		co_return XFS4IoTFramework::CashManagement::ResetDeviceResult{
			completionCode
		};
	}

	//boost::asio::awaitable<XFS4IoTFramework::CashManagement::ResetDeviceResult> CashAcceptorSample::ResetDeviceAsync(
	//	std::shared_ptr<XFS4IoTFramework::CashManagement::ResetCommandEvents> events,
	//	const XFS4IoTFramework::CashManagement::ResetDeviceRequest& resetDeviceInfo,
	//	std::stop_token cancellation)
	//{
	//	(void)events;
	//	(void)resetDeviceInfo;
	//	//(void)cancellation;

	//	boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor);
	//	timer.expires_after(std::chrono::seconds(1));

	//	boost::system::error_code ec;
	//	co_await timer.async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, ec));

	//	if (cancellation.stop_requested())
	//	{
	//		throw XFS4IoTServer::TimeoutCanceledException("Reset был отменен", nullptr, true);
	//	}

	//	acceptedItems_.clear();

	//	positionStatus_.SetTransport(XFS4IoTFramework::Common::CashManagementStatusClass::TransportEnum::Ok);
	//	positionStatus_.SetShutter(XFS4IoTFramework::Common::CashManagementStatusClass::ShutterEnum::Closed);
	//	cashAcceptorStatus_->SetIntermediateStacker(XFS4IoTFramework::Common::CashAcceptorStatusClass::IntermediateStackerEnum::Empty);
	//	cashAcceptorStatus_->SetStackerItems(XFS4IoTFramework::Common::CashAcceptorStatusClass::StackerItemsEnum::NoItems);


	//	// Создаем токен отмены ожидания
	//	auto pResetTerminator = std::make_shared<StateMachine::BlockedWaitTermination>();
	//	// Сохраняем токен в обработчике
	//	m_p_async_reset_terminator = pResetTerminator;

	//	// Ожидаемые исходы
	//	StateMachine::CEventsSequenceList outcomes;

	//	using enum FS365::HW::Dors::DorsHW::POLL_RES;
	//	outcomes.push_back({
	//		StateMachine::EventTo
	//		{
	//			{
	//				UnitDisabled,
	//				Idling,
	//				DropCassetteFull
	//			}
	//		}
	//		});
	//	outcomes.push_back({
	//		{
	//			StateMachine::EventTo
	//		{
	//			{
	//				DropCassetteOutOfPosition,
	//				ValidatorJammed,
	//				DropCassetteJammed,
	//				StackMotorFail,
	//				TransportMotorFail,
	//				InitialCassetteStatusFail,
	//				OpticCanalFail,
	//				MagneticCanalFail,
	//				StartTrayFailure,
	//				Group47UnknownFailure,
	//				PortError
	//			}
	//		}
	//		}
	//		});


	//	// Запоминаем UIDs текущего состояния для пропуска в ожидании
	//	const auto skipUIDs = m_stateMachine.GetCurrentStateUID();
	//	// Запоминаем ID банкноты, находящейся в процессе обработки
	//	USHORT usNoteID = m_usCurrentNoteID;

	//	// Выполняем аппаратный сброс
	//	auto resetResult = m_pDevice->Reset();
	//	if (resetResult != FS365::HW::Dors::DorsHW::RESULT::Ok) {
	//		// Если мы попали в состояние Err_Port, скорее всего придется физический перезагружать устройство
	//		if (resetResult == FS365::HW::Dors::DorsHW::RESULT::Err_Port) {
	//			// При ошибке порта выполняем перезагрузку устройства
	//			m_pDevice->Reboot();
	//		}
	//		else {
	//			logger_->warn(std::format("{}() - ошибка аппаратного сброса: Reset() = {}", __FUNCTION__, FS365::HW::Dors::DorsHW::ResultToString(resetResult)), LOGLEVEL1);
	//			co_return XFS4IoTFramework::CashManagement::ResetDeviceResult(
	//				XFS4IoT::MessageHeader::CompletionCodeEnum::HardwareError);
	//		}
	//	}

	//	const auto waitResult = m_stateMachine.BlockedWait(outcomes, std::chrono::milliseconds(30000), pResetTerminator, skipUIDs);
	//	switch (waitResult) {
	//		// --- Анализ результата ожидания ---
	//	case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_0:
	//	{
	//		//SendDeviceStatusEvent(m_pHandler->GetLogicalServiceName().c_str(), WFS_CIM_DEVONLINE);
	//		// Успешное завершение ресета
	//		co_return XFS4IoTFramework::CashManagement::ResetDeviceResult(
	//			XFS4IoT::MessageHeader::CompletionCodeEnum::Success);
	//		break;
	//	}
	//	case StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_1:
	//		// Ошибка устройства
	//		co_return XFS4IoTFramework::CashManagement::ResetDeviceResult(
	//			XFS4IoT::MessageHeader::CompletionCodeEnum::HardwareError);
	//		break;
	//	case StateMachine::BlockedWaitResult::BWR_CANCELLED:
	//		// Операция была отменена
	//		co_return XFS4IoTFramework::CashManagement::ResetDeviceResult(
	//			XFS4IoT::MessageHeader::CompletionCodeEnum::Canceled);
	//		break;
	//	default:
	//		co_return XFS4IoTFramework::CashManagement::ResetDeviceResult(
	//			XFS4IoT::MessageHeader::CompletionCodeEnum::HardwareError);
	//	}
	//}

	boost::asio::awaitable<XFS4IoTFramework::CashManagement::CalibrateCashUnitResult> CashAcceptorSample::CalibrateCashUnitAsync(std::shared_ptr<XFS4IoTFramework::CashManagement::CalibrateCashUnitCommandEvents> events, const XFS4IoTFramework::CashManagement::CalibrateCashUnitRequest& calibrationInfo, std::stop_token cancellation)
	{
		return boost::asio::awaitable<XFS4IoTFramework::CashManagement::CalibrateCashUnitResult>();
	}

	boost::asio::awaitable<XFS4IoTFramework::CashManagement::GetTellerInfoResult> CashAcceptorSample::GetTellerInfoAsync(const XFS4IoTFramework::CashManagement::GetTellerInfoRequest& request, std::stop_token cancellation)
	{
		return boost::asio::awaitable<XFS4IoTFramework::CashManagement::GetTellerInfoResult>();
	}

	boost::asio::awaitable<XFS4IoTFramework::CashManagement::SetTellerInfoResult> CashAcceptorSample::SetTellerInfoAsync(const XFS4IoTFramework::CashManagement::SetTellerInfoRequest& request, std::stop_token cancellation)
	{
		return boost::asio::awaitable<XFS4IoTFramework::CashManagement::SetTellerInfoResult>();
	}

	XFS4IoTFramework::CashManagement::GetItemInfoResult CashAcceptorSample::GetItemInfoInfo(const XFS4IoTFramework::CashManagement::GetItemInfoRequest& request)
	{
		return XFS4IoTFramework::CashManagement::GetItemInfoResult();
	}

	void CashAcceptorSample::SendMediaDetected(const std::string& storageId, XFS4IoTFramework::CashManagement::ItemTargetEnum target, std::optional<int> index)
	{
		auto provider =
			std::dynamic_pointer_cast<XFS4IoTServer::CashAcceptorServiceProvider>(
				setServiceProvider_);

		if (!provider) {
			logger_->warn("SendMediaDetected: service provider is null or invalid");
			return;
		}

		boost::asio::co_spawn(
			provider->getIoContext(),
			provider->MediaDetectedEvent(storageId, target, index),
			[logger = logger_](std::exception_ptr e) {
				if (!e) return;

				try {
					std::rethrow_exception(e);
				}
				catch (const std::exception& ex) {
					logger->error(std::format(
						"SendMediaDetected failed: {}",
						ex.what()));
				}
			});
	}

	bool CashAcceptorSample::GetCashStorageConfiguration(
		std::map<std::string, XFS4IoTFramework::Storage::CashUnitStorageConfiguration>& newCashUnits)
	{
		std::lock_guard lock(m_mtx);

		newCashUnits.clear();

		for (const auto& [id, info] : cashUnitInfo_)
		{
			newCashUnits.emplace(id, info.cashUnitStorageConfig_);
		}

		return true;
	}

	bool CashAcceptorSample::GetCashUnitCounts(
		std::map<std::string, XFS4IoTFramework::Storage::CashUnitCountClass>& unitCounts)
	{
		std::lock_guard lock(m_mtx);

		unitCounts.clear();

		if (cashUnitInfo_.empty()) {
			logger_->warn(std::format(
				"{}() - cashUnitInfo_ пустой",
				__FUNCTION__));
			return false;
		}

		for (const auto& [storageId, cashInfo] : cashUnitInfo_)
		{
			unitCounts.emplace(storageId, cashInfo.unitCount_);
		}

		return !unitCounts.empty();
	}

	bool CashAcceptorSample::GetCashUnitInitialCounts(std::map<std::string, XFS4IoTFramework::Storage::StorageCashCountClass>& initialCounts)
	{
		return false;
	}

	bool CashAcceptorSample::GetCashStorageStatus(std::map<std::string, XFS4IoTFramework::Storage::CashUnitStorage::StatusEnum>& storageStatus)
	{
		return false;
	}

	bool CashAcceptorSample::GetCashUnitStatus(std::map<std::string, XFS4IoTFramework::Storage::CashStatusClass::ReplenishmentStatusEnum>& unitStatus)
	{
		return false;
	}

	void CashAcceptorSample::GetCashUnitAccuracy(const std::string& storageId, XFS4IoTFramework::Storage::CashStatusClass::AccuracyEnum& unitAccuracy)
	{
	}


	boost::asio::awaitable<XFS4IoTFramework::Storage::SetCashStorageResult> CashAcceptorSample::SetCashStorageAsync(const XFS4IoTFramework::Storage::SetCashStorageRequest& request, std::stop_token cancellation)
	{
		return boost::asio::awaitable<XFS4IoTFramework::Storage::SetCashStorageResult>();
	}

	boost::asio::awaitable<XFS4IoTFramework::Storage::StartExchangeResult> CashAcceptorSample::StartExchangeAsync(std::stop_token cancellation)
	{
		return boost::asio::awaitable<XFS4IoTFramework::Storage::StartExchangeResult>();
	}

	boost::asio::awaitable<XFS4IoTFramework::Storage::EndExchangeResult> CashAcceptorSample::EndExchangeAsync(std::stop_token cancellation)
	{
		return boost::asio::awaitable<XFS4IoTFramework::Storage::EndExchangeResult>();
	}

}