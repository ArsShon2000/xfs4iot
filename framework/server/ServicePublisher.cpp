#include "ServicePublisher.hpp"
#include "../core/common/XFSConstants.hpp"
#include "../core/Configurations.hpp"
#include "../core/Contracts.hpp"
#include <boost/url.hpp>
#include <ranges>
#include "../core/ServicePublisher/Commands/GetServicesCommand.hpp"
#include "GetServiceHandler.hpp"
#include "EndPoint.hpp"
#include "../core/Common/Commands/CapabilitiesCommand.hpp"
#include "../core/SettingModule/SettingModule.h"
#include "ServiceProvider.hpp"

//using namespace XFS4IoT;

namespace XFS4IoTServer
{
    ServicePublisher::ServicePublisher(
        boost::asio::io_context& ioContext,
        std::shared_ptr<ILogger> logger,
        std::shared_ptr<IJsonSchemaValidator> jsonSchemaValidator)
        : CommandDispatcher({ XFS4IoT::XFSConstants::ServiceClass::Publisher }, logger)
		, ioContext_(ioContext)
        , logger_(logger)
        , jsonSchemaValidator_(std::move(jsonSchemaValidator))
        , name_("")
    {

        XFS4IoT::Contracts::IsNotNull(logger.get(),
            std::format("В конструкторе {} получен недопустимый параметр.. {}",
                "ServicePublisher", "logger"));

        //if (!serviceConfiguration) {
        //    logger_->trace(std::format("{}() - {}: Объект конфигурации не задан, используется значение по умолчанию. {}", __FUNCTION__, XFS4IoT::Constants::Framework,
        //        XFS4IoT::Configurations::ServerAddressUri));
        //}

		// устанавливаем поддерживаемые команды для издателя сервиса
		// для издателя сервиса поддерживаем только одну версию каждой команды, которая является последней
		// В C++ это делается через регистрацию, а не через рефлексию
        //
		// Регистрацию перенес в функцтию Initialize, которая вызывается после добавления всех служб, чтобы гарантировать, что все команды зарегистрированы до запуска издателя
    }

    ServicePublisher::~ServicePublisher()
    {
		// Поскольку ServicePublisher управляет жизненным циклом своих служб, убедитесь, что они корректно останавливаются и очищаются при уничтожении издателя.
    }

    boost::asio::awaitable<void> ServicePublisher::RunAsync(
        std::shared_ptr<CancellationSource> cancellationSource)
    {
        try {
            if (jsonSchemaValidator_) {
                co_await jsonSchemaValidator_->LoadSchemaAsync();
            }

            if (endPoint_) {
                endPoint_->SetJsonSchemaValidator(jsonSchemaValidator_);
            }

            auto executor = co_await boost::asio::this_coro::executor;

            auto logException = [logger = logger_](std::exception_ptr e) {
                if (!e) return;

                try {
                    std::rethrow_exception(e);
                }
                catch (const std::exception& ex) {
                    logger->error(std::format("Child coroutine failed: {}", ex.what()));
                }
                };

            boost::asio::co_spawn(
                executor,
                endPoint_->RunAsync(cancellationSource->GetToken()),
                logException
            );

            /*boost::asio::co_spawn(
                executor,
                CommandDispatcher::RunAsync(cancellationSource),
                logException
            );*/
            CommandDispatcher::StartCommandQueue(cancellationSource);

            /*for (auto& service : services_) {
                boost::asio::co_spawn(
                    executor,
                    service->RunAsync(cancellationSource),
                    logException
                );
            }*/
            for (auto& service : services_) {
                if (auto provider = std::dynamic_pointer_cast<ServiceProvider>(service)) {
                    provider->StartCommandQueue(cancellationSource);
                }
            }

            boost::asio::steady_timer keepAlive(executor);
            keepAlive.expires_at(std::chrono::steady_clock::time_point::max());

            boost::system::error_code ec;
            co_await keepAlive.async_wait(
                boost::asio::redirect_error(boost::asio::use_awaitable, ec)
            );
        }
        catch (const std::exception& ex) {
            logger_->warn(
                std::format("{}() - {}: Обнаружено исключение, возникшее в процессе обработки {}: {}", __FUNCTION__, XFS4IoT::Constants::Component,
                    "ServicePublisher", ex.what()));

			// Критическая ошибка, которая не может быть обработана, поэтому завершаем процесс
            std::terminate();
        }
    }
    //boost::asio::awaitable<void> ServicePublisher::RunAsync(
    //    std::shared_ptr<CancellationSource> cancellationSource)
    //{
    //    try {
    //        if (jsonSchemaValidator_) {
    //            co_await jsonSchemaValidator_->LoadSchemaAsync();
    //        }

