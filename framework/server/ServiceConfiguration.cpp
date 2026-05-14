#include "ServiceConfiguration.hpp"
#include <fstream>
#include <stdexcept>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace XFS4IoTServer
{
    ServiceConfiguration::ServiceConfiguration(std::shared_ptr<ILogger> logger)
        : logger_(logger)
        , settings_()
    {
        try
        {
            loadSettings();
        }
        catch (const std::exception& ex)
        {
            logger_->warn(
                std::format("{}() — ServiceConfiguration: исключение перехвачено в конструкторе ServiceConfiguration. {}", __FUNCTION__, ex.what()));
        }
    }

    std::optional<std::string> ServiceConfiguration::get(const std::string& name) const
    {
        auto it = settings_.find(name);

        std::optional<std::string> configValue =
            (it != settings_.end()) ? std::optional<std::string>(it->second) : std::nullopt;

        logger_->trace(
            std::format("{}() - ServiceConfiguration: получение конфигурации Get({} = {} + в ServiceConfiguration"
                , __FUNCTION__, name, (configValue.has_value() ? configValue.value() : "null")));

        return configValue;
    }

    void ServiceConfiguration::loadSettings()
    {
        // Option 1: Using Boost Property Tree for INI files
        try
        {
            boost::property_tree::ptree pt;
            boost::property_tree::ini_parser::read_ini("app.config", pt);

            for (const auto& section : pt)
            {
                for (const auto& key : section.second)
                {
                    std::string fullKey = section.first + "." + key.first;
                    settings_[fullKey] = key.second.get_value<std::string>();
                }
            }
        }
        catch (const boost::property_tree::ini_parser_error& ex)
        {
            throw std::runtime_error(
                std::string("Не удалось разобрать файл конфигурации: ") + ex.what());
        }
    }
}