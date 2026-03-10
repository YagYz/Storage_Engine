#ifndef CONFIG_MANAGER_HPP
#define CONFIG_MANAGER_HPP

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ConfigManager
{
private:
    json configData;    

    ConfigManager() {}

public:

    static ConfigManager& getInstance() {
        static ConfigManager instance;
        return instance;
    }

    bool load(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Hata: Config Dosyasi Acilamadi: " << path << std::endl;
            return false;
        }
        file >> configData;
        return true;
    }

    template<typename T>
    T get(const std::string& category, const std::string& key) {
        return configData[category][key].get<T>();
    }

    const json& data() const { return configData; }

};

#endif