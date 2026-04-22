#include "Logger.h"
#include <iomanip>
#include <sstream>
#include <ctime>

Logger* Logger::instance = nullptr;
std::mutex Logger::mtx;

// Open file once
Logger::Logger() {
    logFile.open("trade_engine.log", std::ios::app); // append mode
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file\n";
    }
}

Logger::~Logger() {
    if (logFile.is_open())
        logFile.close();
}


std::string Logger::getTimestamp() {
    using namespace std::chrono;

    auto now = system_clock::now()+ hours(5)+minutes(30);
    std::time_t time = system_clock::to_time_t(now);

    std::tm tm{};
    localtime_r(&time, &tm);   //  thread-safe (Linux/WSL)

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::DEBUG: return "DEBUG";
    }
    return "UNKNOWN";
}

Logger& Logger::getInstance() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!instance)
        instance = new Logger();
    return *instance;
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);

    std::string logLine =
      getTimestamp()+  "[" + levelToString(level) + "] " + message;

    //  Console output (optional)
   // std::cout << logLine << std::endl;

    //  File output
    if (logFile.is_open()) {
        logFile << logLine << std::endl;
        logFile.flush();  // important for crash safety
    }
}

void Logger::info(const std::string& msg)  { log(LogLevel::INFO, msg); }
void Logger::warn(const std::string& msg)  { log(LogLevel::WARN, msg); }
void Logger::error(const std::string& msg) { log(LogLevel::ERROR, msg); }
void Logger::debug(const std::string& msg) { log(LogLevel::DEBUG, msg); }