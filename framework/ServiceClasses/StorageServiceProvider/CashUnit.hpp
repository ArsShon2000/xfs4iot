#pragma once

#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <map>
#include "StorageChangedBase.hpp"
#include "UnitStorageBase.hpp"
#include "../CommonServiceProvider/StatusBase.hpp"
#include "../../core/Events/PropertyChangedEventArgs.hpp"

//namespace XFS4IoT::CashManagement
//{
//    // Forward declaration for CopyTo method
//    class StorageCashCountsClass;
//    class StorageCashCountClass;
//}

namespace XFS4IoTFramework::Storage
{
    class StorageCashOutCountClass;
    class StorageCashInCountClass;
    class StorageCashCountsClass;
    class StorageCashCountClass;

//    /// <summary>
///// Capabilities of the cash unit
///// </summary>
//    class CashCapabilitiesClass
//    {
//    public:
//        enum class TypesEnum
//        {
//            CashIn = 1 << 0,
//            CashOut = 1 << 1,
//            Replenishment = 1 << 2,
//            CashInRetract = 1 << 3,
//            CashOutRetract = 1 << 4,
//            Reject = 1 << 5
//        };
//
//        enum class ItemsEnum
//        {
//            Fit = 1 << 0,
//            Unfit = 1 << 1,
//            Unrecognized = 1 << 2,
//            Counterfeit = 1 << 3,
//            Suspect = 1 << 4,
//            Inked = 1 << 5,
//            Coupon = 1 << 6,
//            Document = 1 << 7
//        };
//
//        CashCapabilitiesClass(
//            TypesEnum types,
//            ItemsEnum items,
//            bool hardwareSensors,
//            int retractAreas,
//            std::vector<std::string> banknoteItems)
//            : types_(types)
//            , items_(items)
//            , hardwareSensors_(hardwareSensors)
//            , retractAreas_(retractAreas)
//            , retractThresholds_(false)
//            , banknoteItems_(std::move(banknoteItems))
//        {
//        }
//
//        /// <summary>
//        /// The types of operation the unit is capable to perform. This is a combination of one or 
//        /// more operations
//        /// </summary>
//        TypesEnum GetTypes() const { return types_; }
//        void SetTypes(TypesEnum types) { types_ = types; }
//
//        /// <summary>
//        /// The types of cash media the unit is capable of storing to store. This is a combination of
//        /// one or more item types. May only be modified in an exchange state if applicable.
//        /// </summary>
//        ItemsEnum GetItems() const { return items_; }
//        void SetItems(ItemsEnum items) { items_ = items; }
//
//        /// <summary>
//        /// The storage unit has hardware sensors that can detect threshold states.
//        /// </summary>
//        bool GetHardwareSensors() const { return hardwareSensors_; }
//        void SetHardwareSensors(bool hardwareSensors) { hardwareSensors_ = hardwareSensors; }
//
//        /// <summary>
//        /// If items can be retracted into this storage unit, this is the number of areas within the storage unit which 
//        /// allow physical separation of different bunches. If there is no physical separation of retracted bunches
//        /// within this storage unit, this value is 1.
//        /// </summary>
//        int GetRetractAreas() const { return retractAreas_; }
//        void SetRetractAreas(int retractAreas) { retractAreas_ = retractAreas; }
//
//        /// <summary>
//        /// If true, indicates that retract capacity is based on counts.
//        /// If false, indicates that retract capacity is based on the number of commands which resulted in items
//        /// being retracted into the storage unit.
//        /// </summary>
//        bool GetRetractThresholds() const { return retractThresholds_; }
//        void SetRetractThresholds(bool retractThresholds) { retractThresholds_ = retractThresholds; }
//
//        /// <summary>
//        /// Lists the cash items which the storage unit is physically capable of handling.
//        /// </summary>
//        const std::vector<std::string>& GetBanknoteItems() const { return banknoteItems_; }
//        void SetBanknoteItems(const std::vector<std::string>& banknoteItems) { banknoteItems_ = banknoteItems; }
//
//    private:
//        TypesEnum types_;
//        ItemsEnum items_;
//        bool hardwareSensors_;
//        int retractAreas_;
//        bool retractThresholds_;
//        std::vector<std::string> banknoteItems_;
//    };
//
//    // Bitwise operators for enum flags
//    inline CashCapabilitiesClass::TypesEnum operator|(CashCapabilitiesClass::TypesEnum a, CashCapabilitiesClass::TypesEnum b)
//    {
//        return static_cast<CashCapabilitiesClass::TypesEnum>(static_cast<int>(a) | static_cast<int>(b));
//    }
//
//    inline CashCapabilitiesClass::TypesEnum operator&(CashCapabilitiesClass::TypesEnum a, CashCapabilitiesClass::TypesEnum b)
//    {
//        return static_cast<CashCapabilitiesClass::TypesEnum>(static_cast<int>(a) & static_cast<int>(b));
//    }
//
//    inline CashCapabilitiesClass::ItemsEnum operator|(CashCapabilitiesClass::ItemsEnum a, CashCapabilitiesClass::ItemsEnum b)
//    {
//        return static_cast<CashCapabilitiesClass::ItemsEnum>(static_cast<int>(a) | static_cast<int>(b));
//    }
//
//    inline CashCapabilitiesClass::ItemsEnum operator&(CashCapabilitiesClass::ItemsEnum a, CashCapabilitiesClass::ItemsEnum b)
//    {
//        return static_cast<CashCapabilitiesClass::ItemsEnum>(static_cast<int>(a) & static_cast<int>(b));
//    }

    /// <summary>
   /// Details of recognised banknote count
   /// </summary>
    class CashItemCountClass
    {
    public:
        /// <summary>
        /// Default constructor - initializes all counts to 0
        /// </summary>
        CashItemCountClass()
            : fit_(0)
            , unfit_(0)
            , suspect_(0)
            , counterfeit_(0)
            , inked_(0)
        {
        }

        /// <summary>
        /// Parameterized constructor
        /// </summary>
        CashItemCountClass(int fit, int unfit, int suspect, int counterfeit, int inked)
            : fit_(fit)
            , unfit_(unfit)
            , suspect_(suspect)
            , counterfeit_(counterfeit)
            , inked_(inked)
        {
        }

        /// <summary>
        /// Copy constructor
        /// </summary>
        CashItemCountClass(const CashItemCountClass& cashItemCount)
            : fit_(cashItemCount.fit_)
            , unfit_(cashItemCount.unfit_)
            , suspect_(cashItemCount.suspect_)
            , counterfeit_(cashItemCount.counterfeit_)
            , inked_(cashItemCount.inked_)
        {
        }

        /// <summary>
        /// Copy assignment operator
        /// </summary>
        CashItemCountClass& operator=(const CashItemCountClass& cashItemCount)
        {
            if (this != &cashItemCount)
            {
                fit_ = cashItemCount.fit_;
                unfit_ = cashItemCount.unfit_;
                suspect_ = cashItemCount.suspect_;
                counterfeit_ = cashItemCount.counterfeit_;
                inked_ = cashItemCount.inked_;
            }
            return *this;
        }

        CashItemCountClass& operator+=(const CashItemCountClass& other)
        {
            fit_ += other.fit_;
            unfit_ += other.unfit_;
            suspect_ += other.suspect_;
            counterfeit_ += other.counterfeit_;
            inked_ += other.inked_;
            return *this;
        }

