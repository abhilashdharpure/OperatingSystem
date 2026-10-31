#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct FsEntry {
    std::string name;
    bool        is_dir  = false;
    bool        is_exec = false;        // regular file that starts with the ELF magic
    uint64_t    size    = 0;
};

// Implement this on top of your kernel (getdents / stat / mkdir / unlink / rename).
// Return false and fill 'err' on failure; the UI shows 'err' in the status bar.
class IFileBackend {
public:
    virtual ~IFileBackend() = default;
    virtual bool list_dir   (const std::string& path, std::vector<FsEntry>& out, std::string& err) = 0;
    virtual bool make_dir   (const std::string& path, std::string& err) = 0;
    virtual bool remove_path(const std::string& path, std::string& err) = 0;
    virtual bool rename_path(const std::string& from, const std::string& to, std::string& err) = 0;
    virtual bool copy_path(const std::string& from, const std::string& to, std::string& err) = 0;
};

// Demo implementation with fake data, so the UI runs before the kernel side exists.
// IFileBackend& demo_backend();
IFileBackend& kernel_file_backend();