#include "CashManagementServiceClass.hpp"
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include "../../core/Persistent/IPersistentData.hpp"
#include "../../core/Validation.hpp"
#include "ItemClassificationClass.hpp"
#include "../../core/Exceptions.hpp"
#include "../../core/CashManagement/Events/MediaDetectedEvent.hpp"

namespace XFS4IoTServer
{
    CashManagementServiceClass::CashManagementServiceClass(
        std::shared_ptr<IServiceProvider> serviceProvider,
        std::shared_ptr<ILogger> logger,
        std::shared_ptr<IPersistentData> persistentData)
        : serviceProvider_(Validation::isNotNull(serviceProvider, "serviceProvider"))
        , logger_(logger)
        , persistentData_(Validation::isNotNull(
            persistentData,
            "В CashManagementServiceClass не установлен интерфейс постоянных данных"))
        , cashInStatusManaged_(nullptr)
        , lastCashManagementPresentStatus_()
        , itemClassificationList_(nullptr)
    {
        device_ = std::dynamic_pointer_cast<XFS4IoTFramework::CashManagement::ICashManagementDevice>(
            Validation::isNotNull(
                serviceProvider_->GetDevice(),
                "Получен недопустимый параметр в конструкторе CashManagementServiceClass: Device"));

        if (!device_)
        {
            throw std::invalid_argument(
                "Устройство не реализует интерфейс ICashManagementDevice");
        }

        commonService_ = std::dynamic_pointer_cast<XFS4IoTFramework::Common::ICommonService>(serviceProvider_);
        if (!commonService_)
        {
            throw std::invalid_argument(
                "Указан недопустимый параметр интерфейса для общего сервиса: CashManagementServiceClass");
        }

        storageService_ = std::dynamic_pointer_cast<XFS4IoTFramework::Storage::IStorageService>(serviceProvider_);
        if (!storageService_)
        {
            throw std::invalid_argument(
                "Указан недопустимый параметр интерфейса для сервиса хранения: CashManagementServiceClass");
        }

        getStatus();
        GetCapabilities();

        // Load CashInStatus
        std::string cashInStatusKey = serviceProvider_->GetName() + typeid(XFS4IoTFramework::CashManagement::CashInStatusClass).name();
        auto loadedCashInStatus = persistentData_->Load<XFS4IoTFramework::CashManagement::CashInStatusClass>(cashInStatusKey);

        if (loadedCashInStatus)
        {
            cashInStatusManaged_ = std::make_shared<XFS4IoTFramework::CashManagement::CashInStatusClass>(*loadedCashInStatus);
        }
        else
        {
            cashInStatusManaged_ = std::make_shared<XFS4IoTFramework::CashManagement::CashInStatusClass>();
            StoreCashInStatus();
        }

        //    /* */
        //// Load LastCashManagementPresentStatus
        //std::string presentStatusKey = serviceProvider_->GetName() + typeid(XFS4IoTFramework::CashManagement::CashManagementPresentStatus).name();
        //using CashManagementPresentStatusMapPersistent =
        //    std::unordered_map<
        //    std::string,
        //    XFS4IoTFramework::CashManagement::CashManagementPresentStatus>;

        //auto loaded = persistentData_->Load<CashManagementPresentStatusMapPersistent>(presentStatusKey);

        //lastCashManagementPresentStatus_.clear();

        //if (loaded)
        //{
        //    for (auto& [positionStr, value] : *loaded)
        //    {
        //        auto pos = XFS4IoTFramework::Common::PositionEnumFromString(positionStr);
        //        lastCashManagementPresentStatus_[pos] =
        //            std::make_shared<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>(std::move(value));
        //    }
        //}
        //else
        //{
        //    initializeDefaultPresentStatus();
        //    StoreCashManagementPresentStatus();
        //}

        //// Load ItemClassificationList
        //std::string classificationListKey = serviceProvider_->GetName() + typeid(XFS4IoTFramework::CashManagement::ItemClassificationListClass).name();
        //auto loadedClassificationList = persistentData_->Load<XFS4IoTFramework::CashManagement::ItemClassificationListClass>(classificationListKey);

        //if (loadedClassificationList)
        //{
        //    itemClassificationList_ = std::make_shared<XFS4IoTFramework::CashManagement::ItemClassificationListClass>(
        //        *loadedClassificationList);
        //}
        //else
        //{
        //    itemClassificationList_ = std::make_shared<XFS4IoTFramework::CashManagement::ItemClassificationListClass>();
        //    StoreItemClassificationList();
        //}
    }

