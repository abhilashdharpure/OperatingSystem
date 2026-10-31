#include "term_controller.h"
#include "luma_app.h"

TermController::TermController(slint::ComponentHandle<TerminalWindow> ui)
    : ui_(ui),
      model_(std::make_shared<slint::VectorModel<TermLine>>()),
      shell_(ShellHooks{
          [this](const std::string& t, int k) { push(t, k); },
          [this] { while (model_->row_count() > 0) model_->erase(0); },
          [] { if (g_win) g_win->close_window(); } })
{
    auto& s = ui->global<TermState>();
    s.set_lines(model_);

    s.on_submit([this](slint::SharedString line) {
        std::string cmd(line);
        std::printf("[term] submit '%s'\n", cmd.c_str());          // shows whether Enter reaches this code

        std::string prompt;
        if (auto ui = ui_.lock()) {
            auto& st = (*ui)->global<TermState>();
            prompt = std::string(st.get_prompt());
            st.set_input("");
        }

        bool blank = cmd.find_first_not_of(" \t") == std::string::npos;
        push(blank ? prompt : prompt + " " + cmd, 1);              // blank Enter: just a new prompt line
        if (blank) return;

        if (hist_.empty() || hist_.back() != cmd) hist_.push_back(cmd);
        hpos_ = hist_.size();
        shell_.run(cmd);
        refresh_prompt();
    });
    
    s.on_history([this](int dir) {
        if (hist_.empty()) return;
        if (dir < 0 && hpos_ > 0) hpos_--;
        else if (dir > 0 && hpos_ < hist_.size()) hpos_++;
        std::string v = hpos_ < hist_.size() ? hist_[hpos_] : "";
        if (auto ui = ui_.lock()) (*ui)->global<TermState>().set_input(slint::SharedString(v));
    });

    s.on_complete([this] {
        if (auto ui = ui_.lock()) {
            auto& st = (*ui)->global<TermState>();
            st.set_input(slint::SharedString(shell_.complete(std::string(st.get_input()))));
        }
    });

    push("Luma terminal. Type 'help' for commands.", 0);
    refresh_prompt();
}

void TermController::push(const std::string& text, int kind)
{
    model_->push_back(TermLine{ slint::SharedString(text), kind });
    while (model_->row_count() > 1000) model_->erase(0);          // scrollback limit
    if (auto ui = ui_.lock()) (*ui)->global<TermState>().set_scroll_tick(++tick_);
}

void TermController::refresh_prompt()
{
    if (auto ui = ui_.lock())
        (*ui)->global<TermState>().set_prompt(slint::SharedString(shell_.cwd() + " $"));
}