#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Fungsi untuk mengompresi file ke dalam archive
// output_path: path hasil archive (misal .zip, .7z)
// files: array path file yang akan dikompresi
// num_files: jumlah file
// password: kata sandi untuk enkripsi (NULL jika tidak ada)
// format: format kompresi ("zip" atau "7z")
// error_buf: buffer string untuk menampung pesan error jika gagal
// error_buf_size: ukuran dari error_buf
// Mengembalikan 0 jika berhasil, negatif jika error
int ampoti_compress(const char *output_path, const char **files, const char **entry_names, int num_files, const char *password, const char *format, char *error_buf, size_t error_buf_size);

// Fungsi untuk mengekstrak archive (termasuk RAR, ZIP, 7z)
// archive_path: path ke file archive
// output_dir: direktori tujuan ekstraksi
// password: kata sandi untuk dekripsi (NULL jika tidak ada)
// error_buf: buffer string untuk menampung pesan error jika gagal
// error_buf_size: ukuran dari error_buf
// Mengembalikan 0 jika berhasil, negatif jika error
int ampoti_extract(const char *archive_path, const char *output_dir, const char *password, char *error_buf, size_t error_buf_size);

#ifdef __cplusplus
}
#endif

#endif // CORE_H
