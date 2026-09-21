use std::collections::VecDeque;
use std::fmt::{Result, Write};
use std::iter;
use std::marker::PhantomData;

use jiff::civil::{self, DateTime};
use jiff::tz::TimeZone;
use jiff::{Span, Timestamp, ToSpan, Unit};

use crate::website::backend::Backend;
use crate::website::html::Children;
use crate::website::path_printer::PathPrinter;

#[derive(Debug)]
pub struct Stat {
    hourly: Hourly,
    daily: Daily,
    monthly: Monthly,
}

impl Stat {
    pub fn add(&mut self, dt: DateTime, amount: u64) {
        self.modify(dt, |v| *v += amount);
    }

    pub fn max(&mut self, dt: DateTime, value: u64) {
        self.modify(dt, |v| *v = value.max(*v));
    }

    fn modify(&mut self, dt: DateTime, f: impl Fn(&mut u64)) {
        self.hourly.modify(dt, &f);
        self.daily.modify(dt, &f);
        self.monthly.modify(dt, &f);
    }

    pub fn write(&self, name: &str, file_name: &str, backend: &mut impl Backend) {
        let write = |body: &mut Children<_>, _: &PathPrinter| {
            let mut div = body.element("div")?;
            div.attribute("class")?.value("stats")?;
            let mut div = div.children()?;
            self.hourly.write(&mut div)?;
            self.daily.write(&mut div)?;
            self.monthly.write(&mut div)?;
            div.finish()
        };

        backend.write_page(format_args!("{name} · Stats"), write, "stats", file_name, 5 * 60);
    }
}

impl Default for Stat {
    fn default() -> Self {
        let now = Timestamp::now().to_zoned(TimeZone::UTC).into();
        Self { hourly: Hourly::new(now), daily: Daily::new(now), monthly: Monthly::new(now) }
    }
}

type Hourly = Periodic<Hour>;
type Daily = Periodic<Day>;
type Monthly = Periodic<Month>;

#[derive(Debug)]
struct Periodic<P> {
    dt: DateTime,
    values: VecDeque<u64>,
    marker: PhantomData<P>,
}

impl<P: Period> Periodic<P> {
    fn new(now: DateTime) -> Self {
        Self { dt: P::trunc(now), values: VecDeque::new(), marker: PhantomData }
    }

    fn modify(&mut self, dt: DateTime, f: impl Fn(&mut u64)) {
        let dt = P::trunc(dt);
        let span = self.dt.since((P::UNIT, dt)).unwrap();
        let index = P::get_index(span);
        if let Ok(index) = usize::try_from(index) {
            let len = self.values.len().max(index + 1).min(P::MAX_LEN);
            self.values.resize(len, 0);
            if let Some(value) = self.values.get_mut(index) {
                f(value);
            }
        } else {
            for _ in index..-1 {
                self.values.truncate(P::MAX_LEN);
                self.values.push_front(0);
            }
            self.values.truncate(P::MAX_LEN);
            f(self.values.push_front_mut(0));
            self.dt = dt;
        }
    }

    fn write(&self, parent: &mut Children<impl Write>) -> Result {
        let mut div = parent.element("div")?;
        div.attribute("class")?.value("stat")?;
        let mut div = div.children()?;

        div.element("h2")?.content(P::NAME)?;

        let mut table = div.element("table")?.children()?;
        for (dt, value) in iter::zip(self.dt.series(-P::period()), &self.values) {
            let mut tr = table.element("tr")?.children()?;
            tr.element("td")?.content(dt.strftime(P::FORMAT))?;
            tr.element("td")?.content(value)?;
            tr.finish()?;
        }
        table.finish()?;

        div.finish()?;

        Ok(())
    }
}

trait Period {
    const UNIT: Unit;
    const MAX_LEN: usize;
    const NAME: &str;
    const FORMAT: &str;

    fn trunc(dt: DateTime) -> DateTime;
    fn get_index(span: Span) -> i32;
    fn period() -> Span;
}

#[derive(Debug)]
enum Hour {}

impl Period for Hour {
    const UNIT: Unit = Unit::Hour;
    const MAX_LEN: usize = 24;
    const NAME: &str = "Hourly";
    const FORMAT: &str = "%Y-%m-%d %H:%M";

    fn trunc(dt: DateTime) -> DateTime {
        civil::date(dt.year(), dt.month(), dt.day()).at(dt.hour(), 0, 0, 0)
    }

    fn get_index(span: Span) -> i32 {
        span.get_hours()
    }

    fn period() -> Span {
        1.hour()
    }
}

#[derive(Debug)]
enum Day {}

impl Period for Day {
    const UNIT: Unit = Unit::Day;
    const MAX_LEN: usize = 30;
    const NAME: &str = "Daily";
    const FORMAT: &str = "%Y-%m-%d";

    fn trunc(dt: DateTime) -> DateTime {
        civil::date(dt.year(), dt.month(), dt.day()).at(0, 0, 0, 0)
    }

    fn get_index(span: Span) -> i32 {
        span.get_days()
    }

    fn period() -> Span {
        1.day()
    }
}

#[derive(Debug)]
enum Month {}

impl Period for Month {
    const UNIT: Unit = Unit::Month;
    const MAX_LEN: usize = 5 * 12;
    const NAME: &str = "Monthly";
    const FORMAT: &str = "%Y-%m";

    fn trunc(dt: DateTime) -> DateTime {
        civil::date(dt.year(), dt.month(), 1).at(0, 0, 0, 0)
    }

    fn get_index(span: Span) -> i32 {
        span.get_months()
    }

    fn period() -> Span {
        1.month()
    }
}
