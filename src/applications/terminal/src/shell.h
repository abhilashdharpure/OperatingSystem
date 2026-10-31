#pragma once
#include <functional>
#include <string>
#include <utility>
#include <vector>

struct ShellHooks {
    std::function<void(const std::string&, int)> out;      // text, kind (0 output, 2 error)
    std::function<void()> clear;
    std::function<void()> quit;
};

class Shell {
public:
    explicit Shell(ShellHooks h) : hooks_(std::move(h)), out_(hooks_.out) {}
    void run(const std::string& line);
    std::string complete(const std::string& line);          // Tab: returns the new line
    const std::string& cwd() const { return cwd_; }

private:
    void dispatch(const std::vector<std::string>& a);
    void emit(const std::string& text, int kind = 0);
    void err(const std::string& text) { emit(text, 2); }
    std::string resolve(const std::string& p) const;
    std::vector<std::pair<std::string, bool>> list(const std::string& dir) const;   // name, is_dir
    void launch(const std::string& path, const std::vector<std::string>& args);
    bool copy_file(const std::string& from, const std::string& to);

    ShellHooks hooks_;
    std::function<void(const std::string&, int)> out_;      // swapped while redirecting
    std::string cwd_ = "/";
};