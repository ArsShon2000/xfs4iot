#include "StorageErrorCommandEvent.hpp"
//#include "IStorageService.h"
//#include "StorageUnitClass.h"
//#include "StorageErrorEvent.h"
//
//// CashManagement event interfaces
//#include "ICalibrateCashUnitEvents.h"
//#include "IResetEvents.h"
//#include "IRetractEvents.h"
//
//// CashAcceptor event interfaces
//#include "ICashInEvents.h"
//#include "IReplenishEvents.h"
//#include "ICashInEndEvents.h"
//#include "ICashInRollbackEvents.h"
//#include "ICashUnitCountEvents.h"
//#include "IPreparePresentEvents.h"
//#include "IDepleteEvents.h"
//
//// CashDispenser event interfaces
//#include "IDispenseEvents.h"
//#include "IRejectEvents.h"
//#include "ITestCashUnitsEvents.h"
//#include "ICountEvents.h"

#include <stdexcept>
#include "../../../core/Validation.hpp"

namespace XFS4IoTFramework::Storage
{
#pragma region CashManagement Constructors

    StorageErrorCommandEvent::StorageErrorCommandEvent(
        std::shared_ptr<IStorageService> storageService,
        std::shared_ptr<XFS4IoTFramework::CashManagement::ICalibrateCashUnitEvents> events)
        : storageService_(nullptr)
        , retractEvents_(nullptr)
        , cashManagementResetEvents_(nullptr)
        , calibrateCashUnitEvents_(nullptr)
        , dispenseEvents_(nullptr)
        , rejectEvents_(nullptr)
        , testCashUnitsEvents_(nullptr)
        , countEvents_(nullptr)
        , cashInEvents_(nullptr)
        , cashInEndEvents_(nullptr)
        , cashInRollbackEvents_(nullptr)
        , preparePresentEvents_(nullptr)
        , cashUnitCountEvents_(nullptr)
        //, depleteEvents_(nullptr)
        , replenishEvents_(nullptr)
    {
        XFS4IoTServer::Validation::isA<IStorageService>(
            storageService,
            std::string("Передан недопустимый интерфейс хранилища. StorageErrorCommandEvent"));
        storageService_ = storageService;

        XFS4IoTServer::Validation::isNotNull(events, "Передан недопустимый параметр. StorageErrorCommandEvent");
        XFS4IoTServer::Validation::isA<CashManagement::ICalibrateCashUnitEvents>(
            events,
            std::string("Передан недопустимый интерфейс. StorageErrorCommandEvent"));
        calibrateCashUnitEvents_ = events;
    }

    StorageErrorCommandEvent::StorageErrorCommandEvent(
        std::shared_ptr<IStorageService> storageService,
        std::shared_ptr<XFS4IoTFramework::CashManagement::IResetEvents> events)
        : storageService_(nullptr)
        , retractEvents_(nullptr)
        , cashManagementResetEvents_(nullptr)
        , calibrateCashUnitEvents_(nullptr)
        , dispenseEvents_(nullptr)
        , rejectEvents_(nullptr)
        , testCashUnitsEvents_(nullptr)
        , countEvents_(nullptr)
        , cashInEvents_(nullptr)
        , cashInEndEvents_(nullptr)
        , cashInRollbackEvents_(nullptr)
        , preparePresentEvents_(nullptr)
        , cashUnitCountEvents_(nullptr)
        //, depleteEvents_(nullptr)
        , replenishEvents_(nullptr)
    {
        XFS4IoTServer::Validation::isA<IStorageService>(
            storageService,
            std::string("Передан недопустимый интерфейс хранилища. StorageErrorCommandEvent"));
        storageService_ = storageService;

        XFS4IoTServer::Validation::isNotNull(events, "Передан недопустимый параметр. StorageErrorCommandEvent");
        XFS4IoTServer::Validation::isA<CashManagement::IResetEvents>(
            events,
            std::string("Передан недопустимый интерфейс. StorageErrorCommandEvent"));
        cashManagementResetEvents_ = events;
    }