    CashManagementServiceClass::~CashManagementServiceClass() = default;

    void CashManagementServiceClass::initializeDefaultPresentStatus()
    {
        using PositionEnum = XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum;

        lastCashManagementPresentStatus_ = {
            { PositionEnum::InBottom, std::make_shared<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>() },
            { PositionEnum::InCenter, std::make_shared<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>() },
            { PositionEnum::InDefault, std::make_shared<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>() },
            { PositionEnum::InFront, std::make_shared<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>() },
            { PositionEnum::InLeft, std::make_shared<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>() },
            { PositionEnum::InRear, std::make_shared<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>() },
            { PositionEnum::InRight, std::make_shared<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>() },
            { PositionEnum::InTop, std::make_shared<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>() },
            { PositionEnum::OutBottom, std::make_shared<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>() },
            { PositionEnum::OutCenter, std::make_shared<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>() },
            { PositionEnum::OutDefault, std::make_shared<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>() },
            { PositionEnum::OutFront, std::make_shared<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>() },
            { PositionEnum::OutLeft, std::make_shared<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>() },
            { PositionEnum::OutRear, std::make_shared<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>() },
            { PositionEnum::OutRight, std::make_shared<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>() },
            { PositionEnum::OutTop, std::make_shared<XFS4IoTFramework::CashManagement::CashManagementPresentStatus>() }
        };
    }

    void CashManagementServiceClass::getStatus()
    {
        logger_->trace(std::format("{}() - DevClass: CashManagementDev.CashManagementStatus", __FUNCTION__));
        auto status = device_->GetCashManagementStatus();
        commonService_->SetCashManagementStatus(status);
        logger_->trace(std::format("{}() - DevClass: CashManagementDev.CashManagementStatus = {}", __FUNCTION__, static_cast<const void*>(status.get())));

        Validation::isNotNull(
            commonService_->GetCashManagementStatus(),
            "Класс устройства установил свойство CashManagementStatus в значение null. "
            "Класс устройства должен сообщать о состоянии устройства.");

        // Subscribe to property changed events
        commonService_->GetCashManagementStatus()->onPropertyChanged(
            [this](auto sender, auto propertyInfo) {
                statusChangedEventForwarder(sender, propertyInfo);
            });
    }

    void CashManagementServiceClass::GetCapabilities()
    {
        logger_->trace(std::format("{}() - DevClass: CashManagementDev.CashManagementCapabilities", __FUNCTION__));
        auto capabilities = device_->GetCashManagementCapabilities();
        commonService_->SetCashManagementCapabilities(capabilities);
        logger_->trace(std::format("{}() - DevClass: CashManagementDev.CashManagementCapabilities = {}", __FUNCTION__, static_cast<const void*>(capabilities.get())));

        Validation::isNotNull(
            commonService_->GetCashManagementCapabilities(),
            "Класс устройства установил свойство CashManagementCapabilities в значение null. "
            "Класс устройства должен сообщать о возможностях устройства.");
    }

    void CashManagementServiceClass::StoreCashInStatus()
    {
        //std::string key = serviceProvider_->GetName() + typeid(XFS4IoTFramework::CashManagement::CashInStatusClass).name();
        //if (!persistentData_->Store(key, *cashInStatusManaged_))
        //{
        //    logger_->warn(std::format("{}() - Framework: Failed to save persistent data. {}", __FUNCTION__, key));
        //}
    }

    void CashManagementServiceClass::StoreCashManagementPresentStatus()
    {
        const std::string storageKey =
            serviceProvider_->GetName() +
            typeid(XFS4IoTFramework::CashManagement::CashManagementPresentStatus).name();

        using CashManagementPresentStatusMapPlain =
            std::unordered_map<
            std::string,
            XFS4IoTFramework::CashManagement::CashManagementPresentStatus>;

        CashManagementPresentStatusMapPlain plainUnits;

        for (const auto& [position, value] : lastCashManagementPresentStatus_)
        {
            if (value) {
                plainUnits.emplace(PositionEnumToString(position), *value);
            }
        }

        //if (!persistentData_->Store(storageKey, plainUnits))
        //{
        //    logger_->warn(std::format(
        //        "{}() - Framework: Failed to save persistent data. {}",
        //        __FUNCTION__, storageKey));
        //}
    }

    void CashManagementServiceClass::StoreItemClassificationList()
    {
        //std::string key = serviceProvider_->GetName() + typeid(XFS4IoTFramework::CashManagement::ItemClassificationListClass).name();
        //if (!persistentData_->Store(key, *itemClassificationList_))
        //{
        //    logger_->warn(std::format("{}() - Framework: Failed to save persistent data. {}", __FUNCTION__, key));
        //}
    }

