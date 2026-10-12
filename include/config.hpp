
#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace config {
    std::filesystem::path homePath();
    std::filesystem::path dotfilesPath();
    std::filesystem::path linksFile();

    // Profile support
    std::optional<std::string> activeProfile();
    std::filesystem::path profilePath(
        const std::string& name
    );

    std::filesystem::path expandHome(
        const std::filesystem::path& path
    );
}