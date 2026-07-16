#include "../include/core.h"
#include <archive.h>
#include <archive_entry.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper untuk menyalin data saat ekstraksi
static int copy_data(struct archive *ar, struct archive *aw) {
    int r;
    const void *buff;
    size_t size;
    int64_t offset;

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r < ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r < ARCHIVE_OK) {
            return (r);
        }
    }
}

int ampoti_compress(const char *output_path, const char **files, int num_files, const char *password, const char *format) {
    struct archive *a;
    struct archive_entry *entry;
    int r;
    
    a = archive_write_new();
    
    if (format != NULL && strcmp(format, "7z") == 0) {
        archive_write_set_format_7zip(a);
    } else {
        archive_write_set_format_zip(a);
    }
    
    if (password != NULL) {
        archive_write_set_passphrase(a, password);
    }
    
    r = archive_write_open_filename(a, output_path);
    if (r != ARCHIVE_OK) {
        archive_write_free(a);
        return -1;
    }
    
    // Iterasi untuk mengompresi setiap file (versi dasar)
    for (int i = 0; i < num_files; i++) {
        struct archive *disk = archive_read_disk_new();
        archive_read_disk_set_standard_lookup(disk);
        
        entry = archive_entry_new();
        archive_entry_set_pathname(entry, files[i]);
        
        // Membaca info file dari disk
        archive_read_disk_entry_from_file(disk, entry, -1, NULL);
        
        r = archive_write_header(a, entry);
        if (r == ARCHIVE_OK) {
            FILE *f = fopen(files[i], "rb");
            if (f) {
                char buff[65536]; // Diperbesar dari 8KB menjadi 64KB untuk I/O bottleneck
                size_t len = fread(buff, 1, sizeof(buff), f);
                while (len > 0) {
                    archive_write_data(a, buff, len);
                    len = fread(buff, 1, sizeof(buff), f);
                }
                fclose(f);
            }
        }
        
        archive_entry_free(entry);
        archive_read_free(disk);
    }
    
    archive_write_close(a);
    archive_write_free(a);
    
    return 0;
}

int ampoti_extract(const char *archive_path, const char *output_dir, const char *password) {
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int flags;
    int r;
    
    flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;
    // SECURITY: Mencegah serangan Zip Slip (Directory Traversal)
    flags |= ARCHIVE_EXTRACT_SECURE_NODOTDOT | ARCHIVE_EXTRACT_SECURE_SYMLINKS;
    
    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);
    
    if (password != NULL) {
        archive_read_add_passphrase(a, password);
    }
    
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);
    
    if ((r = archive_read_open_filename(a, archive_path, 10240))) {
        archive_read_free(a);
        archive_write_free(ext);
        return -1;
    }
    
    for (;;) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r < ARCHIVE_OK) {
            // Tangani error jika diperlukan
        }
        
        // Di sini seharusnya prepending output_dir pada entry pathname dilakukan
        // Prepend output_dir ke entry pathname untuk mengekstrak ke folder yang benar
        const char *current_path = archive_entry_pathname(entry);
        char *full_path = NULL;
        if (current_path != NULL && output_dir != NULL) {
            size_t out_len = strlen(output_dir);
            size_t path_len = strlen(current_path);
            full_path = (char*)malloc(out_len + path_len + 2);
            if (full_path) {
                sprintf(full_path, "%s/%s", output_dir, current_path);
                archive_entry_set_pathname(entry, full_path);
            }
        }
        
        r = archive_write_header(ext, entry);
        
        if (full_path) {
            free(full_path); // Bebaskan memori setelah header ditulis
        }
        
        if (r < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(ext));
        } else if (archive_entry_size(entry) > 0) {
            r = copy_data(a, ext);
            if (r < ARCHIVE_OK)
                fprintf(stderr, "%s\n", archive_error_string(ext));
        }
        
        r = archive_write_finish_entry(ext);
        if (r < ARCHIVE_OK)
            fprintf(stderr, "%s\n", archive_error_string(ext));
    }
    
    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
    
    return 0;
}
