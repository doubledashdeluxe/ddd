use std::iter;
use std::time::SystemTime;
use std::vec::IntoIter;

use crate::formats::online::*;
use crate::formats::version;
use crate::storage::batch::Batch;

pub fn write(batch: Batch, buf: &mut Vec<u8>) {
    let Batch {
        clients: mut race_clients,
        inputs,
        room_state,
        team_state,
        poll_state,
        race_states,
        race,
        ..
    } = batch;

    let mut message = [0u8; 4 * 1024];

    let replay = Replay {
        magic: REPLAY_MAGIC,
        replay_version: REPLAY_VERSION,
        reserved: 0,
        version: version::VERSION.as_bytes().try_into().unwrap(),
    };
    write_message(buf, &mut message, &replay, Replay::write);

    let mut clients = heapless::Vec::new();
    let mut client_pk = None;
    for kart in &race.karts {
        if Some(kart.client_pk) != client_pk {
            clients
                .push(ReplayClient {
                    pk: kart.client_pk,
                    region: kart.region.into(),
                    platform: kart.platform.clone().into(),
                    players: heapless::Vec::new(),
                    teams: heapless::Vec::new(),
                })
                .unwrap();
            client_pk = Some(kart.client_pk);
        }
        let client = clients.last_mut().unwrap();
        for player in &kart.players {
            client
                .players
                .push(ClientPlayer { profile: player.profile, name: player.name.0 })
                .unwrap();
        }
        client.teams.push(kart.team).unwrap();
    }
    let time = SystemTime::from(race.end)
        .duration_since(SystemTime::UNIX_EPOCH)
        .unwrap_or_default()
        .as_secs();
    let replay_race = ReplayRace {
        frame_rate: race.frame_rate,
        mode_index: race.mode,
        pack_course_count: race.pack_course_count as u8,
        pack_hash: race.pack_hash,
        course_index: race.karts[race.selected_kart_index as usize].course_index,
        clients,
        time,
        room_type: if race.host_pk.is_none() { RoomType::Worldwide } else { RoomType::Personal },
        format: race.format,
        room_code: room_state.room_code,
    };
    write_message(buf, &mut message, &replay_race, ReplayRace::write);

    let server_room_state = ServerRoomState::Main(room_state);
    let room = ServerStateRoom { server_room_state };
    let server_state = ServerState::Room(room);
    write_message(buf, &mut message, &server_state, ServerState::write);

    if let Some(team_state) = team_state {
        let server_team_state = ServerTeamState::Main(team_state);
        let team = ServerStateTeam { server_team_state };
        let server_state = ServerState::Team(team);
        write_message(buf, &mut message, &server_state, ServerState::write);
    }

    let server_poll_state = ServerPollState::Ready(poll_state);
    let poll = ServerStatePoll { server_poll_state };
    let server_state = ServerState::Poll(poll);
    write_message(buf, &mut message, &server_state, ServerState::write);

    let mut clients: heapless::Vec<_, MAX_REPLAY_CLIENT_COUNT> = heapless::Vec::new();
    let mut client_pk = None;
    for (inputs, kart) in iter::zip(inputs, race.karts) {
        if Some(kart.client_pk) != client_pk {
            let client = race_clients.remove(&kart.client_pk).unwrap_or_default();
            let client = Client { frames: client.frames, inputs: heapless::Vec::new() };
            clients.push(client).unwrap();
            client_pk = Some(kart.client_pk);
        }
        let client = clients.last_mut().unwrap();
        for inputs in inputs {
            client.inputs.push(inputs.into_iter()).unwrap();
        }
    }
    let mut client_states: heapless::Vec<_, MAX_REPLAY_CLIENT_COUNT> = clients
        .into_iter()
        .map(|mut client| {
            client
                .frames
                .into_iter()
                .map_while(move |frames| {
                    let inputs: Option<_> = client.inputs.iter_mut().map(Iterator::next).collect();
                    let inputs = inputs?;
                    Some(ReplayClientState {
                        replay_server_frame: frames.server_frame,
                        replay_client_frame: frames.client_frame,
                        replay_inputs: inputs,
                    })
                })
                .peekable()
        })
        .collect();
    for race_state in race_states {
        let race_state_frame = race_state.frame;
        let server_race_state = ServerRaceState::Main(race_state);
        let race = ServerStateRace { server_race_state };
        let server_state = ServerState::Race(race);
        write_message(buf, &mut message, &server_state, ServerState::write);

        let client_states = client_states
            .iter_mut()
            .map(|client_states| {
                iter::from_fn(|| {
                    client_states.next_if(|client_state| {
                        let server_frame = client_state.replay_server_frame;
                        let client_frame = client_state.replay_client_frame;
                        let frame = server_frame.min(client_frame);
                        frame <= race_state_frame
                    })
                })
                .take(MAX_KART_INPUT_COUNT)
                .collect()
            })
            .collect();
        let replay_state = ReplayState { client_states };
        write_message(buf, &mut message, &replay_state, ReplayState::write);
    }
}

fn write_message<T>(
    buf: &mut Vec<u8>,
    message: &mut [u8],
    x: &T,
    write: for<'a> fn(&T, &'a mut [u8]) -> Result<&'a mut [u8], ()>,
) {
    let message_len = message.len() - write(x, message).unwrap().len();
    buf.extend(&message[..message_len]);
}

#[derive(Debug)]
struct Client {
    frames: Vec<ClientRaceFrames>,
    inputs: heapless::Vec<IntoIter<u16>, MAX_CLIENT_PLAYER_COUNT>,
}
