#ifndef STORAGE_CORE_HPP
#define STORAGE_CORE_HPP

#include <iostream>
#include <fstream>
#include <string>
#include "Logger.hpp"

template <typename T>
class StorageCore {
private:
    std::string filePath;
    std::fstream dbFile;

public:
    StorageCore(const std::string& path) : filePath(path) {}

    // Veritabani Acma Olusturma
    bool open() {
        dbFile.open(filePath, std::ios::in | std::ios::out | std::ios::binary);
        
        if (!dbFile.is_open()) {
            Logger::getInstance().log(LogLevel::WARNING, "Yeni veritabani olusturuluyor: " + filePath);
            dbFile.open(filePath, std::ios::out | std::ios::binary | std::ios::trunc);
            dbFile.close();
            dbFile.open(filePath, std::ios::in | std::ios::out | std::ios::binary);
        }

        if (!dbFile.is_open()) {
            Logger::getInstance().log(LogLevel::ERROR, "Veritabani acilamadi: " + filePath);
            return false;
        }

        Logger::getInstance().log(LogLevel::INFO, "StorageCore baglandi. Record Size: " + std::to_string(sizeof(T)) + " bytes.");
        return true;
    }

    // Kayit Yazma
    bool insertRecord(const T& record) {
        if (!dbFile.is_open()) return false;

        dbFile.seekp(0, std::ios::end);
        dbFile.write(reinterpret_cast<const char*>(&record), sizeof(T));
        dbFile.flush();
        return true;
    }

    // Kayit Okuma (siraya gore)
    bool readRecord(int index, T& outRecord) {
        if (!dbFile.is_open()) return false;

        long offset = index * sizeof(T); 
        
        dbFile.seekg(offset, std::ios::beg);
        
        if (dbFile.read(reinterpret_cast<char*>(&outRecord), sizeof(T))) {
            return true;
        }
        
        dbFile.clear();
        return false;
    }

    // Kayit Guncelleme (siraya gore)
    bool updateRecord(int index, const T& record) {
        if (!dbFile.is_open()) return false;

        long offset = index * sizeof(T);

        dbFile.seekp(offset, std::ios::beg);

        dbFile.write(reinterpret_cast<const char*>(&record), sizeof(T));
        dbFile.flush();

        return true;
    }

    // Veritabani Kapatma
    void close() {
        if (dbFile.is_open()) {
            dbFile.close();
        }
    }

    // Killllllll
    ~StorageCore() {
        close();
    }
};

#endif