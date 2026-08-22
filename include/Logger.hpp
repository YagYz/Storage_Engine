#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <filesystem>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class Logger
{
private:
    std::ofstream logFile;
    std::mutex logMutex;

    Logger() {}

public:

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    bool init(const std::string& filename) {
        // Dosya yolu kontrolu
        std::filesystem::path p(filename);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }

        // Log dosyasi kontrolu
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Hata: Log Dosyasi Bulunamadi!" << std::endl;
            return false;
        }
        return true;
    }

    void log(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);

        // Log level
        std::string levelStr;
        switch (level) {
            case LogLevel::INFO:    levelStr = "[INFO]"; break;
            case LogLevel::WARNING: levelStr = "[WARNING]"; break;
            case LogLevel::ERROR:   levelStr = "[ERROR]"; break;
        }

        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X")
           << " " << levelStr << " " << message;
        
        if (logFile.is_open()) {
            logFile << ss.str() << std::endl;
            logFile.flush();
        }

        if (level == LogLevel::ERROR) {
            std::cerr << ss.str() << std::endl;
        }

    }

    ~Logger() {
        if (logFile.is_open()) logFile.close();
    }

};

#endif