use std::ffi::CString;
use std::os::raw::{c_char, c_int};
use std::ptr;

extern "C" {
    fn ampoti_compress(
        output_path: *const c_char,
        files: *const *const c_char,
        num_files: c_int,
        password: *const c_char,
        format: *const c_char,
    ) -> c_int;

    fn ampoti_extract(
        archive_path: *const c_char,
        output_dir: *const c_char,
        password: *const c_char,
    ) -> c_int;
}

pub fn compress_files(
    output_path: &str,
    files: &[&str],
    password: Option<&str>,
    format: &str,
) -> Result<(), String> {
    let out_path_c = CString::new(output_path).map_err(|_| "Invalid output path")?;
    let format_c = CString::new(format).map_err(|_| "Invalid format")?;
    
    let password_c = password.map(|p| CString::new(p).unwrap());

    // Mempersiapkan array CStrings agar alokasinya tetap hidup selama FFI call
    let files_c: Vec<CString> = files
        .iter()
        .map(|&f| CString::new(f).unwrap())
        .collect();

    // Mempersiapkan array of pointers ke CStrings
    let files_ptrs: Vec<*const c_char> = files_c
        .iter()
        .map(|cs| cs.as_ptr())
        .collect();

    let pwd_ptr = password_c.as_ref().map_or(ptr::null(), |p| p.as_ptr());

    // Pemanggilan fungsi C Engine secara Unsafe
    let result = unsafe {
        ampoti_compress(
            out_path_c.as_ptr(),
            files_ptrs.as_ptr(),
            files_ptrs.len() as c_int,
            pwd_ptr,
            format_c.as_ptr(),
        )
    };

    if result == 0 {
        Ok(())
    } else {
        Err(format!("Compression failed with code: {}", result))
    }
}

pub fn extract_archive(
    archive_path: &str,
    output_dir: &str,
    password: Option<&str>,
) -> Result<(), String> {
    let arch_path_c = CString::new(archive_path).map_err(|_| "Invalid archive path")?;
    let out_dir_c = CString::new(output_dir).map_err(|_| "Invalid output directory")?;
    let password_c = password.map(|p| CString::new(p).unwrap());

    let pwd_ptr = password_c.as_ref().map_or(ptr::null(), |p| p.as_ptr());

    // Pemanggilan fungsi C Engine secara Unsafe
    let result = unsafe {
        ampoti_extract(
            arch_path_c.as_ptr(),
            out_dir_c.as_ptr(),
            pwd_ptr,
        )
    };

    if result == 0 {
        Ok(())
    } else {
        Err(format!("Extraction failed with code: {}", result))
    }
}
