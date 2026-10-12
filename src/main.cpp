
#include "config.hpp"
#include "git.hpp"
#include "file.hpp"
#include "tui.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <optional>
#include "operations.hpp"
#include <stdexcept>

void checkDotfilesPath() {
    if (!std::filesystem::exists(config::dotfilesPath()) ||
        !std::filesystem::exists(config::linksFile())) {
        throw std::runtime_error(
            "Dotfiles directory does not exist or is invalid. "
            "Run init first."
        );
    }
}
int main(int argc, char* argv[]) {
    if (argc < 2) {
        return tui::run();
        auto links = file::readLinks();
    }
    std::string config_path;
    std::string dotfile_path;
    std::string add_message;
    const std::string command = argv[1];
    
    if (command == "help" || command == "--help" ||
        command == "-h") {
std::cout << R"(Usage: dotfiles-managers <command> [arguments] [options]

Commands:
path                         Show the dotfiles directory path
init                         Initialize the dotfiles Git repository
link SOURCE TARGET           Register a config file link
unlink TARGET                Remove a registered link
add CONFIG DOTFILE [OPTIONS] Add a config file to dotfiles
list                         List all registered links
update [OPTIONS]             Create or update hard links
checkout [BRANCH]            Switch branches (default: main)
help                         Show this help message
                
Arguments:
SOURCE                       File inside the dotfiles directory
TARGET                       Destination path for a link
CONFIG                       Existing config file to add
DOTFILE                      Destination path inside dotfiles
BRANCH                       Git branch name
                
Options for add:
-m, --move, --no-link        Move instead of registering a link
-p, --create-parents         Create missing parent directories
                
Options for update:
-y                           Override existing targets
-n                           Skip existing targets
    )";
        return 0;
    }

    try {
        if (command == "path") {
            std::cout << config::dotfilesPath().string() << '\n';
            return 0;
        }

        if (command == "init") {
            const auto dotfiles = config::dotfilesPath();
            const auto links = config::linksFile();

            std::filesystem::create_directories(dotfiles);

            git::init(dotfiles);

            if (!std::filesystem::exists(links)) {
                std::ofstream createLinks(links);
            }

            git::add(dotfiles, links.filename());
git::commit(dotfiles, "Initial commit");

            std::cout << "Dotfiles initialized.\n";
            return 0;
        }
        
        if (command == "link") {
            if (argc != 4) {
                std::cerr << "Usage: dotfiles-managers link SOURCE TARGET\n";
                return 1;
            }
            checkDotfilesPath();
            const auto dotfiles = config::dotfilesPath();
            const auto source = std::filesystem::weakly_canonical(
                dotfiles / argv[2]
            );
            const auto target = std::filesystem::absolute(
                argv[3]
            ).lexically_normal();
        
            if (!std::filesystem::is_regular_file(source)) {
                throw std::runtime_error("Source file not found: " + source.string());
            }
            
        
            const auto relativeSource = source.lexically_relative(dotfiles);
            if (relativeSource.empty() || relativeSource.is_absolute()
                || *relativeSource.begin() == "..") {
                throw std::runtime_error("Source must be inside the dotfiles directory");
            }
        
            const auto relativeTarget = target.lexically_relative(dotfiles);
            if (target == dotfiles ||
                (!relativeTarget.empty() && !relativeTarget.is_absolute()
                 && *relativeTarget.begin() != "..")) {
                throw std::runtime_error("Target cannot be inside the dotfiles directory");
            }
        
            for (const auto& link : file::readLinks()) {
                if (link.target == target) {
                    throw std::runtime_error("A config with this target already exists");
                }
            }
        
            if (std::filesystem::exists(target)) {
                std::cout << "Warning: Target file already exists\n";
            }
        
            file::appendLink({source, target});
            return 0;
        }

if (command == "unlink") {
    if (argc != 3) {
        std::cerr << "Usage: dotfiles-managers unlink TARGET\n";
        return 1;
    }
    checkDotfilesPath();

    const auto target = std::filesystem::absolute(
        argv[2]
    ).lexically_normal();

    auto links = file::readLinks();
    bool found = false;

    for (auto it = links.begin(); it != links.end();) {
        if (it->target == target) {
            it = links.erase(it);
            found = true;
        } else {
            ++it;
        }
    }


    file::writeLinks(links);

    if (!found) {
        return 0;
    }

    if (!std::filesystem::exists(target)) {
        return 0;
    }

    std::cout << "Also remove file? [y/N] ";
    std::string answer;
    std::getline(std::cin, answer);

    if (answer == "y" || answer == "Y" ||
        answer == "yes" || answer == "YES") {
        std::filesystem::remove(target);
    }

    return 0;
}
if (command == "about") {
    std::cout << R"(Maintainer -> SAMURAISETH
    Name -> DOTF
    Job -> to main tain dot files.
    Made in -> Project was made in C++
)";
}

