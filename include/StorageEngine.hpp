#ifndef STORAGE_ENGINE_HPP
#define STORAGE_ENGINE_HPP

#include <string>
#include <unordered_map>
#include <iostream>
#include <shared_mutex>
#include <mutex>
#include "DataFile.hpp"
#include "Record.hpp"
#include "Logger.hpp"

struct IndexEntry {
    uint64_t offset{0};
    uint32_t valueSize{0};
    uint64_t timestamp{0};
};

class StorageEngine
{
private:
    DataFile dataFile;
    std::unordered_map<std::string, IndexEntry> keyDir;
    bool isReady{false};

    mutable std::shared_mutex rwMutex;

    void buildKeyDir();

public:
    StorageEngine() = default;
    ~StorageEngine() { close(); }

    bool open(const std::string& dbPath);
    bool put(const std::string& key, const std::string& value);
    bool get(const std::string& key, std::string& outValue);
    bool remove(const std::string& key);
    bool contains(const std::string& key) const;
    bool compact();
    void close();
};

bool StorageEngine::open(const std::string& dbPath) {
    std::unique_lock<std::shared_mutex> lock(rwMutex);

    if (!dataFile.open(dbPath)) { return false; }
    buildKeyDir();
    isReady = true;
    return true;
}

void StorageEngine::buildKeyDir() {

    keyDir.clear();
    uint64_t currectOffset = 0;
    uint64_t fileSize = dataFile.getWriteOffset();

    while (currectOffset < fileSize) {
        Record rec;
        if (!dataFile.readRecord(currectOffset, rec)) {
            break;
        }
        if (rec.header.tombstone == 1) {
            keyDir.erase(rec.key);
        }
        else {
            IndexEntry entry;
            entry.offset = currectOffset;
            entry.valueSize = rec.header.value_size;
            entry.timestamp = rec.header.timestamp;

            keyDir[rec.key] = entry;
        }

        currectOffset += rec.totalSize();
    }

}

bool StorageEngine::put(const std::string& key, const std::string& value) {

    if (!isReady) {
        return false;
    }

    std::unique_lock<std::shared_mutex> lock(rwMutex);

    Record r = Record::create(key, value);

    uint64_t offset = dataFile.appendRecord(r);
    IndexEntry newIndex;
    newIndex.offset = offset;
    newIndex.valueSize = r.header.value_size;
    newIndex.timestamp = r.header.timestamp;
    keyDir[key] = newIndex;
    
    return true;

}

bool StorageEngine::get(const std::string& key, std::string& outValue) {

    if (!isReady) {
        return false;
    }

    std::shared_lock<std::shared_mutex> lock(rwMutex);

    if (keyDir.find(key) == keyDir.end()) {
        return false;
    } else {
        IndexEntry entry = keyDir[key];
        Record rec; 

        if (!dataFile.readRecord(entry.offset, rec)) {
            return false;
        }
        outValue = rec.value;

        return true;
    }

}

bool StorageEngine::remove(const std::string& key) {

    if (!isReady) {
        return false;
    }

    std::unique_lock<std::shared_mutex> lock(rwMutex);

    if (keyDir.find(key) == keyDir.end()) {
        return false;
    } else {
        Record tomb = Record::createTombstone(key);
        dataFile.appendRecord(tomb);
        keyDir.erase(key);
        return true;
    }

}

bool StorageEngine::contains(const std::string& key) const {

    std::shared_lock<std::shared_mutex> lock(rwMutex);

    if (keyDir.count(key) > 0) {
        return true;
    } else { return false; }

}

bool StorageEngine::compact() {

    if (!isReady) { return false; }

    std::unique_lock<std::shared_mutex> lock(rwMutex);
    DataFile compactFile;
    std::string dbPath = dataFile.getFilePath();
    std::string tempPath = dbPath + ".compact";

    if (!compactFile.open(tempPath)) {
        return false;
    }

    for (auto& [key, entry] : keyDir) {
        Record rec;
        if (dataFile.readRecord(entry.offset, rec)) {
            uint64_t newOffset = compactFile.appendRecord(rec);
            entry.offset = newOffset;
        }
    }

    dataFile.close();
    compactFile.close();
    
    std::filesystem::rename(tempPath, dbPath);

    if (!dataFile.open(dbPath)) {
        isReady = false;
        return false;
    }

    Logger::getInstance().log(LogLevel::INFO, "Compaction basariyla tamamladi.");
    return true;

}

void StorageEngine::close() {
    std::unique_lock<std::shared_mutex> lock(rwMutex);

    keyDir.clear();
    dataFile.close();
    isReady = false;

}


#endif