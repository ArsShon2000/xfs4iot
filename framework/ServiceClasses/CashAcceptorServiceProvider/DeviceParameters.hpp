#pragma once

#include <map>
#include <vector>
#include <string>
#include <optional>
#include "../../server/DeviceResult.hpp"
#include "../../core/common/MessageHeader.hpp"
#include "../StorageServiceProvider/CashManagementPresentStatus.hpp"
#include "../CommonServiceProvider/CashManagementCapabilitiesClass.hpp"
#include <memory>
#include "../StorageServiceProvider/CashUnit.hpp"
#include "../../core/CashAcceptor/PreparePresentCompletion.hpp"
#include "../../core/CashAcceptor/Completions/CashInStartCompletion.hpp"
#include "../../core/CashAcceptor/Completions/CashInCompletion.hpp"
#include "../../core/CashAcceptor/CashInEndCompletion.hpp"
#include "../../core/CashAcceptor/CashInRollbackCompletion.hpp"
#include "../../core/CashAcceptor/Completions/ConfigureNoteTypesCompletion.hpp"
#include "../../core/CashAcceptor/ConfigureNoteReaderCompletion.hpp"
#include "../../core/CashAcceptor/CashUnitCountCompletion.hpp"
#include "../../core/CashAcceptor/PresentMediaCompletion.hpp"
#include "../../core/CashAcceptor/DeviceLockControlCompletion.hpp"

namespace XFS4IoTFramework::CashAcceptor
{

    // ============================================================================
    // CashInStart
    // ============================================================================

    /// <summary>
    /// CashInStartRequest - Before initiating a cash-in operation
    /// </summary>
    class CashInStartRequest
    {
    public:
        CashInStartRequest(
            std::optional<long> tellerID,
            std::optional<bool> useRecycleUnits,
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum outputPosition,
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum inputPosition,
            std::optional<long> totalItemsLimit,
            std::optional < std::unordered_map<std::string, double>> amountLimit)
            : tellerID(tellerID.value_or(0))
            , useRecycleUnits(useRecycleUnits)
            , outputPosition(outputPosition)
            , inputPosition(inputPosition)
            , totalItemsLimit(totalItemsLimit.value_or(0))
            , amountLimit(std::move(amountLimit))
        {
        }

        /// <summary>
        /// Identification of teller. Not applicable to Self-Service devices
        /// </summary>
        std::optional<long> tellerID;

        /// <summary>
        /// Specifies whether or not the recycle storage units should be used when items are cashed in
        /// </summary>
        std::optional<bool> useRecycleUnits;

        /// <summary>
        /// The output position where items will be presented to customer in case of rollback
        /// </summary>
        XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum outputPosition;

        /// <summary>
        /// The position the cash should be inserted
        /// </summary>
        XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum inputPosition;

        /// <summary>
        /// Limit on total number of items to be accepted. 0 = no limit
        /// </summary>
        std::optional<long> totalItemsLimit;

        /// <summary>
        /// Maximum amount of one or more currencies to be accepted
        /// </summary>
        std::optional < std::unordered_map<std::string, double>> amountLimit;
    };

    /// <summary>
    /// CashInStartResult
    /// </summary>
    class CashInStartResult : public XFS4IoTServer::DeviceResult
    {
    public:
        CashInStartResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            std::optional<std::string> errorDescription = std::nullopt,
            std::optional<XFS4IoT::CashAcceptor::Completions::CashInStartCompletionPayloadData::ErrorCodeEnum> errorCode = std::nullopt)
            : XFS4IoTServer::DeviceResult(completionCode, errorDescription)
            , errorCode(errorCode)
        {
        }

