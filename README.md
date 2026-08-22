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
├── README.md             # Proje dokümantasyonu
├── config/
│   └── config.json       # Sunucu ve veritabanı ayarları
├── include/              # Header (.hpp) dosyaları
│   ├── ConfigManager.hpp # Konfigürasyon yöneticisi
│   ├── DataFile.hpp      # Append-only disk I/O yöneticisi
│   ├── Logger.hpp        # Thread-safe loglama
│   ├── Record.hpp        # Binary kayıt formatı
│   └── StorageEngine.hpp # Ana depolama motoru
└── src/                  # Kaynak (.cpp) kodları
    └── main.cpp          # Giriş noktası
```

---

## 🛠️ Derleme ve Çalıştırma

```bash
# Projeyi yapılandır
cmake -B build

# Derle
cmake --build build

# Çalıştır
./build/app
```
