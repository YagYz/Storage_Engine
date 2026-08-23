#ifndef DATA_FILE_HPP
#define DATA_FILE_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <mutex>
#include "Record.hpp"
#include "Logger.hpp"

class DataFile {
private:
    std::string filePath;
    std::fstream file;
    std::mutex fileMutex;
    uint64_t writeOffset{0};

public:
    DataFile() = default;
    ~DataFile() { close(); };

    bool open(const std::string& path);
    uint64_t appendRecord(const Record& record);
    bool readRecord(uint64_t offset, Record& outRecord);
    void close();

    const std::string& getFilePath() const { return filePath; }
    uint64_t getWriteOffset() const { return writeOffset; }

};

// Dosyayi acma
bool DataFile::open(const std::string& path) {

    // Dosya yolu kontrolu
    std::filesystem::path p(path);
    if(p.has_parent_path()) {
        std::filesystem::create_directories(p.parent_path());
    }

    file.open(path, std::ios::in | std::ios::out | std::ios::binary);
        
    if (!file.is_open()) {
        Logger::getInstance().log(LogLevel::WARNING, "Yeni veritabani olusturuluyor: " + path);
        file.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
        file.close();
        file.open(path, std::ios::in | std::ios::out | std::ios::binary);
    }

    if (!file.is_open()) {
        Logger::getInstance().log(LogLevel::ERROR, "Veritabani acilamadi: " + path);
        return false;
    }

    file.seekp(0, std::ios::end);
    writeOffset = static_cast<uint64_t>(file.tellp());
    filePath = path;

    Logger::getInstance().log(LogLevel::INFO, "DataFile baglandi: " + path + " (Boyut: " + std::to_string(writeOffset) + " bytes)");


    return true;

}

// Data ekleme
uint64_t DataFile::appendRecord(const Record& record) {

    if (!file.is_open()) {
        Logger::getInstance().log(LogLevel::ERROR, "Database Acik Degil!");
        return false;
    }

    std::lock_guard<std::mutex> lock(fileMutex);

    uint64_t currectOffset = writeOffset;

    file.seekp(currectOffset, std::ios::beg);
    file.write(reinterpret_cast<const char*>(&record.header), sizeof(RecordHeader));
    file.write(record.key.data(), record.key.size());

    if (!record.header.tombstone && record.header.value_size > 0) {
        file.write(record.value.data(), record.value.size());
    }

    file.flush();
    writeOffset += record.totalSize();

    return currectOffset;

}

bool DataFile::readRecord(uint64_t offset, Record& outRecord) {

    if (!file.is_open()) {
        Logger::getInstance().log(LogLevel::ERROR, "Database Acik Degil!");
        return false;
    }

    std::lock_guard<std::mutex> lock(fileMutex);

    file.clear();

    file.seekg(offset, std::ios::beg);

    file.read(reinterpret_cast<char*>(&outRecord.header), sizeof(RecordHeader));
    outRecord.key.resize(outRecord.header.key_size);
    file.read(outRecord.key.data(), outRecord.header.key_size);

    if (!outRecord.header.tombstone && outRecord.header.value_size > 0) {
        outRecord.value.resize(outRecord.header.value_size);
        file.read(outRecord.value.data(), outRecord.header.value_size);
    }
    else {
        outRecord.value.clear();
    }

    return true;
}

void DataFile::close() {
    if (file.is_open()) {
        file.close();
    }
}

#endif