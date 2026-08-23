<div align="center">
  <p>
    <b>🌐 Language:</b>
    <b>English</b> |
    <a href="README.tr.md">Türkçe</a>
  </p>
</div>

# ⚡ Storage Engine

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg?style=flat&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-064F8C.svg?style=flat&logo=cmake)](https://cmake.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A high-performance, disk-backed, and embedded **Key-Value Storage Engine** built from scratch in C++20, inspired by the **Bitcask architecture**.

---

## 📖 About the Project & Development Journey

This project was conceived to dive deep into **Low-Level Systems Programming**, **Modern C++20**, **Disk I/O Management**, **Memory Models**, and **Concurrency / Multi-threading**.

> 🤖 **Development Methodology & AI Mentorship:**  
> Throughout the design and implementation of the storage engine, **Artificial Intelligence (AI) was utilized as an interactive Pair-Programming Mentor and Architectural Advisor**. All code and engineering logic were written, tested, and understood hands-on by myself, with AI providing code review, debugging feedback, and guidance on modern C++ best practices (RAII idioms, thread-safety primitives, and zero-copy I/O).

---

## 🏗️ Architecture & Core Principles (Bitcask Model)

Storage Engine operates on the principles of an **Append-Only Log (Sequential Disk Writes)** paired with an **In-Memory Hash Index (KeyDir)**:

```
                               ARCHITECTURE FLOW
                                  
     [ CLIENT / CLI ]
            │
            ├── 1. SET / DEL ──────────────────────────────┐
            │                                              ▼
            │                                ┌───────────────────────────┐
            │                                │   Sequential Disk I/O     │
            │                                │  Append-Only (.dat file)  │
            │                                └─────────────┬─────────────┘
            │                                              │ 2. New Offset
            │                                              ▼
            │   3. GET (O(1) Speed)          ┌───────────────────────────┐
            └─── Read Address from RAM ────> │    RAM Index (KeyDir)     │
                                             │ key -> {offset, val_size} │
                                             └───────────────────────────┘
```

### 🌟 Key Highlights

1. **Append-Only Sequential Disk I/O:** Writes are strictly appended to the tail of the data file, eliminating costly random disk seeks and leveraging OS page cache for maximum throughput.
2. **$O(1)$ Read Performance (KeyDir):** All keys and their exact byte offsets are mapped in an in-memory `std::unordered_map`. Every read requires only a single direct disk seek.
3. **Crash-Resistant Recovery:** In the event of a crash or shutdown, no data is corrupted or lost. During startup, the engine scans the binary log sequentially to rebuild the RAM index in milliseconds.
4. **Concurrency & Thread-Safety:** Implements a **Multi-Reader Single-Writer** concurrency model using `std::shared_mutex` and dedicated file locks, allowing dozens of concurrent reader threads alongside safe atomic writers.
5. **Garbage Collection (Compaction):** Obsolete data from updates and deletions (tombstones) are reclaimed on-demand via the `COMPACT` routine, which migrates active records to a defragmented file.
6. **Thread-Safe Logging & JSON Config:** Integrated with `nlohmann/json` for centralized configuration and a timestamped Singleton `Logger`.

---

## 📦 Binary Record Format (Disk Layout)

Every record written to disk consists of a packed 21-byte header followed by variable-length key and value payloads:

```
┌───────────┬──────────────┬────────────┬──────────────┬───────────┬──────────────┬────────────────┐
│ CRC32     │ Timestamp    │ Key Size   │ Value Size   │ Tombstone │ Key Payload  │ Value Payload  │
│ (4 Bytes) │ (8 Bytes)    │ (4 Bytes)  │ (4 Bytes)    │ (1 Byte)  │ (k_size B)   │ (v_size B)     │
└───────────┴──────────────┴────────────┴──────────────┴───────────┴──────────────┴────────────────┘
```

---

## 📂 Project Structure

```
Storage_Engine/
├── CMakeLists.txt             # Modern CMake build system (C++20, FetchContent)
├── README.md                  # English documentation
├── README.tr.md               # Turkish documentation
├── LICENSE                    # MIT License
├── config/
│   └── config.json            # Server port, database path, and logging config
├── include/                   # Header files
│   ├── ConfigManager.hpp      # JSON Singleton configuration manager
│   ├── DataFile.hpp           # Low-level append-only disk I/O manager
│   ├── Logger.hpp             # Thread-safe file & console logger
│   ├── Record.hpp             # Binary record definitions & serialization
│   └── StorageEngine.hpp      # Storage Engine orchestrator (KeyDir, CRUD, Compaction)
└── src/
    └── main.cpp               # Interactive REPL / CLI interface
```

---

## 🛠️ Build & Installation

### Prerequisites
* A C++20 compliant compiler (`GCC 10+` or `Clang 11+`)
* `CMake` (>= 3.20)
* `Git`

### Build Instructions

```bash
# 1. Clone the repository
git clone git@github.com:YagYz/Storage_Engine.git
cd Storage_Engine

# 2. Configure build directory
cmake -B build

# 3. Compile the executable
cmake --build build
```

---

## 🎮 Interactive CLI / REPL

Once built, launch the storage engine shell directly from your terminal:

```bash
./build/app
```

### Supported Commands

| Command | Description | Example |
|---|---|---|
| `SET <key> <value>` | Sets a key to a value (supports spaced strings) | `SET user:1 John Doe` |
| `GET <key>` | Retrieves the value of a key | `GET user:1` |
| `DEL <key>` | Removes a key from the database (appends tombstone) | `DEL user:1` |
| `CONTAINS <key>` | Checks if a key exists (`YES` / `NO`) | `CONTAINS user:1` |
| `COMPACT` | Cleans up stale records and shrinks data file size | `COMPACT` |
| `HELP` | Displays the available command list | `HELP` |
| `EXIT` / `QUIT` | Gracefully closes the database and exits | `EXIT` |

---

## 👤 Author

* **Yağız** - [GitHub Profile](https://github.com/YagYz)
