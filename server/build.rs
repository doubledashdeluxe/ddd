use std::env;
use std::fs;
use std::io::Result;
use std::path::Path;

use ddd_formats;

fn main() -> Result<()> {
    let out_dir = env::var_os("OUT_DIR").unwrap();
    fs::write(Path::new(&out_dir).join("client_state.rs"), ddd_formats::client_state().rs())?;
    fs::write(Path::new(&out_dir).join("server_state.rs"), ddd_formats::server_state().rs())?;
    fs::write(Path::new(&out_dir).join("version.rs"), ddd_formats::version().rs())?;
    println!("cargo::rerun-if-changed=build.rs");
    Ok(())
}