    //        if (endPoint_) {
    //            endPoint_->SetJsonSchemaValidator(jsonSchemaValidator_);
    //        }

    //        auto executor = co_await boost::asio::this_coro::executor;

    //        if (endPoint_) {
    //            boost::asio::co_spawn(
    //                executor,
    //                endPoint_->RunAsync(cancellationSource->GetToken()),
    //                boost::asio::detached);
    //        }

    //        boost::asio::co_spawn(
    //            executor,
    //            CommandDispatcher::RunAsync(cancellationSource),
    //            boost::asio::detached);

    //        for (auto& service : services_) {
    //            boost::asio::co_spawn(
    //                executor,
    //                service->RunAsync(cancellationSource),
    //                boost::asio::detached);
    //        }

    //        boost::asio::steady_timer timer(executor);
    //        timer.expires_at(std::chrono::steady_clock::time_point::max());
    //        co_await timer.async_wait(boost::asio::use_awaitable);
    //    }
    //    catch (const std::exception& ex) {
    //        logger_->warn(std::format(
    //            "{}() - {}: Caught an exception thrown in {}: {}",
    //            __FUNCTION__,
    //            XFS4IoT::Constants::Component,
    //            "ServicePublisher",
    //            ex.what()));
    //        std::terminate();
    //    }
    //}

    void ServicePublisher::Add(std::shared_ptr<IServiceProvider> service)
    {
        logger_->trace(std::format(
            "ServicePublisher::Add service.get() = {}",
            static_cast<const void*>(service.get())));

        services_.push_back(service);
        service->SetJsonSchemaValidator(jsonSchemaValidator_);
    }

    boost::asio::awaitable<void> ServicePublisher::BroadcastEvent(
        std::shared_ptr<XFS4IoT::MessageBase> payload)
    {
        throw std::logic_error(
            std::format("Для издателя сервиса не определены никакие события трансляции. "
                "Не вызывайте метод {} для этого класса.", "BroadcastEvent"));
        co_return;
    }

    boost::asio::awaitable<void> ServicePublisher::BroadcastEvent(
        const std::vector<std::shared_ptr<IConnection>>& connections,
        std::shared_ptr<XFS4IoT::MessageBase> payload)
    {
        throw std::logic_error(
            std::format("Для издателя сервиса не определены никакие события трансляции. "
                "Не вызывайте метод {} для этого класса..", "BroadcastEvent"));
        co_return;
    }

    void ServicePublisher::SetJsonSchemaValidator(
        std::shared_ptr<IJsonSchemaValidator> jsonSchemaValidator)
    {
        jsonSchemaValidator_ = std::move(jsonSchemaValidator);
    }

    void ServicePublisher::SetMessagesSupported(
        const std::map<std::string, XFS4IoT::MessageTypeInfo>& messagesSupported)
    {
        m_messagesSupported = messagesSupported;
    }

    std::map<std::string, XFS4IoT::MessageTypeInfo> ServicePublisher::GetMessagesSupported() const
    {
        return m_messagesSupported;
    }

    std::shared_ptr<XFS4IoT::IMessageDecoder> ServicePublisher::GetMessageDecoder() const
    {
        return commandDecoder_;
    }