    StorageErrorCommandEvent::StorageErrorCommandEvent(
        std::shared_ptr<IStorageService> storageService,
        std::shared_ptr<CashManagement::IRetractEvents> events)
        : storageService_(nullptr)
        , retractEvents_(nullptr)
        , cashManagementResetEvents_(nullptr)
        , calibrateCashUnitEvents_(nullptr)
        , dispenseEvents_(nullptr)
        , rejectEvents_(nullptr)
        , testCashUnitsEvents_(nullptr)
        , countEvents_(nullptr)
        , cashInEvents_(nullptr)
        , cashInEndEvents_(nullptr)
        , cashInRollbackEvents_(nullptr)
        , preparePresentEvents_(nullptr)
        , cashUnitCountEvents_(nullptr)
        //, depleteEvents_(nullptr)
        , replenishEvents_(nullptr)
    {
        XFS4IoTServer::Validation::isA<IStorageService>(
            storageService,
            std::string("Передан недопустимый интерфейс хранилища. StorageErrorCommandEvent"));
        storageService_ = storageService;

        XFS4IoTServer::Validation::isNotNull(events, "Передан недопустимый параметр. StorageErrorCommandEvent");
        XFS4IoTServer::Validation::isA<CashManagement::IRetractEvents>(
            events,
            std::string("Передан недопустимый интерфейс. StorageErrorCommandEvent"));
        retractEvents_ = events;
    }

#pragma endregion

#pragma region CashAcceptor Constructors

    StorageErrorCommandEvent::StorageErrorCommandEvent(
        std::shared_ptr<IStorageService> storageService,
        std::shared_ptr<CashAcceptor::ICashInEvents> events)
        : storageService_(nullptr)
        , retractEvents_(nullptr)
        , cashManagementResetEvents_(nullptr)
        , calibrateCashUnitEvents_(nullptr)
        , dispenseEvents_(nullptr)
        , rejectEvents_(nullptr)
        , testCashUnitsEvents_(nullptr)
        , countEvents_(nullptr)
        , cashInEvents_(nullptr)
        , cashInEndEvents_(nullptr)
        , cashInRollbackEvents_(nullptr)
        , preparePresentEvents_(nullptr)
        , cashUnitCountEvents_(nullptr)
        //, depleteEvents_(nullptr)
        , replenishEvents_(nullptr)
    {
        XFS4IoTServer::Validation::isA<IStorageService>(
            storageService,
            std::string("Передан недопустимый интерфейс хранилища. StorageErrorCommandEvent"));
        storageService_ = storageService;

        XFS4IoTServer::Validation::isNotNull(events, "Передан недопустимый параметр. StorageErrorCommandEvent");
        XFS4IoTServer::Validation::isA<CashAcceptor::ICashInEvents>(
            events,
            std::string("Передан недопустимый интерфейс. StorageErrorCommandEvent"));
        cashInEvents_ = events;
    }

    StorageErrorCommandEvent::StorageErrorCommandEvent(
        std::shared_ptr<IStorageService> storageService,
        std::shared_ptr<CashAcceptor::IReplenishEvents> events)
        : storageService_(nullptr)
        , retractEvents_(nullptr)
        , cashManagementResetEvents_(nullptr)
        , calibrateCashUnitEvents_(nullptr)
        , dispenseEvents_(nullptr)
        , rejectEvents_(nullptr)
        , testCashUnitsEvents_(nullptr)
        , countEvents_(nullptr)
        , cashInEvents_(nullptr)
        , cashInEndEvents_(nullptr)
        , cashInRollbackEvents_(nullptr)
        , preparePresentEvents_(nullptr)
        , cashUnitCountEvents_(nullptr)
        //, depleteEvents_(nullptr)
        , replenishEvents_(nullptr)
    {
        XFS4IoTServer::Validation::isA<IStorageService>(
            storageService,
            std::string("Передан недопустимый интерфейс хранилища. StorageErrorCommandEvent"));
        storageService_ = storageService;

        XFS4IoTServer::Validation::isNotNull(events, "Передан недопустимый параметр. StorageErrorCommandEvent");
        XFS4IoTServer::Validation::isA<CashAcceptor::IReplenishEvents>(
            events,
            std::string("Передан недопустимый интерфейс. StorageErrorCommandEvent"));
        replenishEvents_ = events;
    }

