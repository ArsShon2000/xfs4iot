#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <memory>
#include <cstdint>
#include <stdexcept>

#include "../../core/common/MessageHeader.hpp"
#include "../../server/DeviceResult.hpp"
#include "../StorageServiceProvider/CashUnit.hpp"
#include "../../core/CashManagement/Completions/CalibrateCashUnitCompletion.hpp"
#include "../../core/CashManagement/Completions/ResetCompletion.hpp"
#include "../CommonServiceProvider/CashManagementCapabilitiesClass.hpp"
#include "../../core/CashManagement/Completions/RetractCompletion.hpp"
#include "../../core/CashManagement/Completions/GetTellerInfoCompletion.hpp"
#include "../../core/CashManagement/Completions/SetTellerInfoCompletion.hpp"

namespace XFS4IoTFramework::CashManagement
{
    /// <summary>
    /// Note classification level
    /// </summary>
    enum class NoteLevelEnum
    {
        All,
        Unrecognized,
        Counterfeit,
        Suspect,
        Fit,
        Unfit,
        Inked
    };

    /// <summary>
    /// Note orientation
    /// </summary>
    enum class OrientationEnum
    {
        NotSupported,
        FrontTop,
        FrontBottom,
        BackTop,
        BackBottom,
        Unknown
    };

    /// <summary>
    /// Classification list status
    /// </summary>
    enum class ClassificationListEnum
    {
        NotSupported,
        OnClassificationList,
        NotOnClassificationList,
        ClassificationListUnknown
    };

    /// <summary>
    /// Target destination item to be moved in reset operation
    /// </summary>
    enum class ItemTargetEnum
    {
        SingleUnit,
        Retract,
        Transport,
        Stacker,
        Reject,
        ItemCassette,
        CashIn,
        OutDefault,
        OutLeft,
        OutRight,
        OutCenter,
        OutTop,
        OutBottom,
        OutFront,
        OutRear,
        Default
    };

    /// <summary>
    /// Item information type flags
    /// </summary>
    enum class ItemInfoTypeEnum : uint32_t
    {
        All = 0,
        SerialNumber = 1 << 0,
        Signature = 1 << 1,
        Image = 1 << 2
    };

