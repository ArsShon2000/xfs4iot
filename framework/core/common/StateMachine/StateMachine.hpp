#pragma once

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <ranges>
#include <vector>

#include <boost/circular_buffer.hpp>

namespace XfsCommon {
    /// Шаблонный класс конечного автомата
    /// @param _StateType - тип, характеризующий состояние (например, enum)
    /// @param _bIgnoreStateSelfReturn - игнорировать переход в то же состояние
    /// @param _StateFlowHistoryCapacity - ёмкость кольцевого буфера истории переходов
    ///
    /// StateMachine характеризуется следующими особенностями:
    ///	- класс инкапсулирует внутри себя описание текущего состояния
    ///	    (а также, в служебных целях хранит историю переходов /кольцевой буфер/);
    ///	- информация о переходе в новое состояние сообщается извне;
    ///	- определим понятие простого СОБЫТИЯ машины состояний как совокупность совпавших условий следующих трёх видов:
    ///			- произошёл переход из любого состояния в любое состояние;
    ///			- произошёл переход из состояния S;
    ///			- произошёл переход в состояние S;
    ///	- при этом под последовательностью СОБЫТИЙ будем называть последовательно произошедшую 
    ///		серию из N переходов: S1 -> S2 -> .. -> Sn;
    ///	- имеется возможность подписки на СОБЫТИЯ (как на простые, так и на последовательности).
    ///		В случае наступления события, будет вызван обработчик [в клиентском потоке / в собственном потоке / в отдельном потоке машины состояний];
    ///	- имеется возможность организовать блокирующее ожидание одного из событий.


    template <class _StateType, bool _bIgnoreStateSelfReturn = true, int _StateFlowHistoryCapacity = 100>
    class StateMachine {
    public:
        // Идентификатор подписки
        using SubscriptionId_t = int;

        // Тип функтора-обработчика
        using Handler_t = std::function<void(_StateType)>;

        /// Описание одного перехода
        class _EventDescription {
        public:
            _EventDescription() = default;
            virtual ~_EventDescription() = default;

            std::set<_StateType> from; // Переход из состояний
            std::set<_StateType> to;   // Переход в состояния
            bool bFromAny{ true };       // Из любого состояния
            bool bToAny{ true };         // В любое состояние


            //// FROM/TO ANY EXCEPT: если активен соответствующий модификатор Any,
            //// то перечисленные состояния исключаются из множества ANY
            //std::set< _StateType > fromAnyExcept;
            //std::set< _StateType > toAnyExcept;
        };

        /// Частный случай: переход из указанного состояния в указанное
        class EventFromTo : public _EventDescription {
        public:
            EventFromTo(_StateType _from, _StateType _to) {
                this->from.insert(_from);
                this->to.insert(_to);
                this->bFromAny = false;
                this->bToAny = false;
            }
            EventFromTo(const std::set<_StateType>& _from, _StateType _to) {
                this->from = _from;
                this->to.insert(_to);
                this->bFromAny = false;
                this->bToAny = false;
            }
            EventFromTo(_StateType _from, const std::set<_StateType>& _to) {
                this->to = _to;
                this->bToAny = false;
                this->from.insert(_from);
                this->bFromAny = false;
            }
            EventFromTo(const std::set<_StateType>& _from, const std::set<_StateType>& _to) {
                this->to = _to;
                this->bToAny = false;
                this->from = _from;
                this->bFromAny = false;
            }
        };

        /// Частный случай: переход из любого состояния в указанное
        class EventTo : public _EventDescription {
        public:
            EventTo(_StateType _to) {
                this->to.insert(_to);
                this->bToAny = false;
            }
            EventTo(const std::set<_StateType>& _to) {
                this->to = _to;
                this->bToAny = false;
            }

            //// Добавить в список исключений для any from указанное состояние
            //void FromAnyExcept(_StateType s) {
            //    this->fromAnyExcept.insert(s);
            //}
        };

        /// Частный случай: переход из указанного состояния / подмножества состояний в любое другое состояние
        class EventFrom : public _EventDescription {
        public:

            // Особенность GCC, у MSVC такого нет:

            // In the definition of a class template or a member of a class template,
            // if a base class of the class template depends on a template-parameter, the base class scope
            // is not examined during unqualified name lookup either at the point of definition of the class template
            // or member or during an instantiation of the class template or member.
            //
            // Другими словами, мы вынуждены ставить уточнение перед обращением к базовым атрибутам
            // (например, через this->)
            EventFrom(_StateType _from) {
                this->from.insert(_from);
                this->bFromAny = false;
            }
            EventFrom(const std::set< _StateType >& _from) {
                this->from = _from;
                this->bFromAny = false;
            }
        };

