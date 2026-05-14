#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include "./DeviceParameters.hpp"

namespace XFS4IoTFramework::CashManagement
{
	///// <summary>
	///// Note/item quality/fitness level classification
	///// </summary>
	//enum class NoteLevelEnum
	//{
	//    Fit,
	//    Unfit,
	//    Counterfeit,
	//    Suspect,
	//    Inked,
	//    Unknown
	//};

	/// <summary>
	/// Single item classification entry
	/// </summary>
	class ItemClassificationClass final
	{
	public:
		ItemClassificationClass()
			: serialNumber_()
			, currency_()
			, value_(0.0)
			, level_(XFS4IoTFramework::CashManagement::NoteLevelEnum::All)
		{
		}

		ItemClassificationClass(
			std::string serialNumber,
			std::string currency,
			double value,
			XFS4IoTFramework::CashManagement::NoteLevelEnum level)
			: serialNumber_(std::move(serialNumber))
			, currency_(std::move(currency))
			, value_(value)
			, level_(level)
		{
		}

		const std::string& getSerialNumber() const { return serialNumber_; }
		const std::string& getCurrency() const { return currency_; }
		double getValue() const { return value_; }
		XFS4IoTFramework::CashManagement::NoteLevelEnum getLevel() const { return level_; }

		bool operator==(const ItemClassificationClass& other) const
		{
			return serialNumber_ == other.serialNumber_ &&
				currency_ == other.currency_ &&
				value_ == other.value_ &&
				level_ == other.level_;
		}

		bool operator!=(const ItemClassificationClass& other) const
		{
			return !(*this == other);
		}

	private:
		std::string serialNumber_;
		std::string currency_;
		double value_;
		XFS4IoTFramework::CashManagement::NoteLevelEnum level_;
	};



	inline void to_json(nlohmann::json& j, const ItemClassificationClass& value)
	{
		j = nlohmann::json{
			{"serialNumber", value.getSerialNumber()},
			{"currency", value.getCurrency()},
			{"value", value.getValue()},
			{"level", ToString(value.getLevel())}
		};
	}

	inline void from_json(const nlohmann::json& j, ItemClassificationClass& value)
	{
		std::string serialNumber;
		std::string currency;
		double amount = 0.0;
		std::string levelStr = "unknown";

		if (j.contains("serialNumber") && !j.at("serialNumber").is_null()) {
			j.at("serialNumber").get_to(serialNumber);
		}

		if (j.contains("currency") && !j.at("currency").is_null()) {
			j.at("currency").get_to(currency);
		}

		if (j.contains("value") && !j.at("value").is_null()) {
			j.at("value").get_to(amount);
		}

		if (j.contains("level") && !j.at("level").is_null()) {
			j.at("level").get_to(levelStr);
		}

		value = ItemClassificationClass(
			std::move(serialNumber),
			std::move(currency),
			amount,
			NoteLevelEnumFromString(levelStr)
		);
	}

	/// <summary>
	/// Container for item classification list
	/// </summary>
	class ItemClassificationListClass final
	{
	public:
		/// <summary>
		/// Default constructor
		/// </summary>
		ItemClassificationListClass()
			: version_()
			, itemClassifications_()
		{
		}

		/// <summary>
		/// Constructor with version
		/// </summary>
		explicit ItemClassificationListClass(std::string version)
			: version_(std::move(version))
			, itemClassifications_()
		{
		}

		/// <summary>
		/// The version number of the classification
		/// </summary>
		const std::string& getVersion() const { return version_; }
		void setVersion(std::string version) { version_ = std::move(version); }

		/// <summary>
		/// The list of elements for the item classification
		/// </summary>
		const std::vector<std::shared_ptr<ItemClassificationClass>>& getItemClassifications() const
		{
			return itemClassifications_;
		}

		std::vector<std::shared_ptr<ItemClassificationClass>>& getItemClassifications()
		{
			return itemClassifications_;
		}

		/// <summary>
		/// Add item classification
		/// </summary>
		void addItemClassification(std::shared_ptr<ItemClassificationClass> item)
		{
			itemClassifications_.push_back(std::move(item));
		}

		/// <summary>
		/// Clear all classifications
		/// </summary>
		void clear()
		{
			itemClassifications_.clear();
		}

		/// <summary>
		/// Get number of classifications
		/// </summary>
		size_t size() const
		{
			return itemClassifications_.size();
		}

