use std::env;
use std::path::PathBuf;

fn main() {
    let core_dir = PathBuf::from("core");
    let src_dir = core_dir.join("src");
    let include_dir = core_dir.join("include");

    println!("cargo:rerun-if-changed={}", src_dir.display());
    println!("cargo:rerun-if-changed={}", include_dir.display());

    cc::Build::new()
        .file(src_dir.join("core.c"))
        .include(include_dir)
        .compile("core");
    
    // Link ke libarchive untuk fungsi kompresi dan ekstraksi
    println!("cargo:rustc-link-lib=archive");
}
