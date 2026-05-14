#pragma once

#include <string>
#include <optional>
#include "../../server/DeviceResult.hpp"
#include "../../core/common/MessageHeader.hpp"

namespace XFS4IoTFramework::Common
{
    /// <summary>
    /// GetCommandNonceResult
    /// Return authorisation token for a command
    /// </summary>
    class GetCommandNonceResult : public XFS4IoTServer::DeviceResult
    {
    public:
        /// <summary>
        /// Constructor
        /// </summary>
        GetCommandNonceResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            std::optional<std::string> errorDescription = std::nullopt,
            std::optional<std::string> nonce = std::nullopt)
            : DeviceResult(completionCode, errorDescription)
            , nonce_(nonce)
        {
        }

        /// <summary>
        /// A nonce that should be included in the authorisation token in a command used to provide
        /// end to end protection.
        /// 
        /// The nonce will be given as HEX (upper case.)
        /// </summary>
        std::optional<std::string> GetNonce() const { return nonce_; }

    private:
        std::optional<std::string> nonce_;
    };

    /// <summary>
    /// Transaction state
    /// </summary>
    enum class TransactionStateEnum
    {
        /// <summary>
        /// A customer transaction is in progress.
        /// </summary>
        Active,

        /// <summary>
        /// No customer transaction is in progress.
        /// </summary>
        Inactive
    };

    /// <summary>
    /// GetTransactionStateResult
    /// Return transaction state
    /// </summary>
    class GetTransactionStateResult : public XFS4IoTServer::DeviceResult
    {
    public:
        /// <summary>
        /// Constructor for error case
        /// </summary>
        GetTransactionStateResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            std::optional<std::string> errorDescription)
            : DeviceResult(completionCode, errorDescription)
            , state_(std::nullopt)
            , transactionID_(std::nullopt)
        {
        }

        /// <summary>
        /// Constructor for success case
        /// </summary>
        GetTransactionStateResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum completionCode,
            TransactionStateEnum state,
            std::optional<std::string> transactionID)
            : DeviceResult(completionCode, std::nullopt)
            , state_(state)
            , transactionID_(transactionID)
        {
        }

        /// <summary>
        /// Specifies the transaction state. Following values are possible:
        /// 
        /// "active": A customer transaction is in progress.
        /// 
        /// "inactive": No customer transaction is in progress.
        /// </summary>
        std::optional<TransactionStateEnum> GetState() const { return state_; }

        /// <summary>
        /// Specifies a string which identifies the transaction ID. The value returned in this
        /// parameter is an application defined customer transaction identifier, which was previously 
        /// set in the Common.SetTransactionState command
        /// </summary>
        std::optional<std::string> GetTransactionID() const { return transactionID_; }

    private:
        std::optional<TransactionStateEnum> state_;
        std::optional<std::string> transactionID_;
    };

    /// <summary>
    /// SetTransactionStateRequest
    /// Set transaction information to the device
    /// </summary>
    class SetTransactionStateRequest
    {
    public:
        /// <summary>
        /// Constructor
        /// </summary>
        SetTransactionStateRequest(
            TransactionStateEnum state,
            std::optional<std::string> transactionID)
            : state_(state)
            , transactionID_(transactionID)
        {
        }

        /// <summary>
        /// Specifies the transaction state. Following values are possible:
        /// 
        /// "active": A customer transaction is in progress.
        /// 
        /// "inactive": No customer transaction is in progress.
        /// </summary>
        TransactionStateEnum GetState() const { return state_; }

        /// <summary>
        /// Specifies a string which identifies the transaction ID. The value returned in this
        /// parameter is an application defined customer transaction identifier, which was previously 
        /// set in the Common.SetTransactionState command
        /// </summary>
        std::optional<std::string> GetTransactionID() const { return transactionID_; }

    private:
        TransactionStateEnum state_;
        std::optional<std::string> transactionID_;
    };

    /// <summary>
    /// Helper function to convert TransactionStateEnum to string
    /// </summary>
    inline std::string ToString(TransactionStateEnum state)
    {
        switch (state)
        {
        case TransactionStateEnum::Active:
            return "active";
        case TransactionStateEnum::Inactive:
            return "inactive";
        default:
            return "unknown";
        }
    }

    /// <summary>
    /// Helper function to convert string to TransactionStateEnum
    /// </summary>
    inline TransactionStateEnum TransactionStateEnumFromString(const std::string& str)
    {
        if (str == "active") return TransactionStateEnum::Active;
        if (str == "inactive") return TransactionStateEnum::Inactive;
        throw std::invalid_argument("Invalid TransactionStateEnum string: " + str);
    }

} // namespace XFS4IoTFramework::Common