    StorageErrorCommandEvent::StorageErrorCommandEvent(
        std::shared_ptr<IStorageService> storageService,
        std::shared_ptr<XFS4IoTFramework::CashAcceptor::ICashInEndEvents> events)
        : storageService_(nullptr)
        , retractEvents_(nullptr)
        , cashManagementResetEvents_(nullptr)
        , calibrateCashUnitEvents_(nullptr)
        , dispenseEvents_(nullptr)
        , rejectEvents_(nullptr)
        , testCashUnitsEvents_(nullptr)
        , countEvents_(nullptr)
        , cashInEvents_(nullptr)
        , cashInEndEvents_(nullptr)
        , cashInRollbackEvents_(nullptr)
        , preparePresentEvents_(nullptr)
        , cashUnitCountEvents_(nullptr)
        //, depleteEvents_(nullptr)
        , replenishEvents_(nullptr)
    {
        XFS4IoTServer::Validation::isA<IStorageService>(
            storageService,
            std::string("Передан недопустимый интерфейс хранилища. StorageErrorCommandEvent"));
        storageService_ = storageService;

        XFS4IoTServer::Validation::isNotNull(events, "Передан недопустимый параметр. StorageErrorCommandEvent");
        XFS4IoTServer::Validation::isA<XFS4IoTFramework::CashAcceptor::ICashInEndEvents>(
            events,
            std::string("Передан недопустимый интерфейс. StorageErrorCommandEvent"));
        cashInEndEvents_ = events;
    }

        StorageErrorCommandEvent::StorageErrorCommandEvent(
            std::shared_ptr<IStorageService> storageService,
            std::shared_ptr<XFS4IoTFramework::CashManagement::ICashInRollbackEvents> events)
        : storageService_(nullptr)
        , retractEvents_(nullptr)
        , cashManagementResetEvents_(nullptr)
        , calibrateCashUnitEvents_(nullptr)
        , dispenseEvents_(nullptr)
        , rejectEvents_(nullptr)
        , testCashUnitsEvents_(nullptr)
        , countEvents_(nullptr)
        , cashInEvents_(nullptr)
        , cashInEndEvents_(nullptr)
        , cashInRollbackEvents_(nullptr)
        , preparePresentEvents_(nullptr)
        , cashUnitCountEvents_(nullptr)
        //, depleteEvents_(nullptr)
        , replenishEvents_(nullptr)
    {
        XFS4IoTServer::Validation::isA<IStorageService>(
            storageService,
            std::string("Передан недопустимый интерфейс хранилища. StorageErrorCommandEvent"));
        storageService_ = storageService;

        XFS4IoTServer::Validation::isNotNull(events, "Передан недопустимый параметр. StorageErrorCommandEvent");
        XFS4IoTServer::Validation::isA<XFS4IoTFramework::CashManagement::ICashInRollbackEvents>(
            events,
            std::string("Передан недопустимый интерфейс. StorageErrorCommandEvent"));
        cashInRollbackEvents_ = events;
    }



    StorageErrorCommandEvent::StorageErrorCommandEvent(
        std::shared_ptr<IStorageService> storageService,
        std::shared_ptr<XFS4IoTFramework::CashManagement::ICashUnitCountEvents> events)
        : storageService_(nullptr)
        , retractEvents_(nullptr)
        , cashManagementResetEvents_(nullptr)
        , calibrateCashUnitEvents_(nullptr)
        , dispenseEvents_(nullptr)
        , rejectEvents_(nullptr)
        , testCashUnitsEvents_(nullptr)
        , countEvents_(nullptr)
        , cashInEvents_(nullptr)
        , cashInEndEvents_(nullptr)
        , cashInRollbackEvents_(nullptr)
        , preparePresentEvents_(nullptr)
        , cashUnitCountEvents_(nullptr)
        //, depleteEvents_(nullptr)
        , replenishEvents_(nullptr)
    {
        XFS4IoTServer::Validation::isA<IStorageService>(
            storageService,
            std::string("Передан недопустимый интерфейс хранилища. StorageErrorCommandEvent"));
        storageService_ = storageService;

        XFS4IoTServer::Validation::isNotNull(events, "Передан недопустимый параметр. StorageErrorCommandEvent");
        XFS4IoTServer::Validation::isA<XFS4IoTFramework::CashManagement::ICashUnitCountEvents>(
            events,
            std::string("Передан недопустимый интерфейс. StorageErrorCommandEvent"));
        cashUnitCountEvents_ = events;
    }



