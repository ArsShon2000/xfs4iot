#include "GetInfoCashUnit.h"
#include <XFSCommon/XfsIdentifiersToText.h>

using namespace FS365::Utilities::XfsIdentifiersToText;

GetInfoCashUnit* GetInfoCashUnit::pinstance_ = nullptr;
std::mutex GetInfoCashUnit::m_sendMutex;
std::mutex GetInfoCashUnit::m_instanceMutex;

namespace Xfs::Cim {
    void GetInfoCashUnit::update(const CimCashUnit& cashUnit)
    {
        usNumber = cashUnit.usNumber;
        fwType = cashUnit.fwType;
        fwItemType = cashUnit.fwItemType;
        strUnitID = cashUnit.strUnitID;
        strCurrencyID = cashUnit.strCurrencyID;
        ulValues = cashUnit.ulValues;
        ulCashInCount = cashUnit.ulCashInCount;
        ulCount = cashUnit.ulCount;
        ulMaximum = cashUnit.ulMaximum;
        usStatus = cashUnit.usStatus;
        bAppLock = cashUnit.bAppLock;
        noteNumberList = cashUnit.noteNumberList;
        physicalCashUnitList = cashUnit.physicalCUs;
        noteIDs = cashUnit.noteIDs;
        usCDMType = cashUnit.usCDMType;
        strCashUnitName = cashUnit.strCashUnitName;
        ulInitialCount = cashUnit.ulInitialCount;
        ulDispensedCount = cashUnit.ulDispensedCount;
        ulPresentedCount = cashUnit.ulPresentedCount;
        ulRetractedCount = cashUnit.ulRetractedCount;
        ulRejectCount = cashUnit.ulRejectCount;
        ulMinimum = cashUnit.ulMinimum;
    }
    void GetInfoCashUnit::setHandler(DorsPSHandler& handler) noexcept {
        m_psHandler = &handler;
    }
    void GetInfoCashUnit::setServer(TcpServer& server) noexcept {
        m_server = &server;
    }

    void GetInfoCashUnit::fill_XFS3_Result(CimCashUnit& cashUnit)
    {
        std::lock_guard<std::mutex> lock(m_fillMutex);
        cashUnit.usNumber = usNumber;
        cashUnit.fwType = fwType;
        cashUnit.fwItemType = fwItemType;
        cashUnit.strUnitID = strUnitID;
        cashUnit.strCurrencyID = strCurrencyID;
        cashUnit.ulValues = ulValues;
        cashUnit.ulMaximum = ulMaximum;
        cashUnit.ulCount = ulCount;
        cashUnit.ulCashInCount = ulCashInCount;
        cashUnit.usStatus = usStatus;
        cashUnit.bAppLock = bAppLock;
        cashUnit.noteNumberList = noteNumberList;
        cashUnit.physicalCUs = physicalCashUnitList;
        cashUnit.noteIDs = noteIDs;
        cashUnit.usCDMType = usCDMType;
        cashUnit.strCashUnitName = strCashUnitName;
        cashUnit.ulInitialCount = ulInitialCount;
        cashUnit.ulDispensedCount = ulDispensedCount;
        cashUnit.ulPresentedCount = ulPresentedCount;
        cashUnit.ulRetractedCount = ulRetractedCount;
        cashUnit.ulRejectCount = ulRejectCount;
        cashUnit.ulMinimum = ulMinimum;
    }

    void GetInfoCashUnit::fill_XFS4_Result(CimCashUnit& cashUnit)
    {
    }

