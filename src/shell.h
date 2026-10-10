#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <vector>

class Shell {
private:
    bool running;

    std::vector<std::string> split(const std::string& s, char delim);
    void executeCommand(const std::vector<std::string>& args, 
                        bool inputRedirect, const std::string& inputFile,
                        bool outputRedirect, const std::string& outputFile);
    bool handleBuiltin(const std::vector<std::string>& args);

public:
    Shell();
    ~Shell();

    void run();
};

#endif
