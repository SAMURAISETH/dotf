
#include "operations.hpp"

#include "config.hpp"
#include "file.hpp"
#include <iostream>
#include <filesystem>
#include <stdexcept>
#include <string>

namespace operations {

void addConfig(
    const std::filesystem::path& configPath,
    const std::filesystem::path& dotfilePath,
    bool moveExisting,
    bool createParents
) {
    const auto dotfiles = config::dotfilesPath();

    if (!std::filesystem::exists(dotfiles) ||
        !std::filesystem::exists(config::linksFile())) {
        throw std::runtime_error(
            "Dotfiles directory does not exist or is invalid. Run init first."
        );
    }

    const auto expandedConfigPath = config::expandHome(configPath);
    const auto source = std::filesystem::canonical(expandedConfigPath);
    const auto destination =
        std::filesystem::absolute(dotfiles / dotfilePath).lexically_normal();

    if (!std::filesystem::is_regular_file(source)) {
        throw std::runtime_error(
            "Config file not found: " + source.string()
        );
    }

    if (!moveExisting) {
        for (const auto& link : file::readLinks()) {
            if (link.target == source) {
                throw std::runtime_error(
                    "A link for this target already exists: " + source.string()
                );
            }
        }
    }

    const auto relativeSource = source.lexically_relative(dotfiles);

    if (!relativeSource.empty() && !relativeSource.is_absolute() &&
        *relativeSource.begin() != "..") {
        throw std::runtime_error(
            "Config file cannot be inside dotfiles directory"
        );
    }

    const auto relativeDestination = destination.lexically_relative(dotfiles);

    if (relativeDestination.empty() || relativeDestination.is_absolute() ||
        *relativeDestination.begin() == "..") {
        throw std::runtime_error(
            "Dotfile must be inside dotfiles directory"
        );
    }

    if (std::filesystem::exists(destination)) {
        throw std::runtime_error(
            "File already exists: " + destination.string()
        );
    }

    if (createParents) {
        std::filesystem::create_directories(destination.parent_path());
    } else if (!std::filesystem::exists(destination.parent_path())) {
        throw std::runtime_error(
            "Parent directory does not exist: " +
            destination.parent_path().string() +
            ". Use createParents to create it."
        );
    }

    std::filesystem::create_hard_link(source, destination);

    if (moveExisting) {
        std::filesystem::remove(source);
    } else {
        file::appendLink({destination, source});
    }
}
std::vector<std::string> updateHardLinks() {
    std::vector<std::string> results;
    const auto links = file::readLinks();

    for (const auto& link : links) {
        const bool sourceExists = std::filesystem::exists(link.source);
        const bool targetExists = std::filesystem::exists(link.target);

        if (!sourceExists && !targetExists) {
            results.push_back(
                "Both paths missing: " + link.source.string()
                + " -> " + link.target.string()
            );
        } else if (!sourceExists) {
            results.push_back("Dotfile missing: " + link.source.string());
        } else if (!targetExists) {
            results.push_back("Target missing: " + link.target.string());
        } else if (!std::filesystem::equivalent(link.source, link.target)) {
            results.push_back(
                "Paths exist but are not the same file: "
                + link.source.string() + " -> " + link.target.string()
            );
        } else {
            results.push_back("OK: " + link.source.string());
        }
    }

    if (results.empty()) {
        results.push_back("No registered links found.");
    }

    return results;
}
}