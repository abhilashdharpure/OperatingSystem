#include "editor_controller.h"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

static constexpr size_t MAX_FILE = 1 << 20;            // 1 MB is plenty for a first editor

EditorController::EditorController(slint::ComponentHandle<EditorWindow> ui, const std::string& path)
    : ui_(ui), path_(path)
{
    auto& s = ui->global<EditorState>();
    s.on_save([this] { save(); });
    s.on_text_edited([this] { with_state([](auto& st) { st.set_dirty(true); }); });

    s.set_path(slint::SharedString(path_));
    if (!path_.empty()) load();
    else s.set_status("New file: open a file from the file manager to edit it");
}

bool EditorController::load()
{
    int fd = open(path_.c_str(), O_RDONLY);
    if (fd < 0) {
        std::string m = std::string("Cannot open: ") + strerror(errno);
        with_state([&](auto& s) { s.set_status(slint::SharedString(m)); });
        return false;
    }
    std::string data;
    char buf[4096];
    ssize_t n;
    while ((n = read(fd, buf, sizeof buf)) > 0 && data.size() < MAX_FILE) data.append(buf, n);
    close(fd);
    with_state([&](auto& s) {
        s.set_text(slint::SharedString(data));        // invalid UTF-8 becomes replacement characters
        s.set_dirty(false);
        s.set_status(slint::SharedString(std::to_string(data.size()) + " bytes"));
    });
    return true;
}

bool EditorController::save()
{
    if (path_.empty()) { with_state([](auto& s) { s.set_status("No file name"); }); return false; }
    std::string data;
    with_state([&](auto& s) { data = std::string(s.get_text()); });

    int fd = open(path_.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    ssize_t w = fd >= 0 ? write(fd, data.data(), data.size()) : -1;
    if (fd >= 0) close(fd);
    if (w != (ssize_t)data.size()) {
        std::string m = std::string("Save failed: ") + strerror(errno);
        with_state([&](auto& s) { s.set_status(slint::SharedString(m)); });
        return false;
    }
    with_state([&](auto& s) { s.set_dirty(false); s.set_status("Saved"); });
    return true;
}