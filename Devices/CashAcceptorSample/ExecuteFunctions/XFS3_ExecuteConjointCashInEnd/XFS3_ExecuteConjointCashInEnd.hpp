#pragma once
#include "PatternInterfaces/InterfaceDorsCommands/IDorsCommands.h"
#include "ComTransport/DorsPSHandler.h"

using namespace Interface::DorsRequest;
using namespace Xfs::Cim;


class XFS3_ExecuteConjointCashInEnd : public IDorsCommands
{
public:
    explicit XFS3_ExecuteConjointCashInEnd(DorsPSHandler& handler)
        : m_psHandler(handler)
    {
    }

    void fillResult(LPWFSRESULT* lppWfsResult) override;

    void doBasicLogic(LPWFSRESULT lpWfsResult) override;

    /** * @brief Прервать текущее ожидание событий стейт‑машины.
     *
     * Вызывает метод `AsyncCancel` у асинхронного терминатора ожидания,
     * что приводит к прерыванию блокирующего ожидания в `doBasicLogic`.
     */
    void InterruptRequest() override
    {
        if (auto p = m_p_async_terminator.lock())
        {
            p->AsyncCancel();
        }
    }

private:
    DorsPSHandler& m_psHandler;

    std::weak_ptr<StateMachine::BlockedWaitTermination> m_p_async_terminator;

    HRESULT ValidateProcessingConditions();
};