        /// <summary>
        /// Count of genuine cash items which are fit for recycling.
        /// </summary>
        int GetFit() const { return fit_; }
        void SetFit(int fit) { fit_ = fit; }

        /// <summary>
        /// Count of genuine cash items which are unfit for recycling.
        /// </summary>
        int GetUnfit() const { return unfit_; }
        void SetUnfit(int unfit) { unfit_ = unfit; }

        /// <summary>
        /// Count of suspected counterfeit cash items.
        /// </summary>
        int GetSuspect() const { return suspect_; }
        void SetSuspect(int suspect) { suspect_ = suspect; }

        /// <summary>
        /// Count of counterfeit cash items.
        /// </summary>
        int GetCounterfeit() const { return counterfeit_; }
        void SetCounterfeit(int counterfeit) { counterfeit_ = counterfeit; }

        /// <summary>
        /// Count of cash items which have been identified as ink stained.
        /// </summary>
        int GetInked() const { return inked_; }
        void SetInked(int inked) { inked_ = inked; }

        /// <summary>
        /// Equality operator
        /// </summary>
        bool operator==(const CashItemCountClass& other) const
        {
            return fit_ == other.fit_ &&
                unfit_ == other.unfit_ &&
                suspect_ == other.suspect_ &&
                counterfeit_ == other.counterfeit_ &&
                inked_ == other.inked_;
        }

        bool operator!=(const CashItemCountClass& other) const
        {
            return !(*this == other);
        }

    private:
        int fit_;
        int unfit_;
        int suspect_;
        int counterfeit_;
        int inked_;
    };

	//------------------------------------------------------------------------

    /// <summary>
	/// Эта структура содержит информацию о количестве распознанных 
    /// и нераспознанных предметов, обработанных интерфейсом наличных.
    /// </summary>
    class StorageCashCountClass
    {
    public:
        /// <summary>
        /// Default constructor
        /// </summary>
        StorageCashCountClass()
            : unrecognized_(0)
            , itemCounts_()
        {
        }

        /// <summary>
        /// Parameterized constructor
        /// </summary>
        StorageCashCountClass(
            int unrecognized,
            const std::map<std::string, CashItemCountClass>& itemCounts)
            : unrecognized_(unrecognized)
            , itemCounts_()
        {
            for (const auto& [key, value] : itemCounts)
            {
                itemCounts_[key] = CashItemCountClass(value);
            }
        }

        /// <summary>
        /// Copy constructor
        /// </summary>
        StorageCashCountClass(const StorageCashCountClass& storageCashCount)
            : unrecognized_(storageCashCount.unrecognized_)
            , itemCounts_()
        {
            for (const auto& [key, value] : storageCashCount.itemCounts_)
            {
                itemCounts_[key] = CashItemCountClass(value);
            }
        }

        /// <summary>
        /// Copy assignment operator
        /// </summary>
        StorageCashCountClass& operator=(const StorageCashCountClass& storageCashCount)
        {
            if (this != &storageCashCount)
            {
                unrecognized_ = storageCashCount.unrecognized_;
                itemCounts_.clear();
                for (const auto& [key, value] : storageCashCount.itemCounts_)
                {
                    itemCounts_[key] = CashItemCountClass(value);
                }
            }
            return *this;
        }

        StorageCashCountClass& operator+=(const StorageCashCountClass& other)
        {
            unrecognized_ += other.unrecognized_;

            for (const auto& [itemKey, itemValue] : other.itemCounts_)
            {
                itemCounts_[itemKey] += itemValue;
            }

            return *this;
        }

        /// <summary>
        /// Count of unrecognized items handled by the cash interface
        /// </summary>
        int GetUnrecognized() const { return unrecognized_; }
        void SetUnrecognized(int unrecognized) { unrecognized_ = unrecognized; }

        /// <summary>
        /// Counts of cash items broken down by cash item type and classification
        /// </summary>
         
        std::map<std::string, CashItemCountClass>& GetItemCounts() { return itemCounts_; }
    
        const std::map<std::string, CashItemCountClass>& GetItemCounts() const { return itemCounts_; }
        void SetItemCounts(const std::map<std::string, CashItemCountClass>& itemCounts)
        {
            itemCounts_.clear();
            for (const auto& [key, value] : itemCounts)
            {
                itemCounts_[key] = CashItemCountClass(value);
            }
        }

        /// <summary>
        /// Copy structure to the message class generated automatically.
        /// </summary>
        //std::shared_ptr<StorageCashCountsClass> CopyTo() const;

        /// <summary>
        /// Return total counts of items
        /// </summary>
        int GetTotal() const
        {
            int total = unrecognized_;
            for (const auto& [key, item] : itemCounts_)
            {
                total += item.GetFit();
                total += item.GetUnfit();
                total += item.GetSuspect();
                total += item.GetCounterfeit();
                total += item.GetInked();
            }
            return total;
        }

        /// <summary>
        /// Equality operator
        /// </summary>
        bool operator==(const StorageCashCountClass& other) const
        {
            if (unrecognized_ != other.unrecognized_)
                return false;

            if (itemCounts_.size() != other.itemCounts_.size())
                return false;

            for (const auto& [key, value] : itemCounts_)
            {
                auto it = other.itemCounts_.find(key);
                if (it == other.itemCounts_.end() || !(value == it->second))
                    return false;
            }

            return true;
        }

        bool operator!=(const StorageCashCountClass& other) const
        {
            return !(*this == other);
        }

    private:
        int unrecognized_;
        //  
        std::map<std::string, CashItemCountClass> itemCounts_;
    };

    inline void to_json(nlohmann::json& j, const CashItemCountClass& p)
    {
        j = nlohmann::json::object();

        j["fit"] = p.GetFit();
        j["unfit"] = p.GetUnfit();
        j["suspect"] = p.GetSuspect();
        j["counterfeit"] = p.GetCounterfeit();
        j["inked"] = p.GetInked();
    }

    inline void to_json(nlohmann::json& j, const StorageCashCountClass& p)
    {
        j = nlohmann::json::object();

        j["unrecognized"] = p.GetUnrecognized();

        if (!p.GetItemCounts().empty())
        {
            nlohmann::json itemCounts = nlohmann::json::object();

            for (const auto& [key, value] : p.GetItemCounts())
            {
                itemCounts[key] = value;
            }

            j["itemCounts"] = std::move(itemCounts);
        }
    }

    inline std::string toString(const CashItemCountClass& p)
    {
        nlohmann::json j = p;
        return j.dump();
    }

    inline std::string toString(const StorageCashCountClass& p)
    {
        nlohmann::json j = p;
        return j.dump();
    }

    inline void from_json(const nlohmann::json& j, CashItemCountClass& p)
    {
        if (j.contains("fit") && !j.at("fit").is_null())
        {
            p.SetFit(j.at("fit").get<int>());
        }

        if (j.contains("unfit") && !j.at("unfit").is_null())
        {
            p.SetUnfit(j.at("unfit").get<int>());
        }

        if (j.contains("suspect") && !j.at("suspect").is_null())
        {
            p.SetSuspect(j.at("suspect").get<int>());
        }

        if (j.contains("counterfeit") && !j.at("counterfeit").is_null())
        {
            p.SetCounterfeit(j.at("counterfeit").get<int>());
        }

        if (j.contains("inked") && !j.at("inked").is_null())
        {
            p.SetInked(j.at("inked").get<int>());
        }
    }

