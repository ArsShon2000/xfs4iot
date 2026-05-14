#pragma once

#include <string>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace XFS4IoTServer
{
    class Validation
    {
    public:
        /// <summary>
        /// Checks if pointer is not null, throws exception with message if it is null
        /// Returns the pointer for chaining
        /// </summary>
        template<typename T>
        static std::shared_ptr<T> isNotNull(
            std::shared_ptr<T> ptr,
            const std::string& message = "Pointer is null")
        {
            if (!ptr)
            {
                throw std::invalid_argument(message);
            }
            return ptr;
        }

        /// <summary>
        /// Raw pointer version
        /// </summary>
        template<typename T>
        static T* isNotNull(
            T* ptr,
            const std::string& message = "Pointer is null")
        {
            if (!ptr)
            {
                throw std::invalid_argument(message);
            }
            return ptr;
        }

        /// <summary>
        /// Checks if value is true, throws exception with message if it is false
        /// </summary>
        static void isTrue(
            bool condition,
            const std::string& message = "Condition is false")
        {
            if (!condition)
            {
                throw std::invalid_argument(message);
            }
        }

        /// <summary>
        /// Checks if value is false, throws exception with message if it is true
        /// </summary>
        static void isFalse(
            bool condition,
            const std::string& message = "Condition is true")
        {
            if (condition)
            {
                throw std::invalid_argument(message);
            }
        }

        /// <summary>
        /// Type checking - attempts dynamic_pointer_cast and throws if cast fails
        /// Similar to C# .IsA<T>() extension method
        /// </summary>
        template<typename TTarget, typename TSource>
        static std::shared_ptr<TTarget> isA(
            std::shared_ptr<TSource> ptr,
            const std::string& message = "Invalid type cast")
        {
            auto result = std::dynamic_pointer_cast<TTarget>(ptr);
            if (!result)
            {
                throw std::invalid_argument(message);
            }
            return result;
        }

        /// <summary>
        /// Checks if string is not empty
        /// </summary>
        static void isNotEmpty(
            const std::string& str,
            const std::string& message = "String is empty")
        {
            if (str.empty())
            {
                throw std::invalid_argument(message);
            }
        }

        /// <summary>
        /// Checks if container is not empty
        /// </summary>
        template<typename TContainer>
        static void isNotEmpty(
            const TContainer& container,
            const std::string& message = "Container is empty")
        {
            if (container.empty())
            {
                throw std::invalid_argument(message);
            }
        }
    };

    // Альтернативный подход: класс для контрактов (assertions)
    class Contracts
    {
    public:
        /// <summary>
        /// Assert condition, throws runtime_error if false
        /// Similar to C# Contract.Assert or Debug.Assert
        /// </summary>
        static void Assert(
            bool condition,
            const std::string& message = "Assertion failed")
        {
            if (!condition)
            {
                throw std::runtime_error(message);
            }
        }

        /// <summary>
        /// Require condition at function entry
        /// </summary>
        static void Require(
            bool condition,
            const std::string& message = "Precondition failed")
        {
            if (!condition)
            {
                throw std::invalid_argument(message);
            }
        }

        /// <summary>
        /// Ensure condition at function exit
        /// </summary>
        static void Ensure(
            bool condition,
            const std::string& message = "Postcondition failed")
        {
            if (!condition)
            {
                throw std::logic_error(message);
            }
        }
    };
}