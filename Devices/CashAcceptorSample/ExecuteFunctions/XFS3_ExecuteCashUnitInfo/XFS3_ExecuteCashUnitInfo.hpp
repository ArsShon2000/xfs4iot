
#pragma once
#include "PatternInterfaces/InterfaceDorsCommands/IDorsCommands.h"
#include "ComTransport/DorsPSHandler.h"
#include "MODULE_STRUCTURES/CimCashUnit/CimCashUnit.hpp"

using namespace Interface::DorsRequest;
using namespace Xfs::Cim;

class XFS3_ExecuteCashUnitInfo : public IDorsCommands
{
public:
    explicit XFS3_ExecuteCashUnitInfo(DorsPSHandler& handler, CimCashUnitInfo& params)
        : m_psHandler(handler)
        , m_params(params)
    {
    }
    void fillResult(LPWFSRESULT* lppWfsResult) override;

    void doBasicLogic(LPWFSRESULT lpWfsResult) override;


private:
    DorsPSHandler& m_psHandler;
    CimCashUnitInfo m_params;
    HRESULT ValidateProcessingConditions();
};
