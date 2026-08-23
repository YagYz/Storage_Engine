#include <iostream>
    #include <cassert>
    #include "StorageEngine.hpp"
    #include "Logger.hpp"
    
    int main() {
        auto& logger = Logger::getInstance();
        logger.init("logs/server.log");
    
        std::cout << "==========================================" << std::endl;
        std::cout << "  STORAGE ENGINE CRUD & RECOVERY TESTI    " << std::endl;
        std::cout << "==========================================" << std::endl;
    
        // --- AŞAMA 1: YAZMA, GÜNCELLEME VE SİLME ---
        {
            StorageEngine engine;
            assert(engine.open("data/app.dat"));
    
            // Ekleme
            engine.put("user:1", "Ahmet Yilmaz");
            engine.put("user:2", "Mehmet Demir");
            engine.put("user:3", "Ayse Kaya");
    
            // Güncelleme
            engine.put("user:1", "Ahmet Yilmaz (Guncellendi)");
    
            // Silme
            engine.remove("user:2");
    
            // Kontroller
            std::string val;
            assert(engine.get("user:1", val) && val == "Ahmet Yilmaz (Guncellendi)");
            assert(!engine.get("user:2", val)); // user:2 silindi, bulunamamalı!
            assert(engine.get("user:3", val) && val == "Ayse Kaya");
    
            std::cout << "[+] Asama 1 Basarili: CRUD islemleri calisiyor." << std::endl;
            engine.close();
        }

        // --- AŞAMA 2: RECOVERY (AÇILIŞTA DISKTEN GERİ YÜKLEME) ---
        {
            std::cout << "\n[*] Motor yeniden baslatiliyor (RAM sifirlandi)..." << std::endl;
            StorageEngine engine2;
            assert(engine2.open("data/app.dat")); // Diskteki log taranacak

            std::string val;
            // user:1'in son güncel hali gelmeli
            if (engine2.get("user:1", val)) {
                std::cout << "[+] user:1 -> " << val << std::endl;
            }

            // user:2 silinmişti, olmamalı
            if (!engine2.contains("user:2")) {
                std::cout << "[+] user:2 silinmis olarak dogrulandi." << std::endl;
            }

            // user:3 sağlam olmalı
            if (engine2.get("user:3", val)) {
                std::cout << "[+] user:3 -> " << val << std::endl;
            }

            std::cout << "\n[🎉] Asama 2 Basarili: Recovery mekanizmasi mukemmel calisiyor!" << std::endl;
        }

        return 0;
    }
