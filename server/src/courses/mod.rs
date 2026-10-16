pub use crate::courses::course::Course;

use std::collections::HashMap;
use std::fs;
use std::io::ErrorKind;
use std::ops::Deref;
use std::sync::Arc;

use anyhow::Result;
use arc_swap::ArcSwap;
use log::debug;

use crate::base64;
use crate::dir_entry;

mod course;

#[derive(Debug)]
pub struct Courses(HashMap<[u8; 32], Course>);

impl Courses {
    pub fn read() -> Result<Self> {
        let mut courses = Self::default();
        let entries = match fs::read_dir("run/courses") {
            Err(e) if e.kind() == ErrorKind::NotFound => {
                debug!("run/courses not found, using default courses");
                return Ok(courses);
            }
            r => r?,
        };
        for entry in entries {
            let entry = entry?;

            let name: String = dir_entry::extract_json_stem(&entry, "course")?;

            let name_courses: Vec<Course> = dir_entry::read_json(&entry, "course")?;
            for mut course in name_courses {
                course.name.clone_from(&name);
                if courses.0.insert(course.hash, course).is_some() {
                    anyhow::bail!("duplicate course {}", entry.file_name().display());
                }
            }
        }
        Ok(courses)
    }
}

impl Default for Courses {
    fn default() -> Self {
        let courses = [
            ("Baby Park", "Nintendo", "3kbQWOMqTqeF2tagQfnKR0tJZTMcyrA8vxQoydXdCKA="),
            ("Block City", "Nintendo", "KauvV96GcRWraa8bPVt1SyYLhI2h3zlz1ycsfVXx1l0="),
            ("Bowser's Castle", "Nintendo", "_OKXDagQxcVqXyJ9SIi61m6HwQOQOYczj_P0bSy_E1Y="),
            ("Cookie Land", "Nintendo", "-A2p18LIETFRJIcseNfU2LjG-SvMXYm7kF07benpH3c="),
            ("DK Mountain", "Nintendo", "a_hiKemNcddN794dOxdL3EICKLxu8MhBznMpkwZm1a4="),
            ("Daisy Cruiser", "Nintendo", "HahnEFNmYXnUDQrNpykdwCLR9bpc5vMfjnBDYq-LDp0="),
            ("Dino Dino Jungle", "Nintendo", "VnlSQvx3dYato9SZ8q7yftgenPoSHdvDxS0u9bA0ER0="),
            ("Dry Dry Desert", "Nintendo", "co6PQ0Q_3R4dLNiClhAFQpuRP38RLDzVV3hxnXufub4="),
            ("Luigi Circuit", "Nintendo", "V_DwDLkEx2s_F5WVAWojRKZsUTFDU-ZyJrJTHVwMvC8="),
            ("Luigi's Mansion", "Nintendo", "QSAMbAGh6G3f8aPwU_rXfV-Hvqc63-EOSDltzClY_7Y="),
            ("Mario Circuit", "Nintendo", "k856RRZt0GHLtbAGyqARwCemR543BTU2UWRLf4-hY9M="),
            ("Mushroom Bridge", "Nintendo", "SdK2RDLzg4qcRU6Nn9y_W1_iR5SXX7an8uOk_lrE47w="),
            ("Mushroom City", "Nintendo", "GHpf8spOkOjAG3ghOc7PUr5V3xVoSP8CclpIF6nQp-o="),
            ("Nintendo GameCube", "Nintendo", "6juaR4eCjBrzv8053FFgqmorZ2xilDAzP-7O9-fdfJA="),
            ("Peach Beach", "Nintendo", "RVWUEU7z2CLMZ10BAn80oEPQj9opk0p12V09aBk0Gds="),
            ("Pipe Plaza", "Nintendo", "Ail5hpCklHMaWd9wp9pWRKEwAc1FX0c1F0OSLKPiMy4="),
            ("Rainbow Road", "Nintendo", "SN9AU_Rrr0JPbfZ8qeAYZuePZ9ofcuhFYMtDycFzP-A="),
            ("Sherbet Land", "Nintendo", "3qllmhZxpJ0MsuYGZuAoZK4gB-jwPSN6105749-ljV4="),
            ("Tilt-a-Kart", "Nintendo", "rLROoiOasAzS2DkEB_6sfgcJvU8Yqn8IXRxYt42mmEs="),
            ("Waluigi Stadium", "Nintendo", "cUvbTECvAAVys6MU-DPOTSz3VfH6judYzLC3y8V10xU="),
            ("Wario Colosseum", "Nintendo", "FF-hW-L5uXwB43oCcodoKbVxCfQ247JgbDlI7gPXVRA="),
            ("Yoshi Circuit", "Nintendo", "tnal-riOpTfktS2Y4oDoJb0Y0vShdtuAoUpxip3Ozqs="),
        ];
        for [(n0, _, _), (n1, _, _)] in courses.array_windows() {
            assert!(n0 <= n1);
        }
        let courses = courses
            .into_iter()
            .map(|(name, author, hash)| {
                let hash = base64::decode(hash).unwrap();
                (hash, Course { name: name.to_owned(), author: author.to_owned(), hash })
            })
            .collect();
        Self(courses)
    }
}

impl Deref for Courses {
    type Target = HashMap<[u8; 32], Course>;

    fn deref(&self) -> &HashMap<[u8; 32], Course> {
        &self.0
    }
}

pub type SharedCourses = Arc<ArcSwap<Courses>>;
