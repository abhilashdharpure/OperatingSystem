#include "shell.h"
#include "luma_spawn.h"
#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {

std::vector<std::string> tokenize(const std::string& s)
{
    std::vector<std::string> t;
    std::string cur;
    bool quoted = false, has = false;
    char qc = 0;
    for (char c : s) {
        if (quoted) { if (c == qc) quoted = false; else cur += c; }
        else if (c == '"' || c == '\'') { quoted = true; qc = c; has = true; }
        else if (c == ' ' || c == '\t') {
            if (has || !cur.empty()) { t.push_back(cur); cur.clear(); has = false; }
        } else cur += c;
    }
    if (has || !cur.empty()) t.push_back(cur);
    return t;
}

std::string normalize(const std::string& in)
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

std::string join(const std::string& d, const std::string& n) { return d == "/" ? "/" + n : d + "/" + n; }

bool is_dir(const std::string& p)
{
    struct stat st;
    if (stat(p.c_str(), &st) == 0) return S_ISDIR(st.st_mode);    // authoritative when stat works

    DIR* d = opendir(p.c_str());                                  // fallback if stat is unavailable
    if (!d) return false;
    closedir(d);
    return true;
}

bool exists(const std::string& p)
{
    int fd = open(p.c_str(), O_RDONLY);
    if (fd >= 0) { close(fd); return true; }
    return is_dir(p);
}

}  // namespace

std::string Shell::resolve(const std::string& p) const
{
    return normalize(!p.empty() && p[0] == '/' ? p : cwd_ + "/" + p);
}

std::vector<std::pair<std::string, bool>> Shell::list(const std::string& dir) const
{
    std::vector<std::pair<std::string, bool>> v;
    DIR* d = opendir(dir.c_str());
    if (!d) return v;
    while (dirent* e = readdir(d)) {
        std::string n = e->d_name;
        if (n == "." || n == "..") continue;
        v.push_back({n, is_dir(join(dir, n))});
    }
    closedir(d);
    std::sort(v.begin(), v.end(), [](auto& a, auto& b) {
        if (a.second != b.second) return a.second;
        return a.first < b.first;
    });
    return v;
}

void Shell::emit(const std::string& text, int kind)
{
    size_t start = 0;
    while (start <= text.size()) {
        size_t nl = text.find('\n', start);
        std::string line = text.substr(start, nl == std::string::npos ? std::string::npos : nl - start);
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        for (auto& c : line) if (c == '\t') c = ' ';
        if (line.empty()) out_("", kind);
        for (size_t i = 0; i < line.size(); i += 110) out_(line.substr(i, 110), kind);   // no wrapping in the list
        if (nl == std::string::npos) break;
        start = nl + 1;
        if (start == text.size()) break;                    // a trailing newline adds no blank line
    }
}

void Shell::launch(const std::string& path, const std::vector<std::string>& args)
{
    std::string base = path.substr(path.find_last_of('/') + 1);
    if (base == "comp" || base == "init") { err("refusing to start " + base + " a second time"); return; }
    long pid = luma_spawn(path, args, 1);                   // detached: the kernel reaps it
    if (pid > 0) emit("started " + path + " (pid " + std::to_string(pid) + ")");
    else err("cannot start " + path);
}

bool Shell::copy_file(const std::string& from, const std::string& to)
{
    int in = open(from.c_str(), O_RDONLY);
    if (in < 0) { err("cp: " + from + ": " + strerror(errno)); return false; }
    int out = open(to.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out < 0) { err("cp: " + to + ": " + strerror(errno)); close(in); return false; }
    char buf[8192];
    ssize_t n;
    bool ok = true;
    while ((n = read(in, buf, sizeof buf)) > 0) {
        ssize_t off = 0;
        while (off < n) {
            ssize_t w = write(out, buf + off, (size_t)(n - off));
            if (w <= 0) { err(std::string("cp: write failed: ") + strerror(errno)); ok = false; break; }
            off += w;
        }
        if (!ok) break;
    }
    close(in);
    close(out);
    return ok;
}

void Shell::run(const std::string& line)
{
    auto a = tokenize(line);
    if (a.empty()) return;

    // output redirection: "cmd args > file" or ">> file"
    std::string target;
    bool redirect = false, append = false;
    if (a.size() >= 3 && (a[a.size() - 2] == ">" || a[a.size() - 2] == ">>")) {
        redirect = true;
        append = a[a.size() - 2] == ">>";
        target = resolve(a.back());
        a.resize(a.size() - 2);
    }

    std::string captured;
    auto real = out_;
    if (redirect)
        out_ = [&](const std::string& s, int k) { if (k == 2) real(s, k); else captured += s + "\n"; };

    dispatch(a);
    out_ = real;

    if (redirect) {
        int fd = open(target.c_str(), O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 0644);
        if (fd < 0) { err(target + ": " + strerror(errno)); return; }
        size_t off = 0;
        while (off < captured.size()) {
            ssize_t w = write(fd, captured.data() + off, captured.size() - off);
            if (w <= 0) { err(std::string("write failed: ") + strerror(errno)); break; }
            off += (size_t)w;
        }
        close(fd);
    }
}