    inline void from_json(const nlohmann::json& j, StorageCashCountClass& p)
    {
        if (j.contains("unrecognized") && !j.at("unrecognized").is_null())
        {
            int count = 0;
            for (const auto& [key, value] : j.at("unrecognized").items())
            {
                count += value;
            }
            p.SetUnrecognized(count);
        }

        if (j.contains("itemCounts") && !j.at("itemCounts").is_null())
        {
            std::map<std::string, CashItemCountClass> itemCounts;

            for (const auto& [key, value] : j.at("itemCounts").items())
            {
                itemCounts.emplace(
                    key,
                    value.get<CashItemCountClass>());
            }

            p.SetItemCounts(itemCounts);
        }
    }
	//------------------------------------------------------------------------


    /// <summary>
	/// Этот класс представляет собой структуру, которая может быть использована для представления количества предметов в единице хранения,
    /// включая детали о том, сколько предметов было перемещено из единицы хранения (StorageCashOutCount) 
    /// и сколько предметов было перемещено в единицу хранения (StorageCashInCount) с момента последнего пополнения. 
    /// Это может быть полезно для устройств, которые могут предоставлять информацию о перемещении предметов, 
    /// а также общее количество предметов в единице хранения. Если устройство не предоставляет информацию о перемещении предметов, 
    /// то StorageCashOutCount и StorageCashInCount могут быть null, и только общее количество будет заполнено.
    /// </summary>
    class CashUnitCountClass
    {
    public:
        /// <summary>
        /// Default constructor
        /// </summary>
        CashUnitCountClass()
            : storageCashOutCount_(nullptr)
            , storageCashInCount_(nullptr)
            , count_(0)
        {
        }

        /// <summary>
        /// Parameterized constructor
        /// </summary>
        CashUnitCountClass(
            std::shared_ptr<StorageCashOutCountClass> storageCashOutCount,
            std::shared_ptr<StorageCashInCountClass> storageCashInCount,
            int count)
            : storageCashOutCount_(storageCashOutCount ? std::make_shared<StorageCashOutCountClass>(*storageCashOutCount) : nullptr)
            , storageCashInCount_(storageCashInCount ? std::make_shared<StorageCashInCountClass>(*storageCashInCount) : nullptr)
            , count_(count)
        {
        }

        /// <summary>
        /// Copy constructor
        /// </summary>
        CashUnitCountClass(const CashUnitCountClass& other)
            : storageCashOutCount_(other.storageCashOutCount_ ? std::make_shared<StorageCashOutCountClass>(*other.storageCashOutCount_) : nullptr)
            , storageCashInCount_(other.storageCashInCount_ ? std::make_shared<StorageCashInCountClass>(*other.storageCashInCount_) : nullptr)
            , count_(other.count_)
        {
        }

        /// <summary>
        /// Copy assignment operator
        /// </summary>
        CashUnitCountClass& operator=(const CashUnitCountClass& other)
        {
            if (this != &other)
            {
                storageCashOutCount_ = other.storageCashOutCount_ ? std::make_shared<StorageCashOutCountClass>(*other.storageCashOutCount_) : nullptr;
                storageCashInCount_ = other.storageCashInCount_ ? std::make_shared<StorageCashInCountClass>(*other.storageCashInCount_) : nullptr;
                count_ = other.count_;
            }
            return *this;
        }

        /// <summary>
        /// The items moved from this storage unit by cash commands to another destination
        /// since the last replenishment of this unit.
        /// </summary>
        std::shared_ptr<StorageCashOutCountClass> GetStorageCashOutCount() const { return storageCashOutCount_; }
        void SetStorageCashOutCount(std::shared_ptr<StorageCashOutCountClass> storageCashOutCount)
        {
            storageCashOutCount_ = storageCashOutCount;
        }

        /// <summary>
        /// List of items inserted in this storage unit by cash commands from another source
        /// since the last replenishment of this unit.
        /// </summary>
        std::shared_ptr<StorageCashInCountClass> GetStorageCashInCount() const { return storageCashInCount_; }
        void SetStorageCashInCount(std::shared_ptr<StorageCashInCountClass> storageCashInCount)
        {
            storageCashInCount_ = storageCashInCount;
        }

        /// <summary>
		/// Общее количество предметов в единице хранения. Это может быть использовано для устройств, 
        /// которые не имеют возможности предоставлять подробные сведения о предметах, 
        /// но могут предоставлять общее количество предметов в единице хранения.
        /// </summary>
        int GetCount() const { return count_; }
        void SetCount(int count) { count_ = count; }

        /// <summary>
        /// Equality operator
        /// </summary>
        bool operator==(const CashUnitCountClass& other) const
        {
            // Compare pointers for null, then deep compare if both non-null
            bool outCountEqual = (storageCashOutCount_ == nullptr && other.storageCashOutCount_ == nullptr) ||
                (storageCashOutCount_ != nullptr && other.storageCashOutCount_ != nullptr &&
                    storageCashOutCount_ == other.storageCashOutCount_);

            bool inCountEqual = (storageCashInCount_ == nullptr && other.storageCashInCount_ == nullptr) ||
                (storageCashInCount_ != nullptr && other.storageCashInCount_ != nullptr &&
                    storageCashInCount_ == other.storageCashInCount_);

            return outCountEqual && inCountEqual && count_ == other.count_;
        }

        bool operator!=(const CashUnitCountClass& other) const
        {
            return !(*this == other);
        }

    private:
		std::shared_ptr<StorageCashOutCountClass> storageCashOutCount_; // этот класс должен быть определен отдельно и содержать информацию о количестве предметов, перемещенных из единицы хранения
		std::shared_ptr<StorageCashInCountClass> storageCashInCount_; // этот класс должен быть определен отдельно и содержать информацию о количестве предметов, перемещенных в единицу хранения
		int count_; // общее количество предметов в единице хранения. 
    };


	//------------------------------------------------------------------------

    /// <summary>
	/// Этот класс представляет собой структуру, которая может быть использована 
    /// для представления количества предметов, перемещенных в единицу хранения.
    /// </summary>
    class StorageCashInCountClass
    {
    public:
        /// <summary>
        /// Default constructor
        /// </summary>
        StorageCashInCountClass()
            : retractOperations_(0)
            , deposited_(std::make_shared<StorageCashCountClass>())
            , retracted_(std::make_shared<StorageCashCountClass>())
            , rejected_(std::make_shared<StorageCashCountClass>())
            , distributed_(std::make_shared<StorageCashCountClass>())
            , transport_(std::make_shared<StorageCashCountClass>())
        {
        }

        /// <summary>
        /// Copy constructor
        /// </summary>
        StorageCashInCountClass(const StorageCashInCountClass& storageCashInCount)
            : retractOperations_(storageCashInCount.retractOperations_)
            , deposited_(storageCashInCount.deposited_ ? std::make_shared<StorageCashCountClass>(*storageCashInCount.deposited_) : std::make_shared<StorageCashCountClass>())
            , retracted_(storageCashInCount.retracted_ ? std::make_shared<StorageCashCountClass>(*storageCashInCount.retracted_) : std::make_shared<StorageCashCountClass>())
            , rejected_(storageCashInCount.rejected_ ? std::make_shared<StorageCashCountClass>(*storageCashInCount.rejected_) : std::make_shared<StorageCashCountClass>())
            , distributed_(storageCashInCount.distributed_ ? std::make_shared<StorageCashCountClass>(*storageCashInCount.distributed_) : std::make_shared<StorageCashCountClass>())
            , transport_(storageCashInCount.transport_ ? std::make_shared<StorageCashCountClass>(*storageCashInCount.transport_) : std::make_shared<StorageCashCountClass>())
        {
        }

