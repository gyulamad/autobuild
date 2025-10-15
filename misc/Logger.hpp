#pragma once

#include <string>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>
#include "ms_to_datetime.hpp"
#include "safe.hpp"
#include "F.hpp"

using namespace std;

#ifndef LOG_LEVEL_OUTPUT
    #ifdef DEBUG
        #define LOG_LEVEL_OUTPUT LOGLVL_ALL
    #else
        // NOTE:
        // Set LOG_LEVEL_OUTPUT to LOGLVL_INFO (or lower) in production 
        // if you don't want to see everything in the log
        #define LOG_LEVEL_OUTPUT LOGLVL_ALL 
    #endif
#endif

#ifndef LOG_LEVEL_FILELN
    #ifdef DEBUG
        // NOTE:
        // Set LOG_LEVEL_FILELN to LOGLVL_ALL (or lower) in DEBUG mode
        // if you want to see file:line infos for every log call in the log
        #define LOG_LEVEL_FILELN LOGLVL_ALERT
    #else
        #define LOG_LEVEL_FILELN LOGLVL_ALERT 
    #endif
#endif

enum LogLevel {
    LOGLVL_NONE = 0,
    LOGLVL_THROW,
    LOGLVL_ERROR,
    LOGLVL_ALERT,
    LOGLVL_WARNING,
    LOGLVL_INFO,
    LOGLVL_NOTE,
    LOGLVL_SUCCESS,
    LOGLVL_DEBUG,
    LOGLVL_ALL
};

static const map<LogLevel, string> logLevelMap = {
    { LOGLVL_THROW, F(F_ERROR, "throw") },
    { LOGLVL_ERROR, F(F_ERROR, "error") },
    { LOGLVL_ALERT, F(F_ALERT, "alert") },
    { LOGLVL_WARNING, F(F_WARNING, "warning") },
    { LOGLVL_INFO, F(F_INFO, "info") },
    { LOGLVL_NOTE, F(F_NOTE, "note") },
    { LOGLVL_SUCCESS, F(F_SUCCESS, "success") },
    { LOGLVL_DEBUG, F(F_DEBUG, "debug") },
};

class Logger {
public:
    Logger() {}
    virtual ~Logger() {}

    void set_level_output(LogLevel level_output) { this->level_output = level_output; } 
    void set_level_fileln(LogLevel level_fileln) { this->level_fileln = level_fileln; } 

    void throws(const string& message, const string& fileln, bool throws = true) { log(LOGLVL_THROW, message, fileln, throws); }
    void error(const string& message, const string& fileln, bool throws = false) { log(LOGLVL_ERROR, message, fileln, throws); }
    void alert(const string& message, const string& fileln, bool throws = false) { log(LOGLVL_ALERT, message, fileln, throws); }
    void warning(const string& message, const string& fileln, bool throws = false) { log(LOGLVL_WARNING, message, fileln, throws); }
    void info(const string& message, const string& fileln, bool throws = false) { log(LOGLVL_INFO, message, fileln, throws); }
    void note(const string& message, const string& fileln, bool throws = false) { log(LOGLVL_NOTE, message, fileln, throws); }
    void success(const string& message, const string& fileln, bool throws = false) { log(LOGLVL_SUCCESS, message, fileln, throws); }
    void debug(const string& message, const string& fileln, bool throws = false) { log(LOGLVL_DEBUG, message, fileln, throws); }

protected:

    virtual void log(LogLevel level, const string& message, const string& fileln, bool throws = false) {
        if (level > LOGLVL_NONE && level <= this->level_output) {
            string output = "[" + logLevelMap.at(level == LOGLVL_ALL ? LOGLVL_THROW : level) + "] " + message;            
            
            if (level == LOGLVL_ALL)
                throw ERROR("Logger can not write to all level (LOGLVL_ALL): " + output);
            
            if (level <= this->level_fileln || level == LOGLVL_DEBUG) {
                output += fileln;
            }

            string time = this->time();
            if (!time.empty())
                output = time + " " + output;

            {
                lock_guard<mutex> lock(mtx);
                this->write(output);
            }

            if (level == LOGLVL_THROW || throws)
                throw ERROR("Logger throws: " + output);
        }
    }

    virtual void write(const string& output) = 0;
    virtual string time() { return ms_to_datetime(); }
    
    atomic<LogLevel> level_output = LOG_LEVEL_OUTPUT;
    atomic<LogLevel> level_fileln = LOG_LEVEL_FILELN;
    mutex mtx;
};

class LoggerFactory {
public:
    template<typename L>
    static Logger* createLogger() {
        Logger* logger = getLogger();
        if (logger) return logger;
        static L l;
        instance().setLogger(&l);
        return getLogger();
    }

    static void setLogger(Logger* logger) {
        instance().logger = logger;
    }

    static Logger* getLogger() {
        return instance().logger;
    }

private:
    static LoggerFactory& instance() {
        static LoggerFactory factory;
        return factory;
    }
    Logger* logger = nullptr;
};
template<typename L> static Logger* createLogger() { return LoggerFactory::createLogger<L>(); }
void logger(Logger* logger) { LoggerFactory::setLogger(logger); }
Logger* logger() { return LoggerFactory::getLogger(); }


#define LOG_THROW(msg) SAFE(logger())->throws(std::string("") + msg, FILELN, true)
#define LOG_ERROR(msg) SAFE(logger())->error(std::string("") + msg, FILELN, false)
#define LOG_ALERT(msg) SAFE(logger())->alert(std::string("") + msg, FILELN, false)
#define LOG_WARN(msg) SAFE(logger())->warning(std::string("") + msg, FILELN, false)
#define LOG_INFO(msg) SAFE(logger())->info(std::string("") + msg, FILELN, false)
#define LOG_NOTE(msg) SAFE(logger())->note(std::string("") + msg, FILELN, false)
#define LOG_OK(msg) SAFE(logger())->success(std::string("") + msg, FILELN, false)
#define LOG_DEBUG(msg) SAFE(logger())->debug(std::string("") + msg, FILELN, false)

#define LOG(msg) LOG_NOTE(msg)
#define DBG(msg) LOG_DEBUG(msg)

#define LOG_OR_THROW(msg) { \
    string errmsg = msg + EWHAT; \
    LOG_ERROR(errmsg); \
    if (throwsOnError) \
        throw ERROR(errmsg); \
}

#define LOG_OR_EXIT(msg) { \
    string errmsg = msg + EWHAT; \
    LOG_ERROR(errmsg); \
    if (throwsOnError) \
        exit(1); \
}
