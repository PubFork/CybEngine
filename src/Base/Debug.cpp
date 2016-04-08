#include "Precompiled.h"
#include "Debug.h"
#include "Macros.h"
#include "FileUtils.h"
#include <windows.h>

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
    static char buffer[KILOBYTES(4)];
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
    WriteDataToFile(filename, logStream.str(), logStream.pcount());
}