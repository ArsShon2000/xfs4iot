#pragma once
#include "PatternInterfaces/InterfaceDorsCommands/IDorsCommands.h"
#include "ComTransport/DorsPSHandler.h"

using namespace Interface::DorsRequest;
using namespace Xfs::Cim;

class XFS3_ExecuteCashInRollback : public IDorsCommands
{
public:

    enum RESULT {
        RETURNING = 1,
        HW_ERROR = 2
    };

    explicit XFS3_ExecuteCashInRollback(DorsPSHandler& handler)
        : m_psHandler(handler)
    {
    }

    void fillResult(LPWFSRESULT* lppWfsResult) override;

    void doBasicLogic(LPWFSRESULT lpWfsResult) override;

private:
    DorsPSHandler& m_psHandler; 

    HRESULT ValidateProcessingConditions();
};
