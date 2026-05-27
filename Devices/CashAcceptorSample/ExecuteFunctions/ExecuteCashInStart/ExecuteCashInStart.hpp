#pragma once
#include "../../CashAcceptorSample.hpp"
#include <boost/asio/use_awaitable.hpp>


class ExecuteCashInStart
{
public:
    explicit ExecuteCashInStart(std::shared_ptr<XFS4IoTSP::CashAcceptor::Sample::CashAcceptorSample> handler)
        :  m_pHandler(handler) 
    {
    }

    boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CashInStartResult> doBasicLogic();

private:
    std::shared_ptr<XFS4IoTSP::CashAcceptor::Sample::CashAcceptorSample> m_pHandler{ nullptr }; /**< Указатель на обработчик PS (не владеет) */

};
