#include "NotesInhibitManager.hpp"
#include "../../CashAcceptorSample.hpp"
#include <boost/scope_exit.hpp>
#include <chrono>
#include <cassert>

using namespace std::chrono_literals;

namespace XFS4IoTSP::CashAcceptor::Sample {

    NotesInhibitManager::NotesInhibitManager(std::shared_ptr<XFS4IoTSP::CashAcceptor::Sample::CashAcceptorSample> device
        , uint16_t dwDelayTimeAcceptanceStop)
        : device_(device)
        , m_dwDelayTimeAcceptanceStop(std::chrono::milliseconds(dwDelayTimeAcceptanceStop))
        , m_ulNoteIds(0)
        , m_bNoteIdsCached(false)
    {
    }

    NotesInhibitManager::~NotesInhibitManager()
    {
        // Гарантированно останавливаем поток и пытаемся сохранить устройство в корректном состоянии
        InhibitAccept();
        _stop();
    }

    void NotesInhibitManager::DelayInhibitAccept()
    {
        if (m_dwDelayTimeAcceptanceStop.count() == 0) {
            // Мгновенное запрещение
            _inhibit();
            return;
        }
        _start();
    }

    bool NotesInhibitManager::InhibitAccept()
    {
        // Останавливаем отложенную нить, затем выполняем запрет
        _stop();
        return _inhibit();
    }

    bool NotesInhibitManager::AllowAccept()
    {
        _stop();
        return _allow();
    }

    bool NotesInhibitManager::IsAcceptAllowed()
    {
        std::scoped_lock lock(m_mtx);
        return (m_ulNoteIds > 0);
    }

    // -------------------------------
    // Поток управления задержкой
    // -------------------------------
    void NotesInhibitManager::_start()
    {
        _stop(); // если уже был поток — остановим его

        m_stopRequested.store(false);
        m_threadRunning.store(true);
        m_thread = std::thread(&NotesInhibitManager::_delay_inhibit_thread, this);
    }

    void NotesInhibitManager::_stop()
    {
        // Сигнализируем нити отмены задержки
        m_stopRequested.store(true);
        m_cvStop.notify_all();

        // Если есть активный токен BlockedWait — попытаемся прервать BlockedWait
        if (auto p = m_p_async_terminator.lock()) {
            try {
                p->AsyncCancel();
            }
            catch (...) {
                // игнорируем исключения из токена
            }
        }

        if (m_thread.joinable()) {
            m_thread.join();
        }
        m_threadRunning.store(false);
    }

    void NotesInhibitManager::_delay_inhibit_thread()
    {
        std::unique_lock<std::mutex> lk(m_mtx);

        // Ждем либо отмены, либо таймаута
        if (m_cvStop.wait_for(lk, m_dwDelayTimeAcceptanceStop, [this]() { return m_stopRequested.load(); })) {
            // Отмена пришла раньше таймаута — выходим
            return;
        }

        // Освобождаем мьютекс перед выполнением основной работы, чтобы _inhibit мог захватить лок
        lk.unlock();

        // Выполняем запрет приёма
        _inhibit();
    }

