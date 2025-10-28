use std::env;
use std::fs;
use std::io::{ErrorKind, Result};
use std::path::Path;

fn main() -> Result<()> {
    let out_dir = env::var_os("OUT_DIR").unwrap();
    match fs::remove_dir_all(&out_dir) {
        Err(e) if e.kind() == ErrorKind::NotFound => (),
        r => r?,
    }
    fs::create_dir(&out_dir)?;
    fs::write(Path::new(&out_dir).join("online.rs"), ddd_formats::online().rs())?;
    fs::write(Path::new(&out_dir).join("version.rs"), ddd_formats::version().rs())?;
    println!("cargo::rerun-if-changed=build.rs");
    Ok(())
}
