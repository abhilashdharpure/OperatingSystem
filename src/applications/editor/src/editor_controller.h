#pragma once
#include "app.h"
#include <string>

class EditorController {
public:
    EditorController(slint::ComponentHandle<EditorWindow> ui, const std::string& path);
private:
    bool load(const std::string& path);
    bool save();
    void new_file();
    static std::string normalize(std::string p);
    static bool exists(const std::string& p);
    template <class F> void with_state(F&& f) {
        if (auto ui = ui_.lock()) f((*ui)->global<EditorState>());
    }
    void status(const std::string& m) {
        with_state([&](auto& s) { s.set_status(slint::SharedString(m)); });
    }
    slint::ComponentWeakHandle<EditorWindow> ui_;
    std::string loaded_path_;           // the file the buffer came from
    std::string overwrite_ok_;          // path the user already confirmed overwriting
};