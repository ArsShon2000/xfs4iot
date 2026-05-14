//#include <nlohmann/json.hpp>
//#include <map>
//#include <typeindex>
//#include "IMessageDecoder.hpp"
//
//namespace XFS4IoT
//{
//    /// <summary>
//    /// Example implementation of IMessageDecoder
//    /// </summary>
//    class MessageDecoder : public IMessageDecoder
//    {
//    public:
//        MessageDecoder() = default;
//
//        /// <summary>
//        /// Register a message type with its factory function
//        /// </summary>
//        template<typename MessageType>
//        void RegisterMessageType(const std::string& typeName)
//        {
//            factories_[typeName] = [](const nlohmann::json& j) -> std::shared_ptr<MessageBase> {
//                // This would use your actual deserialization logic
//                // For example: return std::make_shared<MessageType>(j);
//                return nullptr; // Placeholder
//                };
//            supportedTypes_.push_back(typeName);
//        }
//
//        std::shared_ptr<MessageBase> TryUnserialise(const std::string& json) const override
//        {
//            try {
//                auto j = nlohmann::json::parse(json);
//
//                // Extract message type from header
//                if (!j.contains("header") || !j["header"].contains("name")) {
//                    return nullptr;
//                }
//
//                std::string messageType = j["header"]["name"].get<std::string>();
//
//                auto it = factories_.find(messageType);
//                if (it == factories_.end()) {
//                    return nullptr;
//                }
//
//                return it->second(j);
//            }
//            catch (const std::exception&) {
//                return nullptr;
//            }
//        }
//
//        std::vector<std::string> GetSupportedMessageTypes() const override
//        {
//            return supportedTypes_;
//        }
//
//    private:
//        using Factory = std::function<std::shared_ptr<MessageBase>(const nlohmann::json&)>;
//        std::map<std::string, Factory> factories_;
//        std::vector<std::string> supportedTypes_;
//    };
//}


////////////////////////////////////////////////

#pragma once

#include <nlohmann/json.hpp>
#include <map>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "IMessageDecoder.hpp"
#include <iostream>
#include "../framework/core/Logger/ILogger.hpp"

namespace XFS4IoT
{
    class MessageDecoder : public IMessageDecoder
    {
    public:
        MessageDecoder(std::shared_ptr<ILogger> logger)
            : logger_(logger)
        {
        }

        template<typename MessageType>
        void RegisterMessageType(const std::string& typeName)
        {
            factories_[typeName] = [](const nlohmann::json& j) -> std::shared_ptr<MessageBase>
                {
                    return MessageType::FromJson(j);
                };

            if (std::find(supportedTypes_.begin(), supportedTypes_.end(), typeName) == supportedTypes_.end()) {
                supportedTypes_.push_back(typeName);
            }
        }

        std::shared_ptr<MessageBase> TryUnserialise(const std::string& json) const override
        {
            try {
                auto j = nlohmann::json::parse(json);

                logger_->trace(
                    std::format("{}() - Команда успешно преобразована в json", __FUNCTION__), 500);


                if (!j.contains("header") || !j["header"].contains("name")) {
                    return nullptr;
                }

                logger_->trace(
                    std::format("{}() - Команда имеет header", __FUNCTION__), 500);

                std::string messageType = j["header"]["name"].get<std::string>();

                logger_->trace(
                    std::format("{}() - messageType = {}", __FUNCTION__, messageType), 500);

                logger_->trace(
                    std::format("{}() - Количество регистрированных комманд = {}", __FUNCTION__, factories_.size()), 500);

                auto it = factories_.find(messageType);
                if (it == factories_.end()) {
                    return nullptr;
                }

                logger_->trace(
                    std::format("{}() - Отправленная команда регистрирована", __FUNCTION__, messageType), 500);

                return it->second(j);
            }
            catch (const std::exception& ex) {
                std::cerr << "Decoder exception: " << ex.what() << std::endl;
                return nullptr;
            }
        }

        std::vector<std::string> GetSupportedMessageTypes() const override
        {
            return supportedTypes_;
        }

    private:
		// Factory это функция, которая принимает JSON и возвращает указатель на MessageBase (то есть регистрированные команды)
        using Factory = std::function<std::shared_ptr<MessageBase>(const nlohmann::json&)>;
        std::map<std::string, Factory> factories_;
        std::vector<std::string> supportedTypes_; 
        std::shared_ptr<ILogger> logger_;
    };
}