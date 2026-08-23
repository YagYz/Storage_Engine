<div align="center">
  <p>
    <b>🌐 Language:</b>
    <a href="README.md">English</a> |
    <b>Türkçe</b>
  </p>
</div>

# ⚡ Storage Engine

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg?style=flat&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-064F8C.svg?style=flat&logo=cmake)](https://cmake.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

Disk tabanlı, yüksek performanslı ve **Bitcask mimarisinden** esinlenerek C++20 ile sıfırdan geliştirilmiş, gömülü bir **Key-Value (Anahtar-Değer) Depolama Motoru**.

---

## 📖 Proje Hakkında ve Geliştirme Süreci

Bu proje, **Düşük Seviyeli Sistem Programlama (Systems Programming)**, **Modern C++20**, **Disk I/O Yönetimi**, **Bellek Modelleri** ve **Eşzamanlılık (Concurrency / Multi-threading)** konularında kendimi derinleştirmek ve pratik yapmak amacıyla tasarlanmıştır.

> 🤖 **Geliştirme Metodolojisi & Yapay Zeka Mentorluğu:**  
> Bu projenin mimari tasarımı, veri formatları ve algoritmaları oluşturulurken **Yapay Zeka (AI) aktif bir Pair Programming mentoru ve mimari danışman** olarak kullanılmıştır. Kodların tamamı aşama aşama, mantığı kavranarak ve test odaklı şekilde şahsım tarafından yazılmış; AI ise hata ayıklama, kod incelemesi (code review) ve en iyi C++ pratiklerinin (RAII, thread-safety, zero-copy I/O) uygulanmasında kılavuzluk etmiştir.

---

## 🏗️ Mimari & Çalışma Prensibi (Bitcask Mimarisi)

Storage Engine, **Append-Only Log (Sıralı Yazma)** ve **In-Memory Hash Indexing (Bellek İçi Hash Haritası)** ilkelerine dayanır:

```
                                  MİMARİ AKIŞ
                                  
     [ İSTEMCİ / CLI ]
            │
            ├── 1. SET / DEL ──────────────────────────────┐
            │                                              ▼
            │                                ┌───────────────────────────┐
            │                                │  Sıralı Disk Yazma (I/O)  │
            │                                │  Append-Only (.dat file)  │
            │                                └─────────────┬─────────────┘
            │                                              │ 2. Yeni Offset
            │                                              ▼
            │   3. GET (O(1) Hızında)        ┌───────────────────────────┐
            └─── RAM'den Adresi Oku ───────> │    RAM İndeksi (KeyDir)   │
                                             │ key -> {offset, val_size} │
                                             └───────────────────────────┘
```

### 🌟 Temel Özellikler

1. **Append-Only Disk I/O:** Veriler diske sadece dosyanın sonuna sıralı olarak yazılır. Rastgele disk kafası hareketleri (Random Seek) önlenerek maksimum yazma hızı elde edilir.
2. **$O(1)$ Okuma Performansı (KeyDir):** Tüm anahtarlar ve diskteki konumları (offset) RAM'deki `std::unordered_map` içinde tutulur. Okuma işlemi diskte tek bir `seek` ile anında tamamlanır.
3. **Çökme Güvenliği (Crash-Resistant Recovery):** Program kapandığında veya elektrik kesildiğinde veri kaybı yaşanmaz. Başlangıçta diskteki binary log taranarak RAM indeksi saniyeler içinde otomatik yeniden inşa edilir.
4. **Eşzamanlılık & Thread-Safety:** `std::shared_mutex` ve kilit mekanizmaları sayesinde **Çoklu Okuyucu - Tek Yazıcı (Multi-Reader Single-Writer)** modeli uygulanmıştır. Onlarca thread aynı anda veri okuyabilir ve yazabilir.
5. **Otomatik Sıkıştırma (Compaction / Garbage Collection):** Güncellenen veya silinen (Tombstone) eski çöp veriler diskte birikmez; `COMPACT` komutu ile canlı veriler yeni dosyaya taşınarak disk alanı geri kazanılır.
6. **Thread-Safe Loglama & JSON Konfigürasyon:** `nlohmann/json` ile merkezi konfigürasyon ve zaman damgalı Singleton `Logger` entegrasyonu.

---

## 📦 Binary Kayıt Formatı (Disk Layout)

Diskteki her kayıt sabit 21 baytlık bir başlık (Header) ve ardından gelen değişken uzunluktaki anahtar/değer baytlarından oluşur:

```
┌───────────┬──────────────┬────────────┬──────────────┬───────────┬──────────────┬────────────────┐
│ CRC32     │ Timestamp    │ Key Size   │ Value Size   │ Tombstone │ Key Payload  │ Value Payload  │
│ (4 Byte)  │ (8 Byte)     │ (4 Byte)   │ (4 Byte)     │ (1 Byte)  │ (k_size Byte)│ (v_size Byte)  │
└───────────┴──────────────┴────────────┴──────────────┴───────────┴──────────────┴────────────────┘
```

---

## 📂 Proje Yapısı

```
Storage_Engine/
├── CMakeLists.txt             # Modern CMake derleme dosyası (C++20, FetchContent)
├── README.md                  # İngilizce dokümantasyon
├── README.tr.md               # Türkçe dokümantasyon
├── LICENSE                    # MIT Lisansı
├── config/
│   └── config.json            # Port, veritabanı yolu ve log ayarları
├── include/                   # Header dosyaları
│   ├── ConfigManager.hpp      # JSON tabanlı Singleton konfigürasyon yöneticisi
│   ├── DataFile.hpp           # Append-only düşük seviyeli disk I/O sınıfı
│   ├── Logger.hpp             # Thread-safe dosya & konsol loglayıcı
│   ├── Record.hpp             # Binary kayıt struct'ı ve serileştirme
│   └── StorageEngine.hpp      # Ana motor (KeyDir, CRUD, Thread-Safety, Compaction)
└── src/
    └── main.cpp               # Etkileşimli REPL / CLI arayüzü
```

---

## 🛠️ Kurulum ve Derleme

### Gereksinimler
* C++20 destekleyen bir derleyici (`GCC 10+` veya `Clang 11+`)
* `CMake` (>= 3.20)
* `Git`

### Derleme Adımları

```bash
# 1. Depoyu klonlayın
git clone git@github.com:YagYz/Storage_Engine.git
cd Storage_Engine

# 2. CMake yapılandırmasını oluşturun
cmake -B build

# 3. Projeyi derleyin
cmake --build build
```

---

## 🎮 Kullanım (Etkileşimli CLI / REPL)

Derleme bittikten sonra motoru doğrudan terminalden çalıştırabilirsiniz:

```bash
./build/app
```

### Desteklenen Komutlar

| Komut | Açıklama | Örnek |
|---|---|---|
| `SET <key> <value>` | Anahtara değer atar (boşluklu metinler desteklenir) | `SET user:1 Ahmet Yilmaz` |
| `GET <key>` | Anahtarın güncel değerini getirir | `GET user:1` |
| `DEL <key>` | Anahtarı siler (Tombstone ekler) | `DEL user:1` |
| `CONTAINS <key>` | Anahtarın varlığını sorgular (`YES` / `NO`) | `CONTAINS user:1` |
| `COMPACT` | Diskteki çöp kayıtları temizler, dosyayı küçültür | `COMPACT` |
| `HELP` | Kullanılabilir komut listesini gösterir | `HELP` |
| `EXIT` / `QUIT` | Motoru güvenle kapatır | `EXIT` |

---

## 👤 Geliştirici

* **Yağız** - [GitHub Profili](https://github.com/YagYz)