    // Bitwise operators for ItemInfoTypeEnum
    inline ItemInfoTypeEnum operator|(ItemInfoTypeEnum lhs, ItemInfoTypeEnum rhs)
    {
        return static_cast<ItemInfoTypeEnum>(
            static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    }

    inline ItemInfoTypeEnum operator&(ItemInfoTypeEnum lhs, ItemInfoTypeEnum rhs)
    {
        return static_cast<ItemInfoTypeEnum>(
            static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
    }

    inline bool HasFlag(ItemInfoTypeEnum value, ItemInfoTypeEnum flag)
    {
        return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
    }

    inline std::string ToString(XFS4IoTFramework::CashManagement::NoteLevelEnum level)
    {
        using NoteLevelEnum = XFS4IoTFramework::CashManagement::NoteLevelEnum;

        switch (level)
        {
        case NoteLevelEnum::All:          return "all";
        case NoteLevelEnum::Unrecognized: return "unrecognized";
        case NoteLevelEnum::Counterfeit:  return "counterfeit";
        case NoteLevelEnum::Suspect:      return "suspect";
        case NoteLevelEnum::Fit:          return "fit";
        case NoteLevelEnum::Unfit:        return "unfit";
        case NoteLevelEnum::Inked:        return "inked";
        default:                          return "all";
        }
    }

    inline XFS4IoTFramework::CashManagement::NoteLevelEnum
        NoteLevelEnumFromString(const std::string& value)
    {
        using NoteLevelEnum = XFS4IoTFramework::CashManagement::NoteLevelEnum;

        if (value == "all")          return NoteLevelEnum::All;
        if (value == "unrecognized") return NoteLevelEnum::Unrecognized;
        if (value == "counterfeit")  return NoteLevelEnum::Counterfeit;
        if (value == "suspect")      return NoteLevelEnum::Suspect;
        if (value == "fit")          return NoteLevelEnum::Fit;
        if (value == "unfit")        return NoteLevelEnum::Unfit;
        if (value == "inked")        return NoteLevelEnum::Inked;

        return NoteLevelEnum::All;
    }

    /// <summary>
    /// ItemInfoSummary
    /// Store information for the item type and counts
    /// </summary>
    class ItemInfoSummary final
    {
    public:
        ItemInfoSummary(NoteLevelEnum level, int num_of_items)
            : level_(level)
            , num_of_items_(num_of_items)
        {
        }

        /// <summary>
        /// Level of item
        /// </summary>
        [[nodiscard]] NoteLevelEnum level() const noexcept { return level_; }

        /// <summary>
        /// Number of items classified as level which have information available.
        /// </summary>
        [[nodiscard]] int num_of_items() const noexcept { return num_of_items_; }

    private:
        NoteLevelEnum level_;
        int num_of_items_;
    };

    /// <summary>
    /// This field is used if items are to be moved to internal areas of the device
    /// </summary>
    class Retract final
    {
    public:
        Retract(
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractAreaEnum retract_area,
            std::optional<int> index = std::nullopt)
            : retract_area_(retract_area)
            , index_(index)
        {
        }

        [[nodiscard]] XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractAreaEnum retract_area() const noexcept
        {
            return retract_area_;
        }

        /// <summary>
        /// Index is valid if the RetractArea is set to Retract
        /// </summary>
        [[nodiscard]] const std::optional<int>& index() const noexcept { return index_; }

    private:
        XFS4IoTFramework::Common::CashManagementCapabilitiesClass::RetractAreaEnum retract_area_;
        std::optional<int> index_;
    };

    /// <summary>
    /// RetractPosition - Specifies where the dispensed items should be moved to for retract command
    /// </summary>
    class RetractPosition final
    {
    public:
        // Constructor for CashUnit
        explicit RetractPosition(std::string cash_unit)
            : cash_unit_(std::move(cash_unit))
            , retract_area_(nullptr)
            , output_position_(std::nullopt)
        {
        }

        // Constructor for RetractArea
        explicit RetractPosition(std::shared_ptr<Retract> retract_area)
            : cash_unit_()
            , retract_area_(retract_area)
            , output_position_(std::nullopt)
        {
        }

        // Constructor for OutputPosition
        explicit RetractPosition(std::optional<XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum> output_position)
            : cash_unit_()
            , retract_area_(nullptr)
            , output_position_(output_position)
        {
        }

        /// <summary>
        /// The name of the single cash unit to be used for storage
        /// </summary>
        [[nodiscard]] const std::string& cash_unit() const noexcept { return cash_unit_; }

        /// <summary>
        /// Items to be moved to internal areas of the device
        /// </summary>
        [[nodiscard]] const std::shared_ptr<Retract>& retract_area() const noexcept { return retract_area_; }

        /// <summary>
        /// The output position to which items are to be moved
        /// </summary>
        [[nodiscard]] const std::optional<XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum>& output_position() const noexcept
        {
            return output_position_;
        }

    private:
        std::string cash_unit_;
        std::shared_ptr<Retract> retract_area_;
        std::optional<XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum> output_position_;
    };

    /// <summary>
    /// ItemDestination - Specifies where the dispensed items should be moved to
    /// </summary>
    class ItemDestination final
    {
    public:
        ItemDestination() = default;

        explicit ItemDestination(std::string cash_unit)
            : cash_unit_(std::move(cash_unit))
            , destination_(ItemTargetEnum::SingleUnit)
            , index_of_retract_unit_(1)
        {
        }

        explicit ItemDestination(ItemTargetEnum destination, int index_of_retract_unit = 1)
            : cash_unit_()
            , destination_(destination)
            , index_of_retract_unit_(index_of_retract_unit)
        {
        }

        /// <summary>
        /// The name of the single cash unit to be used for storage
        /// </summary>
        [[nodiscard]] const std::string& cash_unit() const noexcept { return cash_unit_; }

        /// <summary>
        /// The output position to which items are to be moved
        /// </summary>
        [[nodiscard]] ItemTargetEnum destination() const noexcept { return destination_; }

        /// <summary>
        /// Position inside the retract storage units
        /// </summary>
        [[nodiscard]] int index_of_retract_unit() const noexcept { return index_of_retract_unit_; }

    private:
        std::string cash_unit_;
        ItemTargetEnum destination_ = ItemTargetEnum::Default;
        int index_of_retract_unit_ = 1;
    };

    /// <summary>
    /// ItemInfoClass - Detailed information about a cash item
    /// </summary>
    class ItemInfoClass final
    {
    public:
        ItemInfoClass(
            OrientationEnum orientation,
            std::vector<uint8_t> signature,
            NoteLevelEnum level,
            std::string serial_number,
            std::vector<uint8_t> image,
            ClassificationListEnum classification_list,
            std::string item_location)
            : orientation_(orientation)
            , signature_(std::move(signature))
            , level_(level)
            , serial_number_(std::move(serial_number))
            , image_(std::move(image))
            , classification_list_(classification_list)
            , item_location_(std::move(item_location))
        {
        }

        [[nodiscard]] OrientationEnum orientation() const noexcept { return orientation_; }
        [[nodiscard]] const std::vector<uint8_t>& signature() const noexcept { return signature_; }
        [[nodiscard]] NoteLevelEnum level() const noexcept { return level_; }
        [[nodiscard]] const std::string& serial_number() const noexcept { return serial_number_; }
        [[nodiscard]] const std::vector<uint8_t>& image() const noexcept { return image_; }
        [[nodiscard]] ClassificationListEnum classification_list() const noexcept { return classification_list_; }
        [[nodiscard]] const std::string& item_location() const noexcept { return item_location_; }

    private:
        OrientationEnum orientation_;
        std::vector<uint8_t> signature_;
        NoteLevelEnum level_;
        std::string serial_number_;
        std::vector<uint8_t> image_;
        ClassificationListEnum classification_list_;
        std::string item_location_;
    };

    /// <summary>
    /// CalibrateCashUnitRequest - Request to perform calibration of cash unit
    /// </summary>
    class CalibrateCashUnitRequest final
    {
    public:
        CalibrateCashUnitRequest(
            std::string cash_unit,
            int num_of_bills,
            ItemDestination position)
            : cash_unit_(std::move(cash_unit))
            , num_of_bills_(num_of_bills)
            , position_(std::move(position))
        {
        }

        [[nodiscard]] const std::string& cash_unit() const noexcept { return cash_unit_; }
        [[nodiscard]] int num_of_bills() const noexcept { return num_of_bills_; }
        [[nodiscard]] const ItemDestination& position() const noexcept { return position_; }

    private:
        std::string cash_unit_;
        int num_of_bills_;
        ItemDestination position_;
    };

    /// <summary>
    /// CalibrateCashUnitResult - Return result of calibration for cash unit
    /// </summary>
    class CalibrateCashUnitResult final : public XFS4IoTServer::DeviceResult
    {
    public:
        CalibrateCashUnitResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completion_code,
            std::optional<std::string> error_description = std::nullopt,
            std::optional<XFS4IoT::CashManagement::Completions::CalibrateCashUnitCompletionPayloadData::ErrorCodeEnum> error_code = std::nullopt)
            : XFS4IoTServer::DeviceResult(completion_code, error_description)
            , error_code_(error_code)
            , position_(std::nullopt)
            , movement_result_(std::nullopt)
        {
        }

        CalibrateCashUnitResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completion_code,
            ItemDestination position,
            std::optional<std::unordered_map<std::string, Storage::CashUnitCountClass>> movement_result = std::nullopt)
            : XFS4IoTServer::DeviceResult(completion_code, std::nullopt)
            , error_code_(std::nullopt)
            , position_(std::move(position))
            , movement_result_(std::move(movement_result))
        {
        }

        [[nodiscard]] const auto& error_code() const noexcept { return error_code_; }
        [[nodiscard]] const auto& position() const noexcept { return position_; }
        [[nodiscard]] const auto& movement_result() const noexcept { return movement_result_; }

    private:
        std::optional<XFS4IoT::CashManagement::Completions::CalibrateCashUnitCompletionPayloadData::ErrorCodeEnum> error_code_;
        std::optional<ItemDestination> position_;
        std::optional<std::unordered_map<std::string, Storage::CashUnitCountClass>> movement_result_;
    };