    std::string GetInfoCashUnit::PrintFromResult(int nIndent) const {
        std::ostringstream oss;

        //std::string indent(nIndent, '\t');
        //oss << indent << "usNumber: " << usNumber << "\n";
        //oss << indent << "ExchangeType: "  << fwType << "\n";
        //{
        //    oss << indent << "ItemType: " << fwItemType << "\n";
        //    if (fwItemType & CITYPALL) oss << indent << indent << CITYPALL << "\n";
        //    if (fwItemType & CITYPUNFIT) oss << indent << indent << CITYPUNFIT << "\n";
        //    if (fwItemType & CITYPINDIVIDUAL) oss << indent << indent << CITYPINDIVIDUAL << "\n";
        //    if (fwItemType & CITYPLEVEL3) oss << indent << indent << CITYPLEVEL3 << "\n";
        //    if (fwItemType & CITYPLEVEL2) oss << indent << indent << CITYPLEVEL2 << "\n";
        //    if (fwItemType & CITYPIPM) oss << indent << indent << CITYPIPM << "\n";
        //}
        //oss << indent << "UnitID: " << strUnitID << "\t";
        //oss << indent << "CurrencyID: " << strCurrencyID << "\t";
        //oss << indent << "Value: " << ulValues << "\n";
        //oss << indent << "CashInCount: " << ulCashInCount << "\t";
        //oss << indent << "Count: " << ulCount << "\t";
        //oss << indent << "Maximum: " << ulMaximum << "\n";
        //oss << indent << "InitialCount: " << ulInitialCount << "\t";
        //oss << indent << "DispensedCount: " << ulDispensedCount << "\t";
        //oss << indent << "PresentedCount: " << ulPresentedCount << "\n";
        //oss << indent << "Status: " << usStatus << "\n";
        //oss << indent << "AppLock: " << (bAppLock ? "TRUE" : "FALSE") << "\n";
        //{
        //    oss << indent << "NoteNumberList: \n";
        //    oss << indent << indent << "usNumOfNoteNumbers: " << noteNumberList.GetTotalNoteNumbers();
        //    oss << indent << indent << noteNumberList.GetTotalNoteNumbersDatas() << "\n";
        //}
        //oss << indent << "NoteIDs: " << (noteIDs.empty() ? "EMPTY" : std::to_string(noteIDs.size())) << "\n";
        //{
        //    oss << indent << "PhysicalCashUnitList: \n";
        //    if (physicalCashUnitList.empty()) {
        //        oss << indent << indent << "EMPTY\n";
        //    }
        //    else {
        //        for (const auto& phCU : physicalCashUnitList) {
        //            oss << indent << indent << "PhysicalPositionName: " << phCU.strPhysicalPositionName << "\n";
        //            oss << indent << indent << "UnitID: " << phCU.strUnitID << "\n";
        //            oss << indent << indent << "CashInCount: " << phCU.ulCashInCount << "\t";
        //            oss << indent << indent << "Count: " << phCU.ulCount << "\t";
        //            oss << indent << indent << "Maximum: " << phCU.ulMaximum << "\n";
        //            oss << indent << indent << "InitialCount: " << phCU.ulInitialCount << "\t";
        //            oss << indent << indent << "DispensedCount: " << phCU.ulDispensedCount << "\t";
        //            oss << indent << indent << "PresentedCount: " << phCU.ulPresentedCount << "\n";
        //            oss << indent << indent << "RetractedCount: " << phCU.ulRetractedCount << "\t";
        //            oss << indent << indent << "RejectCount: " << phCU.ulRejectCount << "\n";
        //            oss << indent << indent << "PStatus: " << phCU.usPStatus << "\n";
        //            oss << indent << indent << "HardwareSensors: " << (phCU.bHardwareSensors ? "TRUE" : "FALSE") << "\n";
        //            oss << indent << "szExtra:";
        //            if (!extra.empty()) {
        //                for (auto p = extra.begin(); p != extra.end(); ++p) {
        //                    oss << "\n" << indent << "\t" << *p;
        //                }
        //            }
        //            else {
        //                oss << " <empty>";
        //            }
        //            oss << "\n";
        //        }
        //    }
        //}
        return oss.str();
    }


    void GetInfoCashUnit::initialize() noexcept
    {
        usNumber = 0;
        fwType = WFS_CIM_SELFSERVICEBILL;
        fwItemType = WFS_CIM_CITYPALL;
        strUnitID = "CIM  ";
        strCurrencyID = ">  <";
        ulValues = 0;
        ulCashInCount = 0;
        ulCount = 0;
        ulMaximum = 0;
        usStatus = WFS_CIM_STATCUOK;
        bAppLock = false;
        noteNumberList = {};
        physicalCashUnitList = {};
        extra = {};
        noteIDs = {};
        usCDMType = 0;
        strCashUnitName = "UNKNOWN";
        ulInitialCount = 0;
        ulDispensedCount = 0;
        ulPresentedCount = 0;
        ulRetractedCount = 0;
        ulRejectCount = 0;
        ulMinimum = 0;
        m_psHandler = nullptr;
        m_server = nullptr;
    }

}
