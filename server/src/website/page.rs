use std::fmt::{Display, Result, Write};

use jiff::Timestamp;

use crate::website::css;
use crate::website::html::{Children, Element};
use crate::website::path_printer::PathPrinter;
use crate::website::timestamp;

pub fn write(
    name: impl Display,
    write: impl FnOnce(&mut Children<String>, &PathPrinter) -> Result,
    page: &mut String,
    minify: bool,
    path_printer: &PathPrinter,
) -> Result {
    "<!doctype html>\n".clone_into(page);
    write_html(name, write, page, minify, path_printer)
}

fn write_html<W: Write>(
    name: impl Display,
    write: impl FnOnce(&mut Children<W>, &PathPrinter) -> Result,
    page: &mut W,
    minify: bool,
    path_printer: &PathPrinter,
) -> Result {
    let mut html = Element::new(page, minify, 0, "html")?;
    html.attribute("lang")?.value("en-US")?;
    let mut html = html.children()?;
    write_head(&name, &mut html, path_printer)?;
    write_body(&name, write, &mut html, path_printer)?;
    html.finish()
}

fn write_head(
    name: impl Display,
    html: &mut Children<impl Write>,
    path_printer: &PathPrinter,
) -> Result {
    let mut head = html.element("head")?.children()?;
    let mut meta = head.element("meta")?;
    meta.attribute("charset")?.value("utf-8")?;
    meta.empty()?;
    write_meta("viewport", "width=device-width, initial-scale=1", &mut head)?;
    write_meta("color-scheme", "light dark", &mut head)?;
    write_title(format_args!("{name} · Double Dash Deluxe [dev]"), &mut head)?;
    write_link(
        "stylesheet",
        path_printer.print_file(format_args!("css/{}", css::FILE_NAME)),
        &mut head,
    )?;
    write_link("icon", path_printer.print_file("favicon.ico"), &mut head)?;
    head.finish()
}

fn write_meta(name: &str, content: &str, head: &mut Children<impl Write>) -> Result {
    let mut meta = head.element("meta")?;
    meta.attribute("name")?.value(name)?;
    meta.attribute("content")?.value(content)?;
    meta.empty()
}

fn write_title(title: impl Display, head: &mut Children<impl Write>) -> Result {
    head.element("title")?.content(title)
}

fn write_link(rel: &str, href: impl Display, head: &mut Children<impl Write>) -> Result {
    let mut meta = head.element("link")?;
    meta.attribute("rel")?.value(rel)?;
    meta.attribute("href")?.value(href)?;
    meta.empty()
}

fn write_body<W: Write>(
    name: impl Display,
    write: impl FnOnce(&mut Children<W>, &PathPrinter) -> Result,
    html: &mut Children<W>,
    path_printer: &PathPrinter,
) -> Result {
    let mut body = html.element("body")?.children()?;
    write_nav(&mut body, path_printer)?;
    body.element("h1")?.content(name)?;
    write(&mut body, path_printer)?;
    write_footer(&mut body)?;
    body.finish()
}

fn write_nav(body: &mut Children<impl Write>, path_printer: &PathPrinter) -> Result {
    let mut nav = body.element("nav")?.children()?;
    write_a("https://mkdd.org/wiki/Double_Dash_Deluxe", "Double Dash Deluxe", &mut nav)?;
    write_a(path_printer.print_page("rooms"), "Rooms", &mut nav)?;
    write_a(path_printer.print_page("rankings"), "Rankings", &mut nav)?;
    write_a(path_printer.print_page("stats"), "Stats", &mut nav)?;
    write_a("https://github.com/doubledashdeluxe/ddd", "GitHub", &mut nav)?;
    write_a("https://discord.gg/3wQxMKG3mp", "Discord", &mut nav)?;
    nav.finish()
}

fn write_a(href: impl Display, content: &str, nav: &mut Children<impl Write>) -> Result {
    let mut a = nav.element("a")?;
    a.attribute("href")?.value(href)?;
    a.content(content)
}

fn write_footer(body: &mut Children<impl Write>) -> Result {
    let mut footer = body.element("footer")?.children()?;
    footer.element("p")?.content(timestamp::fmt("Generated", Timestamp::now()))?;
    footer.finish()
}
