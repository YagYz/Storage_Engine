# Storage Engine

Disk tabanlı, yüksek performanslı ve Bitcask mimarisinden esinlenen bir **C++20 Key-Value Depolama Motoru**.

---

## 📖 Genel Bakış

Storage Engine; sıralı disk I/O (append-only log) kullanarak hızlı yazma sağlayan, bellekteki hash tablosu (KeyDir) ile $O(1)$ sürede okuma gerçekleştiren gömülü ve ağ destekli bir veritabanı motorudur.

### ✨ Temel Özellikler
* **Append-Only Storage:** Veriler diske sıralı olarak eklenir, disk kafası hareketi minimize edilir.
* **Bellek İçi Hash İndeksi (KeyDir):** Tüm anahtarların diskteki offset bilgileri RAM'de tutulur, tek disk seek ile okuma yapılır.
* **Çökme Güvenliği (Crash-Resistant):** Yeniden başlatıldığında diskteki log taranarak indeks anında kurtarılır.
* **Eşzamanlılık (Thread-Safe):** Çoklu okuyucu - tek yazıcı (Read-Write lock) modeli.
* **Modern C++20:** Temiz, modüler ve modern standartlara uygun kod tabanı.

---

## 📂 Proje Yapısı

```
Storage_Engine/
├── CMakeLists.txt        # CMake derleme yapılandırması
├── ROADMAP.md            # Adım adım geliştirme yol haritası
├── README.md             # Proje dokümantasyonu
├── config/
│   └── config.json       # Sunucu ve veritabanı ayarları
├── include/              # Header (.hpp) dosyaları
│   ├── ConfigManager.hpp # Konfigürasyon yöneticisi
│   ├── Logger.hpp        # Thread-safe loglama
│   └── StorageCore.hpp   # Temel depolama motoru
└── src/                  # Kaynak (.cpp) kodları
    └── main.cpp          # Giriş noktası
```

---

## 🗺️ Geliştirme Yol Haritası

Detaylı fazlar ve adım adım yapılacaklar için [ROADMAP.md](ROADMAP.md) dosyasına göz atabilirsiniz:
1. **Faz 1:** CMake & Temel Altyapı
2. **Faz 2:** Binary Kayıt Formatı & Append-Only Log
3. **Faz 3:** Bellek İçi İndeks (KeyDir) & CRUD
4. **Faz 4:** Eşzamanlılık & Thread-Safety
5. **Faz 5:** Veri Sıkıştırma (Compaction)
6. **Faz 6:** CLI & Mini-Redis Ağ Katmanı

---

## 🛠️ Derleme ve Çalıştırma

```bash
# Projeyi yapılandır
cmake -B build

# Derle
cmake --build build

# Çalıştır
./build/StorageEngine
```
