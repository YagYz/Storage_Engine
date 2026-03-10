#include <iostream>
#include "ConfigManager.hpp"
#include "Logger.hpp"

using namespace std;

int main() {

    auto& config = ConfigManager::getInstance();
    auto& logger = Logger::getInstance();

    if (!logger.init("logs/server.log")) {
        return -1;
    }

    logger.log(LogLevel::INFO, "Sistem Baslatiliyor...");

    if (!config.load("config/config.json")) {
        logger.log(LogLevel::ERROR, "Config Dosyasi Yuklenemedi!");
        return -1;
    }

    logger.log(LogLevel::INFO, "Sistem Basariyla Konfigure Edildi.");

    int port = config.get<int>("server", "port");
    string dbPath = config.get<string>("server", "db_path");
    string admin = config.get<string>("auth", "admin_user");

    cout << "--- YagYz Storage Engine ---" << endl;
    cout << "Port: " << port << endl;
    cout << "Veri Tabani: " << dbPath << endl;
    cout << "Yetkili: " << admin << endl;

}