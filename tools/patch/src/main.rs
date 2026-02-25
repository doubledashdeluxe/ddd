use std::collections::{BTreeSet, HashSet};
use std::fmt::Write;
use std::fs;

use anyhow::{Context, Result};
use bpaf::{OptionParser, Parser};
use object::{File, Object, ObjectSection, ObjectSymbol, SymbolKind};

fn main() -> Result<()> {
    let options = options().run();
    let input_obj = fs::read(options.input_obj)?;
    let file = File::parse(&*input_obj)?;

    let mut replacement_sections = HashSet::new();
    for section in file.sections() {
        if section.name()?.starts_with("replacements") {
            replacement_sections.insert(Some(section.index()));
        }
    }

    let mut replaced_symbols = vec![];
    let mut replacement_symbols = BTreeSet::new();
    let mut other_symbols = HashSet::new();
    for symbol in file.symbols() {
        let name = symbol.name()?;
        if symbol.is_undefined() {
            if let Some(name) = name.strip_prefix("thunk_replaced_") {
                replaced_symbols.push(name);
            }
        } else if replacement_sections.contains(&symbol.section_index()) {
            if symbol.kind() == SymbolKind::Text {
                replacement_symbols.insert(name);
            }
        } else if let SymbolKind::Text | SymbolKind::Data = symbol.kind() {
            other_symbols.insert(name);
        }
    }

    for symbol in &replaced_symbols {
        anyhow::ensure!(
            replacement_symbols.contains(symbol),
            "REPLACED was used without REPLACE for symbol {symbol}!",
        );
    }

    let mut unresolved_symbols = replacement_symbols.clone();
    let input_symbols = fs::read_to_string(options.input_symbols)?;
    let mut output_symbols = String::new();
    for symbol in input_symbols.lines() {
        if symbol.trim().is_empty() {
            writeln!(&mut output_symbols)?;
            continue;
        }

        let (address, symbol) = symbol.split_once(' ').context("Invalid symbol")?;
        let address = u32::from_str_radix(address, 16)?;

        anyhow::ensure!(
            !other_symbols.contains(&symbol),
            "Multiple definitions for symbol {symbol}!",
        );

        if unresolved_symbols.remove(symbol) {
            writeln!(&mut output_symbols, "{address:#08x} replaced_{symbol}")?;
        } else {
            writeln!(&mut output_symbols, "{address:#08x} {symbol}")?;
        }
    }
    if let Some(symbol) = unresolved_symbols.iter().next() {
        anyhow::bail!("Attempted to REPLACE {symbol}, but it doesn't exist in the symbol file!");
    }
    fs::write(options.output_symbols, output_symbols)?;

    let mut output_patches = String::new();
    writeln!(&mut output_patches, "#include \"payload/Patcher.hh\"")?;
    writeln!(&mut output_patches)?;
    writeln!(&mut output_patches, "#pragma section RWX \"thunks\" \"thunks\"")?;
    writeln!(&mut output_patches)?;
    writeln!(
        &mut output_patches,
        "const size_t Patcher::PatchCount = {};",
        replacement_symbols.len()
    )?;
    writeln!(&mut output_patches)?;
    for symbol in &replacement_symbols {
        writeln!(&mut output_patches, "extern \"C\" int replaced_{symbol};")?;
        writeln!(&mut output_patches, "extern \"C\" int {symbol};")?;
        if replaced_symbols.contains(symbol) {
            writeln!(&mut output_patches, "extern \"C\" {{")?;
            writeln!(
                &mut output_patches,
                "__declspec(section \"thunks\") u32 thunk_replaced_{symbol}[2];"
            )?;
            writeln!(&mut output_patches, "}}")?;
        }
        writeln!(&mut output_patches)?;
    }
    writeln!(&mut output_patches, "const Patcher::Patch Patcher::Patches[] = {{")?;
    for symbol in &replacement_symbols {
        writeln!(&mut output_patches, "    {{")?;
        writeln!(&mut output_patches, "        &replaced_{symbol},")?;
        writeln!(&mut output_patches, "        &{symbol},")?;
        if replaced_symbols.contains(symbol) {
            writeln!(&mut output_patches, "        thunk_replaced_{symbol},")?;
        } else {
            writeln!(&mut output_patches, "        nullptr,")?;
        }
        writeln!(&mut output_patches, "    }},")?;
    }
    writeln!(&mut output_patches, "}};")?;
    fs::write(options.output_patches, output_patches)?;
    Ok(())
}

fn options() -> OptionParser<Options> {
    let input_obj = bpaf::positional("input_obj");
    let input_symbols = bpaf::positional("input_symbols");
    let output_symbols = bpaf::positional("output_symbols");
    let output_patches = bpaf::positional("output_patches");
    bpaf::construct!(Options { input_obj, input_symbols, output_symbols, output_patches })
        .to_options()
}

struct Options {
    input_obj: String,
    input_symbols: String,
    output_symbols: String,
    output_patches: String,
}
