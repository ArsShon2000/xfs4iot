#include "ExecuteCashInStart.hpp"


boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CashInStartResult> ExecuteCashInStart::doBasicLogic()
{
    m_pHandler->logger_->trace(std::format("{}() ------------------- Запуск обработчика WFS_CMD_CIM_CASH_IN_START -------------------",  __FUNCTION__), LOGLEVEL1);

    // Разрешаем установку лимитов
    if (m_pHandler->bConjointCashInIsActive) {
		//  их добавлю после реализации обычного cash-in, так как для него не требуется устанавливать лимиты
    }

    if (m_pHandler->m_pDevice->GetSoftwareConfigurationFault()) {
        m_pHandler->logger_->trace(std::format("{}() - Запрос отклонён: Девайс неправильно сконфигугрирован!", __FUNCTION__), LOGLEVELMSG);
        co_return XFS4IoTFramework::CashAcceptor::CashInStartResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum::HardwareError,
            "Девайс неправильно сконфигугрирован!");
    }

    if (m_pHandler->m_pDevice->IsDeviceInitialized()) {
        m_pHandler->logger_->trace(std::format("{}() - Запрос отклонён: Девайс не инициализирован!", __FUNCTION__), LOGLEVELMSG);
        co_return XFS4IoTFramework::CashAcceptor::CashInStartResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum::HardwareError,
            "Девайс не инициализирован!");
    }

    if (m_pHandler->IsCassetteMissing())
    {
        m_pHandler->logger_->trace(std::format("{}() - Запрос отклонён: Внимание отсутствует кассета!", __FUNCTION__), LOGLEVELMSG);
        co_return XFS4IoTFramework::CashAcceptor::CashInStartResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum::DeviceNotReady,
            "Внимание отсутствует кассета!");
    }

    
	if (!m_pHandler->getHasBillTableCache())
	{
		m_pHandler->logger_->trace(std::format("{} - ошибка при считывании таблицы номиналов", __FUNCTION__), LOGLEVEL1);
        co_return XFS4IoTFramework::CashAcceptor::CashInStartResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum::DeviceNotReady,
            "ошибка при считывании таблицы номиналов!");
	}
	else if (m_pHandler->IsReadyForCashIn())
	{
		//m_pHandler->m_pEscrоwManager->OpenTransaction();
	}
	else
	{
        m_pHandler->logger_->trace(std::format("{} - ошибка при считывании таблицы номиналов", __FUNCTION__), LOGLEVEL1);
        co_return XFS4IoTFramework::CashAcceptor::CashInStartResult(
            XFS4IoT::MessageHeader::CompletionCodeEnum::DeviceNotReady,
            "ошибка при считывании таблицы номиналов!");
	}

    m_pHandler->logger_->trace(std::format("{}() ------------------- Завершение обработчика CASH_IN_START [Result = {}] -------------------",  __FUNCTION__
        , ToString(XFS4IoT::MessageHeader::CompletionCodeEnum::DeviceNotReady), LOGLEVEL1)
    );
}
