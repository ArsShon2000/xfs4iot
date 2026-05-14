#pragma once

#include "CashUnit.hpp"

namespace XFS4IoTFramework::CashManagement
{
    //using namespace XFS4IoTFramework::Storage;

    /// <summary>
    /// Cash Management Present Status
    /// </summary>
    class CashManagementPresentStatus
    {
    public:
        enum class PresentStateEnum
        {
            Presented,
            NotPresented,
            Unknown
        };

        enum class AdditionalBunchesEnum
        {
            None,
            OneMore,
            Unknown
        };

        /// <summary>
        /// Default constructor
        /// </summary>
        CashManagementPresentStatus()
            : presentState(PresentStateEnum::Unknown)
            , additionalBunches(AdditionalBunchesEnum::Unknown)
            , bunchesRemaining(0)
            , returnedItems()
            , totalReturnedItems()
            , remainingItems()
        {
        }

        /// <summary>
        /// Constructor with all parameters
        /// </summary>
        CashManagementPresentStatus(
            PresentStateEnum presentState,
            AdditionalBunchesEnum additionalBunches,
            int bunchesRemaining,
            const XFS4IoTFramework::Storage::StorageCashCountClass& returnedItems,
            const XFS4IoTFramework::Storage::StorageCashCountClass& totalReturnedItems,
            const XFS4IoTFramework::Storage::StorageCashCountClass& remainingItems)
            : presentState(presentState)
            , additionalBunches(additionalBunches)
            , bunchesRemaining(bunchesRemaining)
            , returnedItems(returnedItems)
            , totalReturnedItems(totalReturnedItems)
            , remainingItems(remainingItems)
        {
        }


        /// <summary>
        /// Convenience constructor: accept optional additionalBunches (matches calls passing std::nullopt)
        /// and default-initialise the StorageCashCountClass members.
        /// </summary>
        CashManagementPresentStatus(
            PresentStateEnum presentState,
            std::optional<AdditionalBunchesEnum> additionalBunches,
            int bunchesRemaining)
            : presentState(presentState)
            , additionalBunches(additionalBunches.has_value() ? additionalBunches.value() : AdditionalBunchesEnum::Unknown)
            , bunchesRemaining(bunchesRemaining)
            , returnedItems()
            , totalReturnedItems()
            , remainingItems()
        {
        }

        /// <summary>
        /// Copy constructor
        /// </summary>
        CashManagementPresentStatus(const CashManagementPresentStatus& other)
            : presentState(other.presentState)
            , additionalBunches(other.additionalBunches)
            , bunchesRemaining(other.bunchesRemaining)
            , returnedItems(other.returnedItems)
            , totalReturnedItems(other.totalReturnedItems)
            , remainingItems(other.remainingItems)
        {
        }

        /// <summary>
        /// Supplies the status of the items that were to be presented by the most recent
        /// attempt to present or return items to the customer. The following values are possible:
        /// * Presented - The items were presented. This status is set as soon as the customer has access.
        /// * NotPresented - The customer has not had access to the items.
        /// * Unknown - It is not known if the customer had access to the items.
        /// </summary>
        PresentStateEnum GetPresentState() const { return presentState; }
        void SetPresentState(PresentStateEnum state) { presentState = state; }

        /// <summary>
        /// Specifies whether or not additional bunches of items are remaining to be presented
        /// as a result of the most recent operation. The following values are possible:
        /// * None - No additional bunches remain.
        /// * OneMore - At least one additional bunch remains.
        /// * Unknown - It is unknown whether additional bunches remain.
        /// </summary>
        AdditionalBunchesEnum GetAdditionalBunches() const { return additionalBunches; }
        void SetAdditionalBunches(AdditionalBunchesEnum bunches) { additionalBunches = bunches; }

        /// <summary>
        /// If AdditionalBunches is OneMore, specifies the number of additional bunches of
        /// items remaining to be presented as a result of the current operation.
        /// </summary>
        int GetBunchesRemaining() const { return bunchesRemaining; }
        void SetBunchesRemaining(int remaining) { bunchesRemaining = remaining; }

        /// <summary>
        /// A list of counts of banknotes which have been moved to the output position as
        /// a result of the most recent operation.
        /// </summary>
        const XFS4IoTFramework::Storage::StorageCashCountClass& GetReturnedItems() const { return returnedItems; }
        void SetReturnedItems(const XFS4IoTFramework::Storage::StorageCashCountClass& items) { returnedItems = items; }

        /// <summary>
        /// Cumulative counts of banknotes which have been moved to the output position.
        /// This value will be reset when a CashAcceptor.CashInStart, CashAcceptor.CashIn,
        /// CashAcceptor.CashInEnd, CashManagement.Retract, CashManagement.Reset or 
        /// CashAcceptor.CashInRollback command is executed.
        /// </summary>
        const XFS4IoTFramework::Storage::StorageCashCountClass& GetTotalReturnedItems() const { return totalReturnedItems; }
        void SetTotalReturnedItems(const XFS4IoTFramework::Storage::StorageCashCountClass& items) { totalReturnedItems = items; }

