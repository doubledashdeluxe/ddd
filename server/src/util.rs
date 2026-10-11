use std::thread;
use std::time::Duration;

pub fn retry<T, E>(
    attempts: usize,
    duration: Duration,
    mut f: impl FnMut() -> Result<T, E>,
) -> Result<T, E> {
    assert!(attempts != 0);

    let mut attempt = 1;
    loop {
        match f() {
            Err(_) if attempt < attempts => (),
            r => return r,
        }

        thread::sleep(duration);
        attempt += 1;
    }
}

#[cfg(test)]
mod tests {
    use std::time::Duration;

    use crate::util;

    #[test]
    fn retry() {
        let mut sum = 0;
        let r: Result<(), ()> = util::retry(3, Duration::ZERO, || {
            sum += 1;
            Err(())
        });
        r.unwrap_err();
        assert_eq!(sum, 3);
    }
}
