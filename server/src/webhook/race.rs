use std::fmt::{self, Display, Formatter};

use heapless::Vec;
use serde::{Serialize, Serializer};

use crate::base64;
use crate::courses::{Course, Courses};
use crate::formats::online::*;
use crate::player::Name;
use crate::storage;

#[derive(Serialize)]
pub struct Race<'a> {
    username: &'a str,
    avatar_url: &'a str,
    embeds: (Embed<'a>,),
}

impl<'a> Race<'a> {
    pub fn new(
        courses: &'a Courses,
        username: &'a str,
        avatar_url: &'a str,
        url: &'a str,
        race: &mut storage::Race,
    ) -> Self {
        let author = Author {
            name: AuthorName { room_number: race.room_number, has_host: race.host_pk.is_some() },
            url: AuthorUrl { base_url: url, room_number: race.room_number },
        };
        let course_value = courses
            .get(&race.course_hash)
            .map_or(CourseValue::Hash(race.course_hash), CourseValue::Course);
        let course_field =
            CourseField { name: CourseName { is_race: race.mode.is_race() }, value: course_value };
        race.karts.sort_unstable_by_key(|kart| kart.result_index);
        let karts: Vec<_, _> = race
            .karts
            .iter()
            .map(|kart| Kart {
                team: kart.team,
                names: kart.players.iter().map(|player| player.name).collect(),
                time: kart.result_time,
                point_diff: i32::from(kart.result_points) - i32::from(kart.points),
                points: kart.result_points,
            })
            .collect();
        let players_field = PlayersField { name: "Players", value: PlayersValue { karts } };
        let footer_text = FooterText {
            frame_rate: race.frame_rate,
            engine_size: race.engine_size,
            item_mode: race.item_mode,
            lap_count: race.lap_count,
            has_host: race.host_pk.is_some(),
            race_count: race.race_count,
            race_index: race.race_index,
        };
        let embed = Embed {
            author,
            title: Title { race_number: race.number, mode: race.mode },
            url: Url { base_url: url, race_number: race.number },
            fields: (course_field, players_field),
            footer: Footer { text: footer_text },
        };
        Self { username, avatar_url, embeds: (embed,) }
    }
}

#[derive(Serialize)]
struct Embed<'a> {
    author: Author<'a>,
    #[serde(serialize_with = "serialize_display")]
    title: Title,
    #[serde(serialize_with = "serialize_display")]
    url: Url<'a>,
    fields: (CourseField<'a>, PlayersField),
    footer: Footer,
}

#[derive(Serialize)]
struct Author<'a> {
    #[serde(serialize_with = "serialize_display")]
    name: AuthorName,
    #[serde(serialize_with = "serialize_display")]
    url: AuthorUrl<'a>,
}

struct AuthorName {
    room_number: u64,
    has_host: bool,
}

impl Display for AuthorName {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        let name = if self.has_host { "Personal" } else { "Worldwide" };
        write!(f, "{name} Room #{}", self.room_number)
    }
}

struct AuthorUrl<'a> {
    base_url: &'a str,
    room_number: u64,
}

impl Display for AuthorUrl<'_> {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        write!(f, "{}/{}", self.base_url, self.room_number)
    }
}

struct Title {
    race_number: u64,
    mode: ModeIndex,
}

impl Display for Title {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        let mode = match self.mode {
            ModeIndex::Versus => "\u{1f3c1}",
            ModeIndex::Balloon => "\u{1f388}",
            ModeIndex::Escape => "\u{1f31e}",
            ModeIndex::Bomb => "\u{1f4a3}",
            ModeIndex::TimeAttack => "\u{23f1}\u{fe0f}",
        };

        let name = if self.mode.is_race() { "Race" } else { "Battle" };

        write!(f, "{mode} {name} #{}", self.race_number)
    }
}

struct Url<'a> {
    base_url: &'a str,
    race_number: u64,
}

impl Display for Url<'_> {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        write!(f, "{}/{}", self.base_url, self.race_number)
    }
}

#[derive(Serialize)]
struct CourseField<'a> {
    #[serde(serialize_with = "serialize_display")]
    name: CourseName,
    value: CourseValue<'a>,
}

struct CourseName {
    is_race: bool,
}

