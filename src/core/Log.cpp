#include "stdafx.h"
#include "Log.h"

namespace core
{

std::ostrstream logStream;

FatalException::FatalException(const std::string &message) :
    errorMessage( message )
{
#ifdef _DEBUG
    if (IsDebuggerPresent()) {
        _CrtDbgBreak();
    }
#endif
}

const char *FatalException::what() const
{
    return errorMessage.c_str();
}

void LogText(const char *fmt, ...)
{
    assert(fmt);
    static char buffer[1024];
    va_list args;

    va_start(args, fmt);
    int messageLength = vsnprintf(buffer, sizeof(buffer) - 1, fmt, args);
    va_end(args);

    // append a newline to the message
    if (messageLength > 0) {
        buffer[messageLength + 0] = '\n';
        buffer[messageLength + 1] = '\0';

        OutputDebugStringA(buffer);
        logStream << buffer;
    }
}

void LogSaveToFile(const char *filename)
{
    std::ofstream file(filename);
    DEBUG_LOG_TEXT_COND(!file.is_open(), "Coult't open %s for writing", filename);
    if (file.is_open()) {
        file.write(logStream.str(), logStream.pcount());
    }
}

}   // core