	// Тут мы предоставляем реализацию метода getIoContext(), который возвращает ссылку на io_context, используемый издателем сервиса. Это позволяет службам, которые интегрируются с издателем, использовать тот же io_context для своих асинхронных операций, обеспечивая согласованность и эффективное управление ресурсами.
    boost::asio::io_context& ServicePublisher::getIoContext()
    {
        if (!endPoint_) {
            throw std::logic_error("Конечная точка не инициализирована.");
        }
        return endPoint_->getIoContext();
    }

    std::shared_ptr<ICommandDispatcher> ServicePublisher::GetDispatcherPtr()
    {
        return shared_from_this();
        //return std::static_pointer_cast<ICommandDispatcher>(shared_from_this());
    }
    void ServicePublisher::Initialize()
    {
		// Получаем базовый URI из конфигурации
        std::string serverAddressUri;
        serverAddressUri = SettingModule::GetInstance()->getHost();

        if (serverAddressUri.empty()) {
            logger_->trace(
                std::format("{}() - {}: Значение конфигурации '{}' отсутствует, используется значение по умолчанию. {}", __FUNCTION__, XFS4IoT::Constants::Framework,
                    XFS4IoT::Configurations::ServerAddressUri,
                    XFS4IoT::Configurations::Default::ServerAddressUri));
            serverAddressUri = XFS4IoT::Configurations::Default::ServerAddressUri;
        }
        else {
			// Валидация URI: должен быть допустимым HTTP или HTTPS URI без номера порта
            std::regex uriPattern(R"(^https?://[-_.!~*'()a-z0-9%]+$)",
                std::regex::icase);
            bool valid = std::regex_match(serverAddressUri, uriPattern);
            XFS4IoT::Contracts::IsTrue(valid,
                "Настроен недопустимый URI службы. URI не должен содержать номер порта. "
                "т.е. http(s)://Terminal321.ATMNetwork.corporatenet");
        }

		// Получаем список допустимых портов из констант
        std::vector<int> portRanges = XFS4IoT::XFSConstants::PortRanges();

		// Проверяем, есть ли в конфигурации указание на конкретный порт для службы, и если он есть и допустим, перемещаем его в начало списка портов для приоритета при попытке привязки. Это позволяет пользователю указать предпочтительный порт, если это необходимо, при условии, что он входит в допустимые диапазоны.
		std::string specificPort;
        specificPort = SettingModule::GetInstance()->getWSPort();
		if (!specificPort.empty()) {
			try {
				int port = std::stoi(specificPort);
				auto it = std::ranges::find(portRanges, port);
				if (it != portRanges.end()) {
					portRanges.erase(it);
					portRanges.insert(portRanges.begin(), port);
				}
				else {
					logger_->warn(
						std::format("{}() - {}: Неверная конфигурация для '{}'. "
							"Допустимый номер порта: 80, 443 или 5846-5856.", __FUNCTION__, XFS4IoT::Constants::Framework,
							XFS4IoT::Configurations::ServerPort));
				}
			}
			catch (const std::exception&) {
				// Неверный формат порта, игнорируем и используем стандартные порты
			}
		}

		// Если URI начинается с https://, удаляем порт 80 из списка допустимых портов, так как он не подходит для HTTPS. Это гарантирует, что при попытке привязки к HTTPS URI не будет использоваться неподходящий порт.
        std::string lowerUri = serverAddressUri;
        std::ranges::transform(lowerUri, lowerUri.begin(), ::tolower);
        if (lowerUri.starts_with("https://")) {
            auto it = std::ranges::find(portRanges, 80);
            if (it != portRanges.end()) {
                portRanges.erase(it);
            }
        }


		// Попытка привязки к каждому порту из списка, пока не будет успешно привязано. Если ни один порт не удается привязать, выбрасывается исключение. Это обеспечивает гибкость в выборе порта и позволяет избежать конфликтов с другими службами, которые могут уже использовать некоторые из портов.
        bool bound = false;
        for (int port : portRanges) {
            try {
				// Создаем полный URI для HTTP (или HTTPS) сервиса, добавляя порт и путь. Это формирует базовый URI, по которому сервис будет доступен для клиентов.
                uri_ = std::format("{}:{}/xfs4iot/v1.0/", serverAddressUri, port);

				// Создаем соответствующий URI для WebSocket, заменяя схему http на ws и https на wss. Это обеспечивает правильный URI для клиентов, которые будут подключаться через WebSocket, сохраняя при этом согласованность с базовым URI сервиса.
                std::string serverAddressWUri = serverAddressUri;
                if (serverAddressWUri.starts_with("http://")) {
                    serverAddressWUri = "ws://" + serverAddressWUri.substr(7);
                }
                else if (serverAddressWUri.starts_with("https://")) {
                    serverAddressWUri = "wss://" + serverAddressWUri.substr(8);
                }
                wsUri_ = std::format("{}:{}/xfs4iot/v1.0/", serverAddressWUri, port);

                endpointDetails_ = std::make_shared<EndpointDetails>(
                    serverAddressUri, serverAddressWUri, port);

                logger_->trace(
                    std::format("{}() - {}: Попытка привязки к {}", __FUNCTION__, XFS4IoT::Constants::Component, uri_));

				// Создаем экземпляр декодера сообщений и регистрируем типы сообщений, которые поддерживает издатель сервиса. Это позволяет декодеру правильно распознавать и обрабатывать входящие команды от клиентов.
                // В C++ это делается через регистрацию, а не через рефлексию.

                auto rawDecoder = std::make_shared<XFS4IoT::MessageDecoder>(logger_);

                rawDecoder->RegisterMessageType<XFS4IoT::ServicePublisher::Commands::GetServicesCommand>(
                    "ServicePublisher.GetServices");

                RegisterHandler<
                    XFS4IoT::ServicePublisher::Commands::GetServicesCommand,
                    XFS4IoTServer::GetServiceHandler
                >(
                    XFS4IoT::XFSConstants::ServiceClass::Publisher,
                    true
                );


                //SetMessagesSupported({
                //    {
                //        "Common.Capabilities",
                //        XFS4IoT::MessageTypeInfo(
                //            XFS4IoT::MessageTypeInfo::MessageTypeEnum::Command,
                //            { "2.0" }
                //        )
                //    }
                //    });

                //std::shared_ptr<XFS4IoT::IMessageDecoder> decoderShared(std::move(rawDecoder));
				commandDecoder_ = rawDecoder;


                SetMessagesSupported({
                    {
                        "ServicePublisher.GetServices",
                        XFS4IoT::MessageTypeInfo(
                            XFS4IoT::MessageTypeInfo::MessageTypeEnum::Command,
                            { "1.0" }
                        )
                    }
                    });

                // Конвертируем unique_ptr<MessageDecoder> в shared_ptr<IMessageDecoder>
                // shared_ptr имеет конструктор, принимающий unique_ptr и берет на себя владение.
                //std::shared_ptr<XFS4IoT::IMessageDecoder> decoderShared =
                //    std::shared_ptr<XFS4IoT::IMessageDecoder>(std::move(commandDecoder_));
                // 


                // Получаем shared_ptr на this и явно приводим к shared_ptr<ICommandDispatcher>
                // Используем static_pointer_cast чтобы явно указать желаемое преобразование типов.
                // ВАЖНО: вызов shared_from_this() корректен только если объект создаётся через std::make_shared<ServicePublisher>.
                //auto dispatcherShared = std::static_pointer_cast<ICommandDispatcher>(std::enable_shared_from_this<ServicePublisher>::shared_from_this());
                auto dispatcherShared = std::static_pointer_cast<ICommandDispatcher>(shared_from_this());
				// Создаем конечную точку, передавая ей io_context, URI, декодер сообщений, диспетчер команд, логгер и weak_ptr на издателя сервиса. Это устанавливает конечную точку для прослушивания входящих соединений и обработки команд от клиентов.
                endPoint_ = std::make_unique<EndPoint>(
                    ioContext_,
                    uri_,
                    commandDecoder_,
                    dispatcherShared,
                    logger_,
                    weak_from_this());

                bound = true;
                break;
            }
            catch (const std::exception& ex) {
				// Попытка привязки к этому порту не удалась, логируем и пробуем следующий порт
                logger_->trace(
                    std::format("{}() - {}: Не удалось привязаться к порту {}: {}", __FUNCTION__, XFS4IoT::Constants::Component, port, ex.what()));
                continue;
            }
        }

        if (!bound) {
            XFS4IoT::Contracts::Fail("Не удается найти порт XFS4IoT для прослушивания.");
        }
    }

