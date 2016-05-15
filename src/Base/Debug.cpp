#include "Precompiled.h"
#include "Debug.h"
#include "Algorithm.h"
#include "File.h"
#include <windows.h>

#define DEBUG_MESSAGE_BUFFER_SIZE   4096

std::ostrstream logStream;

FatalException::FatalException(const std::string &message) :
    errorMessage(message)
{
#ifdef _DEBUG
    if (IsDebuggerPresent())
        _CrtDbgBreak();
#endif
}

const char *FatalException::what() const
{
    return errorMessage.c_str();
}

void DebugLogText(const char *fmt, ...)
{
    assert(fmt);
    static char buffer[DEBUG_MESSAGE_BUFFER_SIZE];
    va_list args;

    va_start(args, fmt);
    int result = vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, args);
    va_end(args);

    // append a newline to the message
    size_t messageLength = std::min<size_t>(strlen(buffer), sizeof(buffer) - 2);
    buffer[messageLength + 0] = '\n';
    buffer[messageLength + 1] = '\0';

    OutputDebugStringA(buffer);
    logStream << buffer;

    // append a message to the logger is text was truncated
    if (result == -1)
    {
        snprintf(buffer, sizeof(buffer), "[[Message truncated]]\n");
        OutputDebugStringA(buffer);
        logStream << buffer;
    }
}

void SaveDebugLogToFile(const char *filename)
{
    SysFile logFile(filename, FileOpen_WriteTruncate);
    logFile.Write((uint8_t *)logStream.str(), logStream.pcount());
}