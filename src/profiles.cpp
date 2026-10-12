
#include "profiles.hpp"
#include "config.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace profiles {

namespace {

void validateName(const std::string& name) {
    if (name.empty() ||
        name == "." ||
        name == ".." ||
        name.find('/') != std::string::npos ||
        name.find('\\') != std::string::npos) {
        throw std::runtime_error("Invalid profile name");
    }
}

void requireInitialized() {
    if (!std::filesystem::is_directory(config::dotfilesPath())) {
        throw std::runtime_error(
            "Dotfiles directory does not exist. Run init first."
        );
    }
}

} // namespace

void create(const std::string& name) {
    validateName(name);
    requireInitialized();

    const auto directory = config::profilePath(name);

    if (std::filesystem::exists(directory)) {
        throw std::runtime_error(
            "Profile already exists: " + name
        );
    }

    std::filesystem::create_directories(directory);

    std::ofstream links(directory / ".links");
    if (!links) {
        std::filesystem::remove_all(directory);
        throw std::runtime_error(
            "Failed to create profile link registry"
        );
    }

    std::cout << "Created profile: " << name << '\n';
}

void list() {
    requireInitialized();

    const auto directory = config::dotfilesPath() / "profiles";

    if (!std::filesystem::is_directory(directory)) {
        std::cout << "No profiles created.\n";
        return;
    }

    const auto active = config::activeProfile();
    bool found = false;

    for (const auto& entry :
         std::filesystem::directory_iterator(directory)) {
        if (!entry.is_directory()) {
            continue;
        }

        found = true;
        const auto name = entry.path().filename().string();

        std::cout << name;

        if (active.has_value() && *active == name) {
            std::cout << " (active)";
        }

        std::cout << '\n';
    }

    if (!found) {
        std::cout << "No profiles created.\n";
    }
}

void use(const std::string& name) {
    validateName(name);
    requireInitialized();

    const auto directory = config::profilePath(name);

    if (!std::filesystem::is_directory(directory) ||
        !std::filesystem::is_regular_file(directory / ".links")) {
        throw std::runtime_error(
            "Profile does not exist: " + name
        );
    }

    const auto activeFile = config::dotfilesPath() / ".active-profile";
    const auto temporaryFile = config::dotfilesPath() / ".active-profile.tmp";

    {
        std::ofstream file(temporaryFile, std::ios::trunc);
        if (!file || !(file << name << '\n')) {
            throw std::runtime_error(
                "Failed to write active profile"
            );
        }
    }

    std::error_code error;
    std::filesystem::rename(temporaryFile, activeFile, error);

    if (error) {
        std::filesystem::remove(temporaryFile);
        throw std::runtime_error(
            "Failed to select profile: " + error.message()
        );
    }

    std::cout << "Active profile: " << name << '\n';
}
void remove(const std::string& name) {
    validateName(name);
    requireInitialized();

    const auto directory = config::profilePath(name);

    if (!std::filesystem::is_directory(directory)) {
        throw std::runtime_error("Profile does not exist: " + name);
    }

    const auto active = config::activeProfile();

    if (active.has_value() && *active == name) {
        throw std::runtime_error(
            "Cannot delete the active profile. Switch profiles first."
        );
    }

    std::filesystem::remove_all(directory);

    std::cout << "Deleted profile: " << name << '\n';
}
}