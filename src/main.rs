mod app;
mod ffi;

use app::AmpotiApp;
use iced::{Application, Settings};

pub fn main() -> iced::Result {
    AmpotiApp::run(Settings::default())
}
