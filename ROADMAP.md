# 🚀 Storage Engine - Geliştirme Yol Haritası (Roadmap)

Bu yol haritası, uzun süre ara vermiş olsan bile seni karmaşık veri yapıları (B+ Tree, Buffer Pool vs.) içinde boğmadan, adım adım modern ve güçlü bir **Key-Value Storage Engine (Bitcask Mimarisi)** inşa etmen için özel olarak hazırlandı.

---

## 🎯 Neden Bitcask Mimarisi?
* **Anlaşılması Kolay & Mantıklı:** Karmaşık disk içi pointer hesapları yerine sıralı yazma (append-only log) ve bellekte hash indeksleme kullanır.
* **Yüksek Performans:** Disk yazmaları sıralı (sequential I/O) olduğu için çok hızlıdır. Okumalar RAM indeksinden tek bir disk `seek` ile O(1) sürede yapılır.
* **Çökme Güvenliği (Crash Friendly):** Veriler ezilmediği için elektrik kesintisinde veya çökmede veri kaybı minimumdur.
* **Sektörde Kabul Görmüş:** Riak, Bitcask ve modern NoSQL veritabanlarının temel çalışma prensibidir.

---

```mermaid
graph TD
    subgraph RAM [Bellek / RAM]
        KeyDir["KeyDir (Hash Index)\n'user_1' -> {file_id, offset, size, timestamp}"]
    end

    subgraph DISK [Disk / Data Files]
        AppendLog["Append-Only Data File (.dat)\n[CRC | Timestamp | KeySize | ValueSize | Key | Value]"]
    end

    Client["İstemci / Kullanıcı"] -->|1. SET key val| StorageEngine
    StorageEngine -->|2. Sona Ekle (Sequential Write)| AppendLog
    StorageEngine -->|3. Offset Güncelle| KeyDir

    Client -->|4. GET key| StorageEngine
    StorageEngine -->|5. RAM'den Offset Oku| KeyDir
    StorageEngine -->|6. Dosyadan Oku (1 Seek)| AppendLog
```

---

## 📅 Fazlar ve Adım Adım Yapılacaklar

### 📌 Faz 1: Proje Temelini Toparlama & Derleme Altyapısı (Tahmini: 1-2 Gün)
> **Amaç:** Projenin her ortamda tek tuşla derlenebilir, modern ve temiz bir C++20 altyapısına kavuşması.

- [ ] **1.1 Modern CMake Entegrasyonu (`CMakeLists.txt`):**
  - C++20 standardını zorunlu kılmak (`CMAKE_CXX_STANDARD 20`).
  - `FetchContent` ile `nlohmann/json` kütüphanesini otomatik indirmek.
- [ ] **1.2 Mevcut Kodların Temizlenmesi:**
  - `include/Logger.hpp` içindeki `using namespace std;` ifadesini kaldırıp `std::` öneklerini düzenlemek.
  - `data/` ve `logs/` dizinlerinin program açılırken `std::filesystem::create_directories` ile otomatik oluşturulması.
- [ ] **1.3 Temel Test:**
  - Projenin `cmake -B build && cmake --build build` ile sorunsuz derlenip çalışabildiğini doğrulamak.

---

### 📌 Faz 2: Binary Kayıt Formatı & Append-Only Log (Tahmini: 2-4 Gün)
> **Amaç:** Diske hem string hem binary anahtar-değer (Key-Value) ikililerini güvenli bir formatta yazabilmek.

- [ ] **2.1 Disk Kayıt Formatının (Entry / Record) Tasarımı:**
  Diskte her kaydın şu başlık (header) yapısıyla yazılması:
  ```
  ┌──────────┬───────────┬──────────┬────────────┬─────────┬───────────┐
  │ CRC (4B) │ Time (8B) │ KSize(4B)│ VSize (4B) │ Key (..)│ Value (..)│
  └──────────┴───────────┴──────────┴────────────┴─────────┴───────────┘
  ```
- [ ] **2.2 Silme (Tombstone) Mantığı:**
  - Silinen kayıtlar için `VSize = -1` (veya özel bir Tombstone bayrağı) diske eklenir.
- [ ] **2.3 `DataFile` Sınıfı:**
  - Diske sadece sona ekleme (`writeAppend`) ve verilen offset'ten okuma (`readAt`) fonksiyonlarını yazmak.

---

