#pragma once

namespace core
{

class FatalException : public std::exception
{
public:
    FatalException(const std::string &message);
    virtual ~FatalException() = default;

    virtual const char *what() const final;

private:
    std::string errorMessage;
};

void LogText(const char *fmt, ...);
void LogSaveToFile(const char *filename);

}   // core

#pragma warning(disable : 4127)

#define THROW_FATAL_COND(c, msg)        do { if ((c)) { throw core::FatalException(msg); }} while (0)
#define DEBUG_LOG_TEXT(...)             do { core::LogText(__VA_ARGS__); }  while (0)
#define DEBUG_LOG_TEXT_COND(c, ...)     do { if ((c)) { core::LogText(__VA_ARGS__); }} while (0)
