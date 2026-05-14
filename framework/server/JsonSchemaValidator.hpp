#include "IJsonSchemaValidator.hpp"
#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>
#include "../core/Logger/ILogger.hpp"
#include <memory>
#include <filesystem>
#include <fstream>

namespace XFS4IoTServer
{
    /// <summary>
    /// Example implementation of IJsonSchemaValidator using nlohmann/json-schema-validator
    /// </summary>
    class JSONSchemaValidator : public IJsonSchemaValidator
    {
    public:
        explicit JSONSchemaValidator(std::shared_ptr<ILogger> logger)
            : logger_(std::move(logger))
            , schemaLoaded_(false)
        {}

        boost::asio::awaitable<void> LoadSchemaAsync() override
        {
            try {
                // Load JsonSchema from embedded resource or file
                const std::string schemaFileName = "JsonSchema-2021-1.json";

                // Option 1: Load from file
                std::filesystem::path schemaPath = schemaFileName;
                if (std::filesystem::exists(schemaPath)) {
                    std::ifstream file(schemaPath);
                    if (file.is_open()) {
                        nlohmann::json schemaJson;
                        file >> schemaJson;
                        file.close();

                        // Create validator
                        validator_ = std::make_unique<nlohmann::json_schema::json_validator>();
                        validator_->set_root_schema(schemaJson);

                        logger_->trace(
                            std::format("{}() - JSONSchemaValidator: Json Schema is successfully loaded. Size: {} bytes",
                                __FUNCTION__, schemaJson.dump().size()));

                        schemaLoaded_ = true;
                    }
                }
                else {
                    logger_->warn(std::format("{}() - JSONSchemaValidator: Schema file not found: {}", __FUNCTION__, schemaFileName));
                }
            }
            catch (const std::exception& ex) {
                logger_->warn(std::format("{}() - JSONSchemaValidator: Exception caught loading schema. {}", __FUNCTION__, ex.what()));
                schemaLoaded_ = false;
            }

            co_return;
        }

        std::optional<std::string> Validate(const std::string& command) override
        {
            if (!schemaLoaded_ || !validator_) {
                return "Schema not loaded";
            }

            try {
                nlohmann::json commandJson = nlohmann::json::parse(command);

                try {
                    validator_->validate(commandJson);
                    logger_->trace(std::format("{}() - JSONSchemaValidator: Json Schema validation result. Valid: true", __FUNCTION__));
                    return std::nullopt; // Success
                }
                catch (const std::exception& ex) {
                    std::string error = std::format("Validation failed: {}", ex.what());
                    logger_->warn(std::format("{}() - JSONSchemaValidator: Json Schema validation result. Valid: false - {}", __FUNCTION__, error));
                    return error;
                }
            }
            catch (const nlohmann::json::parse_error& ex) {
                std::string error = std::format("JSON parse error: {}", ex.what());
                logger_->warn(std::format("{}() - JSONSchemaValidator err: {}", __FUNCTION__, error));
                return error;
            }
            catch (const std::exception& ex) {
                std::string error = std::format("{}() - JSONSchemaValidator: Exception in Validate: {}", __FUNCTION__, ex.what());
                logger_->warn(std::format("{}() - JSONSchemaValidator err: {}", __FUNCTION__, error));
                return error;
            }
        }

        bool IsSchemaLoaded() const override
        {
            return schemaLoaded_;
        }

        void SetSchemaLoaded(bool loaded) override
        {
            schemaLoaded_ = loaded;
        }

    private:
        std::shared_ptr<ILogger> logger_;
        bool schemaLoaded_;
        std::unique_ptr<nlohmann::json_schema::json_validator> validator_ = nullptr;
    };
}