    XFS4IoT::CashManagement::PositionEnum CashManagementServiceClass::convertPositionToXFS4IoT(
        XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position)
    {
        using PositionEnum = XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum;
        // Assuming XFS4IoT::CashManagement::PositionEnum has same values

        switch (position)
        {
        case PositionEnum::InBottom: return XFS4IoT::CashManagement::PositionEnum::InBottom; // XFS4IoT::CashManagement::PositionEnum::InBottom
        case PositionEnum::InCenter: return XFS4IoT::CashManagement::PositionEnum::InCenter;
        case PositionEnum::InDefault: return XFS4IoT::CashManagement::PositionEnum::InDefault;
        case PositionEnum::InFront: return XFS4IoT::CashManagement::PositionEnum::InFront;
        case PositionEnum::InLeft: return XFS4IoT::CashManagement::PositionEnum::InLeft;
        case PositionEnum::InRear: return XFS4IoT::CashManagement::PositionEnum::InRear;
        case PositionEnum::InRight: return XFS4IoT::CashManagement::PositionEnum::InRight;
        case PositionEnum::InTop: return XFS4IoT::CashManagement::PositionEnum::InTop;
        case PositionEnum::OutBottom: return XFS4IoT::CashManagement::PositionEnum::OutBottom;
        case PositionEnum::OutCenter: return XFS4IoT::CashManagement::PositionEnum::OutCenter;
        case PositionEnum::OutDefault: return XFS4IoT::CashManagement::PositionEnum::OutDefault;
        case PositionEnum::OutFront: return XFS4IoT::CashManagement::PositionEnum::OutFront;
        case PositionEnum::OutLeft: return XFS4IoT::CashManagement::PositionEnum::OutLeft;
        case PositionEnum::OutRear: return XFS4IoT::CashManagement::PositionEnum::OutRear;
        case PositionEnum::OutRight: return XFS4IoT::CashManagement::PositionEnum::OutRight;
        case PositionEnum::OutTop: return XFS4IoT::CashManagement::PositionEnum::OutTop;
        default:
            throw XFS4IoT::InternalErrorException(
                std::string("Invalid position is specified: ") +
                std::to_string(static_cast<int>(position)));
        }
    }

    XFS4IoT::CashManagement::ItemTargetEnumEnum CashManagementServiceClass::convertTargetToXFS4IoT(XFS4IoTFramework::CashManagement::ItemTargetEnum target)
    {
        switch (target)
        {
        case XFS4IoTFramework::CashManagement::ItemTargetEnum::SingleUnit: return XFS4IoT::CashManagement::ItemTargetEnumEnum::SingleUnit;
        case XFS4IoTFramework::CashManagement::ItemTargetEnum::Retract: return XFS4IoT::CashManagement::ItemTargetEnumEnum::Retract;
        case XFS4IoTFramework::CashManagement::ItemTargetEnum::Transport: return XFS4IoT::CashManagement::ItemTargetEnumEnum::Transport;
        case XFS4IoTFramework::CashManagement::ItemTargetEnum::Stacker: return XFS4IoT::CashManagement::ItemTargetEnumEnum::Stacker;
        case XFS4IoTFramework::CashManagement::ItemTargetEnum::Reject: return XFS4IoT::CashManagement::ItemTargetEnumEnum::Reject;
        case XFS4IoTFramework::CashManagement::ItemTargetEnum::ItemCassette: return XFS4IoT::CashManagement::ItemTargetEnumEnum::ItemCassette;
        case XFS4IoTFramework::CashManagement::ItemTargetEnum::CashIn: return XFS4IoT::CashManagement::ItemTargetEnumEnum::CashIn;
        case XFS4IoTFramework::CashManagement::ItemTargetEnum::OutDefault: return XFS4IoT::CashManagement::ItemTargetEnumEnum::OutDefault;
        case XFS4IoTFramework::CashManagement::ItemTargetEnum::OutLeft: return XFS4IoT::CashManagement::ItemTargetEnumEnum::OutLeft;
        case XFS4IoTFramework::CashManagement::ItemTargetEnum::OutRight: return XFS4IoT::CashManagement::ItemTargetEnumEnum::OutRight;
        case XFS4IoTFramework::CashManagement::ItemTargetEnum::OutCenter: return XFS4IoT::CashManagement::ItemTargetEnumEnum::OutCenter;
        case XFS4IoTFramework::CashManagement::ItemTargetEnum::OutTop: return XFS4IoT::CashManagement::ItemTargetEnumEnum::OutTop;
        case XFS4IoTFramework::CashManagement::ItemTargetEnum::OutBottom: return XFS4IoT::CashManagement::ItemTargetEnumEnum::OutBottom;
        case XFS4IoTFramework::CashManagement::ItemTargetEnum::OutFront: return XFS4IoT::CashManagement::ItemTargetEnumEnum::OutFront;
        case XFS4IoTFramework::CashManagement::ItemTargetEnum::OutRear: return XFS4IoT::CashManagement::ItemTargetEnumEnum::OutRear;
        default:
            throw XFS4IoT::InternalErrorException(
                std::string("Unsupported target value specified: ") +
                std::to_string(static_cast<int>(target)));
        }
    }

