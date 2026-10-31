#include "file_backend.h"
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {

std::string join(const std::string& d, const std::string& n)
{
    return d == "/" ? "/" + n : d + "/" + n;
}

static bool has_text_ext(const std::string& n)
{
    static const char* ext[] = { "txt", "md", "log", "cfg", "conf", "ini", "json", "xml", "html",
                                 "css", "c", "h", "cpp", "hpp", "py", "sh", "slint", "csv", nullptr };
    size_t dot = n.find_last_of('.');
    if (dot == std::string::npos) return false;
    std::string e = n.substr(dot + 1);
    for (auto& c : e) if (c >= 'A' && c <= 'Z') c = (char)(c + 32);
    for (int i = 0; ext[i]; i++) if (e == ext[i]) return true;
    return false;
}

static bool is_elf(const std::string& path)
{
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) return false;
    unsigned char m[4];
    ssize_t n = read(fd, m, 4);
    close(fd);
    return n == 4 && m[0] == 0x7F && m[1] == 'E' && m[2] == 'L' && m[3] == 'F';
}

class KernelFileBackend : public IFileBackend {
public:
    bool list_dir(const std::string& path, std::vector<FsEntry>& out, std::string& err) override
    {
        out.clear();
        DIR* d = opendir(path.c_str());
        if (!d) { err = "Cannot open " + path + ": " + strerror(errno); return false; }

        while (dirent* e = readdir(d)) {
            std::string n = e->d_name;
            if (n == "." || n == "..") continue;

            FsEntry fe;
            fe.name = n;
            struct stat st;
            if (stat(join(path, n).c_str(), &st) == 0) {
                fe.is_dir = S_ISDIR(st.st_mode);
                fe.size   = (uint64_t)st.st_size;
                if (!fe.is_dir && fe.size >= 8192 && !has_text_ext(n))
                {
                    fe.is_exec = is_elf(join(path, n));
                }

            } else {
                fe.is_dir = (e->d_type == DT_DIR);   // fall back when stat isn't available
            }
            out.push_back(fe);
        }
        closedir(d);
        return true;
    }

    bool make_dir(const std::string& path, std::string& err) override
    {
        if (mkdir(path.c_str(), 0755) != 0) { err = strerror(errno); return false; }
        return true;
    }

    bool remove_path(const std::string& path, std::string& err) override
    {
        struct stat st;
        bool dir = stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
        if ((dir ? rmdir(path.c_str()) : unlink(path.c_str())) != 0) { err = strerror(errno); return false; }
        return true;
    }

    bool rename_path(const std::string& from, const std::string& to, std::string& err) override
    {
        if (rename(from.c_str(), to.c_str()) != 0) { err = strerror(errno); return false; }
        return true;
    }

    bool copy_path(const std::string& from, const std::string& to, std::string& err) override
    {
        int in = open(from.c_str(), O_RDONLY);
        if (in < 0) { err = strerror(errno); return false; }
        int out = open(to.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
        if (out < 0) { err = strerror(errno); close(in); return false; }

        char buf[16384];
        ssize_t n;
        bool ok = true;
        while ((n = read(in, buf, sizeof buf)) > 0) {
            ssize_t off = 0;
            while (off < n) {
                ssize_t w = write(out, buf + off, (size_t)(n - off));
                if (w <= 0) { err = "write failed"; ok = false; break; }
                off += w;
            }
            if (!ok) break;
        }
        close(in);
        close(out);
        return ok;
    }
};

}  // namespace

IFileBackend& kernel_file_backend() { static KernelFileBackend b; return b; }