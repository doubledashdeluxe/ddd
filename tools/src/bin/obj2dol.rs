use std::fs;
use std::iter;

use anyhow::Result;
use bpaf::{OptionParser, Parser};
use object::elf;
use object::{File, Object, ObjectSegment, SegmentFlags};

fn main() -> Result<()> {
    let options = options().run();
    let input = fs::read(options.input)?;
    let file = File::parse(&*input)?;

    let mut output = vec![0x00; 0x100];
    let mut text_index = 0;
    let mut data_index = 7;
    let mut bss_start = None;
    let mut bss_end = 0;
    for segment in file.segments() {
        let SegmentFlags::Elf { p_flags, .. } = segment.flags() else { continue };
        let data = segment.data()?;
        let index = if p_flags & elf::PF_X == elf::PF_X {
            anyhow::ensure!(text_index < 7);
            &mut text_index
        } else if !data.is_empty() {
            anyhow::ensure!(data_index < 18);
            &mut data_index
        } else {
            if bss_start.is_none() {
                bss_start = Some(segment.address() as u32);
            }
            bss_end = segment.address() + segment.size();
            continue;
        };
        let offset = output.len();
        write_to_dol_header(&mut output, 0x00 + 0x04 * *index, offset as u32);
        write_to_dol_header(&mut output, 0x48 + 0x04 * *index, segment.address() as u32);
        let size = data.len().next_multiple_of(0x20);
        write_to_dol_header(&mut output, 0x90 + 0x04 * *index, size as u32);
        output.extend(data);
        output.extend(iter::repeat_n(0, (0x20 - output.len()) & 0x1f));
        *index += 1;
    }
    if let Some(bss_start) = bss_start {
        write_to_dol_header(&mut output, 0xd8, bss_start);
        let bss_size = bss_end as u32 - bss_start;
        write_to_dol_header(&mut output, 0xdc, bss_size);
    }
    write_to_dol_header(&mut output, 0xe0, file.entry() as u32);

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

fn write_to_dol_header(output: &mut [u8], offset: u32, x: u32) {
    let offset = offset as usize;
    output[offset..offset + 4].copy_from_slice(&x.to_be_bytes());
}
