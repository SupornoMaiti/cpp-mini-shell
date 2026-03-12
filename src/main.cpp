#include <array>
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
    struct RedirectionData
    {
        string input_file;
        string output_file;
        bool append = false;
    };

    static vector<string> tokenize(const string &ip)
    {
        vector<string> tokens;
        string current_token;
        bool in_quotes = false;
        for (const char &c : ip)
        {
            if (c == '"')
            {
                in_quotes = !in_quotes;
                continue;
            }
            else if (c == ' ')
            {
                if (in_quotes) // space in between quotes
                {
                    current_token.push_back(c);
                }
                // a single token is complete
                else
                {
                    if (!current_token.empty())
                    {
                        tokens.push_back(current_token);
                        current_token = "";
                    }
                }
            }
            else
                current_token.push_back(c);
        }
        if (in_quotes)
        {
            cout << "Error: Unclosed quotes." << endl;
            tokens.clear();
            return tokens;
        }
        if (!current_token.empty())
            tokens.push_back(current_token);
        return tokens;
    }

    static vector<vector<string>> split_by_pipe(const vector<string> &tokens)
    {
        vector<vector<string>> split_tokens;
        vector<string> current_tokens;
        for (const string &s : tokens)
        {
            if (s == "|")
            {
                if (current_tokens.empty())
                {
                    cout << "Error: Invalid Syntax." << endl;
                    return {};
                }
                split_tokens.push_back(current_tokens);
                current_tokens.clear();
                continue;
            }
            else
            {
                current_tokens.push_back(s);
            }
        }
        if (current_tokens.empty())
        {
            cout << "Error: Invalid Syntax." << endl;
            return {};
        }
        else
        {
            split_tokens.push_back(current_tokens);
        }
        return split_tokens;
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
            else if (tokens.size() > 2)
            {
                cout << "Error: cd takes only one argument" << endl;
            }
            else if (chdir(tokens[1].c_str()) != 0)
            {
                perror("Error: cd failed");
            }
            return true;
        }
        return false;
    }

    static RedirectionData redirection(vector<string> &tokens)
    {
        RedirectionData data;
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
                    return RedirectionData{};
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
                    return RedirectionData{};
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
                    return RedirectionData{};
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
        RedirectionData data = redirection(tokens);
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
                const auto flags = O_WRONLY | O_CREAT | (data.append ? O_APPEND : O_TRUNC);
                const int fd = open(data.output_file.c_str(), flags, 0644);
                if (fd < 0)
                {
                    perror("Error: Cannot open redirection file");
                    exit(1);
                }
                if (dup2(fd, STDOUT_FILENO) < 0)
                {
                    perror("Error: dup2 failed");
                    exit(1);
                }
                close(fd);
            }
            if (!data.input_file.empty())
            {
                const int fd = open(data.input_file.c_str(), O_RDONLY);
                if (fd < 0)
                {
                    perror("Error: Cannot open redirection file");
                    exit(1);
                }
                if (dup2(fd, STDIN_FILENO) < 0)
                {
                    perror("Error: dup2 failed");
                    exit(1);
                }
                close(fd);
            }

            execvp(c_args[0], c_args.data());
            perror("Error: Command not found");
            exit(1);
        }

        int process_status; //Process Status stored for later use.
        waitpid(pid, &process_status, 0);
    }

    static void execute_pipeline(const vector<vector<string>> &commands)
    {
        size_t n = commands.size();
        if (n == 0) return;
        vector<array<int, 2>> fd(n-1);
        vector<pid_t> pids(n);
        //creating n-1 pipes upfront
        for (size_t i = 0; i < n-1; ++i) {
            if (pipe(fd[i].data()) < 0) {
                perror("Error: Pipe creation failed.");
            }
        }
        //forking n processes
        for (size_t i = 0; i < n; ++i) {
            pids[i] = fork();
            if (pids[i] == 0) {
                //i th child
                if (i > 0) {
                    // If it is not first process.
                    dup2(fd[i - 1][0],STDIN_FILENO);
                }
                if (i < n - 1) {
                    // If it is not last process
                    dup2(fd[i][1],STDOUT_FILENO);
                }
                //As child inherits all the pipes from the parent need to close all.
                for (auto &j: fd) {
                    close(j[0]); //Close all read ends.
                    close(j[1]); //close all write ends.
                }
                //Execute this child's command
                vector<string> cmd = commands[i]; //copying as the function below takes it by reference
                //If there is any redirection
                RedirectionData data = redirection((cmd));
                if (cmd.empty()) exit(1);
                if (!data.output_file.empty()) {
                    // handled both o/p redirection and append
                    const auto flags = O_WRONLY | O_CREAT | (data.append ? O_APPEND : O_TRUNC);
                    const int fd1 = open(data.output_file.c_str(), flags, 0644);
                    if (fd1 < 0)
                    {
                        perror("Error: Cannot open redirection file");
                        exit(1);
                    }
                    if (dup2(fd1, STDOUT_FILENO) < 0)
                    {
                        perror("Error: dup2 failed");
                        exit(1);
                    }
                    close(fd1);
                }
                if (!data.input_file.empty()) {
                    const int fd1 = open(data.input_file.c_str(), O_RDONLY);
                    if (fd1 < 0)
                    {
                        perror("Error: Cannot open redirection file");
                        exit(1);
                    }
                    if (dup2(fd1, STDIN_FILENO) < 0)
                    {
                        perror("Error: dup2 failed");
                        exit(1);
                    }
                    close(fd1);
                }
                //Executing the command
                vector<char*> c_args = tokens_to_c_pointers(cmd);
                execvp(c_args[0],c_args.data());
                perror("Error: Command not found");
                exit(1);
            }
        }
        //closing all pipes in parent process
        for (auto & i : fd) {
            close(i[0]);
            close(i[1]);
        }
        //waiting for all child processes;
        for (auto& i : pids) {
            waitpid(i,nullptr,0);
        }
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
    static void run()
    {
        string ip;

        while (true)
        {
            cout << get_prompt() << flush;
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
            //Splitting for checkin pipes
            vector<vector<string>> commands = split_by_pipe(tokens);
            if (commands.empty()) {
                continue;
            }
            //No pipes found
            if (commands.size() == 1) {
                execute_command(commands[0]);
            }
            //pipes found
            else {
                execute_pipeline(commands);
            }
        }
    }
};

int main()
{
    //Shell myShell;
    Shell::run();
    return 0;
}
