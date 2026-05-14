#pragma once
#include "IServiceProvider.hpp"
#include "CommandDispatcher.hpp"
#include "../core/MessageTypeInfo.hpp"
#include "../core/Logger/ILogger.hpp"
#include "../core/Message.hpp"

#include <boost/asio.hpp>
#include <thread>
#include <utility>
#include "EndpointDetails.hpp"

namespace XFS4IoTServer
{
    /// <summary>
    /// Реализация IServiceProvider
    /// </summary>
    class ServiceProvider : public CommandDispatcher, public IServiceProvider
        , public std::enable_shared_from_this<ServiceProvider>
    {
    public:
        ServiceProvider(const EndpointDetails& endpointDetails,
            const std::string& name,
            const std::vector<XFS4IoT::XFSConstants::ServiceClass>& services,
            std::shared_ptr<IDevice> device,
            std::shared_ptr<ILogger> logger)
            : CommandDispatcher(services, logger)
            , name_(name)
            , uri_(endpointDetails.GetServiceUri(name).uri)
            , wsUri_(endpointDetails.GetServiceUri(name).wsUri)
            , device_(std::move(device))
            , logger_(std::move(logger))
            , ioContext_()
            , workGuard_(boost::asio::make_work_guard(ioContext_))
        {
            // services можно сохранить, если нужно, например:
            // services_ = services;
            ioThread_ = std::thread([this]() {
                try {
                    ioContext_.run();
                }
                catch (...) {
                    // логирование при необходимости
                }
                });
        }

        ~ServiceProvider() override
        {
            // Останавливаем event loop и ждём завершения потока
            workGuard_.reset();          // позволяет loop завершиться когда нет задач
            ioContext_.stop();
            if (ioThread_.joinable())
                ioThread_.join();
        }

        // Реализация метода интерфейса для доступа к io_context
        boost::asio::io_context& getIoContext() override
        {
            return ioContext_;
        }

        std::string GetName() const override { return name_; }
        std::string GetUri() const override { return uri_; }
        std::string GetWSUri() const override { return wsUri_; }
        std::shared_ptr<IDevice> GetDevice() const override { return device_; }

        boost::asio::awaitable<void> BroadcastEvent(
            std::shared_ptr<XFS4IoT::MessageBase> payload) override
        {
            // Get all active connections and broadcast to them
            auto connections = GetAllConnections();
            co_await BroadcastEvent(connections, payload);
        }

        boost::asio::awaitable<void> BroadcastEvent(
            const std::vector<std::shared_ptr<IConnection>>& connections,
            std::shared_ptr<XFS4IoT::MessageBase> payload) override
        {
            if (!payload) {
                throw std::invalid_argument("Payload не может быть пустым");
            }

			// Отправка события каждому соединению. Ошибки при отправке одному соединению не должны мешать отправке другим.
            for (const auto& connection : connections) {
                if (connection) {
                    try {
                        co_await connection->SendMessageAsync(payload);
                    }
                    catch (const std::exception& ex) {
                        logger_->warn(std::format("{}() - ServiceProvider: Не удалось отправить событие соединению: {}", __FUNCTION__,
                            ex.what()));
						// Продолжить отправку другим соединениям, несмотря на ошибку
                    }
                }
            }
        }

        void SetJsonSchemaValidator(
            std::shared_ptr<IJsonSchemaValidator> jsonSchemaValidator) override
        {
            jsonSchemaValidator_ = std::move(jsonSchemaValidator);
        }

        void SetMessagesSupported(
            const std::map<std::string, XFS4IoT::MessageTypeInfo>& messagesSupported) override
        {
            messagesSupported_ = messagesSupported;
            m_messagesSupported = messagesSupported;
        }

        std::map<std::string, XFS4IoT::MessageTypeInfo> GetMessagesSupported() const override
        {
            return messagesSupported_;
        }

		// ICommandDispatcher методы будут реализованы в CommandDispatcher, так что здесь мы можем просто использовать их реализацию без переопределения
        // (Dispatch, DispatchError, RunAsync, и тп.)

        std::shared_ptr<XFS4IoT::IMessageDecoder> GetMessageDecoder() const override
        {
            return decoder_;
        }

        void AddConnection(std::shared_ptr<IConnection> connection)
        {
            std::lock_guard<std::mutex> lock(connectionsMutex_);
            connections_.push_back(std::move(connection));
        }

        void RemoveConnection(const std::shared_ptr<IConnection>& connection)
        {
            std::lock_guard<std::mutex> lock(connectionsMutex_);

            connections_.erase(
                std::remove(connections_.begin(), connections_.end(), connection),
                connections_.end());
        }
    protected:
        virtual std::vector<std::shared_ptr<IConnection>> GetAllConnections() const
        {
            std::lock_guard<std::mutex> lock(connectionsMutex_);
            return connections_;
        }

        std::shared_ptr<ICommandDispatcher> GetDispatcherPtr() override
        {
            return std::static_pointer_cast<ICommandDispatcher>(shared_from_this());
        }

        std::shared_ptr<IServiceProvider> GetServiceProviderPtr()
        {
            return std::static_pointer_cast<IServiceProvider>(shared_from_this());
        }

        std::shared_ptr<XFS4IoT::IMessageDecoder> decoder_;

    private:
        std::string name_;
        std::string uri_;
        std::string wsUri_;
        std::shared_ptr<IDevice> device_;
        std::shared_ptr<ILogger> logger_;
        std::shared_ptr<IJsonSchemaValidator> jsonSchemaValidator_;
        std::map<std::string, XFS4IoT::MessageTypeInfo> messagesSupported_;

        // Добавленные поля для io_context
        boost::asio::io_context ioContext_;
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> workGuard_;
        std::thread ioThread_;
        mutable std::mutex connectionsMutex_;
        std::vector<std::shared_ptr<IConnection>> connections_;
    };
}