#pragma once

namespace System
{
    /**
 * @brief IDisposable
 *
 * Интерфейс для освобождения неуправляемых ресурсов.
 * 
 */
    class IDisposable
    {
    public:
        virtual ~IDisposable() = default;


        [[nodiscard]]
        virtual void Dispose() = 0;
    };
}