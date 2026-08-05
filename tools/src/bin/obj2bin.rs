use std::fs;
use std::iter;

use anyhow::{Context, Result};
use bpaf::{OptionParser, Parser};
use object::elf;
use object::{File, Object, ObjectSegment, SegmentFlags};
use yazi::{CompressionLevel, Format};

fn main() -> Result<()> {
    let options = options().run();
    let input = fs::read(options.input)?;
    let file = File::parse(&*input)?;

    let mut segments: Vec<_> = file
        .segments()
        .filter(|segment| {
            let SegmentFlags::Elf { p_flags, .. } = segment.flags() else { return false };
            let x = p_flags & elf::PF_X == elf::PF_X;
            match options.kind {
                Kind::Insts => x,
                Kind::Data => !x,
            }
        })
        .collect();
    segments.sort_unstable_by_key(ObjectSegment::address);

    let mut address = None;
    let mut output = vec![];
    for segment in segments {
        if let Some(address) = address {
            let size = segment.address().checked_sub(address).context("Overlapping segments")?;
            output.extend(iter::repeat_n(0x00, size as usize));
        }
        let data = segment.data()?;
        output.extend(data);
        output.extend(iter::repeat_n(0x00, segment.size() as usize - data.len()));
        address = Some(segment.address() + segment.size());
    }
    let output = yazi::compress(&output, Format::Raw, CompressionLevel::Specific(10))
        .map_err(|e| anyhow::anyhow!("{e:?}"))?;
    fs::write(options.output, output)?;
    Ok(())
}

fn options() -> OptionParser<Options> {
    let insts = bpaf::short('i').req_flag(Kind::Insts);
    let data = bpaf::short('d').req_flag(Kind::Data);
    let kind = bpaf::construct!([insts, data]);

    let input = bpaf::positional("input");
    let output = bpaf::positional("output");

    bpaf::construct!(Options { kind, input, output }).to_options()
}

struct Options {
    kind: Kind,
    input: String,
    output: String,
}

#[derive(Clone)]
enum Kind {
    Insts,
    Data,
}
