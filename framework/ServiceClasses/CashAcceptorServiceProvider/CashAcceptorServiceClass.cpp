#include "CashAcceptorServiceClass.hpp"
#include "../../server/IServiceProvider.hpp"
#include "../CashAcceptorServiceProvider/ICashAcceptorDevice.hpp"
#include "../CommonServiceProvider/ICommonService.hpp"
#include "../StorageServiceProvider/IStorageService.hpp"
#include "../CashManagementServiceProvider/ICashManagementService.hpp"
#include "DeviceLockStatusClass.hpp"
#include "../CommonServiceProvider/CashAcceptorCapabilitiesClass.hpp"
#include "../CommonServiceProvider/CashAcceptorStatusClass.hpp"
//#include "CashAcceptorPositionStatusClass.hpp"
//#include "CashUnitStorage.hpp"
//#include "PropertyChangedEventArgs.hpp"
//#include "Validation.hpp"
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include "../../core/Validation.hpp"

namespace XFS4IoTServer
{
    CashAcceptorServiceClass::CashAcceptorServiceClass(
        std::shared_ptr<IServiceProvider> serviceProvider,
        std::shared_ptr<ILogger> logger)
        : serviceProvider_(serviceProvider)
        , logger_(logger)
        , commonService_(nullptr)
        , cashManagement_(nullptr)
        , storage_(nullptr)
        , cashInStatus_(nullptr)
        , deviceLockStatus_()
        , depleteCashUnitSources_()
        , replenishTargets_()
    {
        device_ = std::dynamic_pointer_cast<XFS4IoTFramework::CashAcceptor::ICashAcceptorDevice>(
            serviceProvider_->GetDevice());

        if (!device_)
        {
            throw std::invalid_argument(
                "Устройство не реализует интерфейс ICashAcceptorDevice");
        }

        commonService_ = std::dynamic_pointer_cast<XFS4IoTFramework::Common::ICommonService>(serviceProvider_);
        if (!commonService_)
        {
            throw std::invalid_argument(
                "Указан недопустимый параметр интерфейса для общего сервиса: CashAcceptorServiceClass");
        }

        cashManagement_ = std::dynamic_pointer_cast<XFS4IoTFramework::CashManagement::ICashManagementService>(
            serviceProvider_);
        if (!cashManagement_)
        {
            throw std::invalid_argument(
                "Указан недопустимый параметр интерфейса для сервиса управления наличными: CashAcceptorServiceClass");
        }

        storage_ = std::dynamic_pointer_cast<XFS4IoTFramework::Storage::IStorageService>(serviceProvider_);
        if (!storage_)
        {
            throw std::invalid_argument(
                "Указан недопустимый параметр интерфейса для сервиса хранения: CashAcceptorServiceClass");
        }

        GetCapabilities();
        getStatus();

        // Get CashAcceptor specific status and capabilities
        logger_->trace(std::format("{}() - Server: CashAcceptorDev.CashInStatus", __FUNCTION__));
        cashInStatus_ = device_->GetCashInStatus();
        logger_->trace(std::format("{}() - Server: CashAcceptorDev.CashInStatus = {}", __FUNCTION__, static_cast<const void*>(cashInStatus_.get())));

        if (!cashInStatus_)
        {
            logger_->trace(std::format("{}() - Server: Класс устройства не поддерживает CashInStatus", __FUNCTION__));
            // The device class doesn't support cash-in status and use it managed by the framework.
            auto managedStatus = cashManagement_->GetCashInStatusManaged();
            if (managedStatus) {
                cashInStatus_ = std::make_shared<XFS4IoTFramework::CashManagement::CashInStatusClass>(*managedStatus);
            }
            else {
                cashInStatus_ = nullptr;
            }
        }

        logger_->trace(std::format("{}() - Server: CashAcceptorDev.DeviceLockStatus", __FUNCTION__));
        deviceLockStatus_ = device_->GetDeviceLockStatus();
        logger_->trace(std::format("{}() - Server: CashAcceptorDev.DeviceLockStatus = {}", __FUNCTION__, static_cast<const void*>(&deviceLockStatus_)));

        logger_->trace(std::format("{}() - Server: CashAcceptorDev.GetDepleteCashUnitSources()", __FUNCTION__));
        depleteCashUnitSources_ = device_->GetDepleteCashUnitSources();
        logger_->trace(std::format("{}() - Server: CashAcceptorDev.GetDepleteCashUnitSources -> {}", __FUNCTION__, static_cast<const void*>(&depleteCashUnitSources_)));

        if (!depleteCashUnitSources_.empty())
        {
            // Check valid storage id for all sources
            const auto& cashUnits = storage_->GetCashUnits();

            for (const auto& [targetKey, sources] : depleteCashUnitSources_)
            {
                Validation::isTrue(
                    cashUnits.contains(targetKey),
                    std::string("Недопустимый идентификатор хранилища, предоставленный поставщиком сервиса ") +
                    "для целевого хранилища операции изъятия (deplete). " + targetKey);

                for (const auto& source : sources)
                {
                    Validation::isTrue(
                        cashUnits.contains(source),
                        std::string("Недопустимый идентификатор хранилища, предоставленный поставщиком сервиса ") +
                        "для исходного хранилища операции изъятия (deplete). " + source);
                }
            }
        }

        logger_->trace(std::format("{}() - Server: CashAcceptorDev.ReplenishTargets()", __FUNCTION__));
        replenishTargets_ = device_->GetReplenishTargets();
        logger_->trace(std::format("{}() - Server: CashAcceptorDev.ReplenishTargets() -> {}", __FUNCTION__, replenishTargets_));

        if (!replenishTargets_.empty())
        {
            // Check valid storage id for all targets
            const auto& cashUnits = storage_->GetCashUnits();

            for (const auto& target : replenishTargets_)
            {
                Validation::isTrue(
                    cashUnits.contains(target),
                    std::string("Недопустимый идентификатор хранилища, предоставленный поставщиком сервиса ") +
                    "для целевого хранилища операции пополнения (replenish). " + target);
            }
        }
    }