        /// Последовательность событий
        using EventsSequence_t = std::vector<_EventDescription>;

        /// Список последовательностей событий
        class CEventsSequenceList : public std::vector<EventsSequence_t> {};

        /// Подписка на событие
        class Subscription {
        public:
            EventsSequence_t eventsSequence; // Последовательность событий
            Handler_t handler;               // Обработчик
        };

        // Контейнер подписок
        using Subscriptions_t = std::map<SubscriptionId_t, Subscription>;

        /// Токен асинхронной отмены блокирующего ожидания
        class BlockedWaitTermination {
        public:
            void AsyncCancel() {
                std::lock_guard<std::mutex> lock(_lckMtx);
                _bCancel = true;
                _signal.notify_all();
            }

            bool IsCancelSet() const {
                std::lock_guard<std::mutex> lock(_lckMtx);
                return _bCancel;
            }

            void Reset() {
                std::lock_guard<std::mutex> lock(_lckMtx);
                _bCancel = false;
                _nEventOccurred = -1;
            }

            BlockedWaitTermination() = default;

        protected:
            void EventOccurred(int nEventOccurred) {
                std::lock_guard<std::mutex> lock(_lckMtx);
                _nEventOccurred = nEventOccurred;
                _signal.notify_all();
            }

            friend class StateMachine;

            bool _bCancel{ false };
            int _nEventOccurred{ -1 };
            mutable std::mutex _lckMtx;
            std::condition_variable _signal;
        };

        /// Информация о блокирующем ожидании
        struct _BlockedWaitInfo {
            uint64_t operationUniqueId{ 0 };
            EventsSequence_t expectedEvent;
            int nExpectedEventNo{ 0 };
            uint64_t nSkipStatesWithUid_LE{ 0 };
            std::weak_ptr<BlockedWaitTermination> pTerminationToken;
            uint64_t nEventOccurred_StateUid{ 0 };
        };
        using BlockedWaits_t = std::vector<_BlockedWaitInfo>;

        enum class BlockedWaitResult : int {
            BWR_TIMEOUT = -1,
            BWR_CANCELLED = -2,
            BWR_TERMINATED = -3,	
            BWR_FAILED = -4,
            BWR_WAIT_EVENT_0 = 0,
            BWR_WAIT_EVENT_1 = 1,
            BWR_WAIT_EVENT_2 = 2,
            BWR_WAIT_EVENT_3 = 3,
            BWR_WAIT_EVENT_4 = 4,
            BWR_WAIT_EVENT_5 = 5,
            BWR_WAIT_EVENT_6 = 6,
            BWR_WAIT_EVENT_7 = 7,
            BWR_WAIT_EVENT_8 = 8,
            BWR_WAIT_EVENT_9 = 9,
            BWR_WAIT_EVENT_10 = 10
        };

    public:
        StateMachine(_StateType initialState)
            : m_next_subscription_id(1)
            , m_stateFlow(_StateFlowHistoryCapacity)
            , m_nextBlockedOperationId(0)
            , m_next_record_uid(0) {
            m_stateFlow.push_back({ initialState, "", std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) });
            m_stateFlow.back().m_record_uid = ++m_next_record_uid;
        }