        /// <summary>
        /// Copy assignment operator
        /// </summary>
        StorageCashInCountClass& operator=(const StorageCashInCountClass& storageCashInCount)
        {
            if (this != &storageCashInCount)
            {
                retractOperations_ = storageCashInCount.retractOperations_;
                deposited_ = storageCashInCount.deposited_ ? std::make_shared<StorageCashCountClass>(*storageCashInCount.deposited_) : std::make_shared<StorageCashCountClass>();
                retracted_ = storageCashInCount.retracted_ ? std::make_shared<StorageCashCountClass>(*storageCashInCount.retracted_) : std::make_shared<StorageCashCountClass>();
                rejected_ = storageCashInCount.rejected_ ? std::make_shared<StorageCashCountClass>(*storageCashInCount.rejected_) : std::make_shared<StorageCashCountClass>();
                distributed_ = storageCashInCount.distributed_ ? std::make_shared<StorageCashCountClass>(*storageCashInCount.distributed_) : std::make_shared<StorageCashCountClass>();
                transport_ = storageCashInCount.transport_ ? std::make_shared<StorageCashCountClass>(*storageCashInCount.transport_) : std::make_shared<StorageCashCountClass>();
            }
            return *this;
        }

        /// <summary>
        /// Number of cash retract operations which resulted in items entering this storage unit. This can be 
        /// used where devices do not have the capability to count or validate items after presentation.
        /// </summary>
        int GetRetractOperations() const { return retractOperations_; }
        void SetRetractOperations(int retractOperations) { retractOperations_ = retractOperations; }

        /// <summary>
        /// The items deposited in the storage unit during a Cash In transaction.
        /// </summary>
        std::shared_ptr<StorageCashCountClass> GetDeposited() const
        {
            return deposited_ ? deposited_ : std::make_shared<StorageCashCountClass>();
        }
        void SetDeposited(std::shared_ptr<StorageCashCountClass> deposited)
        {
            deposited_ = deposited ? std::make_shared<StorageCashCountClass>(*deposited) : std::make_shared<StorageCashCountClass>();
        }

        /// <summary>
        /// The items deposited in the storage unit after being accessible to a customer. This may be inaccurate 
        /// or not counted if items are not counted or re-validated after presentation, the number of retract
        /// operations is also reported separately in RetractOperations.
        /// </summary>
        std::shared_ptr<StorageCashCountClass> GetRetracted() const
        {
            return retracted_ ? retracted_ : std::make_shared<StorageCashCountClass>();
        }
        void SetRetracted(std::shared_ptr<StorageCashCountClass> retracted)
        {
            retracted_ = retracted ? std::make_shared<StorageCashCountClass>(*retracted) : std::make_shared<StorageCashCountClass>();
        }

        /// <summary>
        /// The items deposited in this storage unit originating from another storage unit but rejected due to being 
        /// invalid. This count may be inaccurate due to the nature of rejected items.
        /// </summary>
        std::shared_ptr<StorageCashCountClass> GetRejected() const
        {
            return rejected_ ? rejected_ : std::make_shared<StorageCashCountClass>();
        }
        void SetRejected(std::shared_ptr<StorageCashCountClass> rejected)
        {
            rejected_ = rejected ? std::make_shared<StorageCashCountClass>(*rejected) : std::make_shared<StorageCashCountClass>();
        }

        /// <summary>
        /// The items deposited in this storage unit originating from another storage unit but not rejected.
        /// </summary>
        std::shared_ptr<StorageCashCountClass> GetDistributed() const
        {
            return distributed_ ? distributed_ : std::make_shared<StorageCashCountClass>();
        }
        void SetDistributed(std::shared_ptr<StorageCashCountClass> distributed)
        {
            distributed_ = distributed ? std::make_shared<StorageCashCountClass>(*distributed) : std::make_shared<StorageCashCountClass>();
        }

        /// <summary>
        /// The items which were intended to be deposited in this storage unit but are not yet deposited. Typical use
        /// case for this property is tracking items after a jam during CashAcceptor.CashInEnd
        /// </summary>
        std::shared_ptr<StorageCashCountClass> GetTransport() const
        {
            return transport_ ? transport_ : std::make_shared<StorageCashCountClass>();
        }
        void SetTransport(std::shared_ptr<StorageCashCountClass> transport)
        {
            transport_ = transport ? std::make_shared<StorageCashCountClass>(*transport) : std::make_shared<StorageCashCountClass>();
        }

        /// <summary>
        /// Equality operator
        /// </summary>
        bool operator==(const StorageCashInCountClass& other) const
        {
            return retractOperations_ == other.retractOperations_ &&
                *deposited_ == *other.deposited_ &&
                *retracted_ == *other.retracted_ &&
                *rejected_ == *other.rejected_ &&
                *distributed_ == *other.distributed_ &&
                *transport_ == *other.transport_;
        }

        bool operator!=(const StorageCashInCountClass& other) const
        {
            return !(*this == other);
        }

    private:
        int retractOperations_;
        std::shared_ptr<StorageCashCountClass> deposited_;
        std::shared_ptr<StorageCashCountClass> retracted_;
        std::shared_ptr<StorageCashCountClass> rejected_;
        std::shared_ptr<StorageCashCountClass> distributed_;
        std::shared_ptr<StorageCashCountClass> transport_;
    };

	//------------------------------------------------------------------------

    /// <summary>
    /// Representing counts moved from the cash unit
    /// </summary>
    class StorageCashOutCountClass
    {
    public:
        /// <summary>
        /// Default constructor
        /// </summary>
        StorageCashOutCountClass()
            : presented_(std::make_shared<StorageCashCountClass>())
            , rejected_(std::make_shared<StorageCashCountClass>())
            , distributed_(std::make_shared<StorageCashCountClass>())
            , unknown_(std::make_shared<StorageCashCountClass>())
            , stacked_(std::make_shared<StorageCashCountClass>())
            , diverted_(std::make_shared<StorageCashCountClass>())
            , transport_(std::make_shared<StorageCashCountClass>())
        {
        }

        /// <summary>
        /// Copy constructor
        /// </summary>
        StorageCashOutCountClass(const StorageCashOutCountClass& storageCashOutCount)
            : presented_(storageCashOutCount.presented_ ? std::make_shared<StorageCashCountClass>(*storageCashOutCount.presented_) : std::make_shared<StorageCashCountClass>())
            , rejected_(storageCashOutCount.rejected_ ? std::make_shared<StorageCashCountClass>(*storageCashOutCount.rejected_) : std::make_shared<StorageCashCountClass>())
            , distributed_(storageCashOutCount.distributed_ ? std::make_shared<StorageCashCountClass>(*storageCashOutCount.distributed_) : std::make_shared<StorageCashCountClass>())
            , unknown_(storageCashOutCount.unknown_ ? std::make_shared<StorageCashCountClass>(*storageCashOutCount.unknown_) : std::make_shared<StorageCashCountClass>())
            , stacked_(storageCashOutCount.stacked_ ? std::make_shared<StorageCashCountClass>(*storageCashOutCount.stacked_) : std::make_shared<StorageCashCountClass>())
            , diverted_(storageCashOutCount.diverted_ ? std::make_shared<StorageCashCountClass>(*storageCashOutCount.diverted_) : std::make_shared<StorageCashCountClass>())
            , transport_(storageCashOutCount.transport_ ? std::make_shared<StorageCashCountClass>(*storageCashOutCount.transport_) : std::make_shared<StorageCashCountClass>())
        {
        }

