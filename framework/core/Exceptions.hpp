#pragma once

#include <stdexcept>
#include <string>

namespace XFS4IoT
{
    /// <summary>
    /// Exception class on detecting an invalid data being received or sent, currently being used on receiving command has unexpected data
    /// and then automatically send a response with the invalid data error code to the command.
    /// </summary>
    /// <param name="message">This string parameter will be set to the error description common payload on response.
    /// it would be good to set meaningful description for an application developer.
    /// </param>
    class InvalidDataException : public std::runtime_error
    {
    public:
        explicit InvalidDataException(const std::string& message)
            : std::runtime_error(message)
        {
        }
    };

    /// <summary>
    /// Exception class on detecting an internal error on handling command in device class or framework.
    /// </summary>
    /// <param name="message">This string parameter will be set to the error description common payload on response.
    /// it would be good to set meaningful description for an application developer.
    /// </param>
    class InternalErrorException : public std::runtime_error
    {
    public:
        explicit InternalErrorException(const std::string& message)
            : std::runtime_error(message)
        {
        }
    };

    /// <summary>
    /// Exception class on detecting unsupported data on execute command if the device doesn't support specified capability.
    /// </summary>
    /// <param name="message">This string parameter will be set to the error description common payload on response.
    /// it would be good to set meaningful description for an application developer.
    /// </param>
    class UnsupportedDataException : public std::runtime_error
    {
    public:
        explicit UnsupportedDataException(const std::string& message)
            : std::runtime_error(message)
        {
        }
    };

    /// <summary>
    /// Exception class on detecting a sequential command error.
    /// </summary>
    /// <param name="message">This string parameter will be set to the error description common payload on response.
    /// it would be good to set meaningful description for an application developer.
    /// </param>
    class SequenceErrorException : public std::runtime_error
    {
    public:
        explicit SequenceErrorException(const std::string& message)
            : std::runtime_error(message)
        {
        }
    };

    /// <summary>
    /// Exception class on detecting an operation requires authorisation data to perform sensitive operations.
    /// </summary>
    /// <param name="message">This string parameter will be set to the error description common payload on response.
    /// it would be good to set meaningful description for an application developer.
    /// </param>
    class AuthorisationRequiredException : public std::runtime_error
    {
    public:
        explicit AuthorisationRequiredException(const std::string& message)
            : std::runtime_error(message)
        {
        }
    };

    /// <summary>
    /// Exception class on facing hardware error while executing an operation.
    /// </summary>
    /// <param name="message">This string parameter will be set to the error description common payload on response.
    /// it would be good to set meaningful description for an application developer.
    /// </param>
    class HardwareErrorException : public std::runtime_error
    {
    public:
        explicit HardwareErrorException(const std::string& message)
            : std::runtime_error(message)
        {
        }
    };

    /// <summary>
    /// Exception class on detecting vandalism
    /// </summary>
    /// <param name="message">This string parameter will be set to the error description common payload on response.
    /// it would be good to set meaningful description for an application developer.
    /// </param>
    class UserErrorException : public std::runtime_error
    {
    public:
        explicit UserErrorException(const std::string& message)
            : std::runtime_error(message)
        {
        }
    };

    /// <summary>
    /// Exception class on detecting an fraud attempt and not possible to process other execute commands until it's being cleared.
    /// </summary>
    /// <param name="message">This string parameter will be set to the error description common payload on response.
    /// it would be good to set meaningful description for an application developer.
    /// </param>
    class FraudAttemptException : public std::runtime_error
    {
    public:
        explicit FraudAttemptException(const std::string& message)
            : std::runtime_error(message)
        {
        }
    };

    /// <summary>
    /// Exception class on detecting on handling command in device class when device is not ready to respond.
    /// </summary>
    /// <param name="message">This string parameter will be set to the error description common payload on response.
    /// it would be good to set meaningful description for an application developer.
    /// </param>
    class DeviceNotReadyException : public std::runtime_error
    {
    public:
        explicit DeviceNotReadyException(const std::string& message)
            : std::runtime_error(message)
        {
        }
    };

    /// <summary>
    /// Exception class on detecting an invalid command on handling command in device class or framework.
    /// </summary>
    /// <param name="message">This string parameter will be set to the error description common payload on response.
    /// it would be good to set meaningful description for an application developer.
    /// </param>
    class InvalidCommandException : public std::runtime_error
    {
    public:
        explicit InvalidCommandException(const std::string& message)
            : std::runtime_error(message)
        {
        }
    };

    /// <summary>
    /// Exception class on there is not enough storage on the device for the requested action.
    /// </summary>
    /// <param name="message">This string parameter will be set to the error description common payload on response.
    /// it would be good to set meaningful description for an application developer.
    /// </param>
    class NotEnoughSpaceException : public std::runtime_error
    {
    public:
        explicit NotEnoughSpaceException(const std::string& message)
            : std::runtime_error(message)
        {
        }
    };

    /// <summary>
    /// Exception class for unsupported commands.
    /// </summary>
    /// <param name="message">This string parameter will be set to the error description common payload on response.
    /// it would be good to set meaningful description for an application developer.
    /// </param>
    class UnsupportedCommandException : public std::runtime_error
    {
    public:
        explicit UnsupportedCommandException(const std::string& message)
            : std::runtime_error(message)
        {
        }
    };

    class NotImplementedException : public std::runtime_error
    {
    public:
        explicit NotImplementedException(const std::string& message)
            : std::runtime_error(message)
        {
        }
    };

    class NotSupportedException : public std::runtime_error
    {
    public:
        explicit NotSupportedException(const std::string& message)
            : std::runtime_error(message)
        {
        }
    };
}