#include <iostream>
#include <string>
#include <sstream>
#include <cctype>
#include "ConfigManager.hpp"
#include "StorageEngine.hpp"
#include "Logger.hpp"

void printBanner(const std::string& dbPath, int port) {
    std::cout << "==================================================" << std::endl;
    std::cout << "        ⚡ YagYz Storage Engine v1.0 ⚡           " << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "[+] Veritabani Dosyasi: " << dbPath << std::endl;
    std::cout << "[+] Port: " << port << std::endl;
    std::cout << "[+] Komut listesi icin 'HELP' yazabilirsiniz." << std::endl;
    std::cout << "==================================================\n" << std::endl;
}

void printHelp() {
    std::cout << "\nKullanilabilir Komutlar:\n"
              << "  SET <key> <value>   : Anahtara bir deger atar (Bosluklu metinler desteklenir)\n"
              << "  GET <key>           : Anahtarin degerini getirir\n"
              << "  DEL <key>           : Anahtari veritabanindan siler\n"
              << "  CONTAINS <key>      : Anahtarin varligini kontrol eder (YES/NO)\n"
              << "  COMPACT             : Diskteki cop kayitlari temizler (Dosya boyutunu kucultur)\n"
              << "  HELP                : Bu yardim menusunu gosterir\n"
              << "  EXIT / QUIT         : Programi guvenle kapatir\n" << std::endl;
}

int main() {
    // Config Yukleme
    auto& config = ConfigManager::getInstance();
    if (!config.load("config/config.json")) {
        std::cerr << "[-] HATA: Config dosyasi yuklenemedi!" << std::endl;
        return -1;
    }

    // Logger baslatma
    std::string logPath = config.get<std::string>("logging", "file_path");
    Logger::getInstance().init(logPath);
    Logger::getInstance().log(LogLevel::INFO, "Sunucu ve depolama motoru baslatiliyor...");

    // Storage engine baslatma
    std::string dbPath = config.get<std::string>("server", "db_path");
    int port = config.get<int>("server", "port");

    StorageEngine engine;
    if (!engine.open(dbPath)) {
        Logger::getInstance().log(LogLevel::ERROR, "Storage Engine baslatilamadi: " + dbPath);
        std::cerr << "[-] HATA: Storage Engine acilamadi!" << std::endl;
        return -1;
    }

    printBanner(dbPath, port);

    // Etkilesimli CLI dongusu
    std::string line;
    while (true) {
        std::cout << "storage-engine> ";
        if (!std::getline(std::cin, line)) {
            break; // Ctrl+D veya stream kapandıysa cik
        }

        // Bos satır girildiyse tekrar sor
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        // Komutu buyuk harfe cevir
        for (char& c : cmd) {
            c = static_cast<char>(std::toupper(c));
        }

        if (cmd == "SET") {
            std::string key, value;
            if (ss >> key) {
                // Keyde sonraki tum metni value olarak al
                std::getline(ss >> std::ws, value);
                if (!value.empty()) {
                    engine.put(key, value);
                    std::cout << "OK" << std::endl;
                } else {
                    std::cout << "(hata) Eksik deger! Kullanim: SET <key> <value>" << std::endl;
                }
            } else {
                std::cout << "(hata) Kullanim: SET <key> <value>" << std::endl;
            }
        } 
        else if (cmd == "GET") {
            std::string key, value;
            if (ss >> key) {
                if (engine.get(key, value)) {
                    std::cout << "\"" << value << "\"" << std::endl;
                } else {
                    std::cout << "(nil)" << std::endl;
                }
            } else {
                std::cout << "(hata) Kullanim: GET <key>" << std::endl;
            }
        } 
        else if (cmd == "DEL") {
            std::string key;
            if (ss >> key) {
                if (engine.remove(key)) {
                    std::cout << "OK (1 key silindi)" << std::endl;
                } else {
                    std::cout << "(nil) (key bulunamadi)" << std::endl;
                }
            } else {
                std::cout << "(hata) Kullanim: DEL <key>" << std::endl;
            }
        } 
        else if (cmd == "CONTAINS") {
            std::string key;
            if (ss >> key) {
                std::cout << (engine.contains(key) ? "YES" : "NO") << std::endl;
            } else {
                std::cout << "(hata) Kullanim: CONTAINS <key>" << std::endl;
            }
        } 
        else if (cmd == "COMPACT") {
            std::cout << "[*] Compaction islemi baslatiliyor..." << std::endl;
            if (engine.compact()) {
                std::cout << "[OK] Compaction basariyla tamamlandi. Cop veriler temizlendi." << std::endl;
            } else {
                std::cout << "[-] Compaction basarisiz oldu!" << std::endl;
            }
        } 
        else if (cmd == "HELP") {
            printHelp();
        } 
        else if (cmd == "EXIT" || cmd == "QUIT") {
            std::cout << "Storage Engine guvenle kapatiliyor. Hoscakalin!" << std::endl;
            break;
        } 
        else {
            std::cout << "(hata) Bilinmeyen komut: '" << cmd << "'. Yardim icin 'HELP' yazin." << std::endl;
        }
    }

    engine.close();
    Logger::getInstance().log(LogLevel::INFO, "Storage Engine kapatildi.");
    return 0;
}