    // -------------------------------
    // Основная логика inhibit / allow
    // -------------------------------
    bool NotesInhibitManager::_inhibit()
    {
        std::scoped_lock lock(m_mtx);

        using namespace FS365::HW::Dors;

        if (m_bNoteIdsCached && m_ulNoteIds == 0) {
            // Уже запрещено
            return true;
        }

        if (!device_) return false;

        // Как и раньше, если устройство в состоянии Escrow/Holding/Accepting — вернем купюру
        switch (device_->m_State) {
        case DorsHW::POLL_RES::EscrowPos:
        case DorsHW::POLL_RES::Holding:
        case DorsHW::POLL_RES::Accepting:
            device_->m_pDevice->Return();
            if (device_->logger_) {
                device_->logger_->warn(std::format("{} - осуществляем отложенный возврат банкноты", __FUNCTION__), LOGLEVEL1);
            }
            break;
        case DorsHW::POLL_RES::PortError:
            // Нет связи — команда бессмысленна
            return false;
        default:
            break;
        }

        // Подписки: при появлении Accepting/Escrow будем ещё раз подавать запрет
        auto subscr1 = device_->m_stateMachine.Subscribe(StateMachine::EventTo{ { DorsHW::POLL_RES::Accepting } },
            [this](DorsHW::POLL_RES) {
                if (device_ && device_->m_pDevice) {
                    device_->m_pDevice->EnableBillTypes(0, 0);
                }
            });

        auto subscr2 = device_->m_stateMachine.Subscribe(StateMachine::EventTo{ { DorsHW::POLL_RES::EscrowPos } },
            [this](DorsHW::POLL_RES) {
                if (device_ && device_->m_pDevice) {
                    device_->m_pDevice->Return();
                    device_->m_pDevice->EnableBillTypes(0, 0);
                }
            });

        // Гарантированно отписываемся при выходе (boost::scope_exit)
        BOOST_SCOPE_EXIT(&device_, subscr1, subscr2) {
            try { device_->m_stateMachine.Unsubscribe(subscr1); }
            catch (...) {}
            try { device_->m_stateMachine.Unsubscribe(subscr2); }
            catch (...) {}
        } BOOST_SCOPE_EXIT_END;

        // Попытка запретить приём: EnableBillTypes(0,0)
        if (DorsHW::RESULT::Ok != device_->m_pDevice->EnableBillTypes(0, 0)) {
            return false;
        }

        // Создаем ожидаемые последовательности (как в старом коде)
        StateMachine::CEventsSequenceList expectedEvents;
        {
            // 1) ANY -> UnitDisabled
            expectedEvents.push_back({ StateMachine::EventTo{ DorsHW::POLL_RES::UnitDisabled } });

            // 2) ANY -> один из "устойчивых" ошибочных состояний
            expectedEvents.push_back({ StateMachine::EventTo{ {
                DorsHW::POLL_RES::DropCassetteFull,
                DorsHW::POLL_RES::DropCassetteOutOfPosition,
                DorsHW::POLL_RES::ValidatorJammed,
                DorsHW::POLL_RES::DropCassetteJammed,
                DorsHW::POLL_RES::StackMotorFail,
                DorsHW::POLL_RES::TransportMotorFail,
                DorsHW::POLL_RES::InitialCassetteStatusFail,
                DorsHW::POLL_RES::OpticCanalFail,
                DorsHW::POLL_RES::MagneticCanalFail,
                DorsHW::POLL_RES::StartTrayFailure,
                DorsHW::POLL_RES::Group47UnknownFailure,
                DorsHW::POLL_RES::PortError
            } } });
        }

        // Создаём токен отмены ожидания и сохраняем weak_ptr в поле (как раньше)
        auto pAsyncTerminator = std::make_shared<StateMachine::BlockedWaitTermination>();
        m_p_async_terminator = pAsyncTerminator; // сохраняем слабую ссылку, чтобы _stop мог AsyncCancel

        // Ждем входа в UnitDisabled или указанную ошибку — до 30 секунд,
        // при каждом таймауте 5s повторно подаём запрет (EnableBillTypes(0,0))
        StateMachine::BlockedWaitResult result = StateMachine::BlockedWaitResult::BWR_TIMEOUT;
        auto start = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start < std::chrono::milliseconds(30000)) {
            // BlockedWait — используется твой StateMachine::BlockedWait
            result = device_->m_stateMachine.BlockedWait(expectedEvents, std::chrono::milliseconds(5000), pAsyncTerminator);

            if (result != StateMachine::BlockedWaitResult::BWR_TIMEOUT) {
                // либо одно из событий произошло, либо отмена пришла
                break;
            }

            // Повторно подаём запрет приёма
            device_->m_pDevice->EnableBillTypes(0, 0);
        }

        // По завершении считаем, что приём завершён (в любом случае)
        m_ulNoteIds = 0;

        // Помечаем кеш в зависимости от результата
        if (result == StateMachine::BlockedWaitResult::BWR_WAIT_EVENT_0) {
            m_bNoteIdsCached = true;
        }
        else {
            m_bNoteIdsCached = false;
        }

        return true;
    }

    bool NotesInhibitManager::_allow()
    {
        std::scoped_lock lock(m_mtx);

        using namespace FS365::HW::Dors;

        if (!device_) return false;

        if (m_bNoteIdsCached && m_ulNoteIds == device_->GetCashManagementCapabilities()->GetBillTypes()) {
            // Прием с таким списком уже разрешен
            return true;
        }

        // Нет связи с устройством — бессмысленно
        if (DorsHW::POLL_RES::PortError == device_->m_State) {
            return false;
        }

        if (DorsHW::RESULT::Ok != device_->m_pDevice->EnableBillTypes(device_->GetCashManagementCapabilities()->GetBillTypes(), device_->GetCashManagementCapabilities()->GetBillTypes())) {
            return false;
        }

        m_ulNoteIds = device_->GetCashManagementCapabilities()->GetBillTypes();
        m_bNoteIdsCached = true;
        return true;
    }
}
