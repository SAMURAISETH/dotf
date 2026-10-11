
#include "config.hpp"

#include <cstdlib>
#include <stdexcept>

namespace config {

std::filesystem::path homePath() {
    const char* home = std::getenv("HOME");

    if (home == nullptr) {
        throw std::runtime_error(
            "HOME environment variable not set"
        );
    }

    return home;
}
std::filesystem::path expandHome(
    const std::filesystem::path& path
) {
    if (path == "~") {
        return homePath();
    }

    const auto path_string = path.string();

    if (path_string.starts_with("~/")) {
        return homePath() / path_string.substr(2);
    }

    return path;
}
std::filesystem::path dotfilesPath() {
    const char* path = std::getenv("DOTFILES_PATH");

    if (path != nullptr && *path != '\0') {
        return std::filesystem::absolute(path).lexically_normal();
    }

    return homePath() / ".dotfile";
}

std::filesystem::path linksFile() {
    return dotfilesPath() / ".links";
}

}