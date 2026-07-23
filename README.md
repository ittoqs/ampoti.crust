# 📦 Ampoti Archiver (CRust)

[![Rust](https://img.shields.io/badge/Rust-1.70%2B-orange.svg?style=flat&logo=rust)](https://www.rust-lang.org/)
[![C Language](https://img.shields.io/badge/C-11-blue.svg?style=flat&logo=c)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Platform](https://img.shields.io/badge/Platform-Desktop-lightgrey.svg?style=flat)](https://github.com/)
[![GUI Framework](https://img.shields.io/badge/UI-Iced-blueviolet.svg?style=flat)](https://github.com/iced-rs/iced)
[![Async Runtime](https://img.shields.io/badge/Async-Tokio-black.svg?style=flat)](https://tokio.rs/)
[![Core Engine](https://img.shields.io/badge/Engine-Libarchive-red.svg?style=flat)](https://www.libarchive.org/)

Ampoti Archiver Crust is a high-performance native desktop archiver that combines Rust's memory safety with the raw I/O speed of C. Built with a pure native GUI, it leverages system-level programming to prioritize memory efficiency, low-level I/O performance, and a buttery-smooth, responsive user experience.

---

## ✨ Features

* **⚡ 100% Native & Lightweight:** Direct GPU/CPU rendering using the [Iced](https://github.com/iced-rs/iced) framework (Elm architecture). Zero overhead from web browsers or Electron engines.
* **🚀 High-Speed C Engine:** Fast compression and extraction operations powered directly by `libarchive` in C, isolated within the core module for maximum throughput.
* **📂 Comprehensive Format Support:**
  * **Compression:** `ZIP`, `7Z` *(Default: ZIP)*.
  * **Extraction:** `RAR`, `ZIP`, `7Z`.
* **🎯 Non-Blocking 60fps UI:** Heavy I/O operations run seamlessly on isolated background threads via FFI and `tokio`, guaranteeing zero frozen windows or "Not Responding" states.
* **🛡️ Hardened Security (Anti-Zip Slip):** Built-in C engine protection against Directory Traversal attacks, preventing malicious `../` paths during extraction.

---

## 🛠️ Tech Stack

This project is built using powerful, modern system-level libraries and frameworks:

* **[Rust](https://www.rust-lang.org/)** - Safe, concurrent, and practical systems programming language.
* **[C Language](https://en.wikipedia.org/wiki/C_(programming_language))** - Ultra-fast low-level file manipulation engine.
* **[Iced](https://github.com/iced-rs/iced)** - Cross-platform GUI library for Rust focused on simplicity and type-safety.
* **[Tokio](https://tokio.rs/)** - Event-driven, non-blocking I/O platform for asynchronous execution.
* **[Libarchive](https://www.libarchive.org/)** - Multi-format archive and compression library.

---

## 🏗️ Project Structure

The project elegantly separates UI logic (Rust) from low-level file processing (C):

```text
ampoti-crust/
├── core/                       # C Engine for file manipulation (libarchive)
├── build.rs                    # Static compiler script joining C into Rust ecosystem
├── Cargo.toml                  # Dependency management (Iced, Tokio, Libc, etc.)
└── src/                        # [UI & BINDING]
    ├── ffi.rs                  # Safe Wrapper bridging C with Rust ecosystem
    ├── app.rs                  # State, Update, and View logic for Iced UI
    └── main.rs                 # Execution entry point
