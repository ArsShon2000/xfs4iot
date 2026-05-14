#pragma once

#include <optional>
#include <string>
#include "StatusBase.hpp"
#include "CashManagementCapabilitiesClass.hpp"
#include "../../core/Events/PropertyChangedEventArgs.hpp"

namespace XFS4IoTFramework::Common
{
    /// <summary>
    /// Cash Management status class
    /// </summary>
    class CashManagementStatusClass 
        : public StatusBase
        , public XFS4IoTServer::NotifyPropertyChangedBase
        , public std::enable_shared_from_this<CashManagementStatusClass>
    {
    public:
        /// <summary>
        /// Position status enumeration
        /// </summary>
        enum class PositionStatusEnum
        {
            Empty,
            NotEmpty,
            Unknown,
            NotSupported,
        };

        /// <summary>
        /// Transport enumeration
        /// </summary>
        enum class TransportEnum
        {
            Ok,
            Inoperative,
            Unknown,
            NotSupported,
        };

        /// <summary>
        /// Transport status enumeration
        /// </summary>
        enum class TransportStatusEnum
        {
            Empty,
            NotEmpty,
            NotEmptyCustomer,
            Unknown,
            NotSupported,
        };

        /// <summary>
        /// Dispenser enumeration
        /// </summary>
        enum class DispenserEnum
        {
            Ok,
            Attention,
            Stop,
            Unknown,
            NotSupported,
        };

        /// <summary>
        /// Acceptor enumeration
        /// </summary>
        enum class AcceptorEnum
        {
            Ok,
            Attention,
            Stop,
            Unknown,
            NotSupported,
        };

        /// <summary>
        /// Shutter enumeration
        /// </summary>
        enum class ShutterEnum
        {
            Closed,
            Open,
            JammedOpen,
            JammedPartiallyOpen,
            JammedClosed,
            JammedUnknown,
            Unknown,
            NotSupported,
        };

        /// <summary>
        /// Position status class - nested class for individual position status
        /// </summary>
        class PositionStatusClass : public StatusBase
            , public XFS4IoTServer::NotifyPropertyChangedBase
        {
        public:
            PositionStatusClass(ShutterEnum shutter,
                PositionStatusEnum positionStatus,
                TransportEnum transport,
                TransportStatusEnum transportStatus)
                : shutter_(shutter)
                , positionStatus_(positionStatus)
                , transport_(transport)
                , transportStatus_(transportStatus)
            {
            }

            PositionStatusClass()
                : shutter_(ShutterEnum::NotSupported)
                , positionStatus_(PositionStatusEnum::NotSupported)
                , transport_(TransportEnum::NotSupported)
                , transportStatus_(TransportStatusEnum::NotSupported)
            {
            }

            // Явный копирующий конструктор.
            // Не копируем внутреннюю мутацию/состояние NotifyPropertyChangedBase (std::mutex некопируем).
            PositionStatusClass(const PositionStatusClass& other)
                : StatusBase(other) // скопировать состояние StatusBase, если оно копируемо
                , XFS4IoTServer::NotifyPropertyChangedBase() // создать новую базу уведомлений
                , cashDispenserPosition_(other.cashDispenserPosition_)
                , cashAcceptorPosition_(other.cashAcceptorPosition_)
                , shutter_(other.shutter_)
                , positionStatus_(other.positionStatus_)
                , transport_(other.transport_)
                , transportStatus_(other.transportStatus_)
            {
            }

            // Перемещающий конструктор (без переноса состояния NotifyPropertyChangedBase)
            PositionStatusClass(PositionStatusClass&& other) noexcept
                : StatusBase(std::move(other))
                , XFS4IoTServer::NotifyPropertyChangedBase()
                , cashDispenserPosition_(std::move(other.cashDispenserPosition_))
                , cashAcceptorPosition_(std::move(other.cashAcceptorPosition_))
                , shutter_(other.shutter_)
                , positionStatus_(other.positionStatus_)
                , transport_(other.transport_)
                , transportStatus_(other.transportStatus_)
            {
            }

            /// <summary>
            /// This property is set by the framework to generate status changed event
            /// </summary>
            std::optional<CashManagementCapabilitiesClass::OutputPositionEnum> GetCashDispenserPosition() const
            {
                return cashDispenserPosition_;
            }

            void SetCashDispenserPosition(
                std::optional<CashManagementCapabilitiesClass::OutputPositionEnum> position)
            {
                cashDispenserPosition_ = position;
            }

            std::optional<CashManagementCapabilitiesClass::PositionEnum> GetCashAcceptorPosition() const
            {
                return cashAcceptorPosition_;
            }

            void SetCashAcceptorPosition(
                std::optional<CashManagementCapabilitiesClass::PositionEnum> position)
            {
                cashAcceptorPosition_ = position;
            }

            /// <summary>
            /// Supplies the state of the shutter. Following values are possible:
            /// 
            /// * Closed - The shutter is operational and is closed.
            /// * Open - The shutter is operational and is open.
            /// * JammedOpen - The shutter is jammed and is not operational (in open position).
            /// * JammedPartiallyOpen - The shutter is jammed partially open.
            /// * JammedClosed - The shutter is jammed in closed position.
            /// * JammedUnknown - The shutter is jammed in unknown position.
            /// * Unknown - Due to a hardware error or other condition, the state of the shutter cannot be determined.
            /// * NotSupported - The physical device has no shutter or shutter state reporting is not supported.
            /// </summary>
            ShutterEnum GetShutter() const { return shutter_; }

            void SetShutter(ShutterEnum value)
            {
                if (shutter_ != value) {
                    shutter_ = value;
                    raisePropertyChanged("Shutter");
                }
            }

            /// <summary>
            /// Returns information regarding items which may be at the output position.
            /// If the device is a recycler it is possible that the output position will not be empty 
            /// due to a previous cash-in operation. Following values are possible:
            /// 
            /// * Empty - The output position is empty.
            /// * NotEmpty - The output position is not empty.
            /// * Unknown - Due to a hardware error or other condition, the state cannot be determined.
            /// * NotSupported - The device is not capable of reporting whether items are at the output position.
            /// </summary>
            PositionStatusEnum GetPositionStatus() const { return positionStatus_; }

            void SetPositionStatus(PositionStatusEnum value)
            {
                if (positionStatus_ != value) {
                    positionStatus_ = value;
                    raisePropertyChanged("PositionStatus");
                }
            }

            /// <summary>
            /// Supplies the state of the transport mechanism. The transport is defined as any area 
            /// leading to or from the position. Following values are possible:
            /// 
            /// * Ok - The transport is in a good state.
            /// * Inoperative - The transport is inoperative due to a hardware failure or media jam.
            /// * Unknown - Due to a hardware error or other condition the state cannot be determined.
            /// * NotSupported - The physical device has no transport or transport state reporting is not supported.
            /// </summary>
            TransportEnum GetTransport() const { return transport_; }

            void SetTransport(TransportEnum value)
            {
                if (transport_ != value) {
                    transport_ = value;
                    raisePropertyChanged("Transport");
                }
            }

            /// <summary>
            /// Returns information regarding items which may be on the transport. If the device is a recycler
            /// device it is possible that the transport will not be empty due to a previous cash-in operation.
            /// Following values are possible:
            /// 
            /// * Empty - The transport is empty.
            /// * NotEmpty - The transport is not empty.
            /// * NotEmptyCustomer - Items which a customer has had access to are on the transport.
            /// * Unknown - Due to a hardware error it is not known whether there are items on the transport.
            /// * NotSupported - The device is not capable of reporting whether items are on the transport.
            /// </summary>
            TransportStatusEnum GetTransportStatus() const { return transportStatus_; }

            void SetTransportStatus(TransportStatusEnum value)
            {
                if (transportStatus_ != value) {
                    transportStatus_ = value;
                    raisePropertyChanged("TransportStatus");
                }
            }

            // Явно определяем оператор присваивания
            PositionStatusClass& operator=(const PositionStatusClass& other)
            {
                if (this != &other)
                {
                    shutter_ = other.shutter_;
                    positionStatus_ = other.positionStatus_;
                    transport_ = other.transport_;
                    transportStatus_ = other.transportStatus_;
                    cashDispenserPosition_ = other.cashDispenserPosition_;
                    cashAcceptorPosition_ = other.cashAcceptorPosition_;
                }
                return *this;
            }

        private:
            std::optional<CashManagementCapabilitiesClass::OutputPositionEnum> cashDispenserPosition_;
            std::optional<CashManagementCapabilitiesClass::PositionEnum> cashAcceptorPosition_;
            ShutterEnum shutter_;
            PositionStatusEnum positionStatus_;
            TransportEnum transport_;
            TransportStatusEnum transportStatus_;
        };

        /// <summary>
        /// Default constructor
        /// </summary>
        CashManagementStatusClass()
            : dispenser_(DispenserEnum::NotSupported)
            , acceptor_(AcceptorEnum::NotSupported)
        {
        }

        /// <summary>
        /// Constructor with parameters
        /// </summary>
        CashManagementStatusClass(DispenserEnum dispenser, AcceptorEnum acceptor)
            : dispenser_(dispenser)
            , acceptor_(acceptor)
        {
        }

        /// <summary>
        /// Supplies the state of the storage units for dispensing cash. Following values are possible:
        /// 
        /// * Ok - All storage units present are in a good state.
        /// * Attention - One or more of the storage units is in a low, empty, inoperative or manipulated condition.
        ///   Items can still be dispensed from at least one of the storage units.
        /// * Stop - Due to a storage unit failure dispensing is impossible. No items can be dispensed because
        ///   all of the storage units are in an empty, inoperative or manipulated condition.
        /// * Unknown - Due to a hardware error or other condition, the state cannot be determined.
        /// </summary>
        DispenserEnum GetDispenser() const { return dispenser_; }

        void SetDispenser(DispenserEnum value)
        {
            if (dispenser_ != value) {
                dispenser_ = value;
                raisePropertyChanged("Dispenser");
            }
        }

        /// <summary>
        /// Supplies the state of the storage units for accepting cash. Following values are possible:
        /// 
        /// * Ok - All storage units present are in a good state.
        /// * Attention - One or more of the storage units is in a high, full, inoperative or manipulated condition.
        ///   Items can still be accepted into at least one of the storage units.
        /// * Stop - Due to a storage unit failure accepting is impossible. No items can be accepted because
        ///   all of the storage units are in a full, inoperative or manipulated condition.
        /// * Unknown - Due to a hardware error or other condition, the state cannot be determined.
        /// </summary>
        AcceptorEnum GetAcceptor() const { return acceptor_; }

        void SetAcceptor(AcceptorEnum value)
        {
            if (acceptor_ != value) {
                acceptor_ = value;
                raisePropertyChanged("Acceptor");
            }
        }

    private:
        DispenserEnum dispenser_;
        AcceptorEnum acceptor_;
    };

