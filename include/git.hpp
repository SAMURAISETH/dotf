#pragma once

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace git {
    void init(const std::filesystem::path& path);

    void add(
        const std::filesystem::path& path,
        const std::filesystem::path& file
    );

    void commit(
        const std::filesystem::path& path,
        const std::string& message
    );

    void checkout(
        const std::filesystem::path& path,
        const std::string& branch
    );
    std::string currentBranch(const std::filesystem::path& path);

std::vector<std::string> branches(
    const std::filesystem::path& path
);

std::vector<std::string> status(
    const std::filesystem::path& path
);
}