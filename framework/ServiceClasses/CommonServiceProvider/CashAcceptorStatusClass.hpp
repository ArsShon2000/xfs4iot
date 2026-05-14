#pragma once

#include <map>
#include <string>
#include "StatusBase.hpp"
#include "CashManagementCapabilitiesClass.hpp"
#include "CashManagementStatusClass.hpp"
#include "../../core/Events/PropertyChangedEventArgs.hpp"

namespace XFS4IoTFramework::Common
{
    /// <summary>
    /// Cash Acceptor status class
    /// </summary>
    class CashAcceptorStatusClass 
        : public StatusBase
        , public XFS4IoTServer::NotifyPropertyChangedBase
        , public std::enable_shared_from_this<CashAcceptorStatusClass> 
    {
    public:
        /// <summary>
        /// Stacker items enumeration
        /// </summary>
        enum class StackerItemsEnum
        {
            CustomerAccess,
            NoCustomerAccess,
            AccessUnknown,
            NoItems,
            NotSupported
        };

        /// <summary>
        /// Banknote reader enumeration
        /// </summary>
        enum class BanknoteReaderEnum
        {
            Ok,
            Inoperable,
            Unknown,
            NotSupported
        };

        /// <summary>
        /// Intermediate stacker enumeration
        /// </summary>
        enum class IntermediateStackerEnum
        {
            Empty,
            NotEmpty,
            Full,
            Unknown,
            NotSupported
        };

        /// <summary>
        /// Constructor
        /// </summary>
        CashAcceptorStatusClass(
            IntermediateStackerEnum intermediateStacker,
            StackerItemsEnum stackerItems,
            BanknoteReaderEnum banknoteReader,
            bool dropBox,
            std::map<CashManagementCapabilitiesClass::PositionEnum,
            CashManagementStatusClass::PositionStatusClass> positions)
            : intermediateStacker_(intermediateStacker)
            , stackerItems_(stackerItems)
            , banknoteReader_(banknoteReader)
            , dropBox_(dropBox)
            , positions_(std::move(positions))
        {
        }


        // Явный копирующий конструктор.
        // Не копируем NotifyPropertyChangedBase (mutex некопируем), создаём новый экземпляр базового класса.
        CashAcceptorStatusClass(const CashAcceptorStatusClass& other)
            : StatusBase(other) // если StatusBase копируем — скопируем состояние статуса
            , XFS4IoTServer::NotifyPropertyChangedBase() // создаём новую базу уведомлений
            , intermediateStacker_(other.intermediateStacker_)
            , stackerItems_(other.stackerItems_)
            , banknoteReader_(other.banknoteReader_)
            , dropBox_(other.dropBox_)
            , positions_(other.positions_)
        {
            // Обратите внимание: propertyChangedCallback_ (из StatusBase) не копируется здесь явно,
            // при необходимости можно подписаться заново.
        }

        /// <summary>
        /// Supplies the state of the intermediate stacker. The following values are possible:
        /// 
        /// * Empty - The intermediate stacker is empty.
        /// * NotEmpty - The intermediate stacker is not empty.
        /// * Full - The intermediate stacker is full. This may also be reported during a cash-in transaction
        ///   where a limit specified by CashAcceptor.CashInStart has been reached.
        /// * Unknown - Due to a hardware error or other condition, the state of the intermediate stacker
        ///   cannot be determined.
        /// * NotSupported - The physical device has no intermediate stacker.
        /// </summary>
        IntermediateStackerEnum GetIntermediateStacker() const
        {
            return intermediateStacker_;
        }

        void SetIntermediateStacker(IntermediateStackerEnum value)
        {
            if (intermediateStacker_ != value) {
                intermediateStacker_ = value;
                raisePropertyChanged("IntermediateStacker");
            }
        }

        /// <summary>
        /// This field informs the application whether items on the intermediate stacker have been in customer access.
        /// The following values are possible:
        /// 
        /// * CustomerAccess - Items on the intermediate stacker have been in customer access. If the device is a
        ///   cash recycler then the items on the intermediate stacker may be there as a result of a previous
        ///   cash-out operation.
        /// * NoCustomerAccess - Items on the intermediate stacker have not been in customer access.
        /// * AccessUnknown - It is not known if the items on the intermediate stacker have been in customer access.
        /// * NoItems - There are no items on the intermediate stacker or the physical device has no intermediate
        ///   stacker.
        /// </summary>
        StackerItemsEnum GetStackerItems() const
        {
            return stackerItems_;
        }

        void SetStackerItems(StackerItemsEnum value)
        {
            if (stackerItems_ != value) {
                stackerItems_ = value;
                raisePropertyChanged("StackerItems");
            }
        }

        /// <summary>
        /// Supplies the state of the banknote reader. The following values are possible:
        /// 
        /// * Ok - The banknote reader is in a good state.
        /// * Inoperable - The banknote reader is inoperable.
        /// * Unknown - Due to a hardware error or other condition, the state of the banknote reader cannot be
        ///   determined.
        /// * NotSupported - The physical device has no banknote reader.
        /// </summary>
        BanknoteReaderEnum GetBanknoteReader() const
        {
            return banknoteReader_;
        }

        void SetBanknoteReader(BanknoteReaderEnum value)
        {
            if (banknoteReader_ != value) {
                banknoteReader_ = value;
                raisePropertyChanged("BanknoteReader");
            }
        }

        /// <summary>
        /// The drop box is an area within the Cash Acceptor where items which have caused a problem during an operation
        /// are stored. This field specifies the status of the drop box.
        /// If true, some items are stored in the drop box due to a cash-in transaction which caused a problem.
        /// If false, the drop box is empty or there is no drop box.
        /// </summary>
        bool GetDropBox() const
        {
            return dropBox_;
        }

        void SetDropBox(bool value)
        {
            if (dropBox_ != value) {
                dropBox_ = value;
                raisePropertyChanged("DropBox");
            }
        }

        /// <summary>
        /// Array of structures for each position to which items can be dispensed or presented.
        /// </summary>
        std::map<CashManagementCapabilitiesClass::PositionEnum,
            CashManagementStatusClass::PositionStatusClass>& GetPositions()
        {
            return positions_;
        }

        /// <summary>
        /// Set positions (non-const access)
        /// </summary>
        void SetPositions(std::map<CashManagementCapabilitiesClass::PositionEnum,
            CashManagementStatusClass::PositionStatusClass> positions)
        {
            positions_ = std::move(positions);
            raisePropertyChanged("Positions");
        }

        /// <summary>
        /// Update a specific position
        /// </summary>
        void UpdatePosition(CashManagementCapabilitiesClass::PositionEnum position,
            CashManagementStatusClass::PositionStatusClass& status)
        {
            positions_[position] = status;
            raisePropertyChanged("Positions");
        }

    private:
        IntermediateStackerEnum intermediateStacker_;
        StackerItemsEnum stackerItems_;
        BanknoteReaderEnum banknoteReader_;
        bool dropBox_;
        std::map<CashManagementCapabilitiesClass::PositionEnum,
            CashManagementStatusClass::PositionStatusClass> positions_;
    };