    std::shared_ptr<IServiceProvider> ServicePublisher::FindServiceByUri(const std::string& target)
    {
        auto normalize = [](std::string s)
            {
                auto pos = s.find("/xfs4iot/");
                if (pos != std::string::npos)
                    s = s.substr(pos);

                if (!s.empty() && s.back() != '/')
                    s.push_back('/');

                return s;
            };

        const std::string normalizedTarget = normalize(target);

        for (auto& service : services_) {

            // Получаем ws uri сервиса
            const std::string normalizedServiceUri = normalize(service->GetWSUri());

            logger_->trace(std::format("Проверка маршрутизации по отношению к сервису: {}", normalizedServiceUri));

            // Пример:
            // target:      /xfs4iot/v1.0/CimCashAcceptor/
            // serviceUri:  ws://host:port/xfs4iot/v1.0/SimCashAcceptor/

            // оставляем только path часть
            logger_->trace(std::format(
                "Маршрутизация compare target = '{}' service = '{}'",
                normalizedTarget, normalizedServiceUri));

            if (normalizedTarget == normalizedServiceUri) {
                logger_->trace(std::format(
                    "FindServiceByUri совпало service.get() = {}",
                    static_cast<const void*>(service.get())));

                return service;
            }
        }

        return nullptr;
    }
}

