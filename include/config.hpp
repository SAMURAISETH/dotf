
#pragma once

#include <filesystem>

namespace config {
    std::filesystem::path homePath();
    std::filesystem::path dotfilesPath();
    std::filesystem::path linksFile();

    std::filesystem::path expandHome(
        const std::filesystem::path& path
    );
}