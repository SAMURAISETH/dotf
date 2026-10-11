
#include "git.hpp"

#include <stdexcept>
#include <string>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>
namespace {

    void runGit(
        const std::filesystem::path& path,
        const std::vector<std::string>& arguments,
        const std::string& errorMessage
    ) {
        std::vector<std::string> command = {
            "git", "-C", path.string()
        };
    
        command.insert(
            command.end(),
            arguments.begin(),
            arguments.end()
        );
    
        std::vector<char*> argv;
        for (auto& arg : command) {
            argv.push_back(arg.data());
        }
        argv.push_back(nullptr);
    
        const pid_t pid = fork();
    
        if (pid == -1) {
            throw std::runtime_error("Failed to start Git process");
        }
    
        if (pid == 0) {
            execvp(argv[0], argv.data());
            _exit(127);
        }
    
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            throw std::runtime_error("Failed to wait for Git process");
        }
    
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            throw std::runtime_error(errorMessage);
        }
    }
    std::string runGitOutput(
        const std::filesystem::path& path,
        const std::vector<std::string>& arguments,
        const std::string& errorMessage
    ) {
        std::vector<std::string> command = {
            "git", "-C", path.string()
        };
    
        command.insert(
            command.end(),
            arguments.begin(),
            arguments.end()
        );
    
        std::vector<char*> argv;
        for (auto& arg : command) {
            argv.push_back(arg.data());
        }
        argv.push_back(nullptr);
    
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            throw std::runtime_error("Failed to create Git output pipe");
        }
    
        const pid_t pid = fork();
    
        if (pid == -1) {
            close(pipefd[0]);
            close(pipefd[1]);
            throw std::runtime_error("Failed to start Git process");
        }
    
        if (pid == 0) {
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);
    
            execvp(argv[0], argv.data());
            _exit(127);
        }
    
        close(pipefd[1]);
    
        std::string output;
        char buffer[4096];
        ssize_t bytesRead;
    
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            output.append(buffer, static_cast<std::size_t>(bytesRead));
        }
    
        close(pipefd[0]);
    
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            throw std::runtime_error("Failed to wait for Git process");
        }
    
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            throw std::runtime_error(errorMessage);
        }
    
        return output;
    }
}

namespace git {

    void init(const std::filesystem::path& path) {
        runGit(path, {"init", "-b", "main"}, "Git initialization failed");
    }

    void add(
        const std::filesystem::path& path,
        const std::filesystem::path& file
    ) {
        runGit(path, {"add", file.string()}, "Git add failed");
    }
    
    

    void commit(
        const std::filesystem::path& path,
        const std::string& message
    ) {
        runGit(path, {"commit", "-m", message}, "Git commit failed");
    }
    void checkout(
        const std::filesystem::path& path,
        const std::string& branch
    ) {
        runGit(
            path,
            {"checkout", branch},
            "Git checkout failed: " + branch
        );
    }
  
    std::string currentBranch(const std::filesystem::path& path) {
        std::string branch = runGitOutput(
            path,
            {"branch", "--show-current"},
            "Failed to get current Git branch"
        );

        if (!branch.empty() && branch.back() == '\n') {
            branch.pop_back();
        }

        return branch;
    }


std::vector<std::string> branches(const std::filesystem::path& path) {
    std::string output = runGitOutput(
        path,
        {"branch", "--format=%(refname:short)"},
        "Failed to list Git branches"
    );

    std::vector<std::string> result;
    std::string branch;

    for (char character : output) {
        if (character == '\n') {
            if (!branch.empty()) {
                result.push_back(branch);
                branch.clear();
            }
        } else {
            branch += character;
        }
    }

    if (!branch.empty()) {
        result.push_back(branch);
    }

    return result;
}

std::vector<std::string> status(const std::filesystem::path& path) {
    std::string output = runGitOutput(
        path,
        {"status", "--short"},
        "Failed to get Git status"
    );

    std::vector<std::string> result;
    std::string line;

    for (char character : output) {
        if (character == '\n') {
            if (!line.empty()) {
                result.push_back(line);
                line.clear();
            }
        } else {
            line += character;
        }
    }

    if (!line.empty()) {
        result.push_back(line);
    }

    if (result.empty()) {
        result.push_back("Working tree clean.");
    }

    return result;
}




}

