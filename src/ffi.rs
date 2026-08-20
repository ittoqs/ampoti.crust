use std::ffi::CString;
use std::os::raw::{c_char, c_int};
use std::ptr;

extern "C" {
    fn ampoti_compress(
        output_path: *const c_char,
        files: *const *const c_char,
        entry_names: *const *const c_char,
        num_files: c_int,
        password: *const c_char,
        format: *const c_char,
        error_buf: *mut c_char,
        error_buf_size: usize,
    ) -> c_int;

    fn ampoti_extract(
        archive_path: *const c_char,
        output_dir: *const c_char,
        password: *const c_char,
        error_buf: *mut c_char,
        error_buf_size: usize,
    ) -> c_int;
}

pub fn compress_files(
    output_path: &str,
    files: &[(&str, &str)],
    password: Option<&str>,
    format: &str,
) -> Result<(), String> {
    let out_path_c = CString::new(output_path).map_err(|_| "Invalid output path")?;
    let format_c = CString::new(format).map_err(|_| "Invalid format")?;

    let password_c = match password {
        Some(p) => Some(CString::new(p).map_err(|_| "Invalid password")?),
        None => None,
    };

    // Mempersiapkan array CStrings agar alokasinya tetap hidup selama FFI call
    let mut files_c = Vec::new();
    let mut entries_c = Vec::new();
    for &(f, e) in files {
        files_c.push(CString::new(f).map_err(|_| "Invalid file path")?);
        entries_c.push(CString::new(e).map_err(|_| "Invalid entry path")?);
    }

    // Mempersiapkan array of pointers ke CStrings
    let files_ptrs: Vec<*const c_char> = files_c.iter().map(|cs| cs.as_ptr()).collect();
    let entries_ptrs: Vec<*const c_char> = entries_c.iter().map(|cs| cs.as_ptr()).collect();

    let pwd_ptr = password_c.as_ref().map_or(ptr::null(), |p| p.as_ptr());

    let mut error_buf = vec![0u8; 1024];

    // Pemanggilan fungsi C Engine secara Unsafe
    let result = unsafe {
        ampoti_compress(
            out_path_c.as_ptr(),
            files_ptrs.as_ptr(),
            entries_ptrs.as_ptr(),
            files_ptrs.len() as c_int,
            pwd_ptr,
            format_c.as_ptr(),
            error_buf.as_mut_ptr() as *mut c_char,
            error_buf.len(),
        )
    };

    if result == 0 {
        Ok(())
    } else {
        let err_msg = std::ffi::CStr::from_bytes_until_nul(&error_buf)
            .map(|cs| cs.to_string_lossy().into_owned())
            .unwrap_or_else(|_| "Unknown error".to_string());

        let trimmed_err = err_msg.trim();
        if trimmed_err.is_empty() {
             Err(format!("Compression failed with code: {}", result))
        } else {
             Err(trimmed_err.to_string())
        }
    }
}

pub fn extract_archive(
    archive_path: &str,
    output_dir: &str,
    password: Option<&str>,
) -> Result<(), String> {
    let arch_path_c = CString::new(archive_path).map_err(|_| "Invalid archive path")?;
    let out_dir_c = CString::new(output_dir).map_err(|_| "Invalid output directory")?;
    let password_c = match password {
        Some(p) => Some(CString::new(p).map_err(|_| "Invalid password")?),
        None => None,
    };

    let pwd_ptr = password_c.as_ref().map_or(ptr::null(), |p| p.as_ptr());

    let mut error_buf = vec![0u8; 1024];

    // Pemanggilan fungsi C Engine secara Unsafe
    let result = unsafe {
        ampoti_extract(
            arch_path_c.as_ptr(),
            out_dir_c.as_ptr(),
            pwd_ptr,
            error_buf.as_mut_ptr() as *mut c_char,
            error_buf.len(),
        )
    };

    if result == 0 {
        Ok(())
    } else {
        let err_msg = std::ffi::CStr::from_bytes_until_nul(&error_buf)
            .map(|cs| cs.to_string_lossy().into_owned())
            .unwrap_or_else(|_| "Unknown error".to_string());

        let trimmed_err = err_msg.trim();
        if trimmed_err.is_empty() {
             Err(format!("Extraction failed with code: {}", result))
        } else {
             Err(trimmed_err.to_string())
        }
    }
}
