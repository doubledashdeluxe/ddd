pub use crate::courses::course::Course;

use std::collections::HashMap;
use std::fs;
use std::ops::Deref;
use std::sync::Arc;

use anyhow::Result;
use arc_swap::ArcSwap;

use crate::dir_entry;

mod course;

#[derive(Debug)]
pub struct Courses(HashMap<[u8; 32], Course>);

impl Courses {
    pub fn read() -> Result<Self> {
        let mut courses = HashMap::new();
        for entry in fs::read_dir("data/courses")? {
            let entry = entry?;

            let name: String = dir_entry::extract_json_stem(&entry, "course")?;

            let name_courses: Vec<Course> = dir_entry::read_json(&entry, "course")?;
            for mut course in name_courses {
                course.name.clone_from(&name);
                if courses.insert(course.hash, course).is_some() {
                    anyhow::bail!("duplicate course {}", entry.file_name().display());
                }
            }
        }
        Ok(Self(courses))
    }
}

impl Deref for Courses {
    type Target = HashMap<[u8; 32], Course>;

    fn deref(&self) -> &HashMap<[u8; 32], Course> {
        &self.0
    }
}

pub type SharedCourses = Arc<ArcSwap<Courses>>;