    CashAcceptorServiceClass::~CashAcceptorServiceClass() = default;

    void CashAcceptorServiceClass::GetCapabilities()
    {
        logger_->trace(std::format("{}() - Server: CashAcceptorDev.CashAcceptorCapabilities", __FUNCTION__));
        auto capabilities = device_->GetCashAcceptorCapabilities();
        commonService_->SetCashAcceptorCapabilities(std::make_shared<XFS4IoTFramework::Common::CashAcceptorCapabilitiesClass>(capabilities));
        logger_->trace(std::format("{}() - Server: CashAcceptorDev.CashAcceptorCapabilities=", __FUNCTION__));

        Validation::isNotNull(
            commonService_->GetCashAcceptorCapabilities(),
            "Класс устройства установил свойство CashAcceptorCapabilities в значение null. "
            "Класс устройства должен сообщать о возможностях устройства.");
    }

    void CashAcceptorServiceClass::getStatus()
    {
        logger_->trace(std::format("{}() - Server: CashAcceptorDev.CashAcceptorStatus", __FUNCTION__));
        const XFS4IoTFramework::Common::CashAcceptorStatusClass& status = device_->GetCashAcceptorStatus();
        commonService_->SetCashAcceptorStatus(std::make_shared<XFS4IoTFramework::Common::CashAcceptorStatusClass>(status));
        logger_->trace(std::format("{}() - Server: CashAcceptorDev.CashAcceptorStatus = {}", __FUNCTION__, static_cast<const void*>(&status)));

        auto cashAcceptorStatus = commonService_->GetCashAcceptorStatus();

        Validation::isNotNull(
            cashAcceptorStatus,
            "Класс устройства установил свойство CashAcceptorStatus в значение null. "
            "Класс устройства должен сообщать о состоянии устройства.");

        // Подписка на PropertyChanged события
        cashAcceptorStatus->onPropertyChanged(
            [this](std::shared_ptr<void> sender, std::shared_ptr<PropertyChangedEventArgs> propertyInfo) {
                statusChangedEventForwarder(sender, propertyInfo);
            });

        // ИСПРАВЛЕНО: GetPositions() возвращает ссылку, а не указатель
        auto& positions = cashAcceptorStatus->GetPositions();
        if (!positions.empty())  // Убрана проверка positions && !positions->empty()
        {
            for (auto& [positionKey, positionValue] : positions)
            {
                if (!positionValue.GetCashAcceptorPosition().has_value())  // Убран ->
                {
                    // Set the position for the first time
                    positionValue.SetCashAcceptorPosition(std::make_optional(positionKey));

                    // Если PositionStatusClass тоже поддерживает PropertyChanged:
                    positionValue.onPropertyChanged(
                        [this](std::shared_ptr<void> sender, std::shared_ptr<PropertyChangedEventArgs> propertyInfo) {
                            statusChangedEventForwarder(sender, propertyInfo);
                        });
                }
                else
                {
                    // Combine positions using bitwise OR
                    auto currentPosition = positionValue.GetCashAcceptorPosition().value();
                    positionValue.SetCashAcceptorPosition(
                        static_cast<XFS4IoTFramework::Common::CashManagementCapabilitiesClass::PositionEnum>(
                            static_cast<uint32_t>(currentPosition) | static_cast<uint32_t>(positionKey)));
                }
            }
        }
    }

    void CashAcceptorServiceClass::statusChangedEventForwarder(
        std::shared_ptr<void> sender,
        std::shared_ptr<PropertyChangedEventArgs> propertyInfo)
    {
        boost::asio::co_spawn(
            serviceProvider_->getIoContext(),
            commonService_->StatusChangedEvent(sender, propertyInfo),
            boost::asio::detached);
    }
}