        ~StateMachine() {

            std::lock_guard<std::mutex> lock(m_state_mtx);
            m_subscriptions.clear();

            if (!m_blocked_waits.empty()) {
                std::stringstream strMsg;
                strMsg << "StateMachine::~StateMachine() - m_blocked_waits is not empty (size = " << m_blocked_waits.size() << ")!";
                for (const auto& wait : m_blocked_waits) {
                    if (auto token = wait.pTerminationToken.lock()) {
                        token->AsyncCancel();
                    }
                }

                for (int i = 0; i < 30 && !m_blocked_waits.empty(); ++i) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        }

        SubscriptionId_t Subscribe(const _EventDescription& simpleEvent, Handler_t handler) {
            EventsSequence_t sequenceOfEvents{ simpleEvent };
            return Subscribe(sequenceOfEvents, handler);
        }

        SubscriptionId_t Subscribe(const EventsSequence_t& sequenceOfEvents, Handler_t handler) {
            std::lock_guard<std::mutex> lock(m_subscribers_mtx);
            SubscriptionId_t id = m_next_subscription_id++;
            Subscription newSubscription{ sequenceOfEvents, handler };
            m_subscriptions.emplace(id, std::move(newSubscription));
            return id;
        }

        bool Unsubscribe(SubscriptionId_t id) {
            std::lock_guard<std::mutex> lock(m_subscribers_mtx);
            return m_subscriptions.erase(id) > 0;
        }

        void ChangeState(_StateType newState, const std::string& strReasonId = {}) {
            std::lock_guard<std::mutex> lock(m_state_mtx);

            if (_bIgnoreStateSelfReturn && newState == m_stateFlow.back().state) {
                return;
            }

            m_stateFlow.push_back({ newState, strReasonId, std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) });
            m_stateFlow.back().m_record_uid = ++m_next_record_uid;

            // Проверка подписок
            std::vector<SubscriptionId_t> eventsHappened;
            {
                std::lock_guard<std::mutex> subLock(m_subscribers_mtx);
                for (const auto& [id, subscription] : m_subscriptions) {
                    if (_has_happened(subscription.eventsSequence)) {
                        eventsHappened.push_back(id);
                    }
                }
            }

            // Вызов обработчиков
            for (const auto id : eventsHappened) {
                std::lock_guard<std::mutex> subLock(m_subscribers_mtx);
                if (auto it = m_subscriptions.find(id); it != m_subscriptions.end()) {
                    it->second.handler(m_stateFlow.back().state);
                }
            }

            // Проверка блокирующих ожиданий
            std::lock_guard<std::mutex> waitLock(m_blocked_waits_mtx);
            for (auto it = m_blocked_waits.begin(); it != m_blocked_waits.end(); ++it) {
                if (_has_happened(it->expectedEvent, it->nSkipStatesWithUid_LE)) {
                    if (auto token = it->pTerminationToken.lock()) {
                        token->EventOccurred(it->nExpectedEventNo);
                        it->nEventOccurred_StateUid = m_next_record_uid;
                    }
                }
            }
        }

        uint64_t GetCurrentStateUID(void)
        {
            std::lock_guard<std::mutex> lock(m_state_mtx);
            return m_stateFlow.back().m_record_uid;
        }

        _StateType GetCurrentState() const {
            std::lock_guard<std::mutex> lock(m_state_mtx);
            return m_stateFlow.back().state;
        }

        BlockedWaitResult BlockedWait(const CEventsSequenceList& arrExpectedEventSequences,
            std::chrono::milliseconds timeout,
            std::shared_ptr<BlockedWaitTermination> pTerminationToken,
            uint64_t skipStatesWithUID_LE = 0,
            uint64_t* pCurrentUID = nullptr) {

            if (!pTerminationToken) {
                pTerminationToken = std::make_shared<BlockedWaitTermination>();
            }

            if (arrExpectedEventSequences.empty()) {
                return BlockedWaitResult::BWR_FAILED;
            }

            // Проверка текущего состояния
            {
                std::lock_guard<std::mutex> lock(m_state_mtx);
                int nSequenceNumber = 0;
                for (const auto& seq : arrExpectedEventSequences) {
                    if (!seq.empty() && _has_happened(seq, skipStatesWithUID_LE, skipStatesWithUID_LE > 0, pCurrentUID)) {
                        std::stringstream stream;
                        stream << "StateMachine::BlockedWait() ‹‹‹ event already occurred : BWR_WAIT_EVENT_" << nSequenceNumber;
                        return static_cast<BlockedWaitResult>(nSequenceNumber);
                    }
                    ++nSequenceNumber;
                }
            }

            // Регистрация ожидания
            uint64_t operationUniqueId;
            {
                std::lock_guard<std::mutex> lock(m_blocked_waits_mtx);
                operationUniqueId = ++m_nextBlockedOperationId;
                for (auto it = arrExpectedEventSequences.begin(); it != arrExpectedEventSequences.end(); ++it) {
                    _BlockedWaitInfo wait;
                    wait.expectedEvent = *it;
                    wait.nExpectedEventNo = std::distance(arrExpectedEventSequences.begin(), it);
                    wait.pTerminationToken = pTerminationToken;
                    wait.operationUniqueId = operationUniqueId;
                    wait.nSkipStatesWithUid_LE = skipStatesWithUID_LE;
                    m_blocked_waits.push_back(std::move(wait));
                }
            }

            // Ожидание
            BlockedWaitResult result = BlockedWaitResult::BWR_FAILED;
            {
                std::unique_lock<std::mutex> lock(pTerminationToken->_lckMtx);
                bool eventOccurred = pTerminationToken->_signal.wait_for(lock, timeout, [&] {
                    return pTerminationToken->_bCancel || pTerminationToken->_nEventOccurred >= 0;
                    });

                if (!eventOccurred) {
                    result = BlockedWaitResult::BWR_TIMEOUT;
                }
                else if (pTerminationToken->_bCancel) {
                    result = BlockedWaitResult::BWR_CANCELLED;
                }
                else if (pTerminationToken->_nEventOccurred >= 0) {
                    result = static_cast<BlockedWaitResult>(pTerminationToken->_nEventOccurred);
                }
            }

            // Очистка регистрации
            {
                std::lock_guard<std::mutex> lock(m_blocked_waits_mtx);
                m_blocked_waits.erase(
                    std::remove_if(m_blocked_waits.begin(), m_blocked_waits.end(),
                        [&](const auto& wait) {
                            if (wait.operationUniqueId == operationUniqueId) {
                                if (pCurrentUID && pTerminationToken->_nEventOccurred == wait.nExpectedEventNo) {
                                    *pCurrentUID = wait.nEventOccurred_StateUid;
                                }
                                return true;
                            }
                            return false;
                        }),
                    m_blocked_waits.end());
            }

            pTerminationToken->_nEventOccurred = -1;
            return result;
        }

