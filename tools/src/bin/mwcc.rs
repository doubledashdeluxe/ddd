use std::env;
use std::fs;
use std::process::Command;

use anyhow::{Context, Result};

fn main() -> Result<()> {
    let args: Vec<_> = env::args().skip(1).collect();
    let makefile_path = args
        .array_windows()
        .find(|[key, _]| matches!(key.as_str(), "-Mfile" | "-MMfile" | "-MDfile" | "-MMDfile"))
        .map(|[_, value]| value)
        .cloned();

    let exit_status = Command::new("wine")
        .arg("tools/cw/modified_mwcceppc.exe")
        .args(args)
        .env("WINEDEBUG", "-all")
        .spawn()?
        .wait()?;
    anyhow::ensure!(exit_status.success());

    let Some(makefile_path) = makefile_path else { return Ok(()) };
    let makefile = fs::read_to_string(&makefile_path)?;
    let current_dir = env::current_dir()?;
    let current_dir = current_dir.to_str().context("invalid UTF-8")?;
    let prefix = format!("Z:{}\\", current_dir.replace('/', "\\"));
    let makefile =
        makefile.replace(&prefix, "").replace('\\', "/").replace('\r', "").replace("/\n", "\\\n");
    fs::write(makefile_path, makefile)?;
    Ok(())
}