//#include "ServicePublisher.hpp"
//#include "../core/common/XFSConstants.hpp"
//#include "../core/Configurations.hpp"
//#include "../core/Contracts.hpp"
//#include <boost/url.hpp>
//#include <ranges>
//#include "../core/ServicePublisher/Commands/GetServicesCommand.hpp"
//#include "GetServiceHandler.hpp"
//#include "EndPoint.hpp"
//
////using namespace XFS4IoT;
//
//namespace XFS4IoTServer
//{
//    ServicePublisher::ServicePublisher(
//        boost::asio::io_context& ioContext,
//        std::shared_ptr<ILogger> logger,
//        std::shared_ptr<IServiceConfiguration> serviceConfiguration,
//        std::shared_ptr<IJsonSchemaValidator> jsonSchemaValidator)
//        : CommandDispatcher({ XFS4IoT::XFSConstants::ServiceClass::Publisher }, logger)
//        , ioContext_(ioContext)
//        , logger_(logger)
//        , jsonSchemaValidator_(std::move(jsonSchemaValidator))
//        , serviceConfiguration_(std::move(serviceConfiguration))
//        , name_("")
//    {
//
//        XFS4IoT::Contracts::IsNotNull(logger.get(),
//            std::format("Invalid parameter received in the {} constructor. {}",
//                "ServicePublisher", "logger"));
//
//        if (!serviceConfiguration) {
//            logger_->trace(std::format("{}() - {}: No configuration object is set and use default value. {}", __FUNCTION__, XFS4IoT::Constants::Framework,
//                XFS4IoT::Configurations::ServerAddressUri));
//        }
//
//        // Set service publisher specific command name and version to the dispatcher
//        // In C++, this would be done through registration instead of reflection
//        std::map<std::string, XFS4IoT::MessageTypeInfo> messagesSupported;
//        messagesSupported.emplace(
//            "ServicePublisher.GetServices",
//            XFS4IoT::MessageTypeInfo(
//                XFS4IoT::MessageTypeInfo::MessageTypeEnum::Command,
//                { "2.0" }
//            )
//        );
//        SetMessagesSupported(messagesSupported);
//
//
//
//    }
//
//    ServicePublisher::~ServicePublisher()
//    {
//        // Cleanup handled by unique_ptr
//    }
//
//    boost::asio::awaitable<void> ServicePublisher::RunAsync(
//        std::shared_ptr<CancellationSource> cancellationSource)
//    {
//        try {
//            // Load JSON schema if validator is available
//            if (jsonSchemaValidator_) {
//                co_await jsonSchemaValidator_->LoadSchemaAsync();
//            }
//
//            if (endPoint_) {
//                endPoint_->SetJsonSchemaValidator(jsonSchemaValidator_);
//            }
//
//            // Используем vector of functions returning awaitable<void>
//            std::vector<std::function<boost::asio::awaitable<void>()>> tasks;
//
//            if (endPoint_) {
//                tasks.push_back([this, cancellationSource]() {
//                    return endPoint_->RunAsync(cancellationSource->GetToken());
//                    });
//            }
//
//            tasks.push_back([this, cancellationSource]() {
//                return CommandDispatcher::RunAsync(cancellationSource);
//                });
//
//            //for (auto& service : services_) {
//            //    co_await service->RunAsync(cancellationSource);
//            //}
//
//            for (auto& service : services_) {
//                tasks.push_back([service, cancellationSource]() {
//                    return service->RunAsync(cancellationSource);
//                    });
//            }
//
//            // Запускаем и ожидаем каждую корутину
//            for (auto& task : tasks) {
//                co_await task();
//            }
//        }
//        catch (const std::exception& ex) {
//            logger_->warn(
//                std::format("{}() - {}: Caught an exception thrown in {}: {}", __FUNCTION__, XFS4IoT::Constants::Component,
//                    "ServicePublisher", ex.what()));
//
//            // Critical error - terminate process
//            std::terminate();
//        }
//    }
//
//    void ServicePublisher::Add(std::shared_ptr<IServiceProvider> service)
//    {
//        services_.push_back(service);
//        service->SetJsonSchemaValidator(jsonSchemaValidator_);
//    }
//
//    boost::asio::awaitable<void> ServicePublisher::BroadcastEvent(
//        std::shared_ptr<XFS4IoT::MessageBase> payload)
//    {
//        throw std::logic_error(
//            std::format("No broadcast events defined for the service publisher. "
//                "Do not call {} on this class.", "BroadcastEvent"));
//        co_return;
//    }
//
//    boost::asio::awaitable<void> ServicePublisher::BroadcastEvent(
//        const std::vector<std::shared_ptr<IConnection>>& connections,
//        std::shared_ptr<XFS4IoT::MessageBase> payload)
//    {
//        throw std::logic_error(
//            std::format("No broadcast events defined for the service publisher. "
//                "Do not call {} on this class.", "BroadcastEvent"));
//        co_return;
//    }
//
//    void ServicePublisher::SetJsonSchemaValidator(
//        std::shared_ptr<IJsonSchemaValidator> jsonSchemaValidator)
//    {
//        jsonSchemaValidator_ = std::move(jsonSchemaValidator);
//    }
//
//    void ServicePublisher::SetMessagesSupported(
//        const std::map<std::string, XFS4IoT::MessageTypeInfo>& messagesSupported)
//    {
//        m_messagesSupported = messagesSupported;
//    }
//
//    std::map<std::string, XFS4IoT::MessageTypeInfo> ServicePublisher::GetMessagesSupported() const
//    {
//        return m_messagesSupported;
//    }
//
//    std::shared_ptr<XFS4IoT::IMessageDecoder> ServicePublisher::GetMessageDecoder() const
//    {
//        return commandDecoder_;
//    }
//
//	// Provide io_context to satisfy IServiceProvider
//	boost::asio::io_context& ServicePublisher::getIoContext()
//	{
//		if (!endPoint_) {
//			throw std::logic_error("EndPoint is not initialized");
//		}
//		return endPoint_->getIoContext();
//	}
//
//    std::shared_ptr<ICommandDispatcher> ServicePublisher::GetDispatcherPtr()
//    {
//        return shared_from_this();
//        //return std::static_pointer_cast<ICommandDispatcher>(shared_from_this());
//    }
//    void ServicePublisher::Initialize()
//    {
//        // Get service URI from configuration
//        std::string serverAddressUri;
//        if (serviceConfiguration_) {
//            serverAddressUri =
//                serviceConfiguration_->get(XFS4IoT::Configurations::ServerAddressUri)
//                .value_or("");
//        }
//
//        if (serverAddressUri.empty()) {
//            logger_->trace(
//                std::format("{}() - {}: No configuration value '{}' exists and use default value. {}", __FUNCTION__, XFS4IoT::Constants::Framework,
//                    XFS4IoT::Configurations::ServerAddressUri,
//                    XFS4IoT::Configurations::Default::ServerAddressUri));
//            serverAddressUri = XFS4IoT::Configurations::Default::ServerAddressUri;
//        }
//        else {
//            // Validate URI format
//            std::regex uriPattern(R"(^https?://[-_.!~*'()a-z0-9%]+$)",
//                std::regex::icase);
//            bool valid = std::regex_match(serverAddressUri, uriPattern);
//            XFS4IoT::Contracts::IsTrue(valid,
//                "Invalid service URI is configured. URI must be without port number. "
//                "i.e. http(s)://Terminal321.ATMNetwork.corporatenet");
//        }
//
//        // Get port ranges
//        std::vector<int> portRanges = XFS4IoT::XFSConstants::PortRanges();
//
//        // Check for specific port configuration
//        if (serviceConfiguration_) {
//            std::string specificPort;
//            specificPort =
//                serviceConfiguration_->get(XFS4IoT::Configurations::ServerPort)
//                .value_or("");
//            if (!specificPort.empty()) {
//                try {
//                    int port = std::stoi(specificPort);
//                    auto it = std::ranges::find(portRanges, port);
//                    if (it != portRanges.end()) {
//                        portRanges.erase(it);
//                        portRanges.insert(portRanges.begin(), port);
//                    }
//                    else {
//                        logger_->warn(
//                            std::format("{}() - {}: Invalid configuration for '{}'. "
//                                "Valid port number is 80, 443 or 5846-5856", __FUNCTION__, XFS4IoT::Constants::Framework,
//                                XFS4IoT::Configurations::ServerPort));
//                    }
//                }
//                catch (const std::exception&) {
//                    // Invalid port number, ignore
//                }
//            }
//        }
//
//        // Remove port 80 for HTTPS
//        std::string lowerUri = serverAddressUri;
//        std::ranges::transform(lowerUri, lowerUri.begin(), ::tolower);
//        if (lowerUri.starts_with("https://")) {
//            auto it = std::ranges::find(portRanges, 80);
//            if (it != portRanges.end()) {
//                portRanges.erase(it);
//            }
//        }
//
//
//        // Try to bind to each port
//        bool bound = false;
//        for (int port : portRanges) {
//            try {
//                // Create URIs
//                uri_ = std::format("{}:{}/xfs4iot/v1.0/", serverAddressUri, port);
//
//                // Create WebSocket URI (replace http with ws)
//                std::string serverAddressWUri = serverAddressUri;
//                if (serverAddressWUri.starts_with("http://")) {
//                    serverAddressWUri = "ws://" + serverAddressWUri.substr(7);
//                }
//                else if (serverAddressWUri.starts_with("https://")) {
//                    serverAddressWUri = "wss://" + serverAddressWUri.substr(8);
//                }
//                wsUri_ = std::format("{}:{}/xfs4iot/v1.0/", serverAddressWUri, port);
//
//                endpointDetails_ = std::make_shared<EndpointDetails>(
//                    serverAddressUri, serverAddressWUri, port);
//
//                logger_->trace(
//                    std::format("{}() - {}: Attempting to bind to {}", __FUNCTION__, XFS4IoT::Constants::Component, uri_));
//
//                // Create command decoder
//                //commandDecoder_ = std::make_unique<XFS4IoT::MessageDecoder>();
//                // Auto-populate with command types
//                // In C++, you would register message types here
//
//                auto rawDecoder = std::make_unique<XFS4IoT::MessageDecoder>();
//
//                rawDecoder->RegisterMessageType<XFS4IoT::ServicePublisher::Commands::GetServicesCommand>(
//                    "ServicePublisher.GetServices");
//
//                RegisterHandler<
//                    XFS4IoT::ServicePublisher::Commands::GetServicesCommand,
//                    XFS4IoTServer::GetServiceHandler
//                >(
//                    XFS4IoT::XFSConstants::ServiceClass::Publisher,
//                    true
//                );
//
//                std::shared_ptr<XFS4IoT::IMessageDecoder> decoderShared(std::move(rawDecoder));
//
//
//                // Convert unique_ptr<MessageDecoder> to shared_ptr<IMessageDecoder>
//                // shared_ptr has a constructor taking a unique_ptr and will take ownership.
//                // 
//                //std::shared_ptr<XFS4IoT::IMessageDecoder> decoderShared =
//                    //std::shared_ptr<XFS4IoT::IMessageDecoder>(std::move(commandDecoder_));
//                    // 
//                // Auto-populate with command types
//                // In C++, you would register message types here
//
//                // Получаем shared_ptr на this и явно приводим к shared_ptr<ICommandDispatcher>
//                // Используем static_pointer_cast чтобы явно указать желаемое преобразование типов.
//                // ВАЖНО: вызов shared_from_this() корректен только если объект создаётся через std::make_shared<ServicePublisher>.
//                //auto dispatcherShared = std::static_pointer_cast<ICommandDispatcher>(std::enable_shared_from_this<ServicePublisher>::shared_from_this());
//                auto dispatcherShared = std::static_pointer_cast<ICommandDispatcher>(shared_from_this());
//                // Create endpoint
//                endPoint_ = std::make_unique<EndPoint>(
//                    ioContext_,
//                    uri_,
//                    decoderShared,
//                    dispatcherShared,
//                    logger_,
//					weak_from_this());
//
//                bound = true;
//                break;
//            }
//            catch (const std::exception& ex) {
//                // Try next port
//                logger_->trace(
//                    std::format("{}() - {}: Failed to bind to port {}: {}", __FUNCTION__, XFS4IoT::Constants::Component, port, ex.what()));
//                continue;
//            }
//        }
//
//        if (!bound) {
//            XFS4IoT::Contracts::Fail("Can't find an XFS4IoT port to listen on");
//        }
//    }
//
//	std::shared_ptr<IServiceProvider> ServicePublisher::FindServiceByUri(const std::string& target)
//	{
//		for (auto& service : services_) {
//
//			// Получаем ws uri сервиса
//			std::string serviceUri = service->GetWSUri();
//
//			// Пример:
//			// target:      /xfs4iot/v1.0/SimCashAcceptor/
//			// serviceUri:  ws://host:port/xfs4iot/v1.0/SimCashAcceptor/
//
//			// оставляем только path часть
//			auto pos = serviceUri.find("/xfs4iot/");
//			if (pos != std::string::npos) {
//				serviceUri = serviceUri.substr(pos);
//			}
//
//			if (target == serviceUri) {
//				return service;
//			}
//		}
//
//		return nullptr;
//	}
//}
//
