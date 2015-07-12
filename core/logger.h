#pragma once

#include <sstream>
#include <fstream>
#include <list>
#include <memory>
#include <list>
#include <ctime>

#define CYB_TO_STRING( x ) dynamic_cast<std::ostringstream &>( std::ostringstream() << std::dec << x ).str()

#define	CYB_DEFAULT_LOG_DEBUG	true
#define CYB_DEFAULT_LOG_INFO	true
#define CYB_DEFAULT_LOG_WARNING	true
#define CYB_DEFAULT_LOG_ERROR	true
#define CYB_DEFAULT_LOG_FORMAT	"[%L]: %m"
//#define CYB_DEFAULT_LOG_FORMAT	"%t [%L] %F@%f(%l): %m"

#ifdef _MSC_VER
#	include <string.h>
#	define CYB_DEBUG_BREAK() __debugbreak()
#	define __BASE_FILE__ ( strrchr( __FILE__, '\\' ) ? strrchr( __FILE__, '\\' ) + 1 : __FILE__ )
#else
#	define CYB_DEBUG_BREAK()()
#endif

#define CYB_LOG_MESSAGE( level, ... )		cyb::g_logger->Write( level, __BASE_FILE__, __FUNCTION__, __LINE__, __VA_ARGS__ )
#define CYB_DEBUG( ... )					CYB_LOG_MESSAGE( cyb::LogLevel::Debug,   __VA_ARGS__ )
#define CYB_INFO( ... )						CYB_LOG_MESSAGE( cyb::LogLevel::Info,    __VA_ARGS__ )
#define CYB_WARNING( ... )					CYB_LOG_MESSAGE( cyb::LogLevel::Warning, __VA_ARGS__ )
#define CYB_ERROR( ... )					CYB_LOG_MESSAGE( cyb::LogLevel::Error,   __VA_ARGS__ )
#define	CYB_CHECK( condition, ... )			if ( !(condition) ) { CYB_DEBUG( __VA_ARGS__ ); CYB_DEBUG_BREAK(); }
#define CYB_FATAL( condition, ... )			if ( !(condition) ) { CYB_ERROR( __VA_ARGS__ ); abort(); }

namespace cyb {

enum class LogLevel {
	Debug		= 0x01,
	Info		= 0x02,
	Warning		= 0x04,
	Error		= 0x08
};

struct LogMessage {
	std::string	msg;
	LogLevel level;
	time_t timestamp;
	std::string sourceFile;
	std::string sourceFunction;
	uint32_t sourceLine;
};

class LogHistory {
public:
	explicit LogHistory( const size_t size );
	~LogHistory() = default;

	/**
	 * Add a new LogMessage to history, removing any old entries
	 * not fitting withing the max size.
	 *
	 * @param entry Pointer to a message to be copied to the history.
	 * @return Returns true if old message(s) were removed, else false.
	 */
	bool AddMessage( const LogMessage *entry );

	/**
	 * Shrink the history entry list to specified size, if size
	 * is bigger than entry list nothing will be done.
	 *
	 * @param size Shrink the history entry list to this size.
	 * @return Returns the number of entries removed from the history list.
	 */
	size_t ShrinkToFit( const size_t size );

	/**
	 * Change the size of the history entry list.
	 *
	 * @param size New size for the history entry list.
	 * @return Returns number of entries removed due to the size change.
	 */
	size_t ChangeMaxSize( const size_t size );

	/**
	 * Get a const reference to all entries.
	 *
	 * @return Returns a const reference to all entries.
	 */
	const std::list<LogMessage> &Entries() const {
		return m_entries;
	}

private:
	size_t m_maxSize;
	std::list<LogMessage> m_entries;
};

/**
 * Settings for the log policy.
 * Format specifiers:
 *   %t		- Timestamp
 *   %L		- Log level 
 *   %f		- Source file 
 *   %l		- Source line 
 *   %F		- Source function
 *   %m		- The 'raw' log message
 *   %%		- Prints '%'
 */
struct LogPolicySettings {
	bool logDebug;
	bool logInfo;
	bool logWarning;
	bool logError;
	std::string format;
};

class LogPolicy
{
public:
	explicit LogPolicy( const LogPolicySettings *settings );
	~LogPolicy() = default;

	/**
	 * Get a formatted string.
	 * Formats the log message using the policy settings and adds a new line.
	 * 
	 * @param entry The log message to format.
	 * @return Returns a string with the formatted message.
	 */
	std::string	GetFormattedMessage( const LogMessage *entry ) const;

	/**
	 * Check if LogLevel is enabled by the policy settings.
	 *
	 * @param level Level to check against.
	 * @return Returns true if level is enabled.
	 */
	bool ShouldLog( const LogLevel level ) const;

