use std::fmt::Display;
use std::str::FromStr;

use bpaf::params::ParseArgument;
use bpaf::{OptionParser, Parser};

pub fn options() -> OptionParser<Options> {
    let net_sim = net_sim_options();
    bpaf::construct!(Options { net_sim }).to_options()
}

fn net_sim_options() -> impl Parser<NetSimOptions> {
    let drops = net_sim_option('d', "drops", "DROPS", 0.2, "Sets the simulated network drop ratio");
    let latency = net_sim_option(
        'l',
        "latency",
        "LATENCY",
        100,
        "Sets the simulated network latency in milliseconds",
    );
    let jitter = net_sim_option(
        'j',
        "jitter",
        "JITTER",
        30,
        "Sets the simulated network jitter in milliseconds",
    );
    bpaf::construct!(NetSimOptions { drops, latency, jitter })
        .group_help("Network simulation options:")
}

fn net_sim_option<T>(
    short: char,
    long: &'static str,
    metavar: &'static str,
    default: T,
    help: &'static str,
) -> impl Parser<T>
where
    T: Clone + Default + Display + FromStr + 'static,
    ParseArgument<T>: Parser<T>,
{
    let arg = bpaf::short(short).long(long).help(help).argument(metavar);
    let help = format!("Alias for -{short}={default}");
    let flag = bpaf::short(short).long(long).help(&*help).req_flag(default);
    bpaf::construct!([flag, arg]).fallback(T::default()).display_fallback()
}

pub struct Options {
    pub net_sim: NetSimOptions,
}

#[derive(Clone, Copy)]
pub struct NetSimOptions {
    pub drops: f64,
    pub latency: u64,
    pub jitter: u64,
}
