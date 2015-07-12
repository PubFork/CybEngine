#include <iostream>
#include <cassert>
#include <ctime>
#include <chrono>
#include <iomanip>
#include "config.h"
#include "logger.h"

namespace cyb {

static Logger s_defaultLogger;
Logger *g_logger = &s_defaultLogger;

static inline const char *LogLevelToString( const LogLevel level ) {
	switch ( level ) {
	case LogLevel::Debug:	return "Debug";
	case LogLevel::Info:	return "Info";
	case LogLevel::Warning:	return "Warning";
	case LogLevel::Error:	return "Error";
	};

	return "Unknown";
}

LogHistory::LogHistory( const size_t size ) {
	m_maxSize = size;
}

bool LogHistory::AddMessage( const LogMessage *entry ) {
	if ( !entry ) {
		return false;
	}

	// ensure there's room for this message
	const size_t entriesRemoved = ShrinkToFit( m_maxSize - 1 );

	LogMessage msg = {};
	msg.level = entry->level;
	msg.timestamp = entry->timestamp;
	msg.sourceFile = entry->sourceFile;
	msg.sourceFunction = entry->sourceFunction;
	msg.sourceLine = entry->sourceLine;
	msg.msg = entry->msg;
	m_entries.emplace_back( msg );

	return ( entriesRemoved > 0 );
}

size_t LogHistory::ShrinkToFit( const size_t size ) {
	size_t removeCount = 0;
	while ( m_entries.size() > size ) {
		m_entries.pop_front();
		++removeCount;
	}

	return removeCount;
}

size_t LogHistory::ChangeMaxSize( const size_t size ) {
	m_maxSize = size;
	return ShrinkToFit( size );
}

LogPolicy::LogPolicy( const LogPolicySettings *settings ) {
	if ( settings ) {
		memcpy( &m_settings, settings, sizeof( LogPolicySettings ) );
	} else {
		m_settings.logDebug   = CYB_DEFAULT_LOG_DEBUG;
		m_settings.logInfo    = CYB_DEFAULT_LOG_INFO;
		m_settings.logWarning = CYB_DEFAULT_LOG_WARNING;
		m_settings.logError   = CYB_DEFAULT_LOG_ERROR;
		m_settings.format     = CYB_DEFAULT_LOG_FORMAT;
	}
}

std::string LogPolicy::GetFormattedMessage( const LogMessage *entry ) const {
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
				stream << LogLevelToString( entry->level );
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

bool LogPolicy::ShouldLog( const LogLevel level ) const {
	switch ( level ) {
	case LogLevel::Debug:	return m_settings.logDebug;
	case LogLevel::Info:	return m_settings.logInfo;
	case LogLevel::Warning:	return m_settings.logWarning;
	case LogLevel::Error:	return m_settings.logError;
	}

	return false;
}

void LogPolicy::WriteMessage( const LogMessage *entry ) {
	if ( ShouldLog( entry->level ) ) {
		std::string formattedMessage = GetFormattedMessage( entry );
		WriteImpl( formattedMessage.c_str() );
	}
}

void LogPolicy::WriteHistory( const LogHistory *history ) {
	if ( !history ) {
		return;
	}

	for ( const auto &entry : history->Entries() ) {
		WriteMessage( &entry );
	}
}

Logger::Logger() : m_history( LOG_HISTORY_SIZE ) {
}

bool Logger::AddPolicy( std::unique_ptr<LogPolicy> policy, const uint32_t operationFlags ) {
	if ( ( operationFlags & LogPolicyOperation::Force ) == 0 ) {
		for ( const auto &it : m_policies ) {
			if ( it->Hash() == policy->Hash() ) {
				return false;
			}
		}
	}

	CYB_DEBUG( "Adding log policy with hash ", policy->Hash() );

	if ( operationFlags & LogPolicyOperation::CopyHistory ) {
		policy->WriteHistory( &m_history );
	}

	m_policies.push_back( std::move( policy ) );
	return true;
}

}	// namespace cyb