    /// <summary>
    /// OpenCloseShutterRequest - Open or Close shutter for the specified output position
    /// </summary>
    class OpenCloseShutterRequest final
    {
    public:
        enum class ActionEnum
        {
            Open,
            Close
        };

        OpenCloseShutterRequest(
            ActionEnum action,
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum shutter_position)
            : action_(action)
            , shutter_position_(shutter_position)
        {
        }

        [[nodiscard]] ActionEnum action() const noexcept { return action_; }
        [[nodiscard]] XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum shutter_position() const noexcept
        {
            return shutter_position_;
        }

    private:
        ActionEnum action_;
        XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum shutter_position_;
    };

    /// <summary>
    /// OpenCloseShutterResult - Return result of shutter operation
    /// </summary>
    class OpenCloseShutterResult final : public XFS4IoTServer::DeviceResult
    {
    public:
        enum class ErrorCodeEnum
        {
            UnsupportedPosition,
            ShutterNotOpen,
            ShutterOpen,
            ShutterClosed,
            ShutterNotClosed,
            ExchangeActive
        };

        OpenCloseShutterResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completion_code,
            std::optional<std::string> error_description,
            std::optional<ErrorCodeEnum> error_code,
            bool jammed)
            : DeviceResult(completion_code, error_description)
            , error_code_(error_code)
            , jammed_(jammed)
        {
        }

