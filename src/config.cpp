
#include "config.hpp"

#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>

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

    const auto pathString = path.string();

    if (pathString.starts_with("~/")) {
        return homePath() / pathString.substr(2);
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

std::filesystem::path profilePath(
    const std::string& name
) {
    return dotfilesPath() / "profiles" / name;
}

std::optional<std::string> activeProfile() {
    const auto activeFile = dotfilesPath() / ".active-profile";

    if (!std::filesystem::exists(activeFile)) {
        return std::nullopt;
    }

    std::ifstream file(activeFile);
    std::string name;

    if (!std::getline(file, name) || name.empty()) {
        throw std::runtime_error(
            "Active profile file is empty or unreadable"
        );
    }

    return name;
}

std::filesystem::path linksFile() {
    const auto active = activeProfile();

    if (active.has_value()) {
        return profilePath(*active) / ".links";
    }

    // Preserve compatibility with existing installations.
    return dotfilesPath() / ".links";
}

}