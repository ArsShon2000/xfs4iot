#include "GetinfoCurrencyExp.h"

namespace Xfs::Cim {
    GetinfoCurrencyExp* GetinfoCurrencyExp::pinstance_ = nullptr;
    std::mutex GetinfoCurrencyExp::m_sendMutex;
    std::mutex GetinfoCurrencyExp::m_instanceMutex;
    std::mutex GetinfoCurrencyExp::m_fillMutex;

    void GetinfoCurrencyExp::fillResult()
    {
        std::lock_guard<std::mutex> lock(m_fillMutex);
        if (m_psHandler == nullptr) return;

        m_psHandler->EnsureBillTableIsAvailable();

        expList = m_psHandler->GetCurrencyExpList();
    }

    std::string GetinfoCurrencyExp::PrintFromResult() const
    {
        std::ostringstream oss;
        for (auto& exp : expList)
        {
            oss << "CurrencyID: " << exp.strCurrencyID << "\tExponent: " << std::to_string(exp.sExponent);
        }
        return oss.str();
    }
    // ####################################################################
}
