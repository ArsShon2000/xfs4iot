#include "GetInfoCapabilities.h"
#include "XFSCommon/XfsIdentifiersToText.h" 
#include <MODULE_STRUCTURES/StructFillers/FillByExtra.h>

using namespace FS365::Utilities::XfsIdentifiersToText;

    GetInfoCapabilities* GetInfoCapabilities::pinstance_ = nullptr;
    std::mutex GetInfoCapabilities::m_sendMutex;
    std::mutex GetInfoCapabilities::m_instanceMutex;

namespace Xfs::Cim {

    void GetInfoCapabilities::Initialize()
    {
        if (!extra.empty()) extra.clear();
        // В экстра-параметрах идентифицируем модель аппарата
        FillByExtra::GetInstance()->fillExtra(extra);

        wClass = WFS_SERVICE_CLASS_CIM;
        deviceType = DeviceTypePackage::SELFSERVICEBILL;
        maxCashInItems = 1;
        compound = false;
        shutter = false;
        shutterControl = false;
        safeDoor = false;
        cashBox = false;
        intermediateStacker = m_psHandler->bConjointCashInIsActive ? 0 : 1;
        itemsTakenSensor = true;
        itemsInsertedSensor = true;
        refill = false;
        positions = PositionsPackage::POSOUTCENTER | PositionsPackage::POSINCENTER;
        exchangeType = ExchangeTypePackage::EXBYHAND;
        retractAreas = CIM_RA_NOTSUPP; // RA_NOTSUPP
        retractTransportActions = CIM_NOTSUPP; // NOTSUPP
        retractStackerActions = CIM_NOTSUPP; // NOTSUPP

        isGuidLightsSupported = false;
        itemInfoTypes = ItemInfoTypesPackage::ITEM_NOTSUPP;
        compareSignatures = false;
        powerSaveControl = false;


        bReplenish = false;
        fwCashInLimit = CIM_LIMITBYAMOUNT | CIM_LIMITBYTOTALITEMS;
        fwCountActions = CIM_COUNTNOTSUPP;
        bDeviceLockControl = false;
        wMixedMode = 0; // WFS_CIM_MIXEDMEDIANOTSUPP
        bMixedDepositAndRollback = false;
        bAntiFraudModule = false;
        bDeplete = false;
        bBlacklist = false;
        lpdwSynchronizableCommands = nullptr;
        bClassificationList = false;
        bPhysicalNoteList = false;

        // XFS4 будет использовать вместо extra следующие поля
        serviceVersion = "Версия физ сервиса";
        //deviceInformation
        modelName = m_psHandler->m_idn.strPartNumber;
        serialNumber = m_psHandler->m_idn.strSerialNumber;
        revisionNumber = "Что это";
        modelDescription = "Купюроприемник DORS210BA";
        // firmware прошивка
        firmwareName = "Firmware";
        firmwareVersion = "version";
        hardwareRevision = "version";
        // software ПО
        softwareName = "DORSHW";
        softwareVersion = "version";
    }

    void GetInfoCapabilities::fill_XFS3_Result(CimCaps &cashUnit)
    {
        FillByExtra::GetInstance()->fillExtra(extra);

        cashUnit.wClass = wClass;
        cashUnit.fwType = deviceType;
        cashUnit.wMaxCashInItems = maxCashInItems;
        cashUnit.bCompound = compound == TRUE;
        cashUnit.bShutter = shutter == TRUE;
        cashUnit.bShutterControl = shutterControl == TRUE;
        cashUnit.bSafeDoor = safeDoor == TRUE;
        cashUnit.bCashBox = cashBox == TRUE;
        cashUnit.bRefill = refill == TRUE;
        cashUnit.fwIntermediateStacker = intermediateStacker;
        cashUnit.bItemsTakenSensor = itemsTakenSensor == TRUE;
        cashUnit.bItemsInsertedSensor = itemsInsertedSensor == TRUE;
        cashUnit.fwPositions = positions;
        cashUnit.fwExchangeType = exchangeType;
        cashUnit.fwRetractAreas = retractAreas;
        cashUnit.fwRetractTransportActions = retractTransportActions;
        cashUnit.fwRetractStackerActions = retractStackerActions;
        cashUnit.extra = extra;
        cashUnit.dwItemInfoTypes = itemInfoTypes;
        cashUnit.bCompareSignatures = compareSignatures;
        cashUnit.bPowerSaveControl = powerSaveControl;

        cashUnit.bReplenish = bReplenish; // false;
        cashUnit.fwCashInLimit = fwCashInLimit; // WFS_CIM_LIMITBYAMOUNT | WFS_CIM_LIMITBYTOTALITEMS;
        cashUnit.fwCountActions = fwCountActions; // WFS_CIM_COUNTNOTSUPP;
        cashUnit.bDeviceLockControl = bDeviceLockControl; // false;
        cashUnit.wMixedMode = wMixedMode; // WFS_CIM_MIXEDMEDIANOTSUPP;
        cashUnit.bMixedDepositAndRollback = bMixedDepositAndRollback; // false;
        cashUnit.bAntiFraudModule = bAntiFraudModule;   // false;
        cashUnit.bDeplete = bDeplete; // false;
        cashUnit.bBlacklist = bBlacklist; // false;
        cashUnit.lpdwSynchronizableCommands = lpdwSynchronizableCommands; // nullptr;
        cashUnit.bClassificationList = bClassificationList; // false;
        cashUnit.bPhysicalNoteList = bPhysicalNoteList; // false;
    }
}
