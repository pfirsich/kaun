#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <sstream>
#include <stdio.h>
#include <time.h>
#include <unordered_map>

#include <iostream>

#include "log.hpp"

namespace kaun {
const std::vector<std::string> levelNameMap { "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL" };

std::vector<std::unique_ptr<LoggingHandler>> loggingHandlers;
const std::string loggingFormat(
    "[{y}.{m}.{d} {H}:{M}:{S}] [{filename}:{line}] {levelname} - {message}");

void setupDefaultLogging()
{
    //#ifdef DEBUG
    loggingHandlers.emplace_back(new ConsoleLoggingHandler());
    //#endif
    loggingHandlers.emplace_back(new FileLoggingHandler("log.txt", LogLevel::LVL_INFO));
}

std::string formatString(
    const std::string& format, const std::unordered_map<std::string, std::string>& argMap)
{
    std::string ret = "";
    for (std::string::size_type i = 0; i < format.size(); ++i) {
        if (format[i] == '{') {
            if (format[i + 1] == '{') {
                ret += '{';
                ++i;
            } else {
                std::string::size_type keyStart = ++i;
                std::string key = "";
                for (; i < format.size(); ++i) {
                    if (format[i] == '}') {
                        if (format[i + 1] == '}') {
                            ++i;
                        } else {
                            key = format.substr(keyStart, i - keyStart);
                            break;
                        }
                    }
                }

                auto arg = argMap.find(key);
                if (arg != argMap.end()) {
                    ret += arg->second;
                } else {
                    ret += "{UNKNOWN ARGUMENT:'" + key + "'}";
                }
            }
        } else if (format[i] == '}') {
            if (format[i + 1] == '}') {
                ret += "}";
            } else {
                // What do I do here?
            }
        } else {
            ret += format[i];
        }
    }
    return ret;
}

std::tm* localtime(std::time_t* t)
{
    // Is this sane?
    static std::tm tm;
#ifdef _MSC_VER
    localtime_s(&tm, t);
#else
    localtime_r(t, &tm);
#endif
    return &tm;
}

void log(LogLevel level, const char* filename, int line, const char* format, ...)
{
    std::unordered_map<std::string, std::string> formatArguments;
    unsigned intLevel = static_cast<unsigned>(level);
    formatArguments["levelname"]
        = intLevel < levelNameMap.size() ? levelNameMap[intLevel] : std::to_string(intLevel);
    formatArguments["filename"] = filename;
    formatArguments["line"] = std::to_string(line);

    std::time_t t = std::time(nullptr);
    std::tm* tm = localtime(&t);
    formatArguments["d"] = std::to_string(tm->tm_mday);
    formatArguments["m"] = std::to_string(tm->tm_mon);
    formatArguments["y"] = std::to_string(tm->tm_year + 1900);
    formatArguments["H"] = std::to_string(tm->tm_hour);
    formatArguments["M"] = std::to_string(tm->tm_min);
    formatArguments["S"] = std::to_string(tm->tm_sec);

    va_list args;
    va_start(args, format);
    va_list argsCopy; // It's modified by vsnprintf, so we need a copy to get the length
    va_copy(argsCopy, args);
    // This might be slow
    // +1 for null-termination
    const int len = vsnprintf(nullptr, 0, format, argsCopy) + 1;
    va_end(argsCopy);
    const auto buffer = std::make_unique<char[]>(len);
    vsnprintf(buffer.get(), len, format, args);
    va_end(args);
    formatArguments["message"] = buffer.get();

    const std::string logLine = formatString(loggingFormat, formatArguments);

    for (auto& handler : loggingHandlers) {
        if (static_cast<unsigned>(level) >= static_cast<unsigned>(handler->getLogLevel())) {
            handler->log(level, logLine.c_str());
        }
    }
}

void FileLoggingHandler::log(LogLevel level, const char* str)
{
    // TODO: do something here
}
}
