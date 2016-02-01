#pragma once

class FatalException : public std::exception
{
public:
    FatalException(const std::string &message);
    virtual ~FatalException() = default;

    virtual const char *what() const final;

private:
    std::string errorMessage;
};


void DebugLogText(const char *fmt, ...);
void SaveDebugLogToFile(const char *filename);

#pragma warning(disable : 4127)

#define THROW_FATAL_COND(c, msg)        do { if ((c)) { throw FatalException(msg); }} while (0)
#define DEBUG_LOG_TEXT(...)             do { DebugLogText(__VA_ARGS__); }  while (0)
#define DEBUG_LOG_TEXT_COND(c, ...)     do { if ((c)) { DebugLogText(__VA_ARGS__); }} while (0)
