#pragma once

#include <string>
#include <typeindex>
#include <map>
#include <optional>

namespace XFS4IoT
{
    class MessageRegistry
    {
    public:
        void Register(const std::type_info& type, std::string name, std::string version)
        {
            names_[std::type_index(type)] = std::move(name);
            versions_[std::type_index(type)] = std::move(version);
        }

        std::string GetName(const std::type_info& type) const
        {
            auto it = names_.find(std::type_index(type));
            return it != names_.end() ? it->second : "";
        }

        std::string GetVersion(const std::type_info& type) const
        {
            auto it = versions_.find(std::type_index(type));
            return it != versions_.end() ? it->second : "";
        }

    private:
        std::map<std::type_index, std::string> names_;
        std::map<std::type_index, std::string> versions_;
    };
}

//#pragma once
//
//#include <string>
//#include <typeindex>
//#include <nlohmann/json.hpp>
//#include <map>
//#include <optional>
//#include <functional>
//#include "MessageBase.hpp"
//
//namespace XFS4IoT
//{
//    class MessageRegistry
//    {
//    public:
//        using Factory = std::function<std::shared_ptr<MessageBase>(const nlohmann::json&)>;
//
//        void Register(
//            const std::type_info& type,
//            std::string name,
//            std::string version,
//            Factory factory)
//        {
//            names_[std::type_index(type)] = name;
//            versions_[std::type_index(type)] = version;
//            factories_[name] = std::move(factory);
//        }
//
//        std::shared_ptr<MessageBase> Create(const std::string& name, const nlohmann::json& j) const
//        {
//            auto it = factories_.find(name);
//            if (it == factories_.end()) {
//                return nullptr;
//            }
//            return it->second(j);
//        }
//
//    private:
//        std::map<std::type_index, std::string> names_;
//        std::map<std::type_index, std::string> versions_;
//        std::map<std::string, Factory> factories_;
//    };
//}
//
