
#include "file.hpp"
#include "config.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace file {

std::vector<Link> readLinks() {
    std::vector<Link> links;

    const auto linksPath = config::linksFile();

    if (!std::filesystem::exists(linksPath)) {
        return links;
    }

    std::ifstream input(linksPath);
    if (!input) {
        throw std::runtime_error(
            "Failed to open links file: " + linksPath.string()
        );
    }

    std::string line;
    std::size_t lineNumber = 0;

    
    while (std::getline(input, line)) {
        const auto separator = line.find(" -> ");
    
        if (separator == std::string::npos) {
            throw std::runtime_error(
                "Failed to parse line " + std::to_string(lineNumber)
                + ": " + line
            );
        }
    
        const auto sourceRelative = line.substr(0, separator);
        const auto targetString = line.substr(separator + 4);
    
        const auto source = config::dotfilesPath() / sourceRelative;
    
        std::filesystem::path target;
    
        if (targetString.starts_with("$HOME/")) {
            target = config::homePath() / targetString.substr(6);
        } else if (targetString.starts_with("~/")) {
            target = config::homePath() / targetString.substr(2);
        } else {
            target = targetString;
        }
    
        if (!std::filesystem::exists(source)) {
            std::cerr << "Warning: Line " << lineNumber
                      << ": File " << source
                      << " does not exist.\n";
        }
    
        if (!target.is_absolute()) {
            std::cerr << "Warning: Line " << lineNumber
                      << ": Target is not absolute. "
                      << "This may cause unexpected behavior.\n";
        }
    
        const Link newLink{source, target};

const bool duplicate = std::any_of(
    links.begin(),
    links.end(),
    [&](const Link& existing) {
        return existing.target == newLink.target;
    }
);

if (!duplicate) {
    links.push_back(newLink);
}
        ++lineNumber;
    }
    return links;

}

void appendLink(const Link& link) {
    const auto links = readLinks();

    for (const auto& existing : links) {
        if (existing.target == link.target) {
            throw std::runtime_error(
                "A link for target already exists: "
                + link.target.string()
            );
        }
    }

    std::ofstream output(config::linksFile(), std::ios::app);

    if (!output) {
        throw std::runtime_error("Failed to open links file for writing");
    }

    const auto dotfiles = config::dotfilesPath();
    const auto home = config::homePath();

    const auto sourceRelative = link.source.lexically_relative(dotfiles);
    const auto targetRelative = link.target.lexically_relative(home);

    const std::string targetString =
        !targetRelative.empty()
            && !targetRelative.is_absolute()
            && *targetRelative.begin() != ".."
                ? "$HOME/" + targetRelative.string()
                : link.target.string();

    output << sourceRelative.string() << " -> "
           << targetString << '\n';

    if (!output) {
        throw std::runtime_error("Failed to write link");
    }
}

void writeLinks(const std::vector<Link>& links) {
    std::ofstream output(config::linksFile());

    if (!output) {
        throw std::runtime_error("Failed to open links file for writing");
    }

    const auto dotfiles = config::dotfilesPath();
    const auto home = config::homePath();

    for (const auto& link : links) {
        const auto sourceRelative =
        link.source.lexically_relative(dotfiles);

        const auto targetRelative =
            link.target.lexically_relative(home);

        const std::string targetString =
            !targetRelative.empty()
            && !targetRelative.is_absolute()
            && *targetRelative.begin() != ".."
                ? "$HOME/" + targetRelative.string()
                : link.target.string();

        output << sourceRelative.string() << " -> "
               << targetString << '\n';
    }

    if (!output) {
        throw std::runtime_error("Failed to write links file");
    }
}
std::string formatLink(const Link& link) {
    const auto sourceRelative =
        link.source.lexically_relative(config::dotfilesPath());

    const auto targetRelative =
        link.target.lexically_relative(config::homePath());

    std::string target;

    if (!targetRelative.empty() &&
        !targetRelative.is_absolute() &&
        *targetRelative.begin() != "..") {
        target = "$HOME/" + targetRelative.string();
    } else {
        target = link.target.string();
    }

    return sourceRelative.string() + " -> " + target;
}
}