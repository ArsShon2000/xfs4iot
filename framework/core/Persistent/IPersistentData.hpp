#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

class IPersistentData
{
public:
    virtual ~IPersistentData() = default;

    virtual bool StoreJson(const std::string& name, const nlohmann::json& j) = 0;
    virtual std::shared_ptr<nlohmann::json> LoadJson(const std::string& name) = 0;

    template<typename TValue>
    bool Store(const std::string& name, const TValue& obj)
    {
        try {
            nlohmann::json j = obj;
            return StoreJson(name, j);
        }
        catch (...) {
            return false;
        }
    }

    template<typename TValue>
    std::shared_ptr<TValue> Load(const std::string& name)
    {
        try {
            auto j = LoadJson(name);
            if (!j) {
                return nullptr;
            }

            return std::make_shared<TValue>(j->get<TValue>());
        }
        catch (...) {
            return nullptr;
        }
    }
};