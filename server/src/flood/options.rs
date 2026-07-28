use std::time::Duration;

use crate::crypto::PublicKey;
use crate::flood::connection::Connection;
use crate::flood::cookie::Cookie;
use crate::flood::counters::Counters;
use crate::flood::flood::Flood;
use crate::flood::identity::Identity;
use crate::flood::kx::Kx;
use crate::flood::mpmc::Receiver;
use crate::flood::mpsc::Sender;
use crate::flood::search::Search;

use bpaf::{OptionParser, Parser};

pub fn options() -> OptionParser<Options> {
    let parallelism =
        bpaf::short('p').long("parallelism").argument("PARALLELISM").fallback(1).display_fallback();
    let duration =
        bpaf::short('d').long("duration").argument("DURATION").fallback(5).display_fallback();
    let cookie = bpaf::long("cookie").req_flag(Cookie::flood as F);
    let kx = bpaf::long("kx").req_flag(Kx::flood as F);
    let connection = bpaf::long("connection").req_flag(Connection::flood as F);
    let identity = bpaf::long("identity").req_flag(Identity::flood as F);
    let search = bpaf::long("search").req_flag(Search::flood as F);
    let flood = bpaf::construct!([cookie, kx, connection, identity, search]);
    bpaf::construct!(Options { parallelism, duration, flood }).to_options()
}

#[derive(Clone, Copy, Debug)]
pub struct Options {
    pub parallelism: usize,
    pub duration: u64,
    pub flood: F,
}

pub type F = fn(&Sender, &Receiver, PublicKey, Duration, usize) -> Counters;