### 📌 Faz 3: Bellek İçi İndeks (KeyDir) ve Motorun Birleştirilmesi (Tahmini: 3-5 Gün)
> **Amaç:** RAM'deki Hash Map ile diskteki dosyaları bağlayıp CRUD operasyonlarını tamamlamak.

- [ ] **3.1 `KeyDir` (RAM Hash Index) Yapısı:**
  ```cpp
  struct IndexEntry {
      uint32_t fileId;
      uint64_t offset;
      uint32_t valueSize;
      uint64_t timestamp;
  };
  std::unordered_map<std::string, IndexEntry> keyDir;
  ```
- [ ] **3.2 CRUD Fonksiyonları:**
  - `put(key, value)`: Diske append yap -> `keyDir[key]` güncelle.
  - `get(key, outValue)`: `keyDir`'den offset bul -> diskten 1 kez oku.
  - `remove(key)`: Diske tombstone yaz -> `keyDir`'den sil.
  - `contains(key)`: Sadece `keyDir`'de var mı diye kontrol et (O(1)).
- [ ] **3.3 Açılışta İndeks Yeniden İnşa Etme (Recovery):**
  - Motor açıldığında veri dosyasını baştan sona 1 kez okuyarak `keyDir`'i sıfırdan oluşturmak.

---

### 📌 Faz 4: Eşzamanlılık (Thread-Safety) ve Concurrency (Tahmini: 2-3 Gün)
> **Amaç:** Aynı anda birden fazla iş parçacığının (thread) güvenle okuma/yazma yapabilmesi.

- [ ] **4.1 Read-Write Kilidi (`std::shared_mutex`):**
  - Okuma işlemlerinde (`get`) birden çok thread aynı anda çalışabilir (`std::shared_lock`).
  - Yazma işlemlerinde (`put`, `remove`) tek bir thread yazma hakkı alır (`std::unique_lock`).
- [ ] **4.2 Basit Multi-Thread Stres Testi:**
  - 10 thread ile aynı anda binlerce rastgele okuma/yazma testi yaparak veri bütünlüğünü doğrulamak.

---

### 📌 Faz 5: Dosya Sıkıştırma (Compaction / Merge) (Tahmini: 3-5 Gün)
> **Amaç:** Silinmiş veya güncellenmiş eski verilerin diskte kapladığı çöp alanı temizlemek.

- [ ] **5.1 Segment / Dosya Döndürme (File Rotation):**
  - Aktif veri dosyası belli bir boyuta (örn. 64 MB) ulaştığında salt-okunur (read-only) yapılıp yeni bir aktif dosya açılması.
- [ ] **5.2 Compaction İşlemi:**
  - Eski veri dosyalarını tarayarak sadece `keyDir`'de güncel olan verileri yeni bir dosyaya kopyalamak, çöp verileri atmak.
  - Eski dosyaları silerek disk alanını geri kazanmak.

---

### 📌 Faz 6: Kullanıcı Arayüzü & Sunucu Katmanı (Tahmini: 3-5 Gün)
> **Amaç:** Storage Engine'i sadece bir C++ kütüphanesi değil, dışarıdan erişilebilir çalışan bir servis haline getirmek.

- [ ] **6.1 Etkileşimli CLI / REPL:**
  - Terminalden interaktif komutlar:
    ```text
    > SET user:1 Ahmet
    OK
    > GET user:1
    "Ahmet"
    > DEL user:1
    OK
    ```
- [ ] **6.2 Basit TCP Sunucusu (Mini-Redis):**
  - `socket` / `epoll` kullanarak dış dünyadan TCP ile `SET`, `GET`, `DEL`, `KEYS` komutlarını alabilmek.
  - Python veya `nc` (netcat) ile veritabanına uzaktan bağlanıp sorgu atabilmek.

---

## 🛠️ Önerilen Teknolojik Stack
* **Dil:** C++20
* **Build Sistemi:** CMake (>= 3.20)
* **Format:** Custom Binary Log (Header + Payload)
* **Test:** GoogleTest veya basit entegre test modülü
* **Bağımlılık:** `nlohmann/json` (Sadece Config için)

---

## 🚀 Başlarken İpucu
Hiç acele etme! **Faz 1** ile başlayacağız (CMake ve temizlik). Her faz bittiğinde çalışan, test edilmiş bir parçamız olacak. İlerledikçe hem C++ reflekslerin geri gelecek hem de keyifle çalışan bir veritabanı motoru ortaya çıkacak.
