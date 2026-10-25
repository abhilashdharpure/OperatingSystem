#include "file_manager.h"
#include <algorithm>
#include <cctype>
#include <cstdio>

// ---------- path helpers ----------
std::string FileManager::normalize(const std::string& in)
{
    std::vector<std::string> parts;
    std::string cur;
    for (size_t i = 0; i <= in.size(); i++) {
        if (i == in.size() || in[i] == '/') {
            if (cur == "..") { if (!parts.empty()) parts.pop_back(); }
            else if (!cur.empty() && cur != ".") parts.push_back(cur);
            cur.clear();
        } else cur += in[i];
    }
    std::string out;
    for (auto& p : parts) out += "/" + p;
    return out.empty() ? "/" : out;
}

std::string FileManager::join(const std::string& dir, const std::string& name)
{
    return dir == "/" ? "/" + name : dir + "/" + name;
}

std::string FileManager::parent_of(const std::string& path)
{
    if (path == "/") return "/";
    size_t pos = path.find_last_of('/');
    return pos == 0 ? "/" : path.substr(0, pos);
}

std::string FileManager::size_text(uint64_t b)
{
    char buf[32];
    if (b < 1024)               std::snprintf(buf, sizeof buf, "%llu B", (unsigned long long)b);
    else if (b < 1024 * 1024)   std::snprintf(buf, sizeof buf, "%.1f KB", b / 1024.0);
    else if (b < 1024ull << 20) std::snprintf(buf, sizeof buf, "%.1f MB", b / (1024.0 * 1024));
    else                        std::snprintf(buf, sizeof buf, "%.1f GB", b / (1024.0 * 1024 * 1024));
    return buf;
}

std::string FileManager::kind_of(const FsEntry& e)
{
    if (e.is_dir) return "Folder";
    size_t dot = e.name.find_last_of('.');
    if (dot == std::string::npos || dot + 1 >= e.name.size()) return "File";
    std::string ext = e.name.substr(dot + 1);
    for (auto& c : ext) c = (char)std::toupper((unsigned char)c);
    return ext + " file";
}

// ---------- controller ----------
FileManager::FileManager(slint::ComponentHandle<AppWindow> ui, IFileBackend& backend)
    : ui_(ui), be_(backend), model_(std::make_shared<slint::VectorModel<FileEntry>>())
{
    auto& state = *&ui->global<FileManagerState>();   // setters/callbacks live on the global
    state.set_entries(model_);

    auto places = std::make_shared<slint::VectorModel<Place>>();
    places->push_back(Place{ "Root", "/" });
    places->push_back(Place{ "bin",  "/bin" });
    places->push_back(Place{ "tmp",  "/tmp" });
    places->push_back(Place{ "dev",  "/dev" });
    places->push_back(Place{ "boot",  "/boot" });
    state.set_places(places);

    state.on_navigate([this](slint::SharedString p) { navigate(std::string(p)); });

    state.on_select_entry([this](int i) {
        selected_ = i;
        with_state([&](auto& s) { s.set_selected_index(i); });
    });

    state.on_open_entry([this](int i) {
        if (i < 0 || i >= (int)rows_.size()) return;
        if (rows_[i].is_dir) navigate(join(cwd_, rows_[i].name));
        else set_status("Open file: " + join(cwd_, rows_[i].name));   // TODO: launch via your spawn syscall
    });

    state.on_go_up([this] { navigate(parent_of(cwd_)); });

    state.on_go_back([this] {
        if (back_.empty()) return;
        fwd_.push_back(cwd_);
        std::string target = back_.back(); back_.pop_back();
        navigate(target, false);
    });

    state.on_go_forward([this] {
        if (fwd_.empty()) return;
        back_.push_back(cwd_);
        std::string target = fwd_.back(); fwd_.pop_back();
        navigate(target, false);
    });

    state.on_refresh([this] { reload(); });

    state.on_create_folder([this](slint::SharedString name) {
        std::string n(name), err;
        if (n.empty() || n.find('/') != std::string::npos) { set_status("Invalid folder name"); return; }
        if (!be_.make_dir(join(cwd_, n), err)) { set_status("Create failed: " + err); return; }
        reload(n);
        set_status("Created " + n);
    });

    state.on_rename_selected([this](slint::SharedString name) {
        if (selected_ < 0 || selected_ >= (int)rows_.size()) return;
        std::string n(name), err;
        if (n.empty() || n.find('/') != std::string::npos) { set_status("Invalid name"); return; }
        std::string from = join(cwd_, rows_[selected_].name);
        if (!be_.rename_path(from, join(cwd_, n), err)) { set_status("Rename failed: " + err); return; }
        reload(n);
        set_status("Renamed to " + n);
    });

    state.on_delete_selected([this] {
        if (selected_ < 0 || selected_ >= (int)rows_.size()) return;
        std::string err, victim = rows_[selected_].name;
        if (!be_.remove_path(join(cwd_, victim), err)) { set_status("Delete failed: " + err); return; }
        reload();
        set_status("Deleted " + victim);
    });
}

void FileManager::navigate(const std::string& path, bool record_history)
{
    std::string target = normalize(path);
    std::vector<FsEntry> probe;
    std::string err;
    if (!be_.list_dir(target, probe, err)) {          // validate before leaving the current folder
        set_status(err.empty() ? "Cannot open " + target : err);
        with_state([&](auto& s) { s.set_current_path(slint::SharedString(cwd_)); });
        return;
    }
    if (record_history && target != cwd_) { back_.push_back(cwd_); fwd_.clear(); }
    cwd_ = target;
    reload();
}

void FileManager::reload(const std::string& keep_selected_name)
{
    std::vector<FsEntry> list;
    std::string err;
    if (!be_.list_dir(cwd_, list, err)) {
        set_status("Read failed: " + err);
        return;
    }

    std::sort(list.begin(), list.end(), [](const FsEntry& a, const FsEntry& b) {
        if (a.is_dir != b.is_dir) return a.is_dir;      // folders first
        return a.name < b.name;
    });
    rows_ = std::move(list);

    std::vector<FileEntry> view;
    view.reserve(rows_.size());
    selected_ = -1;
    for (size_t i = 0; i < rows_.size(); i++) {
        const auto& e = rows_[i];
        view.push_back(FileEntry{
            slint::SharedString(e.name),
            e.is_dir,
            slint::SharedString(e.is_dir ? "" : size_text(e.size)),
            slint::SharedString(kind_of(e)),
        });
        if (!keep_selected_name.empty() && e.name == keep_selected_name) selected_ = (int)i;
    }
    model_->set_vector(view);

    with_state([&](auto& s) {
        s.set_current_path(slint::SharedString(cwd_));
        s.set_selected_index(selected_);
    });
    update_nav_buttons();
    set_status(std::to_string(rows_.size()) + " items");
}

void FileManager::update_nav_buttons()
{
    with_state([&](auto& s) {
        s.set_can_go_back(!back_.empty());
        s.set_can_go_forward(!fwd_.empty());
    });
}

void FileManager::set_status(const std::string& text)
{
    with_state([&](auto& s) { s.set_status(slint::SharedString(text)); });
}