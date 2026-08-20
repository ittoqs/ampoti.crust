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

static void set_error(char *error_buf, size_t error_buf_size, const char *msg) {
    if (error_buf && error_buf_size > 0) {
        if (msg != NULL) {
            strncpy(error_buf, msg, error_buf_size - 1);
        } else {
            strncpy(error_buf, "Unknown error", error_buf_size - 1);
        }
        error_buf[error_buf_size - 1] = '\0';
    }
}

int ampoti_compress(const char *output_path, const char **files, const char **entry_names, int num_files, const char *password, const char *format, char *error_buf, size_t error_buf_size) {
    struct archive *a;
    struct archive_entry *entry;
    int r;
    
    if (error_buf && error_buf_size > 0) {
        error_buf[0] = '\0';
    }

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
        set_error(error_buf, error_buf_size, archive_error_string(a));
        archive_write_free(a);
        return -1;
    }
    
    // Iterasi untuk mengompresi setiap file (versi dasar)
    for (int i = 0; i < num_files; i++) {
        struct archive *disk = archive_read_disk_new();
        archive_read_disk_set_standard_lookup(disk);
        
        entry = archive_entry_new();
        archive_entry_set_pathname(entry, entry_names[i]);
        
        // Membaca info file dari disk
        archive_read_disk_entry_from_file(disk, entry, -1, NULL);
        
        r = archive_write_header(a, entry);
        if (r < ARCHIVE_OK) {
            set_error(error_buf, error_buf_size, archive_error_string(a));
            fprintf(stderr, "Failed to write header: %s\n", archive_error_string(a));
            archive_entry_free(entry);
            archive_read_free(disk);
            archive_write_close(a);
            archive_write_free(a);
            return -1;
        }

        FILE *f = fopen(files[i], "rb");
        if (f) {
            size_t buff_size = 1024 * 1024; // 1MB buffer
            char *buff = (char *)malloc(buff_size);
            if (buff) {
                size_t len = fread(buff, 1, buff_size, f);
                while (len > 0) {
                    if (archive_write_data(a, buff, len) < 0) {
                        set_error(error_buf, error_buf_size, archive_error_string(a));
                        fprintf(stderr, "Failed to write data: %s\n", archive_error_string(a));
                        free(buff);
                        fclose(f);
                        archive_entry_free(entry);
                        archive_read_free(disk);
                        archive_write_close(a);
                        archive_write_free(a);
                        return -1;
                    }
                    len = fread(buff, 1, buff_size, f);
                }
                free(buff);
            } else {
                set_error(error_buf, error_buf_size, "Failed to allocate memory for buffer");
                fprintf(stderr, "Failed to allocate memory for buffer\n");
                fclose(f);
                archive_entry_free(entry);
                archive_read_free(disk);
                archive_write_close(a);
                archive_write_free(a);
                return -1;
            }
            fclose(f);
        } else {
            char msg[512];
            snprintf(msg, sizeof(msg), "Failed to open file for reading: %s", files[i]);
            set_error(error_buf, error_buf_size, msg);
            fprintf(stderr, "Failed to open file for reading: %s\n", files[i]);
            archive_entry_free(entry);
            archive_read_free(disk);
            archive_write_close(a);
            archive_write_free(a);
            return -1;
        }
        
        archive_entry_free(entry);
        archive_read_free(disk);
    }
    
    archive_write_close(a);
    archive_write_free(a);
    
    return 0;
}

int ampoti_extract(const char *archive_path, const char *output_dir, const char *password, char *error_buf, size_t error_buf_size) {
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int flags;
    int r;

    if (error_buf && error_buf_size > 0) {
        error_buf[0] = '\0';
    }
    
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
        set_error(error_buf, error_buf_size, archive_error_string(a));
        archive_read_free(a);
        archive_write_free(ext);
        return -1;
    }
    
    for (;;) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r < ARCHIVE_OK) {
            if (r == ARCHIVE_FATAL) {
                set_error(error_buf, error_buf_size, archive_error_string(a));
                fprintf(stderr, "Fatal: %s\n", archive_error_string(a));
                break;
            } else {
                fprintf(stderr, "Warning: %s\n", archive_error_string(a));
            }
        }
        
        // Di sini seharusnya prepending output_dir pada entry pathname dilakukan
        // Prepend output_dir ke entry pathname untuk mengekstrak ke folder yang benar
        const char *current_path = archive_entry_pathname(entry);
        char *full_path = NULL;
        if (current_path != NULL && output_dir != NULL) {
            while (*current_path == '/' || *current_path == '\\') {
                current_path++;
            }
            size_t out_len = strlen(output_dir);
            size_t path_len = strlen(current_path);
            full_path = (char*)malloc(out_len + path_len + 2);
            if (full_path) {
                sprintf(full_path, "%s/%s", output_dir, current_path);
                archive_entry_set_pathname(entry, full_path);
            } else {
                set_error(error_buf, error_buf_size, "Failed to allocate memory for path");
                fprintf(stderr, "Failed to allocate memory for path\n");
                archive_read_close(a);
                archive_read_free(a);
                archive_write_close(ext);
                archive_write_free(ext);
                return -1;
            }
        }
        
        r = archive_write_header(ext, entry);
        
        if (full_path) {
            free(full_path); // Bebaskan memori setelah header ditulis
        }
        
        if (r < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(ext));
            if (r == ARCHIVE_FATAL) {
                set_error(error_buf, error_buf_size, archive_error_string(ext));
                archive_read_close(a);
                archive_read_free(a);
                archive_write_close(ext);
                archive_write_free(ext);
                return -1;
            }
        } else if (archive_entry_size(entry) > 0) {
            r = copy_data(a, ext);
            if (r < ARCHIVE_OK) {
                fprintf(stderr, "%s\n", archive_error_string(ext));
                if (r == ARCHIVE_FATAL) {
                    set_error(error_buf, error_buf_size, archive_error_string(ext));
                    archive_read_close(a);
                    archive_read_free(a);
                    archive_write_close(ext);
                    archive_write_free(ext);
                    return -1;
                }
            }
        }
        
        r = archive_write_finish_entry(ext);
        if (r < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(ext));
            if (r == ARCHIVE_FATAL) {
                set_error(error_buf, error_buf_size, archive_error_string(ext));
                archive_read_close(a);
                archive_read_free(a);
                archive_write_close(ext);
                archive_write_free(ext);
                return -1;
            }
        }
    }
    
    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
    
    return 0;
}
