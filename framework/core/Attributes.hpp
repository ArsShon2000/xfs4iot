#include <string>

namespace XFS4IoT
{
    /// <summary>
    /// Command attribute metadata
    /// Used for registering command handlers
    /// </summary>
    struct CommandAttribute
    {
        std::string name;

        explicit CommandAttribute(std::string n) : name(std::move(n)) {}
    };


    /// <summary>
    /// XFS4 version attribute
    /// </summary>
    struct XFS4VersionAttribute
    {
        std::string version;

        explicit XFS4VersionAttribute(std::string v) : version(std::move(v)) {}
    };

    /// <summary>
    /// Completion attribute metadata
    /// </summary>
    struct CompletionAttribute
    {
        std::string name;

        explicit CompletionAttribute(std::string n) : name(std::move(n)) {}
    };

    /// <summary>
    /// Event attribute metadata
    /// </summary>
    struct EventAttribute
    {
        std::string name;

        explicit EventAttribute(std::string n) : name(std::move(n)) {}
    };

    /// <summary>
    /// Acknowledge attribute metadata
    /// </summary>
    struct AcknowledgeAttribute
    {
        std::string name;

        explicit AcknowledgeAttribute(std::string n) : name(std::move(n)) {}
    };
}