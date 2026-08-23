#include <iostream>
    #include <vector>
    #include <thread>
    #include <chrono>
    #include "StorageEngine.hpp"
    #include "Logger.hpp"
    
    int main() {
        auto& logger = Logger::getInstance();
        logger.init("logs/server.log");
    
        std::cout << "==========================================" << std::endl;
        std::cout << "  STORAGE ENGINE MULTI-THREAD STRES TESTI " << std::endl;
        std::cout << "==========================================" << std::endl;
    
        StorageEngine engine;
        if (!engine.open("data/stress.dat")) {
            std::cerr << "Motor acilamadi!" << std::endl;
            return -1;
        }
    
        std::vector<std::thread> threads;
        const int NUM_WRITERS = 4;
        const int NUM_READERS = 4;
        const int OPS_PER_THREAD = 500;
    
        std::cout << "[*] " << NUM_WRITERS << " Yazici ve " << NUM_READERS 
                  << " Okuyucu thread baslatiliyor (" << OPS_PER_THREAD << " islem/thread)..." << std::endl;
    
        auto startTime = std::chrono::high_resolution_clock::now();
    
        // 1. Yazıcı Thread'leri Başlat
        for (int t = 0; t < NUM_WRITERS; ++t) {
            threads.emplace_back([&engine, t, OPS_PER_THREAD]() {
                for (int i = 0; i < OPS_PER_THREAD; ++i) {
                    std::string key = "th_" + std::to_string(t) + "_k_" + std::to_string(i);
                    std::string val = "Deger_Payload_" + std::to_string(i * 10);
                    engine.put(key, val);
                }
            });
        }
    
        // 2. Okuyucu Thread'leri Başlat
        for (int t = 0; t < NUM_READERS; ++t) {
            threads.emplace_back([&engine, t, OPS_PER_THREAD]() {
                for (int i = 0; i < OPS_PER_THREAD; ++i) {
                    std::string key = "th_" + std::to_string(t) + "_k_" + std::to_string(i);
                    std::string val;
                    engine.get(key, val); // Bulsa da bulmasa da oku
                }
            });
        }

        // 3. Tüm thread'lerin bitmesini bekle (join)
        for (auto& th : threads) {
            if (th.joinable()) {
                th.join();
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

        std::cout << "\n[🎉] Stres Testi Basariyla Tamamlandi!" << std::endl;
        std::cout << "Toplam Sure: " << duration << " ms" << std::endl;
        std::cout << "Toplam Eklenen Kayit: " << (NUM_WRITERS * OPS_PER_THREAD) << std::endl;

        // Son Kontrol: Rastgele bir kaydı oku
        std::string testVal;
        if (engine.get("th_0_k_100", testVal)) {
            std::cout << "[+] Ornek Kayit Dogrulandi: th_0_k_100 -> " << testVal << std::endl;
        }

        engine.close();
        return 0;
    }