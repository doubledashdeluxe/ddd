use std::collections::HashMap;
use std::fmt::{Error, Result, Write};

use crate::formats::online::FrameRate;
use crate::kart::Kart;
use crate::room::Room;
use crate::rooms::Rooms;
use crate::storage::PlayerId;
use crate::website::backend::Backend;
use crate::website::duration::Duration;
use crate::website::html::Children;
use crate::website::link;
use crate::website::pack_link;
use crate::website::path_printer::PathPrinter;
use crate::website::spectator_count;

pub fn write(
    player_numbers: &HashMap<PlayerId, u64>,
    room_numbers: &HashMap<u128, u64>,
    rooms: &Rooms,
    backend: &mut impl Backend,
) {
    let write = |body: &mut Children<_>, path_printer: &PathPrinter| {
        for frame_rate in FrameRate::VARIANTS {
            rooms
                .iter(frame_rate, |room| {
                    write_room(player_numbers, room_numbers, frame_rate, room, body, path_printer)
                        .is_ok()
                })
                .ok_or(Error)?;
        }
        Ok(())
    };

    backend.write_page("Rooms", write, None, "rooms", 5);
}

fn write_room(
    player_numbers: &HashMap<PlayerId, u64>,
    room_numbers: &HashMap<u128, u64>,
    frame_rate: FrameRate,
    room: &Room,
    body: &mut Children<impl Write>,
    path_printer: &PathPrinter,
) -> Result {
    write_h2(room_numbers, room, body, path_printer)?;
    write_ul(frame_rate, room, body, path_printer)?;
    write_table(player_numbers, room, body, path_printer)?;
    Ok(())
}

fn write_h2(
    room_numbers: &HashMap<u128, u64>,
    room: &Room,
    body: &mut Children<impl Write>,
    path_printer: &PathPrinter,
) -> Result {
    let h2 = body.element("h2")?;
    let name = if room.has_host() { "Personal" } else { "Worldwide" };
    if let Some(number) = room_numbers.get(&room.id()) {
        let mut h2 = h2.children()?;
        let mut a = h2.element("a")?;
        let href = format_args!("rooms/{number}");
        a.attribute("href")?.value(path_printer.print_page(href))?;
        a.content(format_args!("{name} Room #{number}"))?;
        h2.finish()
    } else {
        h2.content(format_args!("{name} Room #?"))
    }
}

fn write_ul(
    frame_rate: FrameRate,
    room: &Room,
    body: &mut Children<impl Write>,
    path_printer: &PathPrinter,
) -> Result {
    let mut ul = body.element("ul")?.children()?;
    ul.element("li")?.content(format_args!("Mode: {}", room.mode_index()))?;
    pack_link::write(room.pack().hash(), &mut ul, path_printer)?;
    ul.element("li")?.content(frame_rate)?;
    spectator_count::write(room.spectator_count() as u64, &mut ul)?;
    ul.element("li")?.content(format_args!("Uptime: {}", Duration(room.start().elapsed())))?;
    ul.finish()
}

fn write_table(
    player_numbers: &HashMap<PlayerId, u64>,
    room: &Room,
    body: &mut Children<impl Write>,
    path_printer: &PathPrinter,
) -> Result {
    let mut table = body.element("table")?;
    table.attribute("class")?.value("players room-players")?;
    let mut table = table.children()?;

    for kart in room.karts() {
        write_tr(player_numbers, room, kart, &mut table, path_printer)?;
    }

    table.finish()
}

fn write_tr(
    player_numbers: &HashMap<PlayerId, u64>,
    room: &Room,
    kart: &Kart,
    table: &mut Children<impl Write>,
    path_printer: &PathPrinter,
) -> Result {
    let mut tr = table.element("tr")?.children()?;

    for player in kart.players() {
        let mut td = tr.element("td")?;
        if kart.players().len() == 1 {
            td.attribute("colspan")?.value(2)?;
        }
        let mut td = td.children()?;
        let id = PlayerId { client_pk: *kart.client_pk(), index: player.index() };
        if let Some(number) = player_numbers.get(&id) {
            let mut a = td.element("a")?;
            let href = format_args!("players/{number}");
            a.attribute("href")?.value(path_printer.print_page(href))?;
            a.content(player.name())?;
        } else {
            td.content(player.name())?;
        }
        td.finish()?;
    }

    tr.element("td")?.content(format_args!("{} pts", kart.points))?;

    if let Some(host_pk) = room.host_pk() {
        let role = if kart.client_pk() == host_pk { "Host" } else { "" };
        tr.element("td")?.content(role)?;
    }

    link::write("td", "rankings/regions", kart.region(), &mut tr, path_printer)?;
    link::write("td", "rankings/platforms", kart.platform(), &mut tr, path_printer)?;

    tr.finish()
}