    boost::asio::awaitable<void> CashManagementServiceClass::ItemsTakenEvent(
        XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position,
        std::optional<std::string> additionalBunches)
    {
        // Create payload
        auto payload = std::make_shared<XFS4IoT::CashManagement::Events::ItemsTakenEventPayloadData>(
            convertPositionToXFS4IoT(position),
            additionalBunches ? std::nullopt : additionalBunches);

        co_await ItemsTakenEventImpl(payload);
    }

    boost::asio::awaitable<void> CashManagementServiceClass::ItemsInsertedEvent(
        XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position)
    {
        auto payload = std::make_shared<XFS4IoT::CashManagement::Events::ItemsInsertedEventPayloadData>(
            convertPositionToXFS4IoT(position));

        co_await ItemsInsertedEventImpl(payload);
    }

    boost::asio::awaitable<void> CashManagementServiceClass::ItemsPresentedEvent(
        XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum position,
        std::optional<std::string> additionalBunches)
    {
        auto payload = std::make_shared<XFS4IoT::CashManagement::Events::ItemsPresentedEventPayloadData>(
            convertPositionToXFS4IoT(position),
            additionalBunches);

        co_await ItemsPresentedEventImpl(payload);
    }

    boost::asio::awaitable<void> CashManagementServiceClass::MediaDetectedEvent(
        std::optional<std::string> storageId, 
        XFS4IoTFramework::CashManagement::ItemTargetEnum target, 
        std::optional<int> index)
    {
        std::optional<std::string> unit = std::nullopt;

        if (target == XFS4IoTFramework::CashManagement::ItemTargetEnum::SingleUnit)
        {
            if (!storageId.has_value() || storageId->empty())
            {
                co_return; // или throw InvalidDataException
            }

            unit = storageId;
        }

        auto payload = std::make_shared<
            XFS4IoT::CashManagement::Events::MediaDetectedEventPayloadData>(
                convertTargetToXFS4IoT(target),
                std::move(unit),
                index);

        co_await MediaDetectedEventImpl(payload);
    }

/*   boost::asio::awaitable<void> CashManagementServiceClass::MediaDetectedEvent(
        const std::string& storageId,
       XFS4IoTFramework::CashManagement::ItemTargetEnum target,
        std::optional<int> index)
    {
        const auto& cashUnits = storageService_->GetCashUnits();

        Validation::isFalse(
            cashUnits.empty(),
            "Попытка отправить событие MediaDetectedEvent при отсутствии настроенных единиц хранения.");

        std::string unitName;
        for (const auto& [key, unit] : cashUnits)
        {
            if (unit->GetId() == storageId)
            {
                unitName = key;
                break;
            }
        }

        if (unitName.empty() && target ==XFS4IoTFramework::CashManagement::ItemTargetEnum::SingleUnit)
        {
            throw std::runtime_error(
                std::string("Элементы перемещены в single unit, но отсутствует ") +
                "StorageId, соответствующий указанному типу устройства. " + storageId);
        }

        auto payload = std::make_shared<XFS4IoT::CashManagement::Events::MediaDetectedEventPayloadData>(
            convertTargetToXFS4IoT(target),
            unitName.empty() ? std::nullopt : std::optional<std::string>(unitName),
            index);

        co_await MediaDetectedEventImpl(payload);
    }*/

    void CashManagementServiceClass::statusChangedEventForwarder(
        std::shared_ptr<void> sender,
        std::shared_ptr<PropertyChangedEventArgs> propertyInfo)
    {
        boost::asio::co_spawn(
            serviceProvider_->getIoContext(),
            commonService_->StatusChangedEvent(sender, propertyInfo),
            boost::asio::detached);
    }
}