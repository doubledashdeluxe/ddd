use anyhow::Result;
use sd_notify::NotifyState;

pub fn notify_ready() -> Result<()> {
    Ok(sd_notify::notify(&[NotifyState::Ready])?)
}

pub fn notify_reloading() -> Result<()> {
    Ok(sd_notify::notify(&[NotifyState::Reloading, NotifyState::monotonic_usec_now()?])?)
}
