#pragma once

#include <memory>

namespace System
{
    class ICloneable
    {
    public:
        virtual ~ICloneable() = default;

        //virtual std::unique_ptr<ICloneable> Clone() const = 0;
    };
}
