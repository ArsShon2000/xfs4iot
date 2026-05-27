
#pragma once
#include "PatternInterfaces/InterfaceDorsCommands/IDorsCommands.h"
#include "ComTransport/DorsPSHandler.h"
#include "MODULE_STRUCTURES/CimItemPosition/CimItemPosition.hpp"

using namespace Interface::DorsRequest;
using namespace Xfs::Cim;

class XFS3_ExecuteReset : public IDorsCommands
{
public:
    explicit XFS3_ExecuteReset(DorsPSHandler& handler, CimItemPosition* params)
        : m_psHandler(handler)
    {
        if (params) {
            m_ptrParams.reset(new CimItemPosition(*params));
        }
    }
    void fillResult(LPWFSRESULT* lppWfsResult) override;

    void doBasicLogic(LPWFSRESULT lpWfsResult) override;


private:
    DorsPSHandler& m_psHandler;
    std::unique_ptr< CimItemPosition > m_ptrParams;
    HRESULT ValidateProcessingConditions();
};
