#pragma once

#include <nlohmann/json.hpp>
#include <map>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "IMessageDecoder.hpp"
#include <iostream>
#include "./Logger/ILogger.hpp"
#include "common/Commands/UnsupportedCommand.hpp"

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
        void RegisterMessageType(const std::string& typeName,
            const std::vector<std::string>& versions)
        {
            for (const auto& version : versions) {
                factories_[{ typeName, version }] =
                    [](const nlohmann::json& j) -> std::shared_ptr<MessageBase>
                    {
                        return MessageType::FromJson(j);
                    };
            }

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
                std::string version = j["header"]["version"].get<std::string>();

                logger_->trace(
                    std::format("{}() - messageType = {}", __FUNCTION__, messageType), 500);

                logger_->trace(
                    std::format("{}() - Количество регистрированных комманд = {}", __FUNCTION__, factories_.size()), 500);

                auto it = factories_.find({ messageType, version });
                if (it == factories_.end()) {
                    logger_->warn(std::format("Команда или данная версия команды не поддерживается!"));
                    return XFS4IoT::Common::Commands::UnsupportedCommand::FromJson(j);
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

        using MessageKey = std::pair<std::string, std::string>;
        std::map<MessageKey, Factory> factories_;
        std::vector<std::string> supportedTypes_; 
        std::shared_ptr<ILogger> logger_;
    };
}