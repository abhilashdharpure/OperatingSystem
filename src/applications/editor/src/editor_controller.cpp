#include "editor_controller.h"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

static constexpr size_t MAX_FILE = 1 << 20;

std::string EditorController::normalize(std::string p)
{
    size_t a = p.find_first_not_of(" \t"), b = p.find_last_not_of(" \t");
    p = a == std::string::npos ? "" : p.substr(a, b - a + 1);
    if (!p.empty() && p[0] != '/') p = "/" + p;
    return p;
}

bool EditorController::exists(const std::string& p)
{
    int fd = open(p.c_str(), O_RDONLY);
    if (fd < 0) return false;
    close(fd);
    return true;
}

EditorController::EditorController(slint::ComponentHandle<EditorWindow> ui, const std::string& path)
    : ui_(ui)
{
    auto& s = ui->global<EditorState>();
    s.on_save([this]      { save(); });
    s.on_new_file([this]  { new_file(); });
    s.on_open_path([this] {
        std::string p;
        with_state([&](auto& st) { p = std::string(st.get_path()); });
        load(normalize(p));
    });
    s.on_text_edited([this] { with_state([](auto& st) { st.set_dirty(true); }); });

    if (!path.empty()) load(normalize(path));
    else new_file();
}

void EditorController::new_file()
{
    loaded_path_.clear();
    overwrite_ok_.clear();
    with_state([](auto& s) {
        s.set_text("");
        s.set_path("/untitled.txt");
        s.set_dirty(false);
    });
    status("New file: type a name in the box above and press Save");
}

bool EditorController::load(const std::string& path)
{
    if (path.empty()) { status("Type a file name first"); return false; }
    with_state([&](auto& s) { s.set_path(slint::SharedString(path)); });   // show the name even if opening fails

    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) { status(std::string("Cannot open ") + path + ": " + strerror(errno)); return false; }

    std::string data;
    char buf[4096];
    ssize_t n;
    while ((n = read(fd, buf, sizeof buf)) > 0 && data.size() < MAX_FILE) data.append(buf, (size_t)n);
    close(fd);

    loaded_path_ = path;
    overwrite_ok_.clear();
    with_state([&](auto& s) {
        s.set_text(slint::SharedString(data));
        s.set_path(slint::SharedString(path));
        s.set_dirty(false);
    });
    status(std::to_string(data.size()) + " bytes");
    return true;
}

bool EditorController::save()
{
    std::string target, data;
    with_state([&](auto& s) {
        target = normalize(std::string(s.get_path()));
        data = std::string(s.get_text());
    });
    if (target.empty()) { status("Type a file name first"); return false; }

    // saving under a new name over an existing file needs a second click
    if (target != loaded_path_ && target != overwrite_ok_ && exists(target)) {
        overwrite_ok_ = target;
        status(target + " exists. Press Save again to overwrite it");
        return false;
    }

    int fd = open(target.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) { status(std::string("Cannot create ") + target + ": " + strerror(errno)); return false; }

    size_t off = 0;
    while (off < data.size()) {
        ssize_t w = write(fd, data.data() + off, data.size() - off);
        if (w <= 0) { status(std::string("Write failed: ") + strerror(errno)); close(fd); return false; }
        off += (size_t)w;
    }
    close(fd);

    loaded_path_ = target;
    overwrite_ok_.clear();
    with_state([&](auto& s) { s.set_path(slint::SharedString(target)); s.set_dirty(false); });
    status("Saved " + target + " (" + std::to_string(data.size()) + " bytes)");
    return true;
}