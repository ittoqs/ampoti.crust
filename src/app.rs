use iced::{
    executor,
    widget::{button, column, container, row, text, text_input},
    Application, Command, Element, Length, Theme,
};
use crate::ffi;

#[derive(Debug, Clone)]
pub enum Message {
    CompressClicked,
    ExtractClicked,
    SetPassword(String),
    SetFile(String),
    SetOutputPath(String),
    OperationCompleted(Result<(), String>),
}

pub struct AmpotiApp {
    password: String,
    target_file: String,
    output_path: String,
    status: String,
    is_processing: bool,
}

impl Application for AmpotiApp {
    type Executor = executor::Default;
    type Message = Message;
    type Theme = Theme;
    type Flags = ();

    fn new(_flags: ()) -> (Self, Command<Message>) {
        (
            Self {
                password: String::new(),
                target_file: String::new(),
                output_path: String::new(),
                status: String::from("Ready"),
                is_processing: false,
            },
            Command::none(),
        )
    }

    fn title(&self) -> String {
        String::from("Ampoti File Archiver")
    }

    fn update(&mut self, message: Message) -> Command<Message> {
        match message {
            Message::SetPassword(pass) => {
                self.password = pass;
                Command::none()
            }
            Message::SetFile(file) => {
                self.target_file = file;
                Command::none()
            }
            Message::SetOutputPath(path) => {
                self.output_path = path;
                Command::none()
            }
            Message::CompressClicked => {
                if self.target_file.is_empty() || self.output_path.is_empty() {
                    self.status = "Please specify target file and output path.".into();
                    return Command::none();
                }

                self.is_processing = true;
                self.status = "Compressing...".into();

                let file_path = self.target_file.clone();
                let out_path = self.output_path.clone();
                let pwd = if self.password.is_empty() { None } else { Some(self.password.clone()) };

                Command::perform(
                    async move {
                        tokio::task::spawn_blocking(move || {
                            let pwd_deref = pwd.as_deref();
                            let files_ref: Vec<&str> = vec![&file_path];
                            ffi::compress_files(&out_path, &files_ref, pwd_deref, "zip")
                        })
                        .await
                        .unwrap_or_else(|e| Err(format!("Task panic: {}", e)))
                    },
                    Message::OperationCompleted,
                )
            }
            Message::ExtractClicked => {
                if self.target_file.is_empty() || self.output_path.is_empty() {
                    self.status = "Please specify archive file and output directory.".into();
                    return Command::none();
                }

                self.is_processing = true;
                self.status = "Extracting...".into();

                let file_path = self.target_file.clone();
                let out_path = self.output_path.clone();
                let pwd = if self.password.is_empty() { None } else { Some(self.password.clone()) };

                Command::perform(
                    async move {
                        // Memastikan thread UI tidak diblokir sesuai aturan Golden Rules (Memori.md)
                        tokio::task::spawn_blocking(move || {
                            let pwd_deref = pwd.as_deref();
                            ffi::extract_archive(&file_path, &out_path, pwd_deref)
                        })
                        .await
                        .unwrap_or_else(|e| Err(format!("Task panic: {}", e)))
                    },
                    Message::OperationCompleted,
                )
            }
            Message::OperationCompleted(result) => {
                self.is_processing = false;
                match result {
                    Ok(_) => self.status = "Operation completed successfully.".into(),
                    Err(e) => self.status = format!("Error: {}", e),
                }
                Command::none()
            }
        }
    }

    fn view(&self) -> Element<Message> {
        let compress_btn = {
            let b = button(text("Compress")).padding(10);
            if self.is_processing { b } else { b.on_press(Message::CompressClicked) }
        };

        let extract_btn = {
            let b = button(text("Extract")).padding(10);
            if self.is_processing { b } else { b.on_press(Message::ExtractClicked) }
        };

        let content = column![
            text("Ampoti File Archiver").size(30),
            text_input("Target File / Archive", &self.target_file)
                .on_input(Message::SetFile)
                .padding(10),
            text_input("Output Path / Directory", &self.output_path)
                .on_input(Message::SetOutputPath)
                .padding(10),
            text_input("Password (Optional)", &self.password)
                .on_input(Message::SetPassword)
                .secure(true)
                .padding(10),
            row![
                compress_btn,
                extract_btn,
            ].spacing(20),
            text(&self.status).size(16),
        ]
        .spacing(20)
        .padding(20)
        .max_width(600);

        container(content)
            .width(Length::Fill)
            .height(Length::Fill)
            .center_x()
            .center_y()
            .into()
    }
}
