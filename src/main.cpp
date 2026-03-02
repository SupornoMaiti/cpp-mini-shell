#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

class Shell
{
private:
    vector<string> tokenize(const string &ip)
    {
        vector<string> tokens;
        stringstream ss(ip);
        string token;
        while (ss >> token)
        {
            tokens.push_back(token);
        }
        return tokens;
    }

    vector<char *> tokens_to_c_pointers(vector<string> &tokens)
    {
        vector<char *> c_pointers;
        for (auto &token : tokens)
        {
            c_pointers.push_back(const_cast<char *>(token.c_str()));
        }
        c_pointers.push_back(nullptr);
        return c_pointers;
    }

    bool handle_cd(vector<string> &tokens)
    {
        if (!tokens.empty() && tokens[0] == "cd")
        {
            if (tokens.size() < 2)
            {
                perror("Error: Provide a valid path");
            }
            else if (chdir(tokens[1].c_str()) != 0)
            {
                perror("Error: cd failed");
            }
            return true;
        }
        return false;
    }

    string redirection(vector<string> &tokens)
    {
        for (int i = 0; i < static_cast<int>(tokens.size()); i++)
        {
            if (tokens[i] == ">")
            {
                if (i + 1 < static_cast<int>(tokens.size()) && (tokens.size() - (i + 1)) == 1)
                {
                    string redirected_file_name = tokens[i + 1];
                    tokens.resize(i);
                    return redirected_file_name;
                }

                cout << "Error: Invalid Redirection Syntax" << endl;
                tokens.clear();
                return "";
            }
        }
        return "";
    }

    void execute_command(vector<string> &tokens)
    {
        string redirected_file_name = redirection(tokens);
        if (tokens.empty())
        {
            return;
        }

        vector<char *> c_args = tokens_to_c_pointers(tokens);

        pid_t pid = fork();
        if (pid < 0)
        {
            cout << "Error: Cannot Execute Command." << endl;
            return;
        }

        if (pid == 0)
        {
            if (!redirected_file_name.empty())
            {
                int fd = open(redirected_file_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0)
                {
                    perror("Error: Cannot open redirection file");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            execvp(c_args[0], c_args.data());
            perror("Error: Command not found");
            exit(1);
        }

        wait(nullptr);
    }

public:
    void run()
    {
        string ip;

        while (true)
        {
            cout << "user@myShell> ";
            if (!getline(cin, ip))
            {
                break;
            }

            if (ip.empty())
            {
                continue;
            }

            vector<string> tokens = tokenize(ip);
            if (tokens.empty())
            {
                continue;
            }

            if (tokens[0] == "exit")
            {
                break;
            }

            if (handle_cd(tokens))
            {
                continue;
            }

            execute_command(tokens);
        }
    }
};

int main()
{
    Shell myShell;
    myShell.run();
    return 0;
}
