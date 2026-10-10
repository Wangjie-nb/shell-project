#include "shell.h"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstring>
#include <vector>

#define MAX_ARGS 64

Shell::Shell() : running(true) {}

Shell::~Shell() {}

std::vector<std::string> Shell::split(const std::string& s, char delim) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delim)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

bool Shell::handleBuiltin(const std::vector<std::string>& args) {
    if (args[0] == "cd") {
        if (args.size() < 2) {
            chdir(getenv("HOME"));
        } else {
            if (chdir(args[1].c_str()) < 0) {
                perror("cd");
            }
        }
        return true;
    } else if (args[0] == "exit" || args[0] == "quit") {
        running = false;
        return true;
    } else if (args[0] == "help") {
        std::cout << "支持命令：" << std::endl;
        std::cout << "  cd <目录>    切换目录" << std::endl;
        std::cout << "  exit         退出Shell" << std::endl;
        std::cout << "  cmd1 | cmd2  管道" << std::endl;
        std::cout << "  cmd > 文件   输出重定向" << std::endl;
        std::cout << "  cmd < 文件   输入重定向" << std::endl;
        std::cout << "  cmd &        后台运行" << std::endl;
        return true;
    }
    return false;
}

void Shell::executeCommand(const std::vector<std::string>& args, 
                          bool inputRedirect, const std::string& inputFile,
                          bool outputRedirect, const std::string& outputFile) {
    char* argv[MAX_ARGS];
    for (int i = 0; i < args.size(); i++) {
        argv[i] = (char*)args[i].c_str();
    }
    argv[args.size()] = nullptr;

    if (inputRedirect) {
        int fd = open(inputFile.c_str(), O_RDONLY);
        if (fd < 0) {
            perror("open input file");
            exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    if (outputRedirect) {
        int fd = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            perror("open output file");
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    execvp(argv[0], argv);
    perror("execvp");
    exit(1);
}

void Shell::run() {
    char cwd[1024];

    while (running) {
        getcwd(cwd, sizeof(cwd));
        std::cout << "\033[32mmyshell\033[0m:\033[34m" << cwd << "\033[0m$ ";

        std::string line;
        std::getline(std::cin, line);
        if (line.empty()) continue;

        bool background = false;
        if (!line.empty() && line.back() == '&') {
            background = true;
            line = line.substr(0, line.size() - 1);
        }

        std::vector<std::string> pipeCmds = split(line, '|');
        int pipeCount = pipeCmds.size();

        int pipefd[2];
        int prevPipeRead = -1;

        for (int i = 0; i < pipeCount; i++) {
            std::string cmd = pipeCmds[i];

            bool inputRedirect = false, outputRedirect = false;
            std::string inputFile, outputFile;
            std::string cleanCmd = cmd;

            size_t outPos = cmd.find('>');
            if (outPos != std::string::npos) {
                outputRedirect = true;
                outputFile = cmd.substr(outPos + 1);
                outputFile.erase(0, outputFile.find_first_not_of(' '));
                outputFile.erase(outputFile.find_last_not_of(' ') + 1);
                cleanCmd = cmd.substr(0, outPos);
            }

            size_t inPos = cmd.find('<');
            if (inPos != std::string::npos) {
                inputRedirect = true;
                inputFile = cmd.substr(inPos + 1);
                inputFile.erase(0, inputFile.find_first_not_of(' '));
                inputFile.erase(inputFile.find_last_not_of(' ') + 1);
                cleanCmd = cmd.substr(0, inPos);
            }

            std::vector<std::string> args = split(cleanCmd, ' ');
            if (args.empty()) continue;

            if (handleBuiltin(args)) {
                prevPipeRead = -1;
                continue;
            }

            if (i < pipeCount - 1) {
                pipe(pipefd);
            }

            pid_t pid = fork();
            if (pid == 0) {
                if (prevPipeRead != -1) {
                    dup2(prevPipeRead, STDIN_FILENO);
                    close(prevPipeRead);
                }

                if (i < pipeCount - 1) {
                    close(pipefd[0]);
                    dup2(pipefd[1], STDOUT_FILENO);
                    close(pipefd[1]);
                }

                executeCommand(args, inputRedirect, inputFile, outputRedirect, outputFile);
            }

            if (prevPipeRead != -1) {
                close(prevPipeRead);
            }
            if (i < pipeCount - 1) {
                close(pipefd[1]);
                prevPipeRead = pipefd[0];
            }

            if (!background && i == pipeCount - 1) {
                waitpid(pid, nullptr, 0);
            }
        }

        if (background) {
            std::cout << "[后台进程已启动]" << std::endl;
        }
    }

    std::cout << "退出Shell" << std::endl;
}