if (command == "add") {
    if (argc < 4 || argc > 6) {
        std::cerr
            << "Usage: dotfiles-managers add CONFIG_PATH DOTFILES_PATH "
            << "[-m] [-p]\n";
        return 1;
    }

    bool moveExisting = false;
    bool createParents = false;

    for (int i = 4; i < argc; ++i) {
        const std::string option = argv[i];

        if (option == "-m" || option == "--move" ||
            option == "--no-link") {
            moveExisting = true;
        } else if (option == "-p" || option == "--create-parents") {
            createParents = true;
        } else {
            throw std::runtime_error("Unknown option: " + option);
        }
    }

    operations::addConfig(
        argv[2],
        argv[3],
        moveExisting,
        createParents
    );

    return 0;
}

if (command == "list") {
    checkDotfilesPath();
    const auto links = file::readLinks();

    if (links.empty()) {
        std::cout << "No links configured.\n";
        return 0;
    }

    for (const auto& link : links) {
        std::cout << file::formatLink(link) << '\n';
    }

    return 0;
}

if (command == "update") {
    std::optional<bool> overrideExisting;

    for (int i = 2; i < argc; ++i) {
        const std::string option = argv[i];

        if (option == "-y") {
            overrideExisting = true;
        } else if (option == "-n") {
            overrideExisting = false;
        } else {
            throw std::runtime_error("Unknown option: " + option);
        }
    }
    checkDotfilesPath();
    const auto links = file::readLinks();

    std::cout << "Reading links...\n";
    std::cout << "Creating file links...\n";

    for (const auto& link : links) {
        if (!std::filesystem::exists(link.source)) {
            std::cerr << "Warning: source file does not exist: "
                      << link.source << '\n';
            continue;
        }

        const bool targetExists =
            std::filesystem::exists(link.target) ||
            std::filesystem::is_symlink(link.target);

        if (targetExists) {
            std::error_code error;

            if (std::filesystem::equivalent(
                    link.source, link.target, error) && !error) {
                continue;
            }

            if (!overrideExisting.has_value()) {
                std::cout << "Target file " << link.target
                          << " already exists. Override? [y/N] ";

                std::string answer;
                std::getline(std::cin, answer);

                overrideExisting =
                    answer == "y" || answer == "Y" ||
                    answer == "yes" || answer == "YES";
            }

            if (*overrideExisting) {
                std::filesystem::remove(link.target);
            } else {
                continue;
            }
        }

        std::cout << "Linking " << link.source
                  << " -> " << link.target << '\n';

        std::filesystem::create_directories(
            link.target.parent_path()
        );

        try {
            std::filesystem::create_hard_link(
                link.source, link.target
            );
        } catch (const std::filesystem::filesystem_error&) {
            std::cerr << "Error: failed to link "
                      << link.source << " -> " << link.target << '\n';
        }
    }

    std::cout << "Done!\n";
    return 0;
}
if (command == "checkout") {
    if (argc > 3) {
        std::cerr << "Usage: dotfiles-managers checkout [BRANCH]\n";
        return 1;
    }

    if (!std::filesystem::exists(config::linksFile())) {
        throw std::runtime_error(
            "Dotfiles directory is not initialized. Run init first."
        );
    }

    const std::string branch = argc == 3 ? argv[2] : "main";
    git::checkout(config::dotfilesPath(), branch);
    return 0;
}
        std::cerr << "Unknown command: " << command << '\n';
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}