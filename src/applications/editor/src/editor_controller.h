#pragma once
#include "app.h"
#include <string>

class EditorController {
public:
    EditorController(slint::ComponentHandle<EditorWindow> ui, const std::string& path);
private:
    bool load();
    bool save();
    template <class F> void with_state(F&& f) {
        if (auto ui = ui_.lock()) f((*ui)->global<EditorState>());
    }
    slint::ComponentWeakHandle<EditorWindow> ui_;
    std::string path_;
};