    StorageErrorCommandEvent::StorageErrorCommandEvent(
        std::shared_ptr<IStorageService> storageService,
        std::shared_ptr<XFS4IoTFramework::CashManagement::IPreparePresentEvents> events)
        : storageService_(nullptr)
        , retractEvents_(nullptr)
        , cashManagementResetEvents_(nullptr)
        , calibrateCashUnitEvents_(nullptr)
        , dispenseEvents_(nullptr)
        , rejectEvents_(nullptr)
        , testCashUnitsEvents_(nullptr)
        , countEvents_(nullptr)
        , cashInEvents_(nullptr)
        , cashInEndEvents_(nullptr)
        , cashInRollbackEvents_(nullptr)
        , preparePresentEvents_(nullptr)
        , cashUnitCountEvents_(nullptr)
        //, depleteEvents_(nullptr)
        , replenishEvents_(nullptr)
    {
        XFS4IoTServer::Validation::isA<IStorageService>(
            storageService,
            std::string("Передан недопустимый интерфейс хранилища. StorageErrorCommandEvent"));
        storageService_ = storageService;

        XFS4IoTServer::Validation::isNotNull(events, "Передан недопустимый параметр. StorageErrorCommandEvent");
        XFS4IoTServer::Validation::isA<XFS4IoTFramework::CashManagement::IPreparePresentEvents>(
            events,
            std::string("Передан недопустимый интерфейс. StorageErrorCommandEvent"));
        preparePresentEvents_ = events;
    }

    // Аналогично для остальных конструкторов CashAcceptor...
    // (CashInEnd, CashInRollback, CashUnitCount, PreparePresent, Deplete)

#pragma endregion

#pragma region CashDispenser Constructors

    StorageErrorCommandEvent::StorageErrorCommandEvent(
        std::shared_ptr<IStorageService> storageService,
        std::shared_ptr<CashDispenser::IDispenseEvents> events)
        : storageService_(nullptr)
        , retractEvents_(nullptr)
        , cashManagementResetEvents_(nullptr)
        , calibrateCashUnitEvents_(nullptr)
        , dispenseEvents_(nullptr)
        , rejectEvents_(nullptr)
        , testCashUnitsEvents_(nullptr)
        , countEvents_(nullptr)
        , cashInEvents_(nullptr)
        , cashInEndEvents_(nullptr)
        , cashInRollbackEvents_(nullptr)
        , preparePresentEvents_(nullptr)
        , cashUnitCountEvents_(nullptr)
        //, depleteEvents_(nullptr)
        , replenishEvents_(nullptr)
    {
        XFS4IoTServer::Validation::isA<IStorageService>(
            storageService,
            std::string("Передан недопустимый интерфейс хранилища. StorageErrorCommandEvent"));
        storageService_ = storageService;

        XFS4IoTServer::Validation::isNotNull(events, "Передан недопустимый параметр. StorageErrorCommandEvent");
        XFS4IoTServer::Validation::isA<CashDispenser::IDispenseEvents>(
            events,
            std::string("Передан недопустимый интерфейс. StorageErrorCommandEvent"));
        dispenseEvents_ = events;
    }