    // Helper functions for enum to string conversion
    inline std::string ToString(CashManagementStatusClass::ShutterEnum value)
    {
        switch (value) {
        case CashManagementStatusClass::ShutterEnum::Closed: return "closed";
        case CashManagementStatusClass::ShutterEnum::Open: return "open";
        case CashManagementStatusClass::ShutterEnum::JammedOpen: return "jammedOpen";
        case CashManagementStatusClass::ShutterEnum::JammedPartiallyOpen: return "jammedPartiallyOpen";
        case CashManagementStatusClass::ShutterEnum::JammedClosed: return "jammedClosed";
        case CashManagementStatusClass::ShutterEnum::JammedUnknown: return "jammedUnknown";
        case CashManagementStatusClass::ShutterEnum::Unknown: return "unknown";
        case CashManagementStatusClass::ShutterEnum::NotSupported: return "notSupported";
        default: return "unknown";
        }
    }

    inline std::string ToString(CashManagementStatusClass::PositionStatusEnum value)
    {
        switch (value) {
        case CashManagementStatusClass::PositionStatusEnum::Empty: return "empty";
        case CashManagementStatusClass::PositionStatusEnum::NotEmpty: return "notEmpty";
        case CashManagementStatusClass::PositionStatusEnum::Unknown: return "unknown";
        case CashManagementStatusClass::PositionStatusEnum::NotSupported: return "notSupported";
        default: return "unknown";
        }
    }

    inline std::string ToString(CashManagementStatusClass::DispenserEnum value)
    {
        switch (value) {
        case CashManagementStatusClass::DispenserEnum::Ok: return "ok";
        case CashManagementStatusClass::DispenserEnum::Attention: return "attention";
        case CashManagementStatusClass::DispenserEnum::Stop: return "stop";
        case CashManagementStatusClass::DispenserEnum::Unknown: return "unknown";
        case CashManagementStatusClass::DispenserEnum::NotSupported: return "notSupported";
        default: return "unknown";
        }
    }

    inline std::string ToString(CashManagementStatusClass::AcceptorEnum value)
    {
        switch (value) {
        case CashManagementStatusClass::AcceptorEnum::Ok: return "ok";
        case CashManagementStatusClass::AcceptorEnum::Attention: return "attention";
        case CashManagementStatusClass::AcceptorEnum::Stop: return "stop";
        case CashManagementStatusClass::AcceptorEnum::Unknown: return "unknown";
        case CashManagementStatusClass::AcceptorEnum::NotSupported: return "notSupported";
        default: return "unknown";
        }
    }
}