        /// <summary>
        /// Copy assignment operator
        /// </summary>
        StorageCashOutCountClass& operator=(const StorageCashOutCountClass& storageCashOutCount)
        {
            if (this != &storageCashOutCount)
            {
                presented_ = storageCashOutCount.presented_ ? std::make_shared<StorageCashCountClass>(*storageCashOutCount.presented_) : std::make_shared<StorageCashCountClass>();
                rejected_ = storageCashOutCount.rejected_ ? std::make_shared<StorageCashCountClass>(*storageCashOutCount.rejected_) : std::make_shared<StorageCashCountClass>();
                distributed_ = storageCashOutCount.distributed_ ? std::make_shared<StorageCashCountClass>(*storageCashOutCount.distributed_) : std::make_shared<StorageCashCountClass>();
                unknown_ = storageCashOutCount.unknown_ ? std::make_shared<StorageCashCountClass>(*storageCashOutCount.unknown_) : std::make_shared<StorageCashCountClass>();
                stacked_ = storageCashOutCount.stacked_ ? std::make_shared<StorageCashCountClass>(*storageCashOutCount.stacked_) : std::make_shared<StorageCashCountClass>();
                diverted_ = storageCashOutCount.diverted_ ? std::make_shared<StorageCashCountClass>(*storageCashOutCount.diverted_) : std::make_shared<StorageCashCountClass>();
                transport_ = storageCashOutCount.transport_ ? std::make_shared<StorageCashCountClass>(*storageCashOutCount.transport_) : std::make_shared<StorageCashCountClass>();
            }
            return *this;
        }

        /// <summary>
        /// The items dispensed from this storage unit which are or were customer accessible.
        /// </summary>
        std::shared_ptr<StorageCashCountClass> GetPresented() const
        {
            return presented_ ? presented_ : std::make_shared<StorageCashCountClass>();
        }
        void SetPresented(std::shared_ptr<StorageCashCountClass> presented)
        {
            presented_ = presented ? std::make_shared<StorageCashCountClass>(*presented) : std::make_shared<StorageCashCountClass>();
        }

        /// <summary>
        /// The items dispensed from this storage unit which were invalid and were diverted to a reject storage
        /// unit and were not customer accessible during the operation.
        /// </summary>
        std::shared_ptr<StorageCashCountClass> GetRejected() const
        {
            return rejected_ ? rejected_ : std::make_shared<StorageCashCountClass>();
        }
        void SetRejected(std::shared_ptr<StorageCashCountClass> rejected)
        {
            rejected_ = rejected ? std::make_shared<StorageCashCountClass>(*rejected) : std::make_shared<StorageCashCountClass>();
        }

        /// <summary>
        /// The items dispensed from this storage unit which were moved to a storage unit other than a reject storage unit
        /// and were not customer accessible during the operation.
        /// </summary>
        std::shared_ptr<StorageCashCountClass> GetDistributed() const
        {
            return distributed_ ? distributed_ : std::make_shared<StorageCashCountClass>();
        }
        void SetDistributed(std::shared_ptr<StorageCashCountClass> distributed)
        {
            distributed_ = distributed ? std::make_shared<StorageCashCountClass>(*distributed) : std::make_shared<StorageCashCountClass>();
        }

        /// <summary>
        /// The items dispensed from this storage unit which moved to an unknown position.
        /// </summary>
        std::shared_ptr<StorageCashCountClass> GetUnknown() const
        {
            return unknown_ ? unknown_ : std::make_shared<StorageCashCountClass>();
        }
        void SetUnknown(std::shared_ptr<StorageCashCountClass> unknown)
        {
            unknown_ = unknown ? std::make_shared<StorageCashCountClass>(*unknown) : std::make_shared<StorageCashCountClass>();
        }

        /// <summary>
        /// The items dispensed from this storage unit which are not customer accessible and are currently stacked
        /// awaiting presentation to the customer. This item list can increase and decrease as items are moved around in the device.
        /// </summary>
        std::shared_ptr<StorageCashCountClass> GetStacked() const
        {
            return stacked_ ? stacked_ : std::make_shared<StorageCashCountClass>();
        }
        void SetStacked(std::shared_ptr<StorageCashCountClass> stacked)
        {
            stacked_ = stacked ? std::make_shared<StorageCashCountClass>(*stacked) : std::make_shared<StorageCashCountClass>();
        }

        /// <summary>
        /// The items dispensed from this storage unit which are not customer accessible and were diverted to a
        /// temporary location due to being invalid and have not yet been deposited in a storage unit. This item
        /// list can increase and decrease as items are moved around in the device.
        /// </summary>
        std::shared_ptr<StorageCashCountClass> GetDiverted() const
        {
            return diverted_ ? diverted_ : std::make_shared<StorageCashCountClass>();
        }
        void SetDiverted(std::shared_ptr<StorageCashCountClass> diverted)
        {
            diverted_ = diverted ? std::make_shared<StorageCashCountClass>(*diverted) : std::make_shared<StorageCashCountClass>();
        }

        /// <summary>
        /// The items dispensed from this storage unit which are not customer accessible and which have jammed in
        /// the transport. This item list can increase and decrease as items are moved around in the device.
        /// </summary>
        std::shared_ptr<StorageCashCountClass> GetTransport() const
        {
            return transport_ ? transport_ : std::make_shared<StorageCashCountClass>();
        }
        void SetTransport(std::shared_ptr<StorageCashCountClass> transport)
        {
            transport_ = transport ? std::make_shared<StorageCashCountClass>(*transport) : std::make_shared<StorageCashCountClass>();
        }

        /// <summary>
        /// Equality operator
        /// </summary>
        bool operator==(const StorageCashOutCountClass& other) const
        {
            return *presented_ == *other.presented_ &&
                *rejected_ == *other.rejected_ &&
                *distributed_ == *other.distributed_ &&
                *unknown_ == *other.unknown_ &&
                *stacked_ == *other.stacked_ &&
                *diverted_ == *other.diverted_ &&
                *transport_ == *other.transport_;
        }

        bool operator!=(const StorageCashOutCountClass& other) const
        {
            return !(*this == other);
        }

    private:
        std::shared_ptr<StorageCashCountClass> presented_;
        std::shared_ptr<StorageCashCountClass> rejected_;
        std::shared_ptr<StorageCashCountClass> distributed_;
        std::shared_ptr<StorageCashCountClass> unknown_;
        std::shared_ptr<StorageCashCountClass> stacked_;
        std::shared_ptr<StorageCashCountClass> diverted_;
        std::shared_ptr<StorageCashCountClass> transport_;
    };











//#pragma once
//
//#include <string>
//#include <vector>
//#include <memory>
//#include "UnitStorageBase.hpp"
//
//namespace XFS4IoTFramework::Storage
//{
//    // Forward declarations
//    class CashCapabilitiesClass;
//    class CashConfigurationClass;
//    class CashStatusClass;
//    class CashUnitAdditionalInfoClass;
//    class CashUnitStorageConfiguration;