        OpenCloseShutterResult(XFS4IoT::MessageHeader::CompletionCodeEnum completion_code)
            : DeviceResult(completion_code, std::nullopt)
            , error_code_(std::nullopt)
            , jammed_(false)
        {
        }

        [[nodiscard]] const auto& error_code() const noexcept { return error_code_; }
        [[nodiscard]] bool jammed() const noexcept { return jammed_; }

    private:
        std::optional<ErrorCodeEnum> error_code_;
        bool jammed_;
    };

    /// <summary>
    /// ResetDeviceRequest - The parameter class for the reset device operation
    /// </summary>
    class ResetDeviceRequest final
    {
    public:
        explicit ResetDeviceRequest(ItemDestination position)
            : position_(std::move(position))
        {
        }

        [[nodiscard]] const ItemDestination& position() const noexcept { return position_; }

    private:
        ItemDestination position_;
    };

    /// <summary>
    /// ResetDeviceResult - Return result of reset device
    /// </summary>
    class ResetDeviceResult final : public XFS4IoTServer::DeviceResult
    {
    public:
        ResetDeviceResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completion_code,
            std::optional<std::string> error_description = std::nullopt,
            std::optional<XFS4IoT::CashManagement::Completions::ResetCompletionPayloadData::ErrorCodeEnum> error_code = std::nullopt,
            std::optional<std::unordered_map<std::string, Storage::CashUnitCountClass>> movement_result = std::nullopt)
            : DeviceResult(completion_code, error_description)
            , error_code_(error_code)
            , movement_result_(std::move(movement_result))
        {
        }

        ResetDeviceResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completion_code,
            std::unordered_map<std::string, Storage::CashUnitCountClass> movement_result)
            : DeviceResult(completion_code, std::nullopt)
            , error_code_(std::nullopt)
            , movement_result_(std::move(movement_result))
        {
        }

        [[nodiscard]] const auto& error_code() const noexcept { return error_code_; }
        [[nodiscard]] const auto& movement_result() const noexcept { return movement_result_; }

    private:
        std::optional<XFS4IoT::CashManagement::Completions::ResetCompletionPayloadData::ErrorCodeEnum> error_code_;
        std::optional<std::unordered_map<std::string, Storage::CashUnitCountClass>> movement_result_;
    };

    /// <summary>
    /// RetractRequest - The parameter class for the retract operation
    /// </summary>
    class RetractRequest final
    {
    public:
        explicit RetractRequest(RetractPosition position)
            : position_(std::move(position))
        {
        }

        [[nodiscard]] const RetractPosition& position() const noexcept { return position_; }

    private:
        RetractPosition position_;
    };

    /// <summary>
    /// RetractResult - Return result of retract items
    /// </summary>
    class RetractResult final : public XFS4IoTServer::DeviceResult
    {
    public:
        RetractResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completion_code,
            std::optional<std::string> error_description = std::nullopt,
            std::optional<XFS4IoT::CashManagement::Completions::RetractCompletionPayloadData::ErrorCodeEnum> error_code = std::nullopt,
            std::optional<std::unordered_map<std::string, Storage::CashUnitCountClass>> movement_result = std::nullopt)
            : DeviceResult(completion_code, error_description)
            , error_code_(error_code)
            , movement_result_(std::move(movement_result))
        {
        }

        RetractResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completion_code,
            std::unordered_map<std::string, Storage::CashUnitCountClass> movement_result)
            : DeviceResult(completion_code, std::nullopt)
            , error_code_(std::nullopt)
            , movement_result_(std::move(movement_result))
        {
        }

        [[nodiscard]] const auto& error_code() const noexcept { return error_code_; }
        [[nodiscard]] const auto& movement_result() const noexcept { return movement_result_; }

    private:
        std::optional<XFS4IoT::CashManagement::Completions::RetractCompletionPayloadData::ErrorCodeEnum> error_code_;
        std::optional<std::unordered_map<std::string, Storage::CashUnitCountClass>> movement_result_;
    };

    /// <summary>
    /// GetTellerInfoRequest - The parameter class for get teller info from the device
    /// </summary>
    class GetTellerInfoRequest final
    {
    public:
        GetTellerInfoRequest(int teller_id, std::string currency_id)
            : teller_id_(teller_id)
            , currency_id_(std::move(currency_id))
        {
        }

        [[nodiscard]] int teller_id() const noexcept { return teller_id_; }
        [[nodiscard]] const std::string& currency_id() const noexcept { return currency_id_; }

    private:
        int teller_id_;
        std::string currency_id_;
    };

    /// <summary>
    /// TellerDetail - Details of the teller information
    /// </summary>
    class TellerDetail final
    {
    public:
        /// <summary>
        /// Teller totals
        /// </summary>
        class TellerTotal final
        {
        public:
            TellerTotal(
                double items_received,
                double items_dispensed,
                double coins_received,
                double coins_dispensed,
                double cash_box_received,
                double cash_box_dispensed)
                : items_received_(items_received)
                , items_dispensed_(items_dispensed)
                , coins_received_(coins_received)
                , coins_dispensed_(coins_dispensed)
                , cash_box_received_(cash_box_received)
                , cash_box_dispensed_(cash_box_dispensed)
            {
            }

            [[nodiscard]] double items_received() const noexcept { return items_received_; }
            [[nodiscard]] double items_dispensed() const noexcept { return items_dispensed_; }
            [[nodiscard]] double coins_received() const noexcept { return coins_received_; }
            [[nodiscard]] double coins_dispensed() const noexcept { return coins_dispensed_; }
            [[nodiscard]] double cash_box_received() const noexcept { return cash_box_received_; }
            [[nodiscard]] double cash_box_dispensed() const noexcept { return cash_box_dispensed_; }

        private:
            double items_received_;
            double items_dispensed_;
            double coins_received_;
            double coins_dispensed_;
            double cash_box_received_;
            double cash_box_dispensed_;
        };

        TellerDetail(
            int teller_id,
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum input_position,
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum output_position,
            std::unordered_map<std::string, TellerTotal> totals)
            : teller_id_(teller_id)
            , input_position_(input_position)
            , output_position_(output_position)
            , totals_(std::move(totals))
        {
        }

        [[nodiscard]] int teller_id() const noexcept { return teller_id_; }
        [[nodiscard]] XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum input_position() const noexcept
        {
            return input_position_;
        }
        [[nodiscard]] XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum output_position() const noexcept
        {
            return output_position_;
        }
        [[nodiscard]] const std::unordered_map<std::string, TellerTotal>& totals() const noexcept
        {
            return totals_;
        }

    private:
        int teller_id_;
        XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum input_position_;
        XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum output_position_;
        std::unordered_map<std::string, TellerTotal> totals_;
    };

    /// <summary>
    /// GetTellerInfoResult - Return result of teller info inquired
    /// </summary>
    class GetTellerInfoResult final : public XFS4IoTServer::DeviceResult
    {
    public:
        GetTellerInfoResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completion_code,
            std::optional<std::string> error_description = std::nullopt,
            std::optional<XFS4IoT::CashManagement::Completions::GetTellerInfoCompletionPayloadData::ErrorCodeEnum> error_code = std::nullopt,
            std::optional<std::vector<TellerDetail>> details = std::nullopt)
            : DeviceResult(completion_code, error_description)
            , error_code_(error_code)
            , details_(std::move(details))
        {
        }

        GetTellerInfoResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completion_code,
            std::optional<std::vector<TellerDetail>> details = std::nullopt)
            : DeviceResult(completion_code, std::nullopt)
            , error_code_(std::nullopt)
            , details_(std::move(details))
        {
        }

        [[nodiscard]] const auto& error_code() const noexcept { return error_code_; }
        [[nodiscard]] const auto& details() const noexcept { return details_; }

    private:
        std::optional<XFS4IoT::CashManagement::Completions::GetTellerInfoCompletionPayloadData::ErrorCodeEnum> error_code_;
        std::optional<std::vector<TellerDetail>> details_;
    };

    /// <summary>
    /// SetTellerInfoRequest - The parameter class for setting teller info
    /// </summary>
    class SetTellerInfoRequest final
    {
    public:
        enum class ActionEnum
        {
            Create,
            Modify,
            Delete
        };

        SetTellerInfoRequest(ActionEnum action, TellerDetail detail)
            : action_(action)
            , detail_(std::move(detail))
        {
        }

        [[nodiscard]] ActionEnum action() const noexcept { return action_; }
        [[nodiscard]] const TellerDetail& detail() const noexcept { return detail_; }

    private:
        ActionEnum action_;
        TellerDetail detail_;
    };

    /// <summary>
    /// SetTellerInfoResult - Return result of setting teller info
    /// </summary>
    class SetTellerInfoResult final : public XFS4IoTServer::DeviceResult
    {
    public:
        SetTellerInfoResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completion_code,
            std::optional<std::string> error_description = std::nullopt,
            std::optional<XFS4IoT::CashManagement::Completions::SetTellerInfoCompletionPayloadData::ErrorCodeEnum> error_code = std::nullopt)
            : DeviceResult(completion_code, error_description)
            , error_code_(error_code)
        {
        }

        [[nodiscard]] const auto& error_code() const noexcept { return error_code_; }

    private:
        std::optional<XFS4IoT::CashManagement::Completions::SetTellerInfoCompletionPayloadData::ErrorCodeEnum> error_code_;
    };

    /// <summary>
    /// GetItemInfoRequest - Which item information to be retrieved
    /// </summary>
    class GetItemInfoRequest final
    {
    public:
        GetItemInfoRequest(
            int index,
            NoteLevelEnum note_level,
            ItemInfoTypeEnum item_info_type)
            : index_(index)
            , note_level_(note_level)
            , item_info_type_(item_info_type)
        {
        }

        [[nodiscard]] int index() const noexcept { return index_; }
        [[nodiscard]] NoteLevelEnum note_level() const noexcept { return note_level_; }
        [[nodiscard]] ItemInfoTypeEnum item_info_type() const noexcept { return item_info_type_; }

    private:
        int index_;
        NoteLevelEnum note_level_;
        ItemInfoTypeEnum item_info_type_;
    };

    /// <summary>
    /// GetItemInfoResult - Return item information requested by the client
    /// </summary>
    class GetItemInfoResult final : public XFS4IoTServer::DeviceResult
    {
    public:
        GetItemInfoResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completion_code,
            std::optional<std::string> error_description = std::nullopt)
            : DeviceResult(completion_code, error_description)
            , item_infos_(std::nullopt)
        {
        }

        GetItemInfoResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completion_code,
            std::unordered_map<std::string, ItemInfoClass> item_infos)
            : DeviceResult(completion_code, std::nullopt)
            , item_infos_(std::move(item_infos))
        {
        }

        GetItemInfoResult() : DeviceResult(XFS4IoT::MessageHeader::CompletionCodeEnum::Success), item_infos_(std::nullopt)
        {
		}

        /// <summary>
        /// Reported requested item information. Key is note type Id.
        /// </summary>
        [[nodiscard]] const auto& item_infos() const noexcept { return item_infos_; }

    private:
        std::optional<std::unordered_map<std::string, ItemInfoClass>> item_infos_;
    };

} // namespace XFS4IoTFramework::CashManagement