use std::fs;
use std::io;

use bpaf::{OptionParser, Parser};

use ddd_formats;

fn main() -> Result<(), io::Error> {
    let options = options().run();
    let format = match (options.format, options.extension) {
        (Format::ClientState, Extension::Rs) => ddd_formats::client_state().rs(),
        (Format::ClientState, Extension::Hh) => ddd_formats::client_state().hh(),
        (Format::ClientState, Extension::Cc) => ddd_formats::client_state().cc(),
        (Format::ServerState, Extension::Rs) => ddd_formats::server_state().rs(),
        (Format::ServerState, Extension::Hh) => ddd_formats::server_state().hh(),
        (Format::ServerState, Extension::Cc) => ddd_formats::server_state().cc(),
        (Format::Version, Extension::Rs) => ddd_formats::version().rs(),
        (Format::Version, Extension::Hh) => ddd_formats::version().hh(),
        (Format::Version, Extension::Cc) => ddd_formats::version().cc(),
    };
    match options.output {
        Some(output) => fs::write(output, &format)?,
        None => print!("{}", format),
    }
    Ok(())
}

fn options() -> OptionParser<Options> {
    let client_state = bpaf::long("client-state").req_flag(Format::ClientState);
    let server_state = bpaf::long("server-state").req_flag(Format::ServerState);
    let version = bpaf::long("version").req_flag(Format::Version);
    let format = bpaf::construct!([client_state, server_state, version]);

    let rs = bpaf::long("rs").req_flag(Extension::Rs);
    let hh = bpaf::long("hh").req_flag(Extension::Hh);
    let cc = bpaf::long("cc").req_flag(Extension::Cc);
    let extension = bpaf::construct!([rs, hh, cc]);

    let output = bpaf::long("output").argument("output").optional();

    bpaf::construct!(Options { format, extension, output }).to_options()
}

struct Options {
    format: Format,
    extension: Extension,
    output: Option<String>,
}

#[derive(Clone)]
enum Format {
    ClientState,
    ServerState,
    Version,
}

#[derive(Clone)]
enum Extension {
    Rs,
    Hh,
    Cc,
}