    StorageErrorCommandEvent::StorageErrorCommandEvent(
        std::shared_ptr<IStorageService> storageService,
        std::shared_ptr<CashDispenser::IRejectEvents> events)
        : storageService_(nullptr)
        , retractEvents_(nullptr)
        , cashManagementResetEvents_(nullptr)
        , calibrateCashUnitEvents_(nullptr)
        , dispenseEvents_(nullptr)
        , rejectEvents_(nullptr)
        , testCashUnitsEvents_(nullptr)
        , countEvents_(nullptr)
        , cashInEvents_(nullptr)
        , cashInEndEvents_(nullptr)
        , cashInRollbackEvents_(nullptr)
        , preparePresentEvents_(nullptr)
        , cashUnitCountEvents_(nullptr)
        //, depleteEvents_(nullptr)
        , replenishEvents_(nullptr)
    {
        XFS4IoTServer::Validation::isA<IStorageService>(
            storageService,
            std::string("Передан недопустимый интерфейс хранилища. StorageErrorCommandEvent"));
        storageService_ = storageService;

        XFS4IoTServer::Validation::isNotNull(events, "Передан недопустимый параметр. StorageErrorCommandEvent");
        XFS4IoTServer::Validation::isA<CashDispenser::IRejectEvents>(
            events,
            std::string("Передан недопустимый интерфейс. StorageErrorCommandEvent"));
        rejectEvents_ = events;
    }

    StorageErrorCommandEvent::StorageErrorCommandEvent(
        std::shared_ptr<IStorageService> storageService,
        std::shared_ptr<CashDispenser::ITestCashUnitsEvents> events)
        : storageService_(nullptr)
        , retractEvents_(nullptr)
        , cashManagementResetEvents_(nullptr)
        , calibrateCashUnitEvents_(nullptr)
        , dispenseEvents_(nullptr)
        , rejectEvents_(nullptr)
        , testCashUnitsEvents_(nullptr)
        , countEvents_(nullptr)
        , cashInEvents_(nullptr)
        , cashInEndEvents_(nullptr)
        , cashInRollbackEvents_(nullptr)
        , preparePresentEvents_(nullptr)
        , cashUnitCountEvents_(nullptr)
        //, depleteEvents_(nullptr)
        , replenishEvents_(nullptr)
    {
        XFS4IoTServer::Validation::isA<IStorageService>(
            storageService,
            std::string("Передан недопустимый интерфейс хранилища. StorageErrorCommandEvent"));
        storageService_ = storageService;

        XFS4IoTServer::Validation::isNotNull(events, "Передан недопустимый параметр. StorageErrorCommandEvent");
        XFS4IoTServer::Validation::isA<CashDispenser::IDispenseEvents>(
            events,
            std::string("Передан недопустимый интерфейс. StorageErrorCommandEvent"));
        testCashUnitsEvents_ = events;
    }

    StorageErrorCommandEvent::StorageErrorCommandEvent(
        std::shared_ptr<IStorageService> storageService,
        std::shared_ptr<CashDispenser::ICountEvents> events)
        : storageService_(nullptr)
        , retractEvents_(nullptr)
        , cashManagementResetEvents_(nullptr)
        , calibrateCashUnitEvents_(nullptr)
        , dispenseEvents_(nullptr)
        , rejectEvents_(nullptr)
        , testCashUnitsEvents_(nullptr)
        , countEvents_(nullptr)
        , cashInEvents_(nullptr)
        , cashInEndEvents_(nullptr)
        , cashInRollbackEvents_(nullptr)
        , preparePresentEvents_(nullptr)
        , cashUnitCountEvents_(nullptr)
        //, depleteEvents_(nullptr)
        , replenishEvents_(nullptr)
    {
        XFS4IoTServer::Validation::isA<IStorageService>(
            storageService,
            std::string("Передан недопустимый интерфейс хранилища. StorageErrorCommandEvent"));
        storageService_ = storageService;

        XFS4IoTServer::Validation::isNotNull(events, "Передан недопустимый параметр. StorageErrorCommandEvent");
        XFS4IoTServer::Validation::isA<CashDispenser::ICountEvents>(
            events,
            std::string("Передан недопустимый интерфейс. StorageErrorCommandEvent"));
        countEvents_ = events;
    }