impl Display for CourseName {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        let name = if self.is_race { "Course" } else { "Stage" };
        write!(f, "{name}")
    }
}

#[derive(Serialize)]
#[serde(untagged)]
enum CourseValue<'a> {
    #[serde(serialize_with = "serialize_display")]
    Course(&'a Course),
    #[serde(with = "base64")]
    Hash([u8; 32]),
}

#[derive(Serialize)]
struct PlayersField {
    name: &'static str,
    #[serde(serialize_with = "serialize_display")]
    value: PlayersValue,
}

struct PlayersValue {
    karts: Vec<Kart, MAX_ROOM_KART_COUNT>,
}

impl Display for PlayersValue {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        let max_name_count = self.karts.iter().map(|kart| kart.names.len()).max().unwrap_or(0);

        let point_diff_width = self
            .karts
            .iter()
            .filter_map(|kart| kart.point_diff.unsigned_abs().checked_ilog10())
            .max()
            .map_or(1, |width| width + 2) as usize;

        let points_width =
            self.karts.iter().filter_map(|kart| kart.points.checked_ilog10()).max().unwrap_or(0)
                as usize
                + 1;

        for (rank, kart) in self.karts.iter().enumerate() {
            if rank != 0 {
                writeln!(f)?;
            }
            let rank = match rank {
                0 => "\u{1f947}",
                1 => "\u{1f948}",
                2 => "\u{1f949}",
                3 => "4\u{fe0f}\u{20e3}",
                4 => "5\u{fe0f}\u{20e3}",
                5 => "6\u{fe0f}\u{20e3}",
                6 => "7\u{fe0f}\u{20e3}",
                _ => "8\u{fe0f}\u{20e3}",
            };
            write!(f, "`{rank} ")?;

            let team = match kart.team {
                0 => "\u{2764}\u{fe0f}",
                1 => "\u{1f499}",
                2 => "\u{1f49a}",
                3 => "\u{1f49b}",
                4 => "\u{1fa77}",
                5 => "\u{1f90e}",
                6 => "\u{1fa75}",
                _ => "\u{1f49c}",
            };
            write!(f, "{team} ")?;

            for i in 0..max_name_count {
                let name = kart.names.get(i);
                let name = name.and_then(|name| str::from_utf8(&name.0).ok());
                let name = name.unwrap_or("   ");
                write!(f, "{name} ")?;
            }

            let milliseconds = kart.time % 1000;
            let seconds = kart.time / 1000;
            let minutes = seconds / 60;
            let seconds = seconds % 60;
            write!(f, "{minutes:02}:{seconds:02}:{milliseconds:03} ")?;

            write!(f, "{:+point_diff_width$} {:points_width$}`", kart.point_diff, kart.points)?;
        }

        Ok(())
    }
}

struct Kart {
    team: u8,
    names: Vec<Name, MAX_KART_PLAYER_COUNT>,
    time: u32,
    point_diff: i32,
    points: u16,
}

#[derive(Serialize)]
struct Footer {
    #[serde(serialize_with = "serialize_display")]
    text: FooterText,
}

struct FooterText {
    frame_rate: FrameRate,
    engine_size: Option<RoomOptionEngineSize>,
    item_mode: Option<RoomOptionItemMode>,
    lap_count: Option<u8>,
    has_host: bool,
    race_count: u8,
    race_index: u8,
}

impl Display for FooterText {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        write!(f, "{}", self.frame_rate)?;

        if let Some(engine_size) = self.engine_size {
            write!(f, " · {engine_size}")?;
        }

        if let Some(item_mode) = self.item_mode {
            write!(f, " · {item_mode}")?;
        }

        if let Some(lap_count) = self.lap_count {
            let name = if lap_count == 1 { "Lap" } else { "Laps" };
            write!(f, " · {lap_count} {name}")?;
        }

        if self.has_host {
            write!(f, " · {}/{}", self.race_index + 1, self.race_count)?;
        } else {
            write!(f, " · #{}", self.race_index + 1)?;
        }

        Ok(())
    }
}

fn serialize_display<S: Serializer>(x: &impl Display, serializer: S) -> Result<S::Ok, S::Error> {
    serializer.collect_str(x)
}
