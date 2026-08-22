#include <iostream>
    #include "DataFile.hpp"
    #include "Record.hpp"
    #include "Logger.hpp"
    
    int main() {
        auto& logger = Logger::getInstance();
        logger.init("logs/server.log");
    
        std::cout << "--- DataFile Testi Basliyor ---" << std::endl;
    
        DataFile db;
        if (!db.open("data/test.dat")) {
            std::cerr << "DataFile acilamadi!" << std::endl;
            return -1;
        }

        // 1. Kayitlari olustur diske yaz
        Record r1 = Record::create("kullanici:101", "Ali Veli");
        Record r2 = Record::create("ayarlar:tema", "dark_mode");

        uint64_t offset1 = db.appendRecord(r1);
        uint64_t offset2 = db.appendRecord(r2);

        std::cout << "Kayit 1 yazildi, Offset: " << offset1 << std::endl;
        std::cout << "Kayit 2 yazildi, Offset: " << offset2 << std::endl;

        // 2. Dosyayi kapat
        db.close();

        // 3. Dosyayi tekrar ac  offsetlerden oku
        DataFile dbRead;
        dbRead.open("data/test.dat");

        Record readR1, readR2;
        dbRead.readRecord(offset1, readR1);
        dbRead.readRecord(offset2, readR2);

        std::cout << "\n--- Diskten Okunan Veriler ---" << std::endl;
        std::cout << "Key: " << readR1.key << " | Value: " << readR1.value << " | Zaman: " << readR1.header.timestamp << std::endl;
        std::cout << "Key: " << readR2.key << " | Value: " << readR2.value << " | Zaman: " << readR2.header.timestamp << std::endl;

        dbRead.close();
        return 0;
    }
