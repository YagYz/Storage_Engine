#include <iostream>
    #include <filesystem>
    #include "StorageEngine.hpp"
    #include "Logger.hpp"
    
    int main() {
        auto& logger = Logger::getInstance();
        logger.init("logs/server.log");
    
        std::cout << "==========================================" << std::endl;
        std::cout << "     STORAGE ENGINE COMPACTION TESTI      " << std::endl;
        std::cout << "==========================================" << std::endl;
    
        std::string dbPath = "data/compact_test.dat";
        std::filesystem::remove(dbPath); // Varsa eski testi temizle
    
        StorageEngine engine;
        if (!engine.open(dbPath)) {
            std::cerr << "Motor acilamadi!" << std::endl;
            return -1;
        }
    
        // 1. Aynı key'e 1000 kez güncelleme basıyoruz (Diski şişiriyoruz)
        std::cout << "[*] 'user:1' anahtarina 1000 adet guncelleme yaziliyor..." << std::endl;
        for (int i = 0; i < 1000; ++i) {
            engine.put("user:1", "Guncelleme_No_" + std::to_string(i));
        }
        // Silinmiş bir çöp veri ekleyelim
        engine.put("silinecek_key", "Bu cop veri silinecek");
        engine.remove("silinecek_key");

        uintmax_t sizeBefore = std::filesystem::file_size(dbPath);
        std::cout << "[+] Compaction Oncesi Dosya Boyutu: " << sizeBefore << " bytes" << std::endl;

        // 2. Compaction (Sıkıştırma) Çalıştır
        std::cout << "\n[*] Compaction calistiriliyor..." << std::endl;
        if (engine.compact()) {
            std::cout << "[+] Compaction basarili!" << std::endl;
        } else {
            std::cerr << "[-] Compaction basarisiz oldu!" << std::endl;
        }

        // 3. Yeni dosya boyutunu ölç
        uintmax_t sizeAfter = std::filesystem::file_size(dbPath);
        std::cout << "[+] Compaction Sonrasi Dosya Boyutu: " << sizeAfter << " bytes" << std::endl;
        std::cout << "[🔥] Disk Tasarrufu: %" << (100 - (sizeAfter * 100 / sizeBefore)) << " yer kazanildi!" << std::endl;

        // 4. Veri sağlam mı kontrol et
        std::string val;
        if (engine.get("user:1", val)) {
            std::cout << "\n[+] Guncel Veri Dogrulandi: user:1 -> " << val << std::endl;
        }

        if (!engine.contains("silinecek_key")) {
            std::cout << "[+] Silinen cop verinin kompakt dosyaya GECMEDIGI dogrulandi." << std::endl;
        }

        engine.close();
        return 0;
    }
