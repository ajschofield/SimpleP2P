use ratatui::{
    backend::CrosstermBackend,
    layout::{Constraint, Direction, Layout},
    widgets::{Block, Borders},
    Terminal,
};

use std::io::{self, stdout};

fn main() -> Result<(), io::Error> {
    let stdout = stdout();
    let backend = CrosstermBackend::new(stdout);
    let mut terminal = Terminal::new(backend)?;

    terminal.draw(|f| {
        let size = f.area();
        let chunks = Layout::default()
            .direction(Direction::Vertical)
            .constraints([Constraint::Percentage(50), Constraint::Percentage(50)])
            .split(size);

        let test1 = Block::default().title("Test 1").borders(Borders::ALL);
        let test2 = Block::default().title("Test 2").borders(Borders::ALL);

        f.render_widget(test1, chunks[0]);
        f.render_widget(test2, chunks[1]);
    })?;

    Ok(())
}
