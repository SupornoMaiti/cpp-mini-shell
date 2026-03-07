#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

struct redirection_data
{
    string input_file = "";
    string output_file = "";
    bool append = false;
};

class Shell
{
private:
    static vector<string> tokenize(const string &ip)
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

    static vector<char *> tokens_to_c_pointers(const vector<string> &tokens)
    {
        vector<char *> c_pointers;
        for (auto &token : tokens)
        {
            c_pointers.push_back(const_cast<char *>(token.c_str()));
        }
        c_pointers.push_back(nullptr);
        return c_pointers;
    }

    static bool handle_cd(const vector<string> &tokens)
    {
        if (!tokens.empty() && tokens[0] == "cd")
        {
            if (tokens.size() < 2)
            {
                cout << "Error: Provide a valid path" << endl;
            }
            else if (chdir(tokens[1].c_str()) != 0)
            {
                perror("Error: cd failed");
            }
            return true;
        }
        return false;
    }

    static redirection_data redirection(vector<string> &tokens)
    {
        redirection_data data;
        vector<string> clean_tokens;
        for (size_t i = 0; i < tokens.size(); i++)
        {
            if (tokens[i] == "<")
            {
                if (i + 1 < tokens.size())
                {
                    data.input_file = tokens[i + 1];
                    i++;
                }
                else
                {
                    cout << "Error: Invalid redirection syntax" << endl;
                    tokens.clear();
                    return data;
                }
            }
            else if (tokens[i] == ">")
            {
                if (i + 1 < tokens.size())
                {
                    data.output_file = tokens[i + 1];
                    i++;
                }
                else
                {
                    cout << "Error: Invalid redirection syntax" << endl;
                    tokens.clear();
                    return data;
                }
            }
            else if (tokens[i] == ">>")
            {
                if (i + 1 < tokens.size())
                {
                    data.append = true;
                    data.output_file = tokens[i + 1];
                    i++;
                }
                else
                {
                    cout << "Error: Invalid redirection syntax" << endl;
                    tokens.clear();
                    return data;
                }
            }
            else
                clean_tokens.emplace_back(tokens[i]);
        }
        tokens = clean_tokens;
        return data;
    }

    static void execute_command(vector<string> &tokens)
    {
        redirection_data data;
        data = redirection(tokens);
        if (tokens.empty())
        {
            return;
        }

        const vector<char *> c_args = tokens_to_c_pointers(tokens);

        const pid_t pid = fork();
        if (pid < 0)
        {
            cout << "Error: Cannot Execute Command." << endl;
            return;
        }

        if (pid == 0)
        {
            if (!data.output_file.empty())
            {
                // handled both o/p redirection and append
                int flags = O_WRONLY | O_CREAT | (data.append ? O_APPEND : O_TRUNC);
                int fd = open(data.output_file.c_str(), flags, 0644);
                if (fd < 0)
                {
                    perror("Error: Cannot open redirection file");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            if (!data.input_file.empty())
            {
                int fd = open(data.input_file.c_str(), O_RDONLY);
                if (fd < 0)
                {
                    perror("Error: Cannot open redirection file");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            execvp(c_args[0], c_args.data());
            perror("Error: Command not found");
            exit(1);
        }

        wait(nullptr);
    }

    static string get_prompt()
    {
        // get username.
        string uname = "user";
        if (const char *user = getenv("USER"); user != nullptr)
            uname = user;

        // get hostname
        char host_buffer[256];
        string host_name = "localhost";
        if (gethostname(host_buffer, sizeof(host_buffer)) == 0)
        {
            host_name = host_buffer;
        }

        // get current working directory
        char cwd_buffer[1024];
        string cwd;
        if (getcwd(cwd_buffer, sizeof(cwd_buffer)) != nullptr)
        {
            cwd = cwd_buffer;
        }

        return "\033[1;32m" + uname + "@" + host_name + "\033[0m:\033[1;34m" + cwd + "\033[0m$ ";
    }

public:
    void run()
    {
        string ip;

        while (true)
        {
            cout << get_prompt();
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
