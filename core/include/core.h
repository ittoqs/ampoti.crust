#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Fungsi untuk mengompresi file ke dalam archive
// output_path: path hasil archive (misal .zip, .7z)
// files: array path file yang akan dikompresi
// num_files: jumlah file
// password: kata sandi untuk enkripsi (NULL jika tidak ada)
// format: format kompresi ("zip" atau "7z")
// Mengembalikan 0 jika berhasil, negatif jika error
int ampoti_compress(const char *output_path, const char **files, int num_files, const char *password, const char *format);

// Fungsi untuk mengekstrak archive (termasuk RAR, ZIP, 7z)
// archive_path: path ke file archive
// output_dir: direktori tujuan ekstraksi
// password: kata sandi untuk dekripsi (NULL jika tidak ada)
// Mengembalikan 0 jika berhasil, negatif jika error
int ampoti_extract(const char *archive_path, const char *output_dir, const char *password);

#ifdef __cplusplus
}
#endif

#endif // CORE_H
