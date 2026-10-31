#include "file_backend.h"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

namespace {
class DemoBackend : public IFileBackend {
public:
    bool list_dir(const std::string& path, std::vector<FsEntry>& out, std::string& err) override {
        out.clear();
        if (path == "/") {
            out = { {"bin", true, 0}, {"tmp", true, 0}, {"dev", true, 0},
                    {"boot.cfg", false, 312}, {"readme.txt", false, 1536} };
        } else if (path == "/bin") {
            out = { {"comp", false, 1572864}, {"luma_os_gui", false, 6291456}, {"init", false, 28672} };
        } else if (path == "/tmp" || path == "/dev") {
            // empty
        } else {
            err = "No such directory: " + path;
            return false;
        }
        return true;
    }
    bool make_dir(const std::string&, std::string& err) override    { err = "make_dir not implemented"; return false; }
    bool remove_path(const std::string&, std::string& err) override { err = "delete not implemented";   return false; }
    bool rename_path(const std::string&, const std::string&, std::string& err) override { err = "rename not implemented"; return false; }

    bool copy_path(const std::string& from, const std::string& to, std::string& err) override
    {
        int in = open(from.c_str(), O_RDONLY);
        if (in < 0) { err = strerror(errno); return false; }
        int out = open(to.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
        if (out < 0) { err = strerror(errno); close(in); return false; }
        char buf[16384];
        ssize_t n;
        bool ok = true;
        while ((n = read(in, buf, sizeof buf)) > 0)
            if (write(out, buf, n) != n) { err = "write failed"; ok = false; break; }
        close(in); close(out);
        return ok;
    }
};
}

IFileBackend& demo_backend() { static DemoBackend b; return b; }