void Shell::dispatch(const std::vector<std::string>& a)
{
    const std::string& c = a[0];

    if (c == "help") {
        emit("help  clear  pwd  cd [dir]  ls [-l] [dir]  cat file...  echo text  touch file");
        emit("mkdir dir  rm path  mv from to  cp from to  edit file  run program [args]  exit");
        emit("Redirect output with   command > file   or   command >> file");
        emit("Typing the name of a program in /bin starts it.");
    }
    else if (c == "clear") hooks_.clear();
    else if (c == "exit" || c == "quit") hooks_.quit();
    else if (c == "pwd") emit(cwd_);
    else if (c == "echo") {
        std::string s;
        for (size_t i = 1; i < a.size(); i++) s += (i > 1 ? " " : "") + a[i];
        emit(s);
    }
    else if (c == "cd") {
        std::string p = a.size() > 1 ? resolve(a[1]) : "/";
        if (is_dir(p)) cwd_ = p; else err("cd: " + (a.size() > 1 ? a[1] : p) + ": not a directory");
    }
    else if (c == "ls") {
        bool lng = false;
        std::string dir = cwd_;
        for (size_t i = 1; i < a.size(); i++) { if (a[i] == "-l") lng = true; else dir = resolve(a[i]); }
        if (!is_dir(dir)) {
            if (exists(dir)) { emit(dir.substr(dir.find_last_of('/') + 1)); return; }
            err("ls: " + dir + ": no such file or directory");
            return;
        }
        auto ents = list(dir);
        std::string row;
        for (auto& [name, dirflag] : ents) {
            if (lng) {
                struct stat st{};
                long long sz = stat(join(dir, name).c_str(), &st) == 0 ? (long long)st.st_size : 0;
                char b[160];
                std::snprintf(b, sizeof b, "%c %10lld  %s", dirflag ? 'd' : '-', dirflag ? 0LL : sz, name.c_str());
                emit(b);
            } else {
                std::string item = name + (dirflag ? "/" : "");
                if (!row.empty() && row.size() + item.size() + 2 > 100) { emit(row); row.clear(); }
                row += (row.empty() ? "" : "  ") + item;
            }
        }
        if (!row.empty()) emit(row);
    }
    else if (c == "cat") {
        if (a.size() < 2) { err("usage: cat file..."); return; }
        for (size_t i = 1; i < a.size(); i++) {
            int fd = open(resolve(a[i]).c_str(), O_RDONLY);
            if (fd < 0) { err("cat: " + a[i] + ": " + strerror(errno)); continue; }
            std::string data;
            char buf[4096];
            ssize_t n;
            while ((n = read(fd, buf, sizeof buf)) > 0 && data.size() < 256 * 1024) data.append(buf, (size_t)n);
            close(fd);
            emit(data);
        }
    }
    else if (c == "touch") {
        for (size_t i = 1; i < a.size(); i++) {
            int fd = open(resolve(a[i]).c_str(), O_WRONLY | O_CREAT, 0644);
            if (fd < 0) err("touch: " + a[i] + ": " + strerror(errno)); else close(fd);
        }
    }
    else if (c == "mkdir") {
        for (size_t i = 1; i < a.size(); i++)
            if (mkdir(resolve(a[i]).c_str(), 0755) != 0) err("mkdir: " + a[i] + ": " + strerror(errno));
    }
    else if (c == "rm") {
        for (size_t i = 1; i < a.size(); i++) {
            std::string p = resolve(a[i]);
            if (unlink(p.c_str()) != 0 && rmdir(p.c_str()) != 0) err("rm: " + a[i] + ": " + strerror(errno));
        }
    }
    else if (c == "mv") {
        if (a.size() != 3) { err("usage: mv from to"); return; }
        if (rename(resolve(a[1]).c_str(), resolve(a[2]).c_str()) != 0) err(std::string("mv: ") + strerror(errno));
    }
    else if (c == "cp") {
        if (a.size() != 3) { err("usage: cp from to"); return; }
        std::string to = resolve(a[2]);
        if (is_dir(to)) to = join(to, a[1].substr(a[1].find_last_of('/') + 1));
        copy_file(resolve(a[1]), to);
    }
    else if (c == "edit") {
        if (a.size() < 2) { launch("/bin/editor", {}); return; }
        launch("/bin/editor", { resolve(a[1]) });
    }
    else if (c == "run") {
        if (a.size() < 2) { err("usage: run program [args]"); return; }
        std::string p = a[1].find('/') == std::string::npos ? "/bin/" + a[1] : resolve(a[1]);
        if (!exists(p)) { err("run: " + a[1] + ": not found"); return; }
        launch(p, std::vector<std::string>(a.begin() + 2, a.end()));
    }
    else {
        std::string p = c.find('/') == std::string::npos ? "/bin/" + c : resolve(c);
        if (exists(p) && !is_dir(p)) launch(p, std::vector<std::string>(a.begin() + 1, a.end()));
        else err(c + ": command not found");
    }
}

std::string Shell::complete(const std::string& line)
{
    size_t sp = line.find_last_of(' ');
    std::string head = sp == std::string::npos ? "" : line.substr(0, sp + 1);
    std::string word = sp == std::string::npos ? line : line.substr(sp + 1);
    size_t sl = word.find_last_of('/');
    std::string dirpart = sl == std::string::npos ? "" : word.substr(0, sl + 1);
    std::string prefix  = sl == std::string::npos ? word : word.substr(sl + 1);

    std::vector<std::string> hits;
    for (auto& [name, d] : list(resolve(dirpart.empty() ? "." : dirpart)))
        if (name.compare(0, prefix.size(), prefix) == 0) hits.push_back(name + (d ? "/" : ""));
    if (hits.empty()) return line;

    std::string common = hits[0];
    for (auto& h : hits) {
        size_t i = 0;
        while (i < common.size() && i < h.size() && common[i] == h[i]) i++;
        common.resize(i);
    }
    if (hits.size() > 1) {
        std::string row;
        for (auto& h : hits) row += h + "  ";
        emit(row);
    }
    return head + dirpart + common;
}