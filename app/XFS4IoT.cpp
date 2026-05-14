#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include "../framework/core/Persistent/FilePersistentData.hpp"
#include "../framework/core/Logger/ConsoleLogger.hpp"
#include "../framework/server/ServicePublisher.hpp"
#include "../Devices/CashAcceptorSample/CashAcceptorSample.hpp"
#include "../framework/core/Logger/ILogger.hpp"
#include "../framework/core/Logger/LoggerSpdlog.hpp"
#include "../framework/ServiceClasses/CommonServiceProvider/CashManagementCapabilitiesClass.hpp"

constexpr uint32_t BAUD_RATE = 921600;
constexpr uint16_t BYTE_SIZE = 8;

int main()
{
	auto logger = std::make_shared<LoggerSpdlog>();
	try {
		SettingModule::GetInstance()->initializeParameters(logger);
		PersistentDatasHandler::GetInstance(logger); // инициализация персистентных данных

		std::string startedText = R"(        ██████  ███████      ███████ ████████  █████  ██████  ████████ ███████ ██████  
        ██   ██ ██           ██         ██    ██   ██ ██   ██    ██    ██      ██   ██
        ██████  ███████      ███████    ██    ███████ ██████     ██    █████   ██   ██
        ██           ██           ██    ██    ██   ██ ██   ██    ██    ██      ██   ██
        ██      ███████      ███████    ██    ██   ██ ██   ██    ██    ███████ ██████
        )";
		logger->trace("\n" + startedText);

		logger->trace("Main: Running ServiceProvider Server");

		// Инициализация настроек

		// IO context для ассинхронных операции
		boost::asio::io_context ioContext;

		// publisher
		auto publisher = std::make_shared<XFS4IoTServer::ServicePublisher>(
			ioContext,
			logger,
			nullptr // JSON schema validator
		);
		publisher->Initialize();

		auto endpointDetails = publisher->GetEndpointDetails();
		if (!endpointDetails) {
			logger->error("Main: Не удалось получить EndpointDetails от ServicePublisher.");
			return 1;
		}

		std::string portName = SettingModule::GetInstance()->getPort();
		logger->trace(std::format("{}() PortName = {}", __FUNCTION__, portName), LOGLEVEL1);

		auto device = std::make_unique<FS365::HW::Dors::DorsHW>(portName,
			BAUD_RATE,
			BYTE_SIZE,
			boost::asio::serial_port_base::parity::none,
			boost::asio::serial_port_base::stop_bits::one,
			logger);

		FS365::HW::Dors::CIdentification m_idn;
		
		std::map<std::string, XFS4IoTFramework::Common::CashManagementCapabilitiesClass::BanknoteItem> allBanknoteIDs;
		
		/// Таблица номиналов (результат GetBillTable())
		std::span<uint8_t, 120> m_Bills{ new uint8_t[120]{}, 120 };
		
		if (device->IsDeviceInitialized())
		{
			device->Identification(m_idn);
			if (device->GetBillTable(m_Bills) != FS365::HW::Dors::DorsHW::RESULT::Ok) {
				logger->error(std::format("Main: Не удалось получить таблицу номиналов")); // Не удалось получить таблицу номиналов
			}

			// Промежуточная карта: ключ - деноминация (5 байт), значение - индексы в m_Bills
			// Из протокола: Структура блока:
			//  1 байт – Наибольшее значащее число номинала, беззнаковое целое
			//  Байты 2 - 4: Имя валюты, ASCII
			//  Байт 5 : Определяет дробный номинал(экспонента) или количество значащих нулей номинала, знаковое целое.
			//  1RUB1  -> 1 * 10^1 = 10 RUB
			//  5RUB2  -> 5 * 10^2 = 500 RUB
			std::map<std::string_view, int> maxValuesRegistered;
			std::string skippedNoteTypes;


			for (const auto noteId : std::views::iota(0, 24)) {
				const uint8_t* pDenomination = m_Bills.data() + 5 * noteId;
				if (pDenomination[0] == 0) {
					continue; // Пропускаем пустые записи
				}

				// Формируем ключ деноминации как string_view для эффективности
				std::string_view denomKey(reinterpret_cast<const char*>(pDenomination), 5);
				std::string_view currency(denomKey.substr(1, 3));

				if (currency == "BAR") {
					continue; // Пропускаем валюту BAR
				}

				// Вычисляем номинал
				int value = static_cast<int>(pDenomination[0]) * std::pow(10, pDenomination[4]);

				// Пропускаем монеты (значение меньше максимального для валюты)
				if (auto it = maxValuesRegistered.find(currency); it != maxValuesRegistered.end()) {
					if (it->second > value) {
						skippedNoteTypes += std::format("{} {}; ", value, currency);
						continue;
					}
				}
				else {
					maxValuesRegistered[currency] = value;
				}

				// typeRUB5
				std::string keyNanknoteName = "type" + std::string(currency) + std::to_string(value);
				allBanknoteIDs[keyNanknoteName] =
					XFS4IoTFramework::Common::CashManagementCapabilitiesClass::BanknoteItem(noteId, std::string(currency), double(value), 1, true);
			}

			if (!skippedNoteTypes.empty()) {
				logger->warn(std::format("{}() - Пропускаем монеты: {}", __FUNCTION__, skippedNoteTypes));
				device->println("Пропускаем монеты: ", skippedNoteTypes);
			}
		}
		// Создание CashAcceptor (PSHandler)
		auto cimCashAcceptorDevice = std::make_shared<XFS4IoTSP::CashAcceptor::Sample::CashAcceptorSample>(
			std::move(device),
			m_idn, 
			logger, 
			allBanknoteIDs);
		cimCashAcceptorDevice->Initialize();

		// Создание persistent data once and reuse
		auto persistentData = std::make_shared<FilePersistentData>(logger);

		// Create CashAcceptor service
		auto cashAcceptorService = std::make_shared<XFS4IoTServer::CashAcceptorServiceProvider>(
			*endpointDetails,
			"CimCashAcceptor", // Service name
			cimCashAcceptorDevice,
			logger,
			persistentData
		);

		logger->trace(std::format(
			"MAIN cashAcceptorService ptr = {}",
			static_cast<const void*>(cashAcceptorService.get())));

		// Инициализировать дочерние сервисы (безопасно — объект уже в shared_ptr)
		cashAcceptorService->Initialize(logger, persistentData);

		logger->trace(std::format(
			"MAIN после инициализации cashAcceptorService ptr = {}",
			static_cast<const void*>(cashAcceptorService.get())));

		// Link device to service
		cimCashAcceptorDevice->SetServiceProvider(cashAcceptorService);

		// Add service to publisher
		publisher->Add(cashAcceptorService);

		logger->trace(std::format(
			"MAIN после добавления cashAcceptorService ptr = {}",
			static_cast<const void*>(cashAcceptorService.get())));

		// Create cancellation source
		auto cancelToken = std::make_shared<XFS4IoTServer::CancellationSource>();

		// Run publisher
		boost::asio::co_spawn(
			ioContext,
			publisher->RunAsync(cancelToken),
			[&logger](std::exception_ptr e) {
				if (e) {
					try {
						std::rethrow_exception(e);
					}
					catch (const std::exception& ex) {
						logger->error(std::format("Main: Exception in RunAsync: {}", ex.what()));
					}
				}
			}
		);

		// Handle Ctrl+C for graceful shutdown
		boost::asio::signal_set signals(ioContext, SIGINT, SIGTERM);
		signals.async_wait([&cancelToken, &logger](const boost::system::error_code&, int) {
			logger->trace("Main: Получен сигнал завершения работы.");
			cancelToken->Cancel();
			});

		// Run IO context
		ioContext.run();

		logger->trace("Main: Отключение сервера завершено");
	}
	catch (const std::exception& e) {
		logger->warn(
			std::format("Main: Необработанное исключение : {}", e.what()));
		return 1;
	}

	return 0;
}
