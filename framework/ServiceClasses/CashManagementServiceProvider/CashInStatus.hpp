//#pragma once
//
//#include "../StorageServiceProvider/CashUnit.hpp"
//
//namespace XFS4IoTFramework::CashManagement
//{
//    //using namespace XFS4IoTFramework::Storage;
//
//    /// <summary>
//    /// Cash-in status class
//    /// </summary>
//    class CashInStatusClass
//    {
//    public:
//        enum class StatusEnum
//        {
//            Ok,
//            Rollback,
//            Active,
//            Retract,
//            Unknown,
//            Reset,
//        };
//
//        /// <summary>
//        /// Default constructor
//        /// </summary>
//        CashInStatusClass()
//            : status(StatusEnum::Unknown)
//            , numOfRefusedItems(0)
//            , cashCounts()
//        {
//        }
//
//        /// <summary>
//        /// Status of the currently active or most recently ended cash-in transaction. 
//        /// The following values are possible:
//        /// 
//        /// * Ok - The cash-in transaction is complete and has ended with CashAcceptor.CashInEnd.
//        /// * Rollback - The cash-in transaction ended with CashAcceptor.CashInRollback.
//        /// * Active - There is a cash-in transaction active.
//        /// * Retract - The cash-in transaction ended with CashManagement.Retract.
//        /// * Unknown - The state of the cash-in transaction is unknown. This status is also set 
//        ///   if the ItemCounts details are not known or are not reliable.
//        /// * Reset - The cash-in transaction ended with CashManagement.Reset.
//        /// </summary>
//        StatusEnum GetStatus() const { return status; }
//        void SetStatus(StatusEnum value) { status = value; }
//
//        /// <summary>
//        /// Specifies the number of items refused during the currently active or most recently 
//        /// ended cash-in transaction period.
//        /// </summary>
//        int GetNumOfRefusedItems() const { return numOfRefusedItems; }
//        void SetNumOfRefusedItems(int value) { numOfRefusedItems = value; }
//
//        /// <summary>
//        /// List of banknote types that were inserted, identified, and accepted during the 
//        /// currently active or most recently ended cash-in transaction period. 
//        /// If items have been rolled back (status is rollback) they will be included in this list.
//        /// Includes any identified notes.
//        /// </summary>
//        const XFS4IoTFramework::Storage::StorageCashCountClass& GetCashCounts() const { return cashCounts; }
//        void SetCashCounts(const XFS4IoTFramework::Storage::StorageCashCountClass& value) { cashCounts = value; }
//
//    private:
//        StatusEnum status;
//        int numOfRefusedItems;
//        XFS4IoTFramework::Storage::StorageCashCountClass cashCounts;
//    };
//
//    /// <summary>
//    /// Helper function to convert StatusEnum to string
//    /// </summary>
//    inline std::string ToString(CashInStatusClass::StatusEnum status)
//    {
//        switch (status) {
//        case CashInStatusClass::StatusEnum::Ok:
//            return "ok";
//        case CashInStatusClass::StatusEnum::Rollback:
//            return "rollback";
//        case CashInStatusClass::StatusEnum::Active:
//            return "active";
//        case CashInStatusClass::StatusEnum::Retract:
//            return "retract";
//        case CashInStatusClass::StatusEnum::Unknown:
//            return "unknown";
//        case CashInStatusClass::StatusEnum::Reset:
//            return "reset";
//        default:
//            return "unknown";
//        }
//    }
//}