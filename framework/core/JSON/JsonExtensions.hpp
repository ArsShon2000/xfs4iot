#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace XFS4IoT
{
    /// <summary>
    /// Type alias for JSON element (used for extension data)
    /// </summary>
    using JsonElement = nlohmann::json;

    /// <summary>
    /// Helper functions for JSON extension data handling
    /// </summary>
    template<typename T>
    inline std::unordered_map<std::string, T> ParseExtendedProperties(
        const std::unordered_map<std::string, JsonElement>& data)
    {
        std::unordered_map<std::string, T> result;

        for (const auto& [key, element] : data)
        {
            result[key] = element.template get<T>();
        }
        return result;
    }

    /// <summary>
    /// Create extension data dictionary from typed objects
    /// </summary>
    template<typename T>
    inline std::unordered_map<std::string, JsonElement> CreateExtensionData(
        const std::unordered_map<std::string, std::shared_ptr<T>>& value)
    {
        std::unordered_map<std::string, JsonElement> result;

        for (const auto& [key, ptr] : value)
        {
            if (ptr)
            {
                // Исправление: используем nlohmann::json::object для сериализации через adl_serializer
                result[key] = nlohmann::json(*ptr);
            }
        }
        return result;
    }

} // namespace XFS4IoT