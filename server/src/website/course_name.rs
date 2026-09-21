use std::fmt::{self, Display};

use crate::base64;
use crate::courses::Courses;

pub fn fmt(courses: &Courses, course_hash: &[u8; 32]) -> impl Display {
    fmt::from_fn(|f| {
        if let Some(course) = courses.get(course_hash) {
            write!(f, "{course}")
        } else {
            let course = base64::display(course_hash);
            write!(f, "{course:.12}...")
        }
    })
}
