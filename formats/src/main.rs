use std::fs;
use std::io;

use bpaf::{OptionParser, Parser};

fn main() -> Result<(), io::Error> {
    let options = options().run();
    let format = match (options.format, options.extension) {
        (Format::Online, Extension::Rs) => ddd_formats::online().rs(),
        (Format::Online, Extension::Hh) => ddd_formats::online().hh(),
        (Format::Version, Extension::Rs) => ddd_formats::version().rs(),
        (Format::Version, Extension::Hh) => ddd_formats::version().hh(),
    };
    match options.output {
        Some(output) => fs::write(output, &format)?,
        #[expect(clippy::print_stdout)]
        None => print!("{format}"),
    }
    Ok(())
}

fn options() -> OptionParser<Options> {
    let online = bpaf::long("online").req_flag(Format::Online);
    let version = bpaf::long("version").req_flag(Format::Version);
    let format = bpaf::construct!([online, version]);

    let rs = bpaf::long("rs").req_flag(Extension::Rs);
    let hh = bpaf::long("hh").req_flag(Extension::Hh);
    let extension = bpaf::construct!([rs, hh]);

    let output = bpaf::long("output").argument("OUTPUT").optional();

    bpaf::construct!(Options { format, extension, output }).to_options()
}

struct Options {
    format: Format,
    extension: Extension,
    output: Option<String>,
}

#[derive(Clone)]
enum Format {
    Online,
    Version,
}

#[derive(Clone)]
enum Extension {
    Rs,
    Hh,
}