    /// <summary>
    /// Additional cash unit information device supports
    /// </summary>
    class CashUnitAdditionalInfoClass
    {
    public:
        CashUnitAdditionalInfoClass(int index, bool accuracySupported)
            : index_(index)
            , accuracySupported_(accuracySupported)
        {
        }

        /// <summary>
        /// Assigned by the device class. Will be a unique number which can be used to determine 
        /// usNumber in XFS 3.x migration. This can change as cash storage units are added and removed
        /// from the storage collection.
        /// </summary>
        int GetIndex() const { return index_; }

        /// <summary>
        /// Accuracy of count supported or not
        /// </summary>
        bool GetAccuracySupported() const { return accuracySupported_; }
        void SetAccuracySupported(bool accuracySupported) { accuracySupported_ = accuracySupported; }

    private:
        int index_;
        bool accuracySupported_;
    };

    /// <summary>
    /// Capabilities of the cash unit
    /// </summary>
    class CashCapabilitiesClass
    {
    public:
        enum class TypesEnum
        {
            None = 0,
            CashIn = 1 << 0,
            CashOut = 1 << 1,
            Replenishment = 1 << 2,
            CashInRetract = 1 << 3,
            CashOutRetract = 1 << 4,
            Reject = 1 << 5
        };

        enum class ItemsEnum
        {
            None = 0,
            Fit = 1 << 0,
            Unfit = 1 << 1,
            Unrecognized = 1 << 2,
            Counterfeit = 1 << 3,
            Suspect = 1 << 4,
            Inked = 1 << 5,
            Coupon = 1 << 6,
            Document = 1 << 7
        };

        CashCapabilitiesClass(
            TypesEnum types,
            ItemsEnum items,
            bool hardwareSensors,
            int retractAreas,
            std::vector<std::string> banknoteItems)
            : types_(types)
            , items_(items)
            , hardwareSensors_(hardwareSensors)
            , retractAreas_(retractAreas)
            , retractThresholds_(false)
            , banknoteItems_(std::move(banknoteItems))
        {
        }

        /// <summary>
        /// The types of operation the unit is capable to perform. This is a combination of one or 
        /// more operations
        /// </summary>
        TypesEnum GetTypes() const { return types_; }
        void SetTypes(TypesEnum types) { types_ = types; }

        /// <summary>
        /// The types of cash media the unit is capable of storing to store. This is a combination of
        /// one or more item types. May only be modified in an exchange state if applicable.
        /// </summary>
        ItemsEnum GetItems() const { return items_; }
        void SetItems(ItemsEnum items) { items_ = items; }

        /// <summary>
        /// The storage unit has hardware sensors that can detect threshold states.
        /// </summary>
        bool GetHardwareSensors() const { return hardwareSensors_; }
        void SetHardwareSensors(bool hardwareSensors) { hardwareSensors_ = hardwareSensors; }

        /// <summary>
        /// If items can be retracted into this storage unit, this is the number of areas within the storage unit which 
        /// allow physical separation of different bunches. If there is no physical separation of retracted bunches
        /// within this storage unit, this value is 1.
        /// </summary>
        int GetRetractAreas() const { return retractAreas_; }
        void SetRetractAreas(int retractAreas) { retractAreas_ = retractAreas; }

        /// <summary>
        /// If true, indicates that retract capacity is based on counts.
        /// If false, indicates that retract capacity is based on the number of commands which resulted in items
        /// being retracted into the storage unit.
        /// </summary>
        bool GetRetractThresholds() const { return retractThresholds_; }
        void SetRetractThresholds(bool retractThresholds) { retractThresholds_ = retractThresholds; }

        /// <summary>
        /// Lists the cash items which the storage unit is physically capable of handling.
        /// </summary>
        const std::vector<std::string>& GetBanknoteItems() const { return banknoteItems_; }
        void SetBanknoteItems(const std::vector<std::string>& banknoteItems) { banknoteItems_ = banknoteItems; }

    private:
        TypesEnum types_;
        ItemsEnum items_;
        bool hardwareSensors_;
        int retractAreas_;
        bool retractThresholds_;
        std::vector<std::string> banknoteItems_;
    };

    // Bitwise operators for enum flags
    inline CashCapabilitiesClass::TypesEnum operator|(CashCapabilitiesClass::TypesEnum a, CashCapabilitiesClass::TypesEnum b)
    {
        return static_cast<CashCapabilitiesClass::TypesEnum>(static_cast<int>(a) | static_cast<int>(b));
    }

    inline CashCapabilitiesClass::TypesEnum operator&(CashCapabilitiesClass::TypesEnum a, CashCapabilitiesClass::TypesEnum b)
    {
        return static_cast<CashCapabilitiesClass::TypesEnum>(static_cast<int>(a) & static_cast<int>(b));
    }

    inline CashCapabilitiesClass::ItemsEnum operator|(CashCapabilitiesClass::ItemsEnum a, CashCapabilitiesClass::ItemsEnum b)
    {
        return static_cast<CashCapabilitiesClass::ItemsEnum>(static_cast<int>(a) | static_cast<int>(b));
    }

    inline CashCapabilitiesClass::ItemsEnum operator&(CashCapabilitiesClass::ItemsEnum a, CashCapabilitiesClass::ItemsEnum b)
    {
        return static_cast<CashCapabilitiesClass::ItemsEnum>(static_cast<int>(a) & static_cast<int>(b));
    }
//
    /// <summary>
	/// Конфигурация единицы хранения наличных
    /// </summary>
    class CashConfigurationClass
    {
    public:
        CashConfigurationClass(
            CashCapabilitiesClass::TypesEnum types,
            CashCapabilitiesClass::ItemsEnum items,
            std::string currency,
            double value,
            int highThreshold,
            int lowThreshold,
            bool appLockIn,
            bool appLockOut,
            std::vector<std::string> banknoteItems)
            : types_(types)
            , items_(items)
            , currency_(std::move(currency))
            , value_(value)
            , highThreshold_(highThreshold)
            , lowThreshold_(lowThreshold)
            , appLockIn_(appLockIn)
            , appLockOut_(appLockOut)
            , banknoteItems_(std::move(banknoteItems))
        {
        }

        /// <summary>
        /// The types of operation the unit is capable of configured to perform. This is a combination of one or 
        /// more operations
        /// </summary>
        CashCapabilitiesClass::TypesEnum GetTypes() const { return types_; }
        void SetTypes(CashCapabilitiesClass::TypesEnum types) { types_ = types; }

        /// <summary>
        /// The types of cash media the unit is configured to store. This is a combination of
        /// one or more item types. May only be modified in an exchange state if applicable.
        /// </summary>
        CashCapabilitiesClass::ItemsEnum GetItems() const { return items_; }
        void SetItems(CashCapabilitiesClass::ItemsEnum items) { items_ = items; }

        /// <summary>
        /// ISO 4217 currency.
        /// </summary>
        const std::string& GetCurrency() const { return currency_; }
        void SetCurrency(const std::string& currency) { currency_ = currency; }

        /// <summary>
		/// Абсолютная стоимость единицы хранения. 
        /// Это может быть использовано для устройств, 
        /// которые не имеют возможности предоставлять подробные 
        /// сведения о предметах, но могут предоставлять общую стоимость предметов в единице хранения. 
        /// Например, если единица хранения содержит 10 банкнот по 20 долларов, то значение будет 200 долларов.
        /// </summary>
        double GetValue() const { return value_; }
        void SetValue(double value) { value_ = value; }