        std::optional<XFS4IoT::CashAcceptor::Completions::CashInStartCompletionPayloadData::ErrorCodeEnum> errorCode;
    };

    // ============================================================================
    // CashIn
    // ============================================================================

    /// <summary>
    /// CashInRequest
    /// </summary>
    class CashInRequest
    {
    public:
        explicit CashInRequest(int timeout) : timeout(timeout) {}

        /// <summary>
        /// Timeout for waiting customer to insert items
        /// </summary>
        int timeout;
    };

    /// <summary>
    /// CashInResult
    /// </summary>
    class CashInResult : public XFS4IoTServer::DeviceResult
    {
    public:

        // Error constructor
        CashInResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            std::optional<std::string> errorDescription = std::nullopt,
            std::optional<XFS4IoT::CashAcceptor::Completions::CashInCompletionPayloadData::ErrorCodeEnum> errorCode = std::nullopt)
            : XFS4IoTServer::DeviceResult(completionCode, errorDescription)
            , errorCode(errorCode)
            , unrecognized(0)
        {
        }

        // Success constructor with data
        CashInResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            std::map<std::string, XFS4IoTFramework::Storage::CashItemCountClass> itemCounts,
            std::optional < std::unordered_map<std::string, std::shared_ptr<XFS4IoTFramework::Storage::CashUnitCountClass>>> movementResult,
            int unrecognized = 0)
            : XFS4IoTServer::DeviceResult(completionCode, std::nullopt)
            , unrecognized(unrecognized)
            , itemCounts(std::move(itemCounts))
            , movementResult(std::move(movementResult))
        {
        }

        std::optional<XFS4IoT::CashAcceptor::Completions::CashInCompletionPayloadData::ErrorCodeEnum> errorCode;
        int unrecognized;
        std::optional<std::map<std::string, XFS4IoTFramework::Storage::CashItemCountClass>> itemCounts;

		// Информация о перемещении купюр по устройству, которая может быть полезна для приложения для определения, какие купюры были вставлены и в какие кассеты они были помещены. Ключом является идентификатор кассеты, а значением - объект CashUnitCountClass, содержащий информацию о количестве купюр, вставленных в эту кассету.
        std::optional < std::unordered_map<std::string, std::shared_ptr<XFS4IoTFramework::Storage::CashUnitCountClass>>> movementResult;
    };

    // ============================================================================
    // CashInEnd
    // ============================================================================

    /// <summary>
    /// CashInEndResult
    /// </summary>
    class CashInEndResult : public XFS4IoTServer::DeviceResult
    {
    public:

        CashInEndResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            std::optional<std::string> errorDescription = std::nullopt,
            std::optional<XFS4IoT::CashAcceptor::Completions::CashInEndPayloadData::ErrorCodeEnum> errorCode = std::nullopt)
            : XFS4IoTServer::DeviceResult(completionCode, errorDescription)
            , errorCode(errorCode)
        {
        }

        CashInEndResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            std::map<std::string, XFS4IoTFramework::Storage::CashUnitCountClass> movementResult)
            : XFS4IoTServer::DeviceResult(completionCode, std::nullopt)
            , movementResult(std::move(movementResult))
        {
        }

        std::optional<XFS4IoT::CashAcceptor::Completions::CashInEndPayloadData::ErrorCodeEnum> errorCode;
        std::optional<std::map<std::string, XFS4IoTFramework::Storage::CashUnitCountClass>> movementResult;
    };

    // ============================================================================
    // CashInRollback
    // ============================================================================

    /// <summary>
    /// CashInRollbackResult
    /// </summary>
    class CashInRollbackResult : public XFS4IoTServer::DeviceResult
    {
    public:

        CashInRollbackResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            std::optional<std::string> errorDescription = std::nullopt,
            std::optional<XFS4IoT::CashAcceptor::Completions::CashInRollbackPayloadData::ErrorCodeEnum> errorCode = std::nullopt)
            : XFS4IoTServer::DeviceResult(completionCode, errorDescription)
            , errorCode(errorCode)
        {
        }

        CashInRollbackResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            std::map<std::string, XFS4IoTFramework::Storage::CashUnitCountClass> movementResult)
            : XFS4IoTServer::DeviceResult(completionCode, std::nullopt)
            , movementResult(std::move(movementResult))
        {
        }

        std::optional<XFS4IoT::CashAcceptor::Completions::CashInRollbackPayloadData::ErrorCodeEnum> errorCode;
        std::optional<std::map<std::string, XFS4IoTFramework::Storage::CashUnitCountClass>> movementResult;
    };

    // ============================================================================
    // ConfigureNoteTypes
    // ============================================================================

    /// <summary>
    /// ConfigureNoteTypesRequest
    /// </summary>
    class ConfigureNoteTypesRequest
    {
    public:
        explicit ConfigureNoteTypesRequest(std::unordered_map<std::string, bool> banknoteItems)
            : banknoteItems(std::move(banknoteItems))
        {
        }

        /// <summary>
        /// Banknote items to be enabled/recognized
        /// </summary>
        std::unordered_map<std::string, bool> banknoteItems;
    };

    /// <summary>
    /// ConfigureNoteTypesResult
    /// </summary>
    class ConfigureNoteTypesResult : public XFS4IoTServer::DeviceResult
    {
    public:

        ConfigureNoteTypesResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            std::optional<std::string> errorDescription = std::nullopt,
            std::optional<XFS4IoT::CashAcceptor::Completions::ConfigureNoteTypesCompletionPayloadData::ErrorCodeEnum> errorCode = std::nullopt)
            : XFS4IoTServer::DeviceResult(completionCode, errorDescription)
            , errorCode(errorCode)
        {
        }

        std::optional<XFS4IoT::CashAcceptor::Completions::ConfigureNoteTypesCompletionPayloadData::ErrorCodeEnum> errorCode;
    };

    // ============================================================================
    // ConfigureNoteReader
    // ============================================================================

    /// <summary>
    /// ConfigureNoteReaderRequest
    /// </summary>
    class ConfigureNoteReaderRequest
    {
    public:
        explicit ConfigureNoteReaderRequest(bool loadAlways)
            : loadAlways(loadAlways)
        {
        }

        /// <summary>
        /// If true, load data even if already loaded
        /// </summary>
        bool loadAlways;
    };

    /// <summary>
    /// ConfigureNoteReaderResult
    /// </summary>
    class ConfigureNoteReaderResult : public XFS4IoTServer::DeviceResult
    {
    public:

        ConfigureNoteReaderResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            std::optional<std::string> errorDescription = std::nullopt,
            std::optional<XFS4IoT::CashAcceptor::Completions::ConfigureNoteReaderPayloadData::ErrorCodeEnum> errorCode = std::nullopt)
            : XFS4IoTServer::DeviceResult(completionCode, errorDescription)
            , errorCode(errorCode)
        {
        }

        std::optional<XFS4IoT::CashAcceptor::Completions::ConfigureNoteReaderPayloadData::ErrorCodeEnum> errorCode;
    };

    // ============================================================================
    // PreparePresent
    // ============================================================================

    /// <summary>
    /// PreparePresentRequest
    /// </summary>
    class PreparePresentRequest
    {
    public:
        explicit PreparePresentRequest(XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum outputPosition)
            : outputPosition(outputPosition)
        {
        }

        XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum outputPosition;
    };

    /// <summary>
    /// PreparePresentResult
    /// </summary>
    class PreparePresentResult : public XFS4IoTServer::DeviceResult
    {
    public:

        PreparePresentResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            std::optional<std::string> errorDescription = std::nullopt,
            std::optional<XFS4IoT::CashAcceptor::Completions::PreparePresentPayloadData::ErrorCodeEnum> errorCode = std::nullopt)
            : XFS4IoTServer::DeviceResult(completionCode, errorDescription)
            , errorCode(errorCode)
        {
        }

        std::optional<XFS4IoT::CashAcceptor::Completions::PreparePresentPayloadData::ErrorCodeEnum> errorCode;
    };

    // ============================================================================
    // CashUnitCount
    // ============================================================================

    /// <summary>
    /// CashUnitCountRequest
    /// </summary>
    class CashUnitCountRequest
    {
    public:
        explicit CashUnitCountRequest(std::vector<std::string> storageIds)
            : storageIds(std::move(storageIds))
        {
        }

        std::vector<std::string> storageIds;
    };

    /// <summary>
    /// CashUnitCountResult
    /// </summary>
    class CashUnitCountResult : public XFS4IoTServer::DeviceResult
    {
    public:

        CashUnitCountResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            std::optional<std::string> errorDescription = std::nullopt,
            std::optional<XFS4IoT::CashAcceptor::Completions::CashUnitCountPayloadData::ErrorCodeEnum> errorCode = std::nullopt)
            : XFS4IoTServer::DeviceResult(completionCode, errorDescription)
            , errorCode(errorCode)
        {
        }

        CashUnitCountResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            std::map<std::string, XFS4IoTFramework::Storage::CashUnitCountClass> movementResult)
            : XFS4IoTServer::DeviceResult(completionCode, std::nullopt)
            , movementResult(std::move(movementResult))
        {
        }

        std::optional<XFS4IoT::CashAcceptor::Completions::CashUnitCountPayloadData::ErrorCodeEnum> errorCode;
        std::optional<std::map<std::string, XFS4IoTFramework::Storage::CashUnitCountClass>> movementResult;
    };

    // ============================================================================
    // DeviceLock
    // ============================================================================

    /// <summary>
    /// DeviceLockRequest
    /// </summary>
    class DeviceLockRequest
    {
    public:
        enum class DeviceActionEnum
        {
            Lock,
            Unlock,
            NoLockAction
        };

        enum class CashUnitActionEnum
        {
            LockAll,
            UnlockAll,
            LockIndividual,
            NoLockAction
        };

        enum class UnitActionEnum
        {
            Lock,
            Unlock
        };

        DeviceLockRequest(
            DeviceActionEnum deviceAction,
            CashUnitActionEnum cashUnitAction,
            std::map<std::string, UnitActionEnum> unitLockControl)
            : deviceAction(deviceAction)
            , cashUnitAction(cashUnitAction)
            , unitLockControl(std::move(unitLockControl))
        {
        }

        DeviceActionEnum deviceAction;
        CashUnitActionEnum cashUnitAction;
        std::map<std::string, UnitActionEnum> unitLockControl;
    };

    /// <summary>
    /// DeviceLockResult
    /// </summary>
    class DeviceLockResult : public XFS4IoTServer::DeviceResult
    {
    public:

        DeviceLockResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            std::optional<std::string> errorDescription = std::nullopt,
            std::optional<XFS4IoT::CashAcceptor::Completions::DeviceLockControlPayloadData::ErrorCodeEnum> errorCode = std::nullopt)
            : XFS4IoTServer::DeviceResult(completionCode, errorDescription)
            , errorCode(errorCode)
        {
        }

        std::optional<XFS4IoT::CashAcceptor::Completions::DeviceLockControlPayloadData::ErrorCodeEnum> errorCode;
    };

    // ============================================================================
    // PresentMedia
    // ============================================================================

    /// <summary>
    /// PresentMediaRequest
    /// </summary>
    class PresentMediaRequest
    {
    public:
        PresentMediaRequest(
            XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position,
            std::shared_ptr<XFS4IoTFramework::CashManagement::CashManagementPresentStatus> currentPresentStatus)
            : position(position)
            , currentPresentStatus(std::move(currentPresentStatus))
        {
        }

        XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position;
        std::shared_ptr<XFS4IoTFramework::CashManagement::CashManagementPresentStatus> currentPresentStatus;
    };

    /// <summary>
    /// PresentMediaResult
    /// </summary>
    class PresentMediaResult : public XFS4IoTServer::DeviceResult
    {
    public:

        PresentMediaResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            std::optional<std::string> errorDescription = std::nullopt,
            std::optional<XFS4IoT::CashAcceptor::Completions::PresentMediaPayloadData::ErrorCodeEnum> errorCode = std::nullopt,
            std::shared_ptr<XFS4IoTFramework::CashManagement::CashManagementPresentStatus> lastPresentStatus = nullptr)
            : XFS4IoTServer::DeviceResult(completionCode, errorDescription)
            , errorCode(errorCode)
            , lastPresentStatus(lastPresentStatus)
        {
        }

        PresentMediaResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            std::shared_ptr<XFS4IoTFramework::CashManagement::CashManagementPresentStatus> lastPresentStatus)
            : XFS4IoTServer::DeviceResult(completionCode, std::nullopt)
            , lastPresentStatus(lastPresentStatus)
        {
        }

        std::optional<XFS4IoT::CashAcceptor::Completions::PresentMediaPayloadData::ErrorCodeEnum> errorCode;
        std::shared_ptr<XFS4IoTFramework::CashManagement::CashManagementPresentStatus> lastPresentStatus;
    };
}