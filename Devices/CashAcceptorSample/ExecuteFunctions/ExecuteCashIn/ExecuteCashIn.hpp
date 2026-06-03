//#pragma once
//
//#include "../../CashAcceptorSample.hpp"
//#include <memory>
//#include <boost/asio/use_awaitable.hpp>
//
///**
// * @file ExecuteCashIn.hpp
// * @brief Реализация обработчика команды WFS_CMD_CIM_CASH_IN (Execute CashIn).
// *
// * Заголовочный файл содержит объявление класса `ExecuteCashIn`, который
// * реализует интерфейс `IDorsCommands` и отвечает за обработку сценариев приёма
// * наличности (CASH_IN) в рамках XFS3 провайдера. Класс инкапсулирует логику
// * валидации условий, запуска основного обработчика и формирования результата
// * для клиента.
// */
//
// /**
//  * @class ExecuteCashIn
//  * @brief Обработчик Execute-команды CASH_IN для XFS3.
//  *
//  * Класс реализует следующие этапы обработки команды:
//  * - проверка условий для начала приёма (ValidateProcessingConditions),
//  * - подготовка и заполнение результата (fillResult),
//  * - выполнение основной логики приёма банкнот (doBasicLogic).
//  *
//  * Объект хранит ссылку на `DorsPSHandler`, через который взаимодействует с
//  * низкоуровневым оборудованием и вспомогательными менеджерами (escrow,
//  * inhibit и пр.). Для возможности отмены ожидания событий используется
//  * асинхронный терминатор `m_p_async_terminator`.
//  */
//namespace XFS4IoTSP::CashAcceptor::Sample
//{
//	class ExecuteCashIn
//	{
//	public:
//		/**
//		 * @brief Конструктор обработчика.
//		 * @param handler Ссылка на `DorsPSHandler` — обработчик устройства (внешняя владение).
//		 */
//		explicit ExecuteCashIn(std::shared_ptr<XFS4IoTSP::CashAcceptor::Sample::CashAcceptorSample> handler
//			, std::map<std::string, XFS4IoTFramework::Storage::CashItemCountClass>& currentCashInItems
//			, std::shared_ptr<std::map<std::string, XFS4IoTFramework::Storage::CashItemCountClass>> acceptedItems
//			, std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorServiceProvider> cashAcceptorService
//			, std::shared_ptr<XFS4IoTFramework::CashAcceptor::CashInCommandEvents> events
//			, std::stop_token& cancellation)
//			: m_psHandler(handler)
//			, m_guard(handler->m_stateMachine)
//			, currentCashInItems_(currentCashInItems)
//			, acceptedItems_(acceptedItems)
//			, cashAcceptorService_(cashAcceptorService)
//			, events_(events)
//			, m_cancellation(cancellation)
//		{
//		}
//
//		/**
//		 * @brief Выполнить основную логику команды (приём банкноты, ожидание событий).
//		 *
//		 * Этот метод содержит основную последовательность действий по приёму
//		 * банкноты: включение приёма, подписки на события стейт‑машины, ожидание
//		 * нужных переходов и формирование итогового ответа клиенту.
//		 *
//		 * @param lpWfsResult Указатель на структуру результата, которая будет
//		 *        заполнена в процессе выполнения.
//		 */
//		boost::asio::awaitable<XFS4IoTFramework::CashAcceptor::CashInStartResult> doBasicLogic();
//
//		/**
//		 * @brief Проверить условия для начала обработки CASH_IN.
//		 *
//		 * Производит базовую валидацию: инициализация устройства, отсутствие
//		 * конфликтующих операций (exchange, отсутствие кассет и т.д.), активность
//		 * транзакции escrow и лимитов приёма.
//		 *
//		 * @return HRESULT - код XFS: WFS_SUCCESS при возможности начать операцию,
//		 *         иначе соответствующий код ошибки.
//		 */
//		XFS4IoT::MessageHeader::CompletionCodeEnum ValidateProcessingConditions();
//
//
//		/** * @brief Прервать текущее ожидание событий стейт‑машины.
//		 *
//		 * Вызывает метод `AsyncCancel` у асинхронного терминатора ожидания,
//		 * что приводит к прерыванию блокирующего ожидания в `doBasicLogic`.
//		 */
//		void InterruptRequest()
//		{
//			if (auto p = m_p_async_terminator.lock())
//			{
//				p->AsyncCancel();
//			}
//		}
//
//	private:
//		std::shared_ptr<XFS4IoTSP::CashAcceptor::Sample::CashAcceptorSample> m_psHandler; /**< Ссылка на обработчик PS (внешне управляемый). */
//		std::shared_ptr<XFS4IoTFramework::Common::CashAcceptorServiceProvider> cashAcceptorService_;
//		std::map<std::string, XFS4IoTFramework::Storage::CashItemCountClass>& currentCashInItems_;
//		std::shared_ptr<std::map<std::string, XFS4IoTFramework::Storage::CashItemCountClass>> acceptedItems_;
//		std::shared_ptr<XFS4IoTFramework::CashAcceptor::CashInCommandEvents> events_;
//		std::stop_token m_cancellation; /**< Токен для отслеживания отмены операции. */
//
//		/** @brief Защитник подписок на события стейт‑машины. */
//		XFS4IoTSP::CashAcceptor::Sample::SubscriptionGuard m_guard;
//
//		/**
//		 * @brief Слабая ссылка на асинхронный терминатор ожидания событий стейт‑машины.
//		 *
//		 * Используется для отмены BlockedWait, если требуется прервать ожидание
//		 * (например, при отмене запроса клиентом).
//		 */
//		std::weak_ptr<XFS4IoTSP::CashAcceptor::Sample::StateMachine::BlockedWaitTermination> m_p_async_terminator;
//
//		/**
//		 * @brief Установить соответствующий бит в маске набора поддерживаемых номиналов.
//		 * @param n Номер бита (идентификатор номинала).
//		 * @param Val Ссылка на целевую маску, в которую устанавливается бит.
//		 */
//		void SetBit(BYTE n, uint32_t& Val);
//	};
//}
