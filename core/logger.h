#pragma once

#include <time.h>
#include <sstream>
#include <vector>

#ifdef _MSC_VER
#	include <string.h>
#	define CYB_DEBUG_BREAK() __debugbreak()
#	define __BASE_FILE__ ( strrchr( __FILE__, '\\' ) ? strrchr( __FILE__, '\\' ) + 1 : __FILE__ )
#else
#	define CYB_DEBUG_BREAK()()
#endif

#define CYB_LOG_MESSAGE( level, ... )		g_logWriter->Write( level, __BASE_FILE__, __FUNCTION__, __LINE__, __VA_ARGS__ )
#define CYB_DEBUG( ... )					CYB_LOG_MESSAGE( logSeverity_t::Debug,   __VA_ARGS__ )
#define CYB_INFO( ... )						CYB_LOG_MESSAGE( logSeverity_t::Info,    __VA_ARGS__ )
#define CYB_WARNING( ... )					CYB_LOG_MESSAGE( logSeverity_t::Warning, __VA_ARGS__ )
#define CYB_ERROR( ... )					CYB_LOG_MESSAGE( logSeverity_t::Error,   __VA_ARGS__ )
#define	CYB_CHECK( condition, ... )			if ( !(condition) ) { CYB_DEBUG( __VA_ARGS__ ); CYB_DEBUG_BREAK(); }
#define CYB_FATAL( condition, ... )			if ( !(condition) ) { CYB_ERROR( __VA_ARGS__ ); abort(); }

/** When used as filtering, any message with sevetiry < filter is dropped. */
enum class logSeverity_t {
	Debug,
	Info,
	Warning,
	Error
};

/** The main message data struct. */
// TODO: Maybe replace the std::strings and make it POD.
struct logMessage_t {
	std::string	msg;
	logSeverity_t level;
	time_t timestamp;
	std::string sourceFile;
	std::string sourceFunction;
	uint32_t sourceLine;
};

/** Log history public interface. */
class ILogHistory {
public:
	// max number of saved logMessages
	enum { HistorySize = 64 };

	virtual ~ILogHistory() {}
	virtual void AddMessage( const logMessage_t *msg ) = 0;
};

/**
*  Format specifiers for log policy:
*   %t		- Timestamp
*   %L		- Log level
*   %f		- Source file
*   %l		- Source line
*   %F		- Source function
*   %m		- The 'raw' log message
*   %%		- Prints '%'
*/
struct logPolicySettings_t {
	logSeverity_t severityFilter;
	std::string format;
};

/** Base level log policy class. */
class LogPolicy {
public:
	explicit LogPolicy( const logPolicySettings_t *settings );
	~LogPolicy() = default;

	std::string	GetFormattedMessage( const logMessage_t *entry ) const;
	bool ShouldLog( const logSeverity_t severity ) const;	
	void WriteMessage( const logMessage_t *msg );
	virtual void WriteString( const char *str ) = 0;
	
private:
	logPolicySettings_t m_settings;
};

/** The main logger class. */
class LogWriter {
public:
	explicit LogWriter();
	~LogWriter();

	void Reset( bool useConsole, logSeverity_t consoleSeverity, bool useFile, logSeverity_t fileSeverity, const char *logfile );
	bool Attach( LogPolicy *policy, bool copyHistory );
	void DestroyAllPolicies();

	/**
	 * Write a new log message entry.
	 * This will create a new message entry, add it to history and then
	 * dispatch it to all added policies.
	 * @param	level			The message log level.
	 * @param	sourceFile		The source filename.
	 * @param	sourceFunction	The source function.
	 * @param	sourceLine		The source line.
	 * @param	args			The log message.
	 */
	template <typename... Args> 
	void Write( const logSeverity_t level, const char *sourceFile, const char *sourceFunction, const uint32_t sourceLine, const Args &...args ) {
		logMessage_t entry = {};
		entry.level = level;
		entry.sourceFile = sourceFile;
		entry.sourceFunction = sourceFunction;
		entry.sourceLine = sourceLine;
		entry.timestamp = time( NULL );

		// write all args to stream buffer
		std::ostringstream stringStream;
		WriteImpl( stringStream, args... );
		entry.msg = stringStream.str();

		// add message to history and distribute it to the policies
		history->AddMessage( &entry );
		for ( auto &policy : policyList ) {
			policy->WriteMessage( &entry );
		}
	}

	/**
	 * Write implementation.
	 * Parses the variadic template, adding one part or the args to the log message.
	 * @param	entry			The log message entry.
	 * @param	first			The arg to parse and add to message.
	 * @param	rest			All the rest of the args.
	 */
	template <typename First, typename... Rest>
	void WriteImpl( std::ostringstream &stream,  const First &first, const Rest &...rest ) const {
		stream << first;
		WriteImpl( stream, rest... );
	}

	/**
	 * Write implementation.
	 * Parses the variadic template, adding the last part or the args to the log message.
	 * @param	entry			The log message entry.
	 * @param	last			The last arg in the variadic template.
	 */
	template <typename Last>
	void WriteImpl( std::ostringstream &stream, const Last &last ) const {
		stream << last;
	}

private:
	std::vector<LogPolicy *> policyList;
	ILogHistory *history;
};

extern LogWriter *g_logWriter;