        /// <summary>
        /// If specified, ReplenishmentStatus is set to High if the count is greater than this number.
        /// </summary>
        int GetHighThreshold() const { return highThreshold_; }
        void SetHighThreshold(int highThreshold) { highThreshold_ = highThreshold; }

        /// <summary>
        /// If specified, ReplenishmentStatus is set to Low if the count is lower than this number.
        /// </summary>
        int GetLowThreshold() const { return lowThreshold_; }
        void SetLowThreshold(int lowThreshold) { lowThreshold_ = lowThreshold; }

        /// <summary>
		/// Если true, предметы не могут быть внесены в единицу хранения в операциях Cash In.
        /// </summary>
        bool GetAppLockIn() const { return appLockIn_; }
        void SetAppLockIn(bool appLockIn) { appLockIn_ = appLockIn; }

        /// <summary>
		/// Если true, предметы не могут быть удалены из единицы хранения в операциях Cash Out.
        /// </summary>
        bool GetAppLockOut() const { return appLockOut_; }
        void SetAppLockOut(bool appLockOut) { appLockOut_ = appLockOut; }

        /// <summary>
        /// Lists the cash items which are configured to this unit.
        /// </summary>
        const std::vector<std::string>& GetBanknoteItems() const { return banknoteItems_; }
        void SetBanknoteItems(const std::vector<std::string>& banknoteItems) { banknoteItems_ = banknoteItems; }

    private:
        CashCapabilitiesClass::TypesEnum types_;
        CashCapabilitiesClass::ItemsEnum items_;
        std::string currency_;
        double value_;
        int highThreshold_;
        int lowThreshold_;
        bool appLockIn_;
        bool appLockOut_;
        std::vector<std::string> banknoteItems_;
    };
//
    /// <summary>
    /// Status of the cash unit
    /// </summary>
    class CashStatusClass : public StorageChangedBaseRecord
    {
    public:
        enum class AccuracyEnum
        {
            NotSupported,
            Accurate,      // The count is expected to be accurate. The notes were previously counted and there have since been no events that might have introduced inaccuracy.
            AccurateSet,   // The count is expected to be accurate. The counts were previously set and there have since been no events that might have introduced inaccuracy.
            Inaccurate,    // The count is likely to be inaccurate. A jam, picking fault, or some other event may have resulted in a counting inaccuracy.
            Unknown        // The accuracy of count cannot be determined. This may be due to storage unit insertion or some other hardware event.
        };

        enum class ReplenishmentStatusEnum
        {
            Healthy,
            Full,
            Low,
            High,
            Empty
        };

        CashStatusClass(const CashUnitAdditionalInfoClass& additionalInfo)
        : index_(additionalInfo.GetIndex())
        , isAccuracySupported_(additionalInfo.GetAccuracySupported())
        , count_(0)
        {}

        /// <summary>
		/// Присвоено классом устройства. 
        /// Будет уникальным номером, который можно использовать для 
        /// определения usNumber при миграции XFS 3.x. Этот номер 
        /// может измениться по мере добавления и 
        /// удаления единиц хранения наличных из коллекции хранения.
        /// </summary>
        int GetIndex() const { return index_; }

        /// <summary>
		/// Поддерживается ли устройством точность подсчета. 
        /// Если false, Accuracy всегда возвращает NotSupported.
        /// </summary>
        std::shared_ptr<StorageCashCountClass> GetInitialCounts() const { return initialCounts_; }
        void SetInitialCounts(std::shared_ptr<StorageCashCountClass> initialCounts)
        {
            if (initialCounts_ != initialCounts)
            {
                initialCounts_ = initialCounts ? std::make_shared<StorageCashCountClass>(*initialCounts) : std::make_shared<StorageCashCountClass>();
                notifyPropertyChanged();
            }
        }

        /// <summary>
        /// The items moved from this storage unit by cash commands to another destination since the last 
        /// replenishment of this unit.
        /// </summary>
        std::shared_ptr<StorageCashOutCountClass> GetStorageCashOutCount() const { return storageCashOutCount_; }
        void SetStorageCashOutCount(std::shared_ptr<StorageCashOutCountClass> storageCashOutCount)
        {
            if (storageCashOutCount_ != storageCashOutCount)
            {
                storageCashOutCount_ = storageCashOutCount ? std::make_shared<StorageCashOutCountClass>(*storageCashOutCount) : std::make_shared<StorageCashOutCountClass>();
                notifyPropertyChanged();
            }
        }

        /// <summary>
        /// List of items inserted in this storage unit by cash commands from another source since the last 
        /// replenishment of this unit.
        /// </summary>
        std::shared_ptr<StorageCashInCountClass> GetStorageCashInCount() const { return storageCashInCount_; }
        void SetStorageCashInCount(std::shared_ptr<StorageCashInCountClass> storageCashInCount)
        {
            if (storageCashInCount_ != storageCashInCount)
            {
                storageCashInCount_ = storageCashInCount ? std::make_shared<StorageCashInCountClass>(*storageCashInCount) : std::make_shared<StorageCashInCountClass>();
                notifyPropertyChanged();
            }
        }

        /// <summary>
        /// Total count of the items in the unit
        /// </summary>
        int GetCount() const { return count_; }
        void SetCount(int count) { count_ = count; }

        /// <summary>
        /// Describes the accuracy of count
        /// </summary>
        AccuracyEnum GetAccuracy() const { return accuracy_; }
        void SetAccuracy(AccuracyEnum accuracy) { accuracy_ = accuracy; }

        /// <summary>
        /// The state of the media in the unit if it can be determined.
        /// </summary>
        ReplenishmentStatusEnum GetReplenishmentStatus() const { return replenishmentStatus_; }
        void SetReplenishmentStatus(ReplenishmentStatusEnum replenishmentStatus)
        {
            if (replenishmentStatus_ != replenishmentStatus)
            {
                replenishmentStatus_ = replenishmentStatus;
                notifyPropertyChanged();
            }
        }

    private:
        int index_;
        std::shared_ptr<StorageCashCountClass> initialCounts_;
        std::shared_ptr<StorageCashOutCountClass> storageCashOutCount_;
        std::shared_ptr<StorageCashInCountClass> storageCashInCount_;
        int count_;
        AccuracyEnum accuracy_;
        bool isAccuracySupported_;
        ReplenishmentStatusEnum replenishmentStatus_;
    };
//
    /// <summary>
    /// Cash Unit structure the device class supports
    /// </summary>
    class CashUnit
    {
    public:
        CashUnit(
            std::shared_ptr<CashCapabilitiesClass> capabilities,
            std::shared_ptr<CashConfigurationClass> configuration,
            const CashUnitAdditionalInfoClass& additionalInfo)
            : capabilities_(capabilities)
            , configuration_(configuration)
            , status_(std::make_shared<CashStatusClass>(additionalInfo))
        {
        }

        std::shared_ptr<CashCapabilitiesClass> GetCapabilities() const { return capabilities_; }
        std::shared_ptr<CashConfigurationClass> GetConfiguration() const { return configuration_; }
        std::shared_ptr<CashStatusClass> GetStatus() const { return status_; }

