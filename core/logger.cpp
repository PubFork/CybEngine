#include <assert.h>
#include <iostream>
#include <fstream>
#include <list>
#include <iomanip>
#include "logger.h"

static LogWriter s_defaultLogger;
LogWriter *g_logWriter = &s_defaultLogger;

static const char *s_logLevelName[4] = {
	"Debug",
	"info",
	"Warning",
	"Error"
};

class LogHistoryLocal : public ILogHistory {
public:
	virtual ~LogHistoryLocal() {}
	virtual void AddMessage( const logMessage_t *msg ) final;
	void WriteToPolicy( LogPolicy *policy );

private:
	std::list<logMessage_t> history;
};

/**
 * Add a new LogMessage to history, removing any old entries
 * not fitting withing the max size.
 * @param	entry		Pointer to a message to be copied to the history.
 */
void LogHistoryLocal::AddMessage( const logMessage_t *msg ) {
	if ( !msg ) {
		return;
	}

	// copy the entry to the history list
	history.push_back( *msg );

	// remove old entries if it's to big
	while ( history.size() > HistorySize ) {
		history.pop_front();
	}
}

/** Write all the history entries to a log policy. */
void LogHistoryLocal::WriteToPolicy( LogPolicy *policy ) {
	assert( policy );
	for ( const auto &entry : history ) {
		policy->WriteMessage( &entry );
	}
}

/** Constructor. Copies settings. */
LogPolicy::LogPolicy( const logPolicySettings_t *settings ) {
	assert( settings );
	m_settings = *settings;
}

/**
 * Get a formatted string.
 * Formats the log message using the policy settings and adds a new line.
 * @param	entry			The log message to format.
 * @return A string with the formatted message.
 */
std::string LogPolicy::GetFormattedMessage( const logMessage_t *entry ) const {
	assert( entry );
	std::ostringstream stream;
	const std::string &format = m_settings.format;
	const std::string::size_type formatStrLength = format.length();

	for ( std::string::size_type i = 0; i < formatStrLength; i++ ) {
		if ( format[i] == '%' ) {
			switch ( format[i + 1] ) {
			case 't': 
				stream <<  std::put_time( gmtime( &entry->timestamp ), "%c" );
				break;
			case 'L':
				stream << s_logLevelName[(uint8_t)entry->level];
				break;
			case 'l':
				stream << entry->sourceLine;
				break;
			case 'f':
				stream << entry->sourceFile;
				break;
			case 'F':
				stream << entry->sourceFunction;
				break;
			case 'm':
				stream << entry->msg;
				break;
			case '%':
				stream << '%';
				break;
			default: break;
			}

			i++;
			continue;
		}

		stream << format[i];
	}

	stream << std::endl;
	return stream.str();
}

/**
 * Check severity agains the policys severity filter.
 * @param	severity		Severity level to check against.
 * @return True if policy severity filter is less than severity.
 */
bool LogPolicy::ShouldLog( const logSeverity_t severity ) const {
	return ( severity >= m_settings.severityFilter );
}

/**
 * Write a message to the policy.
 * Message will be properly formatted before it is written.
 * @param	msg				A non-null pointer to the message.
 */
void LogPolicy::WriteMessage( const logMessage_t *msg ) {
	assert( msg );
	if ( ShouldLog( msg->level ) ) {
		std::string formattedMessage = GetFormattedMessage( msg );
		WriteString( formattedMessage.c_str() );
	}
}

/** A log policy that writes to the standard output (stdout). */
class ConsoleLogPolicy : public LogPolicy {
public:
	explicit ConsoleLogPolicy( const logPolicySettings_t *settings ) : LogPolicy( settings ) {}
	virtual ~ConsoleLogPolicy() = default;
	virtual void WriteString( const char *str ) final {
		std::cout << str;
	}
};

/** A log policy that writes to a file. */
class FileLogPolicy : public LogPolicy {
public:
	explicit FileLogPolicy( const char *logFile, const logPolicySettings_t *settings ) :
		LogPolicy( settings ) {
		filename = logFile;
		file.open( filename );
	}
	virtual ~FileLogPolicy() = default;
	virtual void WriteString( const char *str ) final {
		assert( str );
		if ( file.is_open() ) {
			file << str;
		}
	}

private:
	std::string filename;
	std::ofstream file;
};

/** Constructor. No policies are attached by default. */
LogWriter::LogWriter() {
	history = new LogHistoryLocal();
}

/** Destructor. Delete history and all policies. */
LogWriter::~LogWriter() {
	DestroyAllPolicies();
	delete history;
}

/**
 * Reset the log writer with new settings (history will be kept ).
 * @param	useConsole		If set to true, a console policy will be attached to the log writer.
 * @param	consoleFilter	Severity filter on console policy (ignored if useConsole=false).
 * @param	useFile			If set to true, a file policy will be attached to the log writer.
 * @param	fileFilter		Severity filter on file policy (ignored if useFile=false).
 * @param	logfile			The output file used by the log policy, if nullptr logPolicy will be disabled (ignored if useFile=false)
 */
void LogWriter::Reset( bool useConsole, logSeverity_t consoleFilter, bool useFile, logSeverity_t fileFilter, const char *logfile ) {
	DestroyAllPolicies();

	if ( useConsole ) {
		logPolicySettings_t consoleSettings;
		consoleSettings.severityFilter = consoleFilter;
		consoleSettings.format = "[%L]: %m";
		ConsoleLogPolicy *consolePolicy = new ConsoleLogPolicy( &consoleSettings );
		Attach( consolePolicy, true );
	}

	if ( useFile && logfile != nullptr ) {
		assert( logfile );
		logPolicySettings_t fileSettings;
		fileSettings.severityFilter = fileFilter;
		fileSettings.format = "%t [%L] %F@%f(%l): %m";
		FileLogPolicy *filePolicy = new FileLogPolicy( logfile, &fileSettings );
		Attach( filePolicy, true );
	}
}

/**
 * Attach a new policy to the log writer, the policy has to be pre-allocated, however
 * when passed to the log writer it will take ownership and take care of freeing the
 * allocated memory.
 * @param	policy			The policy to attach.
 * @param	copyHistory		If true, the log writer history will be written to the policy.
 * @return False if policy is nullptr, else true.
 */
bool LogWriter::Attach( LogPolicy *policy, bool copyHistory ) {
	if ( !policy ) {
		return false;
	}

	if ( copyHistory ) {
		dynamic_cast<LogHistoryLocal *>(history)->WriteToPolicy( policy );
	}

	policyList.push_back( policy );
	return true;
}

/** Free all attached policies memory, and remove them from the log writer. */
void LogWriter::DestroyAllPolicies() {
	for ( uint32_t i = 0; i < policyList.size(); i++ ) {
		delete policyList[i];
	}

	policyList.clear();
}
