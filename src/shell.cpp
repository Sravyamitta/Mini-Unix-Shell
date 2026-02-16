#include <signal.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

using namespace std;

// Utility Functions 

// Split input string into tokens
vector<string> tokenize(const string &input) {
    stringstream ss(input);
    vector<string> tokens;
    string token;
    while (ss >> token)
        tokens.push_back(token);
    return tokens;
}

// Convert vector<string> to char* array for execvp
vector<char*> toCharArray(vector<string> &args) {
    vector<char*> result;
    for (auto &arg : args)
        result.push_back(const_cast<char*>(arg.c_str()));
    result.push_back(nullptr);
    return result;
}

// Signal Handling 

// Handle Ctrl+C in parent shell
void handleSigInt(int sig) {
    (void)sig;
    const char* msg = "\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

// Command Execution 

// Execute a simple command (no pipes)
void executeSimple(vector<string> args, bool background) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) {
        // Child process: restore default SIGINT behavior
        signal(SIGINT, SIG_DFL);

        auto c_args = toCharArray(args);
        execvp(c_args[0], c_args.data());

        perror("exec failed");
        exit(EXIT_FAILURE);
    } 
    else {
        // Parent
        if (!background)
            waitpid(pid, nullptr, 0);
    }
}

// Execute two commands connected by a pipe
void executePipe(vector<string> left, vector<string> right) {
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return;
    }

    pid_t pid1 = fork();

    if (pid1 < 0) {
        perror("fork failed");
        return;
    }

    if (pid1 == 0) {
        signal(SIGINT, SIG_DFL);

        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        auto c_args = toCharArray(left);
        execvp(c_args[0], c_args.data());

        perror("exec failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();

    if (pid2 < 0) {
        perror("fork failed");
        return;
    }

    if (pid2 == 0) {
        signal(SIGINT, SIG_DFL);

        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        close(pipefd[0]);

        auto c_args = toCharArray(right);
        execvp(c_args[0], c_args.data());

        perror("exec failed");
        exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);
}

// Main Shell Loop

int main() {
    // Install custom SIGINT handler for shell
    signal(SIGINT, handleSigInt);

    string input;

    while (true) {
        cout << "myshell> ";
        getline(cin, input);

        if (cin.eof())
            break;

        if (input.empty())
            continue;

        vector<string> tokens = tokenize(input);

        if (tokens.empty())
            continue;

        // Built-in: exit
        if (tokens[0] == "exit")
            break;

        // Built-in: cd
        if (tokens[0] == "cd") {
            if (tokens.size() < 2)
                cerr << "cd: missing argument\n";
            else if (chdir(tokens[1].c_str()) != 0)
                perror("cd failed");
            continue;
        }

        // Background execution
        bool background = false;
        if (!tokens.empty() && tokens.back() == "&") {
            background = true;
            tokens.pop_back();
        }

        // Pipe handling
        auto pipePos = find(tokens.begin(), tokens.end(), "|");

        if (pipePos != tokens.end()) {
            vector<string> left(tokens.begin(), pipePos);
            vector<string> right(pipePos + 1, tokens.end());
            executePipe(left, right);
        } 
        else {
            executeSimple(tokens, background);
        }
    }

    return 0;
}
