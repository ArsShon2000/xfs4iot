#include <string>
#include <stdexcept>
#include <source_location>
#include <format>
#include <functional>
#include <algorithm>

namespace XFS4IoT
{
    // ========================================================================
    // Contracts - Design by Contract Support
    // ========================================================================

    /// <summary>
    /// Support for development by contract
    /// </summary>
    class Contracts
    {
    public:
        /// <summary>
        /// Error handler type
        /// </summary>
        using ErrorHandler = std::function<void(const std::string&)>;

        /// <summary>
        /// Set custom error handler
        /// </summary>
        static void SetErrorHandler(ErrorHandler handler)
        {
            GetErrorHandler() = handler;
        }

        /// <summary>
        /// Assert that the given expression is true
        /// </summary>
        static void Assert(bool predicate,
            const std::string& message = "",
            const std::source_location& location = std::source_location::current())
        {
            if (!predicate) {
                InvokeErrorHandler(GetFailedAssertionMessage(message, location));
                throw std::runtime_error("Invalid error handling - Error handler must never return");
            }
        }

        /// <summary>
        /// Fail immediately
        /// </summary>
        [[noreturn]]
        static void Fail(const std::string& message = "",
            const std::source_location& location = std::source_location::current())
        {
            InvokeErrorHandler(GetFailedAssertionMessage(message, location));
            throw std::runtime_error("Invalid error handling - Error handler must never return");
        }

        /// <summary>
        /// Fail with exception type (useful for unreachable code markers)
        /// </summary>
        template<typename T = std::runtime_error>
        [[noreturn]]
        static T FailWithException(const std::string& message = "",
            const std::source_location& location = std::source_location::current())
        {
            InvokeErrorHandler(GetFailedAssertionMessage(message, location));
            throw T("Failed assertion");
        }

        /// <summary>
        /// Check that value satisfies predicate
        /// </summary>
        template<typename T>
        static T& Is(T& value, bool predicate, const std::string& message = "Failed assert")
        {
            if (!predicate) {
                Fail(message);
            }
            return value;
        }

        /// <summary>
        /// Assert value must be null
        /// </summary>
        template<typename T>
        static T IsNull(T value,
            const std::string& message = "",
            const std::source_location& location = std::source_location::current())
        {
            if (value != nullptr) {
                std::string msg = message.empty()
                    ? std::format("Expression should be null ({}:{})",
                        location.file_name(), location.line())
                    : message;
                Fail(msg, location);
            }
            return value;
        }

        /// <summary>
        /// Assert value must not be null
        /// </summary>
        template<typename T>
        static T IsNotNull(T value,
            const std::string& message = "",
            const std::source_location& location = std::source_location::current())
        {
            if (value == nullptr) {
                std::string msg = message.empty()
                    ? std::format("Expression should not be null ({}:{})",
                        location.file_name(), location.line())
                    : message;
                Fail(msg, location);
            }
            return value;
        }

        /// <summary>
        /// Assert bool value must be true
        /// </summary>
        static bool IsTrue(bool value,
            const std::string& message = "",
            const std::source_location& location = std::source_location::current())
        {
            if (!value) {
                std::string msg = message.empty()
                    ? std::format("Expression should be true ({}:{})",
                        location.file_name(), location.line())
                    : message;
                Fail(msg, location);
            }
            return value;
        }

        /// <summary>
        /// Assert bool value must be false
        /// </summary>
        static bool IsFalse(bool value,
            const std::string& message = "",
            const std::source_location& location = std::source_location::current())
        {
            if (value) {
                std::string msg = message.empty()
                    ? std::format("Expression should be false ({}:{})",
                        location.file_name(), location.line())
                    : message;
                Fail(msg, location);
            }
            return value;
        }

        /// <summary>
        /// Assert string should be null or empty
        /// </summary>
        static void IsNullOrWhitespace(const std::string& value,
            const std::string& message = "",
            const std::source_location& location = std::source_location::current())
        {
            bool isEmpty = value.empty() ||
                std::all_of(value.begin(), value.end(),
                    [](unsigned char c) { return std::isspace(c); });
            if (!isEmpty) {
                std::string msg = message.empty()
                    ? std::format("Expression should be null or whitespace ({}:{})",
                        location.file_name(), location.line())
                    : message;
                Fail(msg, location);
            }
        }

        /// <summary>
        /// Assert string should not be null or empty
        /// </summary>
        static const std::string& IsNotNullOrWhitespace(
            const std::string& value,
            const std::string& message = "",
            const std::source_location& location = std::source_location::current())
        {
            bool isEmpty = value.empty() ||
                std::all_of(value.begin(), value.end(),
                    [](unsigned char c) { return std::isspace(c); });
            if (isEmpty) {
                std::string msg = message.empty()
                    ? std::format("Expression should be a non-empty string ({}:{})",
                        location.file_name(), location.line())
                    : message;
                Fail(msg, location);
            }
            return value;
        }

        /// <summary>
        /// Ignore assertions (intentionally no check)
        /// </summary>
        template<typename T>
        static T& Ignore(T& value) { return value; }

        /// <summary>
        /// Assert object is specified type
        /// </summary>
        template<typename T, typename U>
        static std::shared_ptr<T> IsA(std::shared_ptr<U> object,
            const std::string& message = "",
            const std::source_location& location = std::source_location::current())
        {
            auto casted = std::dynamic_pointer_cast<T>(object);
            if (!casted) {
                std::string msg = message.empty()
                    ? std::format("Expression is not an instance of required type ({}:{})",
                        location.file_name(), location.line())
                    : message;
                Fail(msg, location);
            }
            return casted;
        }

    private:
        static std::string GetFailedAssertionMessage(
            const std::string& message,
            const std::source_location& location)
        {
            return std::format("{}Failed assertion\n  at {}:{} in {}",
                message.empty() ? "" : message + "\n",
                location.file_name(),
                location.line(),
                location.function_name());
        }

        [[noreturn]]
        static void InvokeErrorHandler(const std::string& message)
        {
            GetErrorHandler()(message);
            std::terminate(); // Should never reach here
        }

        static ErrorHandler& GetErrorHandler()
        {
            static ErrorHandler handler = [](const std::string& msg) {
                throw std::runtime_error(msg);
                };
            return handler;
        }
    };
}