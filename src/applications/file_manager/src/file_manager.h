#pragma once
#include "app.h"
#include "file_backend.h"
#include <memory>
#include <string>
#include <vector>

class FileManager {
public:
    FileManager(slint::ComponentHandle<AppWindow> ui, IFileBackend& backend);

    void navigate(const std::string& path, bool record_history = true);

private:
    void reload(const std::string& keep_selected_name = "");
    void set_status(const std::string& text);
    void update_nav_buttons();

    template <class F>
    void with_state(F&& f) {
        if (auto ui = ui_.lock()) f((*ui)->global<FileManagerState>());
    }

    void open_in_editor();
    void copy_selected(bool cut);
    void paste_here();
    static std::string base_name(const std::string& path);

    std::string clip_path_;
    bool        clip_cut_ = false;

    static std::string normalize(const std::string& path);
    static std::string join(const std::string& dir, const std::string& name);
    static std::string parent_of(const std::string& path);
    static std::string size_text(uint64_t bytes);
    static std::string kind_of(const FsEntry& e);

    slint::ComponentWeakHandle<AppWindow>        ui_;
    IFileBackend&                                be_;
    std::shared_ptr<slint::VectorModel<FileEntry>> model_;
    std::string                                  cwd_ = "/";
    std::vector<FsEntry>                         rows_;   // same order as the model
    int                                          selected_ = -1;
    std::vector<std::string>                     back_, fwd_;
};