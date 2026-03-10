#include <iostream>
#include "ConfigManager.hpp"

using namespace std;

int main() {

    auto& config = ConfigManager::getInstance();

    if (!config.load("config/config.json")) {
        return -1;
    }

    int port = config.get<int>("server", "port");
    string dbPath = config.get<string>("server", "db_path");
    string admin = config.get<string>("auth", "admin_user");

    cout << "--- YagYz Storage Engine ---" << endl;
    cout << "Port: " << port << endl;
    cout << "Veri Tabani: " << dbPath << endl;
    cout << "Yetkili: " << admin << endl;

}