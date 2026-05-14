#include <string>
#include <map>

namespace XFS4IoT
{
	class Configurations
	{
	public:
		/// <summary>
		/// Server address for the service provider in URI format
		/// </summary>
		static constexpr const char* ServerAddressUri = "ServerAddressUri";

		/// <summary>
		/// Specific port to listen, must be within defined port in the XFS4IoT specification.
		/// 80, 443, 5846-5856.
		/// If this value is not configured or configured outside of range, 
		/// first available port will be used.
		/// </summary>
		static constexpr const char* ServerPort = "ServerPort";

		/// <summary>
		/// Set of default values
		/// </summary>
		struct Default
		{
			/// <summary>
			/// Default value of the server address
			/// </summary>
			static constexpr const char* ServerAddressUri = "http://127.0.0.1";
		};

		/// <summary>
		/// Get configuration value
		/// </summary>
		static std::string GetValue(const std::string& key,
			const std::string& defaultValue = "")
		{
			// Implementation would read from config file or environment variables
			// For example, using boost::property_tree or std::getenv

			// Example with environment variables:
			if (const char* env = std::getenv(key.c_str())) {
				return std::string(env);
			}

			return defaultValue;
		}

		/// <summary>
		/// Set configuration value
		/// </summary>
		static void SetValue(const std::string& key, const std::string& value)
		{
			GetConfigMap()[key] = value;
		}

	private:
		static std::map<std::string, std::string>& GetConfigMap()
		{
			static std::map<std::string, std::string> config;
			return config;
		}
	};

	// ========================================================================
	// Constants
	// ========================================================================

	/// <summary>
	/// Framework constants
	/// </summary>
	struct Constants
	{
		static constexpr const char* Component = "Server";
		static constexpr const char* Framework = "Framework";
	};
}