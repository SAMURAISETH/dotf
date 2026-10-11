
#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace operations {
    void addConfig(
        const std::filesystem::path& configPath,
        const std::filesystem::path& dotfilePath,
        bool moveExisting = false,
        bool createParents = false
    );
    std::vector<std::string> updateHardLinks();
}