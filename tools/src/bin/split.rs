use std::fs;
use std::iter;

use anyhow::{Context, Result};
use bpaf::{OptionParser, Parser};
use object::build::Id;
use object::build::elf::{Builder, SectionData};
use object::elf;

fn main() -> Result<()> {
    let options = options().run();
    let input = fs::read(options.input)?;
    let mut builder = Builder::read(&*input)?;

    let mut rela_sections = vec![None; builder.sections.count()];
    for section in &builder.sections {
        let SectionData::Relocation(_) = &section.data else { continue };
        let index = section.sh_info.checked_sub(1).context("Invalid .rela sh_info")? as usize;
        let rela_section = rela_sections.get_mut(index).context("Invalid .rela sh_info")?;
        anyhow::ensure!(rela_section.is_none());
        *rela_section = Some(section.id());
    }

    for symbol in &mut builder.symbols {
        let Some(section) = symbol.section else { continue };
        let elf::STT_FUNC = symbol.st_type() else { continue };
        let st_value = symbol.st_value;
        let st_size = symbol.st_size;
        let sh_offset = builder.sections.get(section).sh_offset;
        let rela_section = rela_sections[section.index()];

        let section = builder.sections.copy(section);
        let section_id = section.id();
        let section_name = section.name.to_mut();
        section_name.push(b'.');
        section_name.extend(&*symbol.name);
        section.sh_offset = sh_offset + st_value;
        section.sh_size = st_size;
        match &mut section.data {
            SectionData::Data(data) => {
                let data = data.to_mut();
                anyhow::ensure!(st_value as usize <= data.len());
                data.splice(..st_value as usize, iter::empty());
                anyhow::ensure!(st_size as usize <= data.len());
                data.truncate(st_size as usize);
            }
            SectionData::UninitializedData(len) => {
                *len = st_size;
            }
            _ => anyhow::bail!("Unexpected section data {:?}", section.data),
        }
        symbol.section = Some(section_id);
        symbol.st_value = 0;

        let Some(rela_section) = rela_section else { continue };
        let rela_section = builder.sections.copy(rela_section);
        let SectionData::Relocation(relocations) = &mut rela_section.data else {
            anyhow::bail!("Unexpected .rela section data {:?}", rela_section.data);
        };
        let rela_section_name = rela_section.name.to_mut();
        rela_section_name.push(b'.');
        rela_section_name.extend(&*symbol.name);
        rela_section.sh_info_section = Some(section_id);
        relocations.retain(|relocation| {
            relocation.r_offset >= st_value && relocation.r_offset < st_value + st_size
        });
        for relocation in relocations {
            relocation.r_offset -= st_value;
        }
    }

    let mut output = vec![];
    builder.write(&mut output)?;
    fs::write(options.output, output)?;
    Ok(())
}

fn options() -> OptionParser<Options> {
    let input = bpaf::positional("input");
    let output = bpaf::positional("output");

    bpaf::construct!(Options { input, output }).to_options()
}

struct Options {
    input: String,
    output: String,
}
