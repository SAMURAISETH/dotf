#pragma once

#include <string>

namespace profiles {
    void create(const std::string& name);
    void list();
    void use(const std::string& name);
    void remove(const std::string& name);
}