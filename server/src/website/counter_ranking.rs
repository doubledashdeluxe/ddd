use std::cmp::Reverse;
use std::collections::BTreeMap;
use std::fmt::{Display, Result, Write};
use std::hash::Hash;
use std::ops::{AddAssign, SubAssign};

use jiff::SignedDuration;
use jiff::civil::Date;

use crate::sorted::Sorted;
use crate::website::html::{Children, Element};
use crate::website::path_printer::PathPrinter;
use crate::website::ranking;

#[derive(Debug)]
pub struct CounterRanking<T, C> {
    counters: BTreeMap<(Date, T), C>,
    recent: Sorted<T, Reverse<C>>,
    recent_sum: C,
    total: Sorted<T, Reverse<C>>,
    total_sum: C,
}

impl<T: Clone + Hash + Ord, C: AddAssign + Copy + Default + Ord + SubAssign> CounterRanking<T, C> {
    pub fn add(&mut self, now: Date, date: Date, value: T, amount: C) {
        if now.duration_since(date) < RECENT_DURATION {
            *self.counters.entry((date, value.clone())).or_default() += amount;

            self.recent.modify(value.clone(), |counter| counter.0 += amount);
            self.recent_sum += amount;
        }

        self.total.modify(value, |counter| counter.0 += amount);
        self.total_sum += amount;
    }

    pub fn update(&mut self, now: Date) {
        for ((date, value), amount) in self.counters.extract_if(.., |_, _| true) {
            if now.duration_since(date) < RECENT_DURATION {
                break;
            }

            self.recent.modify(value, |counter| counter.0 -= amount);
            self.recent_sum -= amount;
        }
    }
}

impl<T, C: Default + Display + PartialEq> CounterRanking<T, C> {
    pub fn write<W: Write>(
        &self,
        write_value: impl Fn(&T, Element<W>, &PathPrinter) -> Result,
        parent: &mut Children<W>,
        path_printer: &PathPrinter,
    ) -> Result {
        ranking::write(
            "Last 30 days",
            &self.recent,
            Some(&self.recent_sum),
            &write_value,
            parent,
            path_printer,
        )?;
        ranking::write(
            "All-time",
            &self.total,
            Some(&self.total_sum),
            &write_value,
            parent,
            path_printer,
        )?;
        Ok(())
    }
}

impl<T, C: Default> Default for CounterRanking<T, C> {
    fn default() -> Self {
        Self {
            counters: BTreeMap::default(),
            recent: Sorted::default(),
            recent_sum: C::default(),
            total: Sorted::default(),
            total_sum: C::default(),
        }
    }
}

const RECENT_DURATION: SignedDuration = SignedDuration::from_hours(30 * 24);