    private:
        std::shared_ptr<CashCapabilitiesClass> capabilities_;
        std::shared_ptr<CashConfigurationClass> configuration_;
        std::shared_ptr<CashStatusClass> status_;
    };
//
//
    /// <summary>
	/// Класс конфигурации денежного блока, который может быть использован для создания денежного блока и его хранения в CashUnitStorage. Содержит свойства, которые описывают идентификатор денежного блока, физическое расположение, вместимость, серийный номер, аппаратные возможности, текущую конфигурацию и дополнительную информацию о денежном блоке.
    /// </summary>
    class CashUnitStorageConfiguration
    {
    public:
        CashUnitStorageConfiguration(
            std::string id,
            std::string positionName,
            int capacity,
            std::string serialNumber,
            std::shared_ptr<CashCapabilitiesClass> capabilities,
            std::shared_ptr<CashConfigurationClass> configuration,
            CashUnitAdditionalInfoClass cashUnitAdditionalInfo)
            : id_(std::move(id))
            , positionName_(std::move(positionName))
            , capacity_(capacity)
            , serialNumber_(std::move(serialNumber))
            , capabilities_(capabilities)
            , configuration_(configuration)
            , cashUnitAdditionalInfo_(cashUnitAdditionalInfo)
        {
        }

        /// <summary>
		/// идентификатор, который может быть использован для cUnitID в миграции CDM/CIM XFS 3.x. Не требуется, если не применимо.
        /// </summary>
        const std::string& GetId() const { return id_; }

        /// <summary>
        /// Fixed physical name for the position.
        /// </summary>
        const std::string& GetPositionName() const { return positionName_; }

        /// <summary>
        /// Capacity
        /// </summary>
        int GetCapacity() const { return capacity_; }

        /// <summary>
		/// Серийный номер блока хранения, если он может быть считан электронным способом. Если серийный номер не может быть считан, это может быть null или пустой строкой.
        /// </summary>
        const std::string& GetSerialNumber() const { return serialNumber_; }

        /// <summary>
		/// Полные аппаратные возможности денежного блока, поддерживаемые устройством. Если устройство не поддерживает получение возможностей, это может быть null. Если устройство поддерживает получение возможностей, но произошла ошибка при получении возможностей, это также может быть null. Если устройство поддерживает получение возможностей и успешно получает их, это не будет null.
        /// </summary>
        std::shared_ptr<CashCapabilitiesClass> GetCapabilities() const { return capabilities_; }

        /// <summary>
		/// Текущая конфигурация, установленная устройством. Может быть null, если устройство не поддерживает получение конфигурации или если произошла ошибка при получении конфигурации.
        /// </summary>
        std::shared_ptr<CashConfigurationClass> GetConfiguration() const { return configuration_; }

        /// <summary>
		/// Дополнительная информация о денежном блоке, поддерживаемая устройством. Содержит индекс, который может быть использован для usNumber в миграции CDM/CIM XFS 3.x, и информацию о поддержке точности подсчета.
        /// </summary>
        const CashUnitAdditionalInfoClass& GetCashUnitAdditionalInfo() const { return cashUnitAdditionalInfo_; }

    private:
		std::string id_; // Идентификатор, который может быть использован для cUnitID в миграции CDM/CIM XFS 3.x. Не требуется, если не применимо.
		std::string positionName_;  // Фиксированное физическое имя для позиции.
		int capacity_;  // Вместимость
		std::string serialNumber_;  // Серийный номер блока хранения, если он может быть считан электронным способом.
		std::shared_ptr<CashCapabilitiesClass> capabilities_;   // Аппаратные возможности денежного блока
		std::shared_ptr<CashConfigurationClass> configuration_; // Текущая конфигурация, установленная устройством
		CashUnitAdditionalInfoClass cashUnitAdditionalInfo_;    // Дополнительная информация о денежном блоке
    };

    /// <summary>
    /// Cash Unit Storage
    /// </summary>
    class CashUnitStorage : public XFS4IoTFramework::Storage::UnitStorageBase
    {
    public:
        CashUnitStorage(const CashUnitStorageConfiguration& storageConfiguration)
            : UnitStorageBase(
                storageConfiguration.GetPositionName(),
                storageConfiguration.GetCapacity(),
                StatusEnum::NotConfigured,
                storageConfiguration.GetSerialNumber())
            , id_(storageConfiguration.GetId())
            , unit_(std::make_shared<CashUnit>(
                storageConfiguration.GetCapabilities(),
                storageConfiguration.GetConfiguration(),
                storageConfiguration.GetCashUnitAdditionalInfo()))
        {
        }

        /// <summary>
        /// An identifier which can be used for cUnitID in CDM/CIM XFS 3.x migration. Not required if not applicable.
        /// </summary>
        const std::string& GetId() const { return id_; }

        /// <summary>
        /// Cash Unit information
        /// </summary>
        std::shared_ptr<CashUnit> GetUnit() const { return unit_; }

    private:
        std::string id_;
        std::shared_ptr<CashUnit> unit_;
    };
//
//    // Implementation of CashStatusClass constructor
//    inline CashStatusClass::CashStatusClass(const CashUnitAdditionalInfoClass& additionalInfo)
//        : index_(additionalInfo.GetIndex())
//        , initialCounts_(std::make_shared<StorageCashCountClass>())
//        , storageCashOutCount_(std::make_shared<StorageCashOutCountClass>())
//        , storageCashInCount_(std::make_shared<StorageCashInCountClass>())
//        , count_(0)
//        , accuracy_(additionalInfo.GetAccuracySupported() ? AccuracyEnum::Unknown : AccuracyEnum::NotSupported)
//        , replenishmentStatus_(ReplenishmentStatusEnum::Empty)
//    {
//    }
//}
    // Добавьте в конец файла (после определения классов) — обеспечивает
// сериализацию/десериализацию для nlohmann::json через ADL.



        //inline void to_json(nlohmann::json& j, const CashItemCountClass& c)
        //{
        //    j = nlohmann::json{
        //        {"fit", c.GetFit()},
        //        {"unfit", c.GetUnfit()},
        //        {"suspect", c.GetSuspect()},
        //        {"counterfeit", c.GetCounterfeit()},
        //        {"inked", c.GetInked()}
        //    };
        //}

        //inline void from_json(const nlohmann::json& j, CashItemCountClass& c)
        //{
        //    // Используем value(...) чтобы работать с отсутствующими полями
        //    c.SetFit(j.value("fit", 0));
        //    c.SetUnfit(j.value("unfit", 0));
        //    c.SetSuspect(j.value("suspect", 0));
        //    c.SetCounterfeit(j.value("counterfeit", 0));
        //    c.SetInked(j.value("inked", 0));
        //}

        //inline void to_json(nlohmann::json& j, const StorageCashCountClass& s)
        //{
        //    j = nlohmann::json::object();
        //    j["unrecognized"] = s.GetUnrecognized();

        //    nlohmann::json items = nlohmann::json::object();
        //    for (const auto& kv : s.GetItemCounts())
        //        items[kv.first] = kv.second; // вызовет to_json(CashItemCountClass)

        //    j["itemCounts"] = std::move(items);
        //}

        //inline void from_json(const nlohmann::json& j, StorageCashCountClass& s)
        //{
        //    s.SetUnrecognized(j.value("unrecognized", 0));

        //    std::map<std::string, CashItemCountClass> tmp;
        //    if (j.contains("itemCounts") && j["itemCounts"].is_object())
        //    {
        //        for (auto it = j["itemCounts"].begin(); it != j["itemCounts"].end(); ++it)
        //        {
        //            tmp[it.key()] = it.value().get<CashItemCountClass>();
        //        }
        //    }
        //    s.SetItemCounts(tmp);
        //}

}