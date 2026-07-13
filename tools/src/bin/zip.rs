use std::fmt::Write as _;
use std::fs;
use std::io::Write as _;

use anyhow::Result;
use bpaf::{OptionParser, Parser};
use crrl::ed25519::PrivateKey;
use crrl::sha2::Sha512;
use rawzip::{CompressionMethod, ZipArchiveWriter};
use yazi::{CompressionLevel, Encoder};

#[expect(clippy::large_stack_frames)]
#[expect(clippy::print_stderr)]
fn main() -> Result<()> {
    let options = options().run();
    let mut output = vec![];
    if options.key.is_some() {
        output.resize(64, 0);
    }
    let mut output = ZipArchiveWriter::builder().with_offset(output.len() as u64).build(output);
    let mut inputs = vec![options.input.clone().into()];
    while let Some(input) = inputs.pop() {
        for input in fs::read_dir(&input)? {
            let input = input?;
            let input = input.path();
            if input.is_dir() {
                inputs.push(input);
                continue;
            }

            let path = input.strip_prefix(&options.input)?.to_string_lossy();
            let (mut entry, config) =
                output.new_file(&path).compression_method(CompressionMethod::DEFLATE).start()?;
            let mut encoder = Encoder::boxed();
            encoder.set_level(CompressionLevel::Specific(10));
            let stream = encoder.stream(&mut entry);
            let mut output = config.wrap(stream);
            let input = fs::read(input)?;
            output.write_all(&input)?;
            let (stream, output) = output.finish()?;
            stream.finish().map_err(|e| anyhow::anyhow!("{e:?}"))?;
            entry.finish(output)?;
        }
    }
    let mut output = output.finish()?;

    if let Some(private_key) = options.key {
        let private_key = fs::read(private_key)?;
        anyhow::ensure!(private_key.len() == 32);
        let private_key = PrivateKey::from_seed(&private_key);
        if options.verbose {
            let public_key = private_key.public_key.encoded;
            let public_key: Result<_> =
                public_key.into_iter().try_fold(String::new(), |mut public_key, byte| {
                    write!(public_key, "{byte:02x?}")?;
                    Ok(public_key)
                });
            let public_key = public_key?;
            eprintln!("Public key: {public_key}");
        }
        let digest = Sha512::hash(&output[64..]);
        let signature = private_key.sign_ph(&[], &digest);
        output[..64].copy_from_slice(&signature);
    }

    fs::write(options.output, output)?;
    Ok(())
}

fn options() -> OptionParser<Options> {
    let verbose = bpaf::short('v').long("verbose").switch();
    let input = bpaf::positional("input");
    let output = bpaf::positional("output");
    let key = bpaf::positional("key").optional();
    bpaf::construct!(Options { verbose, input, output, key }).to_options()
}

struct Options {
    verbose: bool,
    input: String,
    output: String,
    key: Option<String>,
}
