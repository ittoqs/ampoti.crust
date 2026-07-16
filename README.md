# Ampoti CRust

> Aplikasi *desktop archiver* native berkinerja tinggi, memadukan keamanan memori Rust dengan kecepatan I/O bahasa C.

**Ampoti File** adalah aplikasi *desktop archiver* murni (Native GUI) yang dibangun menggunakan kolaborasi dua bahasa tingkat sistem: **Rust** dan **C**. Proyek ini dirancang khusus untuk mengedepankan keamanan memori, performa I/O tingkat rendah, dan pengalaman pengguna yang responsif tanpa hambatan.

---

## ✨ Fitur

* **100% Native & Ringan:** Antarmuka dirender langsung menggunakan GPU/CPU melalui framework [Iced](https://github.com/iced-rs/iced) (berbasis arsitektur Elm). Tidak ada *overhead* dari peramban web atau Electron.
* **Core Engine C Super Cepat:** Operasi kompresi dan ekstraksi ditangani langsung oleh `libarchive` menggunakan bahasa C. Engine ini diisolasi di dalam `core` untuk memastikan performa maksimal.
* **Dukungan Format Komprehensif:** 
  * **Kompresi:** `ZIP`, `7Z` *(Default: ZIP)*.
  * **Ekstraksi:** `RAR`, `ZIP`, `7Z`.
* **UI Responsif Non-Blocking (60fps):** Proses *blocking* I/O (seperti mengekstrak file berukuran gigabyte) dijalankan pada *background thread* terpisah melalui integrasi FFI dan `tokio`. Aplikasi dijamin tidak akan pernah mengalami *freeze* atau *Not Responding*.
* **Keamanan Ketat (Anti-Zip Slip):** Engine C dilengkapi dengan proteksi bawaan untuk memitigasi celah keamanan *Directory Traversal* (mencegah ekstraksi file berbahaya bermotif `../` ke luar direktori target).

---

## 🏗️ Struktur

Proyek ini memisahkan logika antarmuka (Rust) dan pemrosesan file tingkat rendah (C) secara elegan:

```text
ampoti-file/
├── core/                       # C Engine untuk manipulasi file (libarchive)
├── build.rs                    # Skrip kompilator statis C ke dalam ekosistem Rust
├── Cargo.toml                  # Manajemen dependensi (Iced, Tokio, Libc, dll.)
└── src/                        # [UI & BINDING]
    ├── ffi.rs                  # Safe Wrapper (Jembatan aman) dari C ke ekosistem Rust
    ├── app.rs                  # Logika State, Update, dan View dari antarmuka Iced
    └── main.rs                 # Entry point eksekusi aplikasi
```

---

## 🤝 Kontribusi

Kontribusi, pelaporan *bug* (*issues*), dan permintaan fitur (*feature requests*) sangat dipersilakan! Jangan ragu untuk membuka *pull request* untuk meningkatkan performa atau menambahkan format baru.

## 📄 Lisensi

Proyek ini didistribusikan di bawah lisensi [MIT](LICENSE).