	/**
	 * Write the message to the policy.
	 * This will check if message should be logged, format the message and
	 * dispatch it to the policy's write implementation.
	 *
	 * @param entry The message entry to process.
	 */
	void WriteMessage( const LogMessage *entry );

	/**
	 * Write all entries from a log history.
	 *
	 * @param history The log history to write.
	 */
	void WriteHistory( const LogHistory *history );

	/**
	 * Get a const pointer to the settings.
	 * 
	 * @return Returns the settings.
	 */
	const LogPolicySettings *Settings() const {
		return &m_settings;
	}

	/**
	 * Virtual function.
	 * Get a unique 32bit hash value for the policy.
	 *
	 * @return Returns a 32bit hash value.
	 */
	virtual uint32_t Hash() const = 0;

private:
	/**
	 * Virtual function.
	 * Implements the actual writing for the policy.
	 * 
	 * @param massage The message string to write.
	 */
	virtual void WriteImpl( const char *message ) = 0;
	
	LogPolicySettings m_settings;
};

/**
 * A log policy that writes to the standard output (stdout).
 * All LogPolicy_Stdout has the same hash, thus only one stdout
 * policy should exist in the logger.
 */
class LogPolicy_Stdout : public LogPolicy {
public:
	explicit LogPolicy_Stdout( const LogPolicySettings *settings ) : LogPolicy( settings ) {}
	virtual uint32_t Hash() const final { return 0x12345678; }

private:
	virtual void WriteImpl( const char *message ) final {
		printf( "%s", message );
	}
};

class LogPolicy_File : public LogPolicy {
public:
	explicit LogPolicy_File( const LogPolicySettings *settings );
	virtual uint32_t Hash() const final { return m_hash; }

private:
	virtual void WriteImpl( const char *message ) final;

	uint32_t m_hash;
	std::string m_filename;
	std::ofstream m_file;
};

struct LogPolicyOperation {
	enum Enum {
		None			= 0x00,
		CopyHistory		= 0x01,		//< Copy the history of the logger to the policy.
		Force			= 0x02		//< Add the policy even it it's hash is allready in the logger.
	};
};

class Logger
{
public:
	explicit Logger();
	~Logger() = default;

	/**
	 * Add a new policy to the Logger.
	 * The policy will not be added is it's hash is allready in a 
	 * added policy.
	 *
	 * @param policy A unique policy to add.
	 * @param operationFlags Set of operations to perform on the policy, see LogPolicyOperation for options.
	 * @return Returns false is the policys hash is mathed in the logger, else true.
	 */
	bool AddPolicy( std::unique_ptr<LogPolicy> policy, const uint32_t operationFlags );

	/**
	 * Write a new log message entry.
	 * This will create a new message entry, add it to history and then
	 * dispatch it to all added policies.
	 *
	 * @param level The message log level.
	 * @param sourceFile The source filename.
	 * @param sourceFunction The source function.
	 * @param sourceLine The source line.
	 * @param args The log message.
	 */
	template <typename... Args> 
	void Write( const LogLevel level, const char *sourceFile, const char *sourceFunction, const uint32_t sourceLine, const Args &...args ) {
		LogMessage entry = {};
		entry.level = level;
		entry.sourceFile = sourceFile;
		entry.sourceFunction = sourceFunction;
		entry.sourceLine = sourceLine;
		entry.timestamp = time( NULL );

		WriteImpl( &entry, args... );
		m_history.AddMessage( &entry );

		for ( auto &policy : m_policies ) {
			policy->WriteMessage( &entry );
		}
	}

	/**
	 * Write implementation.
	 * Parses the variadic template, adding one part or the args to the log message.
	 *
	 * @param entry The log message entry.
	 * @param first The arg to parse and add to message.
	 * @param rest All the rest of the args.
	 */
	template <typename First, typename... Rest>
	void WriteImpl( LogMessage *entry, const First &first, const Rest &...rest ) const {
		entry->msg += CYB_TO_STRING( first );
		WriteImpl( entry, rest... );
	}

	/**
	 * Write implementation.
	 * Parses the variadic template, adding the last part or the args to the log message.
	 *
	 * @param entry The log message entry.
	 * @param last The last arg in the variadic template.
	 */
	template <typename Last>
	void WriteImpl( LogMessage *entry, const Last &last ) const {
		entry->msg += CYB_TO_STRING( last );
	}

	/**
	 * Get a const pointer to the log history.
	 *
	 * @return Returns a log history.
	 */
	const LogHistory *History() const {
		return &m_history;
	}

private:
	std::list<std::unique_ptr<LogPolicy>> m_policies;
	LogHistory m_history;
};

extern Logger *g_logger;

}	// namespace cyb