        /// <summary>
        /// Counts of banknotes on the intermediate stacker or transport which have not been
        /// yet moved to the output position.
        /// </summary>
        const XFS4IoTFramework::Storage::StorageCashCountClass& GetRemainingItems() const { return remainingItems; }
        void SetRemainingItems(const XFS4IoTFramework::Storage::StorageCashCountClass& items) { remainingItems = items; }

    private:
        PresentStateEnum presentState;
        AdditionalBunchesEnum additionalBunches;
        int bunchesRemaining;
        XFS4IoTFramework::Storage::StorageCashCountClass returnedItems;
        XFS4IoTFramework::Storage::StorageCashCountClass totalReturnedItems;
        XFS4IoTFramework::Storage::StorageCashCountClass remainingItems;
    };

    /// <summary>
    /// Helper functions for enum to string conversion
    /// </summary>
    inline std::string ToString(CashManagementPresentStatus::PresentStateEnum state)
    {
        switch (state) {
        case CashManagementPresentStatus::PresentStateEnum::Presented:    return "presented";
        case CashManagementPresentStatus::PresentStateEnum::NotPresented: return "notPresented";
        case CashManagementPresentStatus::PresentStateEnum::Unknown:      return "unknown";
        default:                                                         return "unknown";
        }
    }

    inline std::string ToString(CashManagementPresentStatus::AdditionalBunchesEnum bunches)
    {
        switch (bunches) {
        case CashManagementPresentStatus::AdditionalBunchesEnum::None:    return "none";
        case CashManagementPresentStatus::AdditionalBunchesEnum::OneMore: return "oneMore";
        case CashManagementPresentStatus::AdditionalBunchesEnum::Unknown: return "unknown";
        default:                                                          return "unknown";
        }
    }

    inline CashManagementPresentStatus::PresentStateEnum PresentStateFromString(const std::string& value)
    {
        if (value == "presented") {
            return CashManagementPresentStatus::PresentStateEnum::Presented;
        }
        if (value == "notPresented") {
            return CashManagementPresentStatus::PresentStateEnum::NotPresented;
        }
        return CashManagementPresentStatus::PresentStateEnum::Unknown;
    }

    inline CashManagementPresentStatus::AdditionalBunchesEnum AdditionalBunchesFromString(const std::string& value)
    {
        if (value == "none") {
            return CashManagementPresentStatus::AdditionalBunchesEnum::None;
        }
        if (value == "oneMore") {
            return CashManagementPresentStatus::AdditionalBunchesEnum::OneMore;
        }
        return CashManagementPresentStatus::AdditionalBunchesEnum::Unknown;
    }

    inline void to_json(nlohmann::json& j, const CashManagementPresentStatus& value)
    {
        j = nlohmann::json{
            {"presentState", ToString(value.GetPresentState())},
            {"additionalBunches", ToString(value.GetAdditionalBunches())},
            {"bunchesRemaining", value.GetBunchesRemaining()},
            {"returnedItems", value.GetReturnedItems()},
            {"totalReturnedItems", value.GetTotalReturnedItems()},
            {"remainingItems", value.GetRemainingItems()}
        };
    }

    inline void from_json(const nlohmann::json& j, CashManagementPresentStatus& value)
    {
        std::string presentState = "unknown";
        std::string additionalBunches = "unknown";
        int bunchesRemaining = 0;

        if (j.contains("presentState") && !j.at("presentState").is_null()) {
            j.at("presentState").get_to(presentState);
        }

        if (j.contains("additionalBunches") && !j.at("additionalBunches").is_null()) {
            j.at("additionalBunches").get_to(additionalBunches);
        }

        if (j.contains("bunchesRemaining") && !j.at("bunchesRemaining").is_null()) {
            j.at("bunchesRemaining").get_to(bunchesRemaining);
        }

        value.SetPresentState(PresentStateFromString(presentState));
        value.SetAdditionalBunches(AdditionalBunchesFromString(additionalBunches));
        value.SetBunchesRemaining(bunchesRemaining);

        if (j.contains("returnedItems") && !j.at("returnedItems").is_null()) {
            value.SetReturnedItems(j.at("returnedItems").get<XFS4IoTFramework::Storage::StorageCashCountClass>());
        }

        if (j.contains("totalReturnedItems") && !j.at("totalReturnedItems").is_null()) {
            value.SetTotalReturnedItems(j.at("totalReturnedItems").get<XFS4IoTFramework::Storage::StorageCashCountClass>());
        }

        if (j.contains("remainingItems") && !j.at("remainingItems").is_null()) {
            value.SetRemainingItems(j.at("remainingItems").get<XFS4IoTFramework::Storage::StorageCashCountClass>());
        }
    }
}