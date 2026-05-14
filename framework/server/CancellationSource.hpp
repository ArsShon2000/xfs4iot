#pragma once

#include <stop_token>
#include <chrono>
#include <thread>
#include <atomic>
#include <memory>

#include "../core/Logger/ILogger.hpp"

namespace XFS4IoTServer
{
    class CancellationSource
    {
    public:
        CancellationSource() = default;

        std::stop_token GetToken() const
        {
            return stopSource_.get_token();
        }

        void Cancel()
        {
            stopSource_.request_stop();
        }

        bool IsCancellationRequested() const
        {
            return stopSource_.get_token().stop_requested();
        }

    private:
        std::stop_source stopSource_;
    };
}