		/// <summary>
		/// Check if list is empty
		/// </summary>
		bool empty() const
		{
			return itemClassifications_.empty();
		}

		/// <summary>
		/// Find classification by serial number
		/// </summary>
		std::shared_ptr<ItemClassificationClass> findBySerialNumber(const std::string& serialNumber) const
		{
			for (const auto& item : itemClassifications_)
			{
				if (item->getSerialNumber() == serialNumber)
				{
					return item;
				}
			}
			return nullptr;
		}

		/// <summary>
		/// Find all classifications for a specific currency
		/// </summary>
		std::vector<std::shared_ptr<ItemClassificationClass>> findByCurrency(const std::string& currency) const
		{
			std::vector<std::shared_ptr<ItemClassificationClass>> result;
			for (const auto& item : itemClassifications_)
			{
				if (item->getCurrency() == currency)
				{
					result.push_back(item);
				}
			}
			return result;
		}

		/// <summary>
		/// Find all classifications with a specific level
		/// </summary>
		std::vector<std::shared_ptr<ItemClassificationClass>> findByLevel(NoteLevelEnum level) const
		{
			std::vector<std::shared_ptr<ItemClassificationClass>> result;
			for (const auto& item : itemClassifications_)
			{
				if (item->getLevel() == level)
				{
					result.push_back(item);
				}
			}
			return result;
		}

	private:
		std::string version_;
		std::vector<std::shared_ptr<ItemClassificationClass>> itemClassifications_;
	};
	inline void to_json(nlohmann::json& j, const ItemClassificationListClass& value)
	{
		nlohmann::json items = nlohmann::json::array();

		for (const auto& item : value.getItemClassifications())
		{
			if (item) {
				items.push_back(*item);
			}
			else {
				items.push_back(nullptr);
			}
		}

		j = nlohmann::json{
			{"version", value.getVersion()},
			{"itemClassifications", std::move(items)}
		};
	}

	inline void from_json(const nlohmann::json& j, ItemClassificationListClass& value)
	{
		value.clear();

		if (j.contains("version") && !j.at("version").is_null()) {
			value.setVersion(j.at("version").get<std::string>());
		}
		else {
			value.setVersion({});
		}

		if (j.contains("itemClassifications") && j.at("itemClassifications").is_array())
		{
			for (const auto& itemJson : j.at("itemClassifications"))
			{
				if (itemJson.is_null()) {
					value.addItemClassification(nullptr);
				}
				else {
					value.addItemClassification(
						std::make_shared<ItemClassificationClass>(
							itemJson.get<ItemClassificationClass>()
						)
					);
				}
			}
		}
	}

	//// Helper functions for NoteLevelEnum
	//inline std::string noteLevelEnumToString(XFS4IoTFramework::CashManagement::NoteLevelEnum level)
	//{
	//    switch (level)
	//    {
	//    case XFS4IoTFramework::CashManagement::NoteLevelEnum::Fit: return "fit";
	//    case XFS4IoTFramework::CashManagement::NoteLevelEnum::Unfit: return "unfit";
	//    case XFS4IoTFramework::CashManagement::NoteLevelEnum::Counterfeit: return "counterfeit";
	//    case XFS4IoTFramework::CashManagement::NoteLevelEnum::Suspect: return "suspect";
	//    case XFS4IoTFramework::CashManagement::NoteLevelEnum::Inked: return "inked";
	//    case XFS4IoTFramework::CashManagement::NoteLevelEnum::All: return "All";
	//    case XFS4IoTFramework::CashManagement::NoteLevelEnum::Unrecognized: return "Unrecognized";
	//    default:
	//        throw std::invalid_argument("Unknown NoteLevelEnum value");
	//    }
	//}

	//inline XFS4IoTFramework::CashManagement::NoteLevelEnum stringToNoteLevelEnum(const std::string& str)
	//{
	//    if (str == "fit") return NoteLevelEnum::Fit;
	//    if (str == "unfit") return NoteLevelEnum::Unfit;
	//    if (str == "counterfeit") return NoteLevelEnum::Counterfeit;
	//    if (str == "suspect") return NoteLevelEnum::Suspect;
	//    if (str == "inked") return NoteLevelEnum::Inked;
	//    if (str == "All") return NoteLevelEnum::All;
	//    if (str == "Unrecognized") return NoteLevelEnum::Unrecognized;

	//    throw std::invalid_argument("Unknown NoteLevelEnum string: " + str);
	//}
}