#pragma once
#include "app.h"
#include "shell.h"
#include <memory>
#include <string>
#include <vector>

class TermController {
public:
    explicit TermController(slint::ComponentHandle<TerminalWindow> ui);

private:
    void push(const std::string& text, int kind);
    void refresh_prompt();

    slint::ComponentWeakHandle<TerminalWindow>   ui_;
    std::shared_ptr<slint::VectorModel<TermLine>> model_;
    Shell                    shell_;
    std::vector<std::string> hist_;
    size_t                   hpos_ = 0;
    int                      tick_ = 0;
};