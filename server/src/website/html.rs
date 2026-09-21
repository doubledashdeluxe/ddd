use std::fmt::{self, Display, Error, Write};
use std::result;

#[must_use]
pub struct Element<'a, W: Write> {
    writer: &'a mut W,
    minify: bool,
    indent: usize,
    tag: &'static str,
}

impl<'a, W: Write> Element<'a, W> {
    pub fn new(writer: &'a mut W, minify: bool, indent: usize, tag: &'static str) -> Result<Self> {
        write!(writer, "<{}", escaped(tag))?;
        Ok(Self { writer, minify, indent, tag })
    }

    pub fn attribute(&mut self, name: &str) -> Result<Attribute<'_, W>> {
        Attribute::new(self.writer, name)
    }

    pub fn empty(self) -> Result<()> {
        write!(self.writer, ">")
    }

    pub fn content(self, content: impl Display) -> Result<()> {
        write!(self.writer, ">{}</{}>", escaped(content), escaped(self.tag))
    }

    pub fn children(self) -> Result<Children<'a, W>> {
        write!(self.writer, ">")?;
        Ok(Children::new(self))
    }
}

impl<W: Write> Drop for Element<'_, W> {
    fn drop(&mut self) {}
}

#[must_use]
pub struct Attribute<'a, W: Write> {
    writer: &'a mut W,
}

impl<'a, W: Write> Attribute<'a, W> {
    fn new(writer: &'a mut W, name: &str) -> Result<Self> {
        write!(writer, " {}", escaped(name))?;
        Ok(Self { writer })
    }

    #[expect(clippy::unused_self)]
    pub fn empty(self) {}

    pub fn value(self, value: impl Display) -> Result<()> {
        write!(self.writer, "=\"{}\"", escaped(value))
    }
}

impl<W: Write> Drop for Attribute<'_, W> {
    fn drop(&mut self) {}
}

#[must_use]
pub struct Children<'a, W: Write> {
    parent: Element<'a, W>,
}

impl<'a, W: Write> Children<'a, W> {
    const fn new(parent: Element<'a, W>) -> Self {
        Self { parent }
    }

    pub fn element(&mut self, tag: &'static str) -> Result<Element<'_, W>> {
        self.line(self.parent.indent + 1)?;
        Element::new(self.parent.writer, self.parent.minify, self.parent.indent + 1, tag)
    }

    pub fn content(&mut self, content: impl Display) -> Result<()> {
        self.line(self.parent.indent + 1)?;
        write!(self.parent.writer, "{}", escaped(content))
    }

    pub fn finish(mut self) -> Result<()> {
        self.line(self.parent.indent)?;
        write!(self.parent.writer, "</{}>", escaped(self.parent.tag))
    }

    fn line(&mut self, indent: usize) -> Result<()> {
        writeln!(self.parent.writer)?;
        if !self.parent.minify {
            for _ in 0..indent {
                write!(self.parent.writer, "    ")?;
            }
        }
        Ok(())
    }
}

impl<W: Write> Drop for Children<'_, W> {
    fn drop(&mut self) {}
}

pub type Result<T> = result::Result<T, Error>;

fn escaped(x: impl Display) -> impl Display {
    fmt::from_fn(move |f| write!(EscapedWriter(f), "{x}"))
}

struct EscapedWriter<W>(W);

impl<W: Write> Write for EscapedWriter<W> {
    fn write_str(&mut self, s: &str) -> Result<()> {
        for c in s.chars() {
            match c {
                '"' => self.0.write_str("&quot;")?,
                '&' => self.0.write_str("&amp;")?,
                '\'' => self.0.write_str("&#39;")?,
                '<' => self.0.write_str("&lt;")?,
                '>' => self.0.write_str("&gt;")?,
                c => self.0.write_char(c)?,
            }
        }
        Ok(())
    }
}