    // Аналогично для остальных конструкторов CashDispenser...
    // (Reject, TestCashUnits, Count)

#pragma endregion

    boost::asio::awaitable<void> StorageErrorCommandEvent::StorageErrorEvent(
        FailureEnum failure,
        const std::vector<std::string>& unitIds)
    {
        auto storages = storageService_->GetStorages(unitIds);

        // Convert FailureEnum to XFS4IoT enum
        ::XFS4IoT::Storage::Events::StorageErrorEventPayloadData::FailureEnum xfsFailure;
        switch (failure)
        {
        case FailureEnum::Config:
            xfsFailure = ::XFS4IoT::Storage::Events::StorageErrorEventPayloadData::FailureEnum::Config;
            break;
        case FailureEnum::Empty:
            xfsFailure = ::XFS4IoT::Storage::Events::StorageErrorEventPayloadData::FailureEnum::Empty;
            break;
        case FailureEnum::Error:
            xfsFailure = ::XFS4IoT::Storage::Events::StorageErrorEventPayloadData::FailureEnum::Error;
            break;
        case FailureEnum::Full:
            xfsFailure = ::XFS4IoT::Storage::Events::StorageErrorEventPayloadData::FailureEnum::Full;
            break;
        case FailureEnum::Invalid:
            xfsFailure = ::XFS4IoT::Storage::Events::StorageErrorEventPayloadData::FailureEnum::Invalid;
            break;
        case FailureEnum::Locked:
            xfsFailure = ::XFS4IoT::Storage::Events::StorageErrorEventPayloadData::FailureEnum::Locked;
            break;
        default:
            xfsFailure = ::XFS4IoT::Storage::Events::StorageErrorEventPayloadData::FailureEnum::NotConfigured;
            break;
        }

        auto payload = std::make_shared<::XFS4IoT::Storage::Events::StorageErrorEventPayloadData>(
            xfsFailure,
            storages);

#pragma region CashManagement
        if (calibrateCashUnitEvents_)
        {
            co_await calibrateCashUnitEvents_->StorageErrorEvent(*payload);
            co_return;
        }
        if (cashManagementResetEvents_)
        {
            co_await cashManagementResetEvents_->StorageErrorEvent(payload);
            co_return;
        }
        if (retractEvents_)
        {
            co_await retractEvents_->StorageErrorEvent(*payload);
            co_return;
        }
#pragma endregion

#pragma region CashAcceptor
        if (cashInEvents_)
        {
            co_await cashInEvents_->StorageErrorEvent(*payload);
            co_return;
        }
        if (cashInEndEvents_)
        {
            co_await cashInEndEvents_->StorageErrorEvent(payload);
            co_return;
        }
        if (cashInRollbackEvents_)
        {
            co_await cashInRollbackEvents_->StorageErrorEvent(*payload);
            co_return;
        }
        if (cashUnitCountEvents_)
        {
            co_await cashUnitCountEvents_->StorageErrorEvent(*payload);
            co_return;
        }
        if (preparePresentEvents_)
        {
            co_await preparePresentEvents_->StorageErrorEvent(*payload);
            co_return;
        }
        if (replenishEvents_)
        {
            co_await replenishEvents_->StorageErrorEvent(*payload);
            co_return;
        }
      /*  if (depleteEvents_)
        {
            co_await depleteEvents_->StorageErrorEvent(payload);
            co_return;
        }*/
#pragma endregion

#pragma region CashDispenser
        if (dispenseEvents_)
        {
            co_await dispenseEvents_->StorageErrorEvent(*payload);
            co_return;
        }
        if (rejectEvents_)
        {
            co_await rejectEvents_->StorageErrorEvent(*payload);
            co_return;
        }
        if (testCashUnitsEvents_)
        {
            co_await testCashUnitsEvents_->StorageErrorEvent(*payload);
            co_return;
        }
        if (countEvents_)
        {
            co_await countEvents_->StorageErrorEvent(*payload);
            co_return;
        }
#pragma endregion

        throw std::logic_error("Unreachable code. StorageErrorEvent");
    }
}