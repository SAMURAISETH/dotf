
#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct Link {
    std::filesystem::path source;
    std::filesystem::path target;
};

namespace file {
    std::vector<Link> readLinks();
    void appendLink(const Link& link);
    void writeLinks(const std::vector<Link>& links);
    std::string formatLink(const Link& link);
}