        BlockedWaitResult BlockedWait(const EventsSequence_t& expectedEventSequence,
            std::chrono::milliseconds timeout,
            std::shared_ptr<BlockedWaitTermination> pTerminationToken,
            uint64_t skipStatesWithUID_LE = 0,
            uint64_t* pCurrentUID = nullptr) {
            CEventsSequenceList arrExpectedEventSequences;
            arrExpectedEventSequences.push_back(expectedEventSequence);
            return BlockedWait(arrExpectedEventSequences, timeout, pTerminationToken, skipStatesWithUID_LE, pCurrentUID);
        }

        BlockedWaitResult BlockedWait(const _EventDescription& expectedEvent,
            std::chrono::milliseconds timeout,
            std::shared_ptr<BlockedWaitTermination> pTerminationToken,
            uint64_t skipStatesWithUID_LE = 0,
            uint64_t* pCurrentUID = nullptr) {
            EventsSequence_t expectedEventSequence{ expectedEvent };
            return BlockedWait(expectedEventSequence, timeout, pTerminationToken, skipStatesWithUID_LE, pCurrentUID);
        }

    private:
        bool _has_happened(const EventsSequence_t& eventsSequence, uint64_t skipRecordsWithUID_LE = 0, bool bFullSearch = false, uint64_t* pLastStateUID = nullptr) {
            //std::lock_guard<std::mutex> lock(m_state_mtx);

            auto availableHistorySize = m_stateFlow.size();
            if (skipRecordsWithUID_LE > 0) {
                auto notValidIt = std::ranges::find_if(m_stateFlow | std::views::reverse,
                    [&](const auto& ref) { return ref.m_record_uid <= skipRecordsWithUID_LE; });
                if (notValidIt != m_stateFlow.rend()) {
                    availableHistorySize = std::distance(m_stateFlow.rbegin(), notValidIt);
                }
            }

            if (availableHistorySize < eventsSequence.size() + 1) {
                return false;
            }

            int nPassCount = bFullSearch ? availableHistorySize - eventsSequence.size() : 1;
            for (int nPassNo = 0; nPassNo < nPassCount; ++nPassNo) {
                auto it_history = m_stateFlow.end() - eventsSequence.size() - 1 - nPassNo;
                auto it_expected = eventsSequence.begin();
                bool bFailedToSatisfy = false;

                for (; it_expected != eventsSequence.end(); ++it_expected, ++it_history) {
                    const auto& fromState = it_history->state;
                    const auto& toState = (it_history + 1)->state;
                    const auto& expected = *it_expected;

                    if (!expected.bFromAny && !expected.from.contains(fromState)) {
                        bFailedToSatisfy = true;
                        break;
                    }
                    if (!expected.bToAny && !expected.to.contains(toState)) {
                        bFailedToSatisfy = true;
                        break;
                    }
                }

                if (!bFailedToSatisfy) {
                    if (pLastStateUID) {
                        *pLastStateUID = it_history->m_record_uid;
                    }
                    return true;
                }
            }
            return false;
        }

        struct StateInfo {
            _StateType state;
            std::string strReasonId;
            std::time_t timestamp;
            uint64_t m_record_uid{ 0 };
        };

        mutable std::mutex m_state_mtx;
        boost::circular_buffer<StateInfo> m_stateFlow;
        uint64_t m_next_record_uid;

        std::mutex m_subscribers_mtx;
        Subscriptions_t m_subscriptions;
        SubscriptionId_t m_next_subscription_id;

        std::mutex m_blocked_waits_mtx;
        BlockedWaits_t m_blocked_waits;
        uint64_t m_nextBlockedOperationId;
    };

} // namespace XfsCommon
