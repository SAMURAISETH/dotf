
#include "tui.hpp"
#include "git.hpp"
#include "file.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include "config.hpp"
#include <string>
#include <vector>
#include "operations.hpp"


namespace tui {
    std::string config_path;
std::string dotfile_path;
std::string add_message;

std::string git_branch;
std::vector<std::string> git_branches;
std::vector<std::string> git_status;

int run() {
    using namespace ftxui;

    auto screen = ScreenInteractive::Fullscreen();
    auto links = file::readLinks();

    std::vector<std::string> menu_entries = {
        "Dashboard",
        "Manage links",
        "Add configuration",
        "Update hard links",
        "Git / branches",
        "List registered links",
        "Quit"
    };

    int selected = 0;
int active_screen = 0;
std::string update_message;
std::vector<std::string> update_results;
int selected_link = 0;
int selected_branch = 0;
bool confirm_remove = false;


    auto menu_option = MenuOption::Vertical();
    menu_option.on_enter = [&] {
        if (menu_entries[selected] == "Quit") {
            screen.Exit();
        } else if (menu_entries[selected] == "List registered links") {
            active_screen = 1;
        } else if (menu_entries[selected] == "Manage links") {
            active_screen = 2;
            selected_link = 0;
        } else if (menu_entries[selected] == "Add configuration") {
            active_screen = 3;
            config_path.clear();
            dotfile_path.clear();
            add_message.clear();
        }else if (menu_entries[selected] == "Update hard links") {
            try {
                update_results = operations::updateHardLinks();
            } catch (const std::exception& error) {
                update_results = {error.what()};
            }
        
            active_screen = 4;
        }
        else if (menu_entries[selected] == "Git / branches") {
            try {
                const auto repo = config::dotfilesPath();
        
                git_branch = git::currentBranch(repo);
                git_branches = git::branches(repo);
                git_status = git::status(repo);
            } catch (const std::exception& error) {
                git_branch.clear();
                git_branches.clear();
                git_status = {error.what()};
            }
        
            active_screen = 5;
        }
        
        
    };

    auto menu = Menu(&menu_entries, &selected, menu_option);
    
    int add_form_selected = 0;

    auto config_input = Input(&config_path, "e.g. /home/user/.config/hypr/hyprland.conf");
    auto dotfile_input = Input(&dotfile_path, "e.g. hypr/hyprland.conf");               

auto add_button = Button("Add configuration", [&] {
    try {
        operations::addConfig(config_path, dotfile_path, false, true);
        links = file::readLinks();
        add_message = "Configuration added successfully.";
    } catch (const std::exception& error) {
        add_message = error.what();
    }
});

auto add_form = Container::Tab({
    config_input,
    dotfile_input,
    add_button
}, &add_form_selected);
    auto layout = Renderer(menu, [&] {
if (active_screen == 5) {
    Elements rows;

    rows.push_back(
        text("GIT / BRANCHES") | bold | color(Color::Green)
    );
    rows.push_back(separator());

    rows.push_back(text("Current branch: " + git_branch));
    rows.push_back(separator());

    rows.push_back(text("Branches:") | bold);
    
for (size_t i = 0; i < git_branches.size(); ++i) {
    const auto& branch = git_branches[i];

    if (static_cast<int>(i) == selected_branch) {
        rows.push_back(
            text("> " + branch) | bold | color(Color::Green)
        );
    } else {
        rows.push_back(text("  " + branch));
    }
}



    rows.push_back(separator());

    rows.push_back(text("Working tree:") | bold);
    for (const auto& entry : git_status) {
        rows.push_back(text("  " + entry));
    }

    rows.push_back(separator());
    rows.push_back(text("Escape: Back"));

    return vbox(rows) | border | flex;
}
        else if (active_screen == 4) {
            Elements rows;
        
            rows.push_back(
                text("UPDATE HARD LINKS") | bold | color(Color::Green)
            );
            rows.push_back(separator());
        
            for (const auto& result : update_results) {
                rows.push_back(text(result));
            }
        
            rows.push_back(separator());
            rows.push_back(text("Escape: Back"));
        
            return vbox(rows) | border | flex;
        }
if (active_screen == 3) {
    return vbox(
        text("ADD CONFIGURATION") | bold | color(Color::Green),
        separator(),
        text("Existing config file path:"),
        config_input->Render(),
        text("Destination path inside dotfiles:"),
        dotfile_input->Render(),
        add_button->Render(),
        text(add_message),
        separator(),
        text("Tab: Switch fields"),
        text("Escape: Back")
    ) | border | flex;
}
        if (active_screen == 2) {
            Elements rows;
        
            rows.push_back(
                text("MANAGE LINKS") | bold | color(Color::Green)
            );
            rows.push_back(separator());
        
            if (links.empty()) {
                rows.push_back(text("No links configured."));
            } else {
                for (size_t i = 0; i < links.size(); ++i) {
                    std::string label = file::formatLink(links[i]);
        
                    if (static_cast<int>(i) == selected_link) {
                        rows.push_back(
                            text("> " + label) | bold | color(Color::Green)
                        );
                    } else {
                        rows.push_back(text("  " + label));
                    }
                }
            }
        
            rows.push_back(separator());

if (confirm_remove && !links.empty()) {
    rows.push_back(
        text("Remove this registration?") | bold | color(Color::Yellow)
    );
    rows.push_back(text("Y: Confirm removal"));
    rows.push_back(text("N: Cancel"));
} else {
    rows.push_back(text("Up/Down: Select link"));
    rows.push_back(text("D: Remove registration"));
    rows.push_back(text("Escape: Back"));
}
        
            return vbox(rows) | border | flex;
        }
        if (active_screen == 1) {
            Elements rows;
    
            rows.push_back(
                text("REGISTERED LINKS") | bold | color(Color::Green)
            );
            rows.push_back(separator());
    
            if (links.empty()) {
                rows.push_back(text("No links configured."));
            } else {
                for (const auto& link : links) {
                    rows.push_back(
                        text(file::formatLink(link))
                    );
                }
            }
    
            rows.push_back(separator());
            rows.push_back(text("Press Escape to return"));
    
            return vbox(rows) | border | flex;
        }
    
        return hbox(
            vbox(
                text("DOTFILES MANAGER") | bold | color(Color::Green),
                separator(),
                text("CONFIGURATION"),
                menu->Render(),
                filler(),
                text("↑ ↓ Navigate"),
                text("Enter Select")
            ) | border | size(WIDTH, EQUAL, 28),
    
            vbox(
                text("Welcome back.") | bold,
                separator(),
                text("Manage your dotfiles from one place."),
                text(""),
                text("Registered links: " + std::to_string(links.size())),
                text(""),
                text("Selected: " + menu_entries[selected]),
                filler(),
                text("Dotfiles Manager v0.1.0")
            ) | border | flex
        );
    });
    
auto app = CatchEvent(layout, [&](Event event) {
    if (event == Event::Escape && active_screen != 0) {
        active_screen = 0;
        confirm_remove = false;
        return true;
    }
    if (active_screen == 5 && !git_branches.empty()) {
        if (event == Event::ArrowDown) {
            selected_branch =
                (selected_branch + 1) % static_cast<int>(git_branches.size());
            return true;
        }
    
        if (event == Event::ArrowUp) {
            selected_branch =
                (selected_branch - 1 + static_cast<int>(git_branches.size())) %
                static_cast<int>(git_branches.size());
            return true;
        }
    }

    if (active_screen == 3) {
        if (event == Event::Tab) {
            add_form_selected = (add_form_selected + 1) % 3;
            return true;
        }
    
        return add_form->OnEvent(event);
    }

    if (active_screen == 2 && confirm_remove) {
        if (event == Event::Character('y') ||
            event == Event::Character('Y')) {
            if (!links.empty() &&
                selected_link >= 0 &&
                selected_link < static_cast<int>(links.size())) {
                    links.erase(links.begin() + selected_link);
                    file::writeLinks(links);

                if (selected_link >= static_cast<int>(links.size()) &&
                    selected_link > 0) {
                    --selected_link;
                }
            }

            confirm_remove = false;
            return true;
        }

        if (event == Event::Character('n') ||
            event == Event::Character('N')) {
            confirm_remove = false;
            return true;
        }

        return true;
    }

    if (active_screen == 2 && event == Event::Character('d')) {
        if (!links.empty()) {
            confirm_remove = true;
        }
        return true;
    }

    if (active_screen == 2 && !links.empty()) {
        if (event == Event::ArrowDown) {
            selected_link =
                (selected_link + 1) % static_cast<int>(links.size());
            return true;
        }

        if (event == Event::ArrowUp) {
            selected_link =
                (selected_link - 1 + static_cast<int>(links.size())) %
                static_cast<int>(links.size());
            return true;
        }
    }

    return false;
});

    screen.Loop(app);
    return 0;
}

}