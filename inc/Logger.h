#pragma once
#include <iostream>
#include <mutex>
#include <string>
#include <fstream>

enum class LogLevel {
    INFO,
    WARN,
    ERROR,
    DEBUG
};

class Logger {
private:
    static Logger* instance;
    static std::mutex mtx;

    std::ofstream logFile;   // file handle

    Logger();                // constructor will open file
    std::string levelToString(LogLevel level);
    std::string getTimestamp();   

public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static Logger& getInstance();

    void log(LogLevel level, const std::string& message);

    void info(const std::string& msg);
    void warn(const std::string& msg);
    void error(const std::string& msg);
    void debug(const std::string& msg);

    ~Logger();               // close file safely
};