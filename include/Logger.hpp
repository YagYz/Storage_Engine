#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <mutex>

using namespace std;

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class Logger
{
private:
    ofstream logFile;
    mutex logMutex;

    Logger() {}

public:

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    bool init(const string& filename) {
        logFile.open(filename, ios::app);
        if (!logFile.is_open()) {
            cerr << "Hata: Log Dosyasi Bulunamadi!" << endl;
            return false;
        }
        return true;
    }

    void log(LogLevel level, const string& message) {
        lock_guard<mutex> lock(logMutex);

        string levelStr;
        switch (level) {
            case LogLevel::INFO:    levelStr = "[INFO]"; break;
            case LogLevel::WARNING: levelStr = "[WARNING]"; break;
            case LogLevel::ERROR:   levelStr = "[ERROR]"; break;
        }

        auto now = chrono::system_clock::now();
        auto in_time_t = chrono::system_clock::to_time_t(now);

        stringstream ss;
        ss << put_time(localtime(&in_time_t), "%Y-%m-%d %X")
           << " " << levelStr << " " << message;
        
        if (logFile.is_open()) {
            logFile << ss.str() << endl;
            logFile.flush();
        }

        if (level == LogLevel::ERROR) {
            cerr << ss.str() << endl;
        }

    }

    ~Logger() {
        if (logFile.is_open()) logFile.close();
    }

};

#endif