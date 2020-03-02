#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace kaun {
/*
 * I thought about having two hardcoded handlers (for file and console)
 * with changeable log level and enabling, but it is not much more work
 * to make it a variable number of handlers, so I'll do that
 *
 * I also considered having the log handlers get all the information the log
 * command gets, so that a game may later implement a log handler, which
 * can then distinguish the errors and decide what to do in a game specific way
 * but I also had to include a short unique identifier for each error (which is bothersome)
 * Also I decided that for debugging console/file is enough and in release
 * the log can contain further info and the game itself only has to say that
 * *something* went wrong and then only decide if it wants to keep going or not
 * and possibly close
 *
 * Also I don't want the handlers to mess up the formatting and have
 * different formatting through different handlers
 */

// TODO: Thread safety

enum class LogLevel : unsigned {
    // These are named weirdly, because DEBUG and ERROR are common defines
    LVL_DEBUG = 0,
    LVL_INFO = 1,
    LVL_WARNING = 2,
    LVL_ERROR = 3,
    LVL_CRITICAL = 4
};

extern const char* levelNameMap[5];

class LoggingHandler {
private:
    LogLevel mLogLevel;

public:
    LoggingHandler()
        : mLogLevel(LogLevel::LVL_DEBUG)
    {
    }
    virtual ~LoggingHandler()
    {
    }

    void setLogLevel(LogLevel level)
    {
        mLogLevel = level;
    }
    LogLevel getLogLevel() const
    {
        return mLogLevel;
    }

    virtual void log(LogLevel level, const char* str) = 0;
};

class FileLoggingHandler : public LoggingHandler {
private:
    std::string filename;

public:
    FileLoggingHandler(const char* filename, LogLevel level = LogLevel::LVL_DEBUG)
        : filename(filename)
    {
        setLogLevel(level);
    }
    ~FileLoggingHandler()
    {
    }

    void log(LogLevel level, const char* str);
};

class ConsoleLoggingHandler : public LoggingHandler {
public:
    ConsoleLoggingHandler(LogLevel level = LogLevel::LVL_DEBUG)
    {
        setLogLevel(level);
    }
    ~ConsoleLoggingHandler()
    {
    }

    void log(LogLevel level, const char* str)
    {
        std::cout << str << std::endl;
    }
};

extern std::vector<std::unique_ptr<LoggingHandler>> loggingHandlers;
extern std::string loggingFormat;

void setupDefaultLogging();

void log(LogLevel level, const char* filename, int line, const char* format, ...);

// http://stackoverflow.com/questions/5588855/standard-alternative-to-gccs-va-args-trick
#define LOG_DEBUG(_msg, ...)                                                                       \
    kaun::log(kaun::LogLevel::LVL_DEBUG, __FILE__, __LINE__, _msg, ##__VA_ARGS__)
#define LOG_INFO(_msg, ...)                                                                        \
    kaun::log(kaun::LogLevel::LVL_INFO, __FILE__, __LINE__, _msg, ##__VA_ARGS__)
#define LOG_WARNING(_msg, ...)                                                                     \
    kaun::log(kaun::LogLevel::LVL_WARNING, __FILE__, __LINE__, _msg, ##__VA_ARGS__)
#define LOG_ERROR(_msg, ...)                                                                       \
    kaun::log(kaun::LogLevel::LVL_ERROR, __FILE__, __LINE__, _msg, ##__VA_ARGS__)
#define LOG_CRITICAL(_msg, ...)                                                                    \
    kaun::log(kaun::LogLevel::LVL_CRITICAL, __FILE__, __LINE__, _msg, ##__VA_ARGS__)
}
