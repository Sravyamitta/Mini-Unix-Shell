#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

using namespace std;

vector<string> tokenize(const string &input) {
    stringstream ss(input);
    vector<string> tokens;
    string token;
    while (ss >> token)
        tokens.push_back(token);
    return tokens;
}

vector<char*> toCharArray(vector<string> &args) {
    vector<char*> result;
    for (auto &arg : args)
        result.push_back(const_cast<char*>(arg.c_str()));
    result.push_back(nullptr);
    return result;
}

void executeSimple(vector<string> args, bool background) {
    pid_t pid = fork();

    if (pid == 0) {
        auto c_args = toCharArray(args);
        execvp(c_args[0], c_args.data());
        perror("exec failed");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0) {
        if (!background)
            waitpid(pid, nullptr, 0);
    }
    else {
        perror("fork failed");
    }
}

void executePipe(vector<string> left, vector<string> right) {
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return;
    }

    pid_t pid1 = fork();

    if (pid1 == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        auto c_args = toCharArray(left);
        execvp(c_args[0], c_args.data());
        perror("exec failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();

    if (pid2 == 0) {
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

int main() {
    string input;

    while (true) {
        cout << "myshell> ";
        getline(cin, input);

        if (input.empty())
            continue;

        vector<string> tokens = tokenize(input);

        if (tokens[0] == "exit")
            break;

        if (tokens[0] == "cd") {
            if (tokens.size() < 2)
                cerr << "cd: missing argument\n";
            else if (chdir(tokens[1].c_str()) != 0)
                perror("cd failed");
            continue;
        }

        bool background = false;
        if (tokens.back() == "&") {
            background = true;
            tokens.pop_back();
        }

        auto pipePos = find(tokens.begin(), tokens.end(), "|");

        if (pipePos != tokens.end()) {
            vector<string> left(tokens.begin(), pipePos);
            vector<string> right(pipePos + 1, tokens.end());
            executePipe(left, right);
        } else {
            executeSimple(tokens, background);
        }
    }

    return 0;
}