    // Helper functions for enum to string conversion
    inline std::string ToString(CashAcceptorStatusClass::IntermediateStackerEnum value)
    {
        switch (value) {
        case CashAcceptorStatusClass::IntermediateStackerEnum::Empty:
            return "empty";
        case CashAcceptorStatusClass::IntermediateStackerEnum::NotEmpty:
            return "notEmpty";
        case CashAcceptorStatusClass::IntermediateStackerEnum::Full:
            return "full";
        case CashAcceptorStatusClass::IntermediateStackerEnum::Unknown:
            return "unknown";
        case CashAcceptorStatusClass::IntermediateStackerEnum::NotSupported:
            return "notSupported";
        default:
            return "unknown";
        }
    }

    inline std::string ToString(CashAcceptorStatusClass::StackerItemsEnum value)
    {
        switch (value) {
        case CashAcceptorStatusClass::StackerItemsEnum::CustomerAccess:
            return "customerAccess";
        case CashAcceptorStatusClass::StackerItemsEnum::NoCustomerAccess:
            return "noCustomerAccess";
        case CashAcceptorStatusClass::StackerItemsEnum::AccessUnknown:
            return "accessUnknown";
        case CashAcceptorStatusClass::StackerItemsEnum::NoItems:
            return "noItems";
        case CashAcceptorStatusClass::StackerItemsEnum::NotSupported:
            return "notSupported";
        default:
            return "unknown";
        }
    }

    inline std::string ToString(CashAcceptorStatusClass::BanknoteReaderEnum value)
    {
        switch (value) {
        case CashAcceptorStatusClass::BanknoteReaderEnum::Ok:
            return "ok";
        case CashAcceptorStatusClass::BanknoteReaderEnum::Inoperable:
            return "inoperable";
        case CashAcceptorStatusClass::BanknoteReaderEnum::Unknown:
            return "unknown";
        case CashAcceptorStatusClass::BanknoteReaderEnum::NotSupported:
            return "notSupported";
        default:
            return "unknown";
        }
    }

    // String to enum conversion helpers
    inline CashAcceptorStatusClass::IntermediateStackerEnum IntermediateStackerFromString(
        const std::string& str)
    {
        if (str == "empty")
            return CashAcceptorStatusClass::IntermediateStackerEnum::Empty;
        if (str == "notEmpty")
            return CashAcceptorStatusClass::IntermediateStackerEnum::NotEmpty;
        if (str == "full")
            return CashAcceptorStatusClass::IntermediateStackerEnum::Full;
        if (str == "unknown")
            return CashAcceptorStatusClass::IntermediateStackerEnum::Unknown;
        if (str == "notSupported")
            return CashAcceptorStatusClass::IntermediateStackerEnum::NotSupported;

        return CashAcceptorStatusClass::IntermediateStackerEnum::Unknown;
    }

    inline CashAcceptorStatusClass::StackerItemsEnum StackerItemsFromString(
        const std::string& str)
    {
        if (str == "customerAccess")
            return CashAcceptorStatusClass::StackerItemsEnum::CustomerAccess;
        if (str == "noCustomerAccess")
            return CashAcceptorStatusClass::StackerItemsEnum::NoCustomerAccess;
        if (str == "accessUnknown")
            return CashAcceptorStatusClass::StackerItemsEnum::AccessUnknown;
        if (str == "noItems")
            return CashAcceptorStatusClass::StackerItemsEnum::NoItems;
        if (str == "notSupported")
            return CashAcceptorStatusClass::StackerItemsEnum::NotSupported;

        return CashAcceptorStatusClass::StackerItemsEnum::NoItems;
    }

    inline CashAcceptorStatusClass::BanknoteReaderEnum BanknoteReaderFromString(
        const std::string& str)
    {
        if (str == "ok")
            return CashAcceptorStatusClass::BanknoteReaderEnum::Ok;
        if (str == "inoperable")
            return CashAcceptorStatusClass::BanknoteReaderEnum::Inoperable;
        if (str == "unknown")
            return CashAcceptorStatusClass::BanknoteReaderEnum::Unknown;
        if (str == "notSupported")
            return CashAcceptorStatusClass::BanknoteReaderEnum::NotSupported;

        return CashAcceptorStatusClass::BanknoteReaderEnum::Unknown;
    }
}