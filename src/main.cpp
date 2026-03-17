#include <array>
#include <string>
#include <iostream>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

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

    static string expand_tilde(const string& path) {
        if (path.empty() ||  path[0] != '~') return path;

        const char* home = getenv("HOME");
        if (home == nullptr) return  path;
        return string(home) + path.substr(1);
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
            else{
                string path = expand_tilde(tokens[1]);
                if (chdir(path.c_str()) != 0) {
                    perror("Error: cd failed");
                }
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

    static void exec_in_child(vector<string> tokens) {
        //Signal Handling
        struct sigaction not_ignore{};
        not_ignore.sa_handler = SIG_DFL;
        sigemptyset(&not_ignore.sa_mask);
        not_ignore.sa_flags = 0;

        sigaction(SIGINT,&not_ignore,nullptr);
        sigaction(SIGTSTP,&not_ignore,nullptr);
        //---------------
        RedirectionData data = redirection(tokens);
        if (tokens.empty()) exit(1);
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

        const vector<char *> c_args = tokens_to_c_pointers(tokens);
        execvp(c_args[0], c_args.data());
        perror("Error: Command not found");
        exit(1);
    }

    static void execute_command(const vector<string> &tokens)
    {
        const pid_t pid = fork();
        if (pid < 0)
        {
            cout << "Error: Cannot Execute Command." << endl;
            return;
        }

        if (pid == 0)
        {
            exec_in_child(tokens);
        }

        int process_status; //Process Status stored for later use.
        waitpid(pid, &process_status, WUNTRACED);
        if (WIFSTOPPED(process_status)) {
            write(STDERR_FILENO,"\n",1);
        }
    }

    static void execute_pipeline(const vector<vector<string>> &commands)
    {
        size_t n = commands.size();
        if (n == 0) return;
        vector<array<int, 2>> pipes(n-1);
        vector<pid_t> pids(n);
        //creating n-1 pipes upfront
        for (size_t i = 0; i < n-1; ++i) {
            if (pipe(pipes[i].data()) < 0) {
                perror("Error: Pipe creation failed.");
                return;
            }
        }
        //forking n processes
        for (size_t i = 0; i < n; ++i) {
            pids[i] = fork();
            if (pids[i] < 0) {
                perror("Error: Fork failed");
                for (const auto& pipe : pipes) {
                    close(pipe[0]);
                    close(pipe[1]);
                }
                return;
            }
            if (pids[i] == 0) {
                //i th child
                if (i > 0) {
                    // If it is not first process, read from previous pipe.
                    if(dup2(pipes[i - 1][0],STDIN_FILENO) < 0) {
                        perror("Error: dup2 failed");
                        exit(1);
                    }
                }
                if (i < n - 1) {
                    // If it is not last process, write to next pipe.
                    if(dup2(pipes[i][1],STDOUT_FILENO) < 0) {
                        perror("Error: dup2 failed.");
                        exit(1);
                    }
                }
                //As child inherits all the pipes from the parent need to close all.
                for (auto &j: pipes) {
                    close(j[0]); //Close all read ends.
                    close(j[1]); //close all write ends.
                }
                // Handle any file redirection (overrides pipe fds if specified) and execvp.
                exec_in_child(commands[i]);
            }
        }
        //closing all pipes in parent process
        for (auto & i : pipes) {
            close(i[0]);
            close(i[1]);
        }
        //waiting for all child processes;
        vector<int> process_statuses;//process statuses stored for future use.
        for (auto& i : pids) {
            int process_status;
            waitpid(i,&process_status,WUNTRACED);
            process_statuses.push_back(process_status);
            if (WIFSTOPPED(process_status)) {
                write(STDOUT_FILENO, "\n", 1);
            }
        }
    }

    static void home_dir(string& cwd) {
        const char* home_env= getenv("HOME");
        if (home_env == nullptr) {
            return ;
        }
        const string home = home_env;
        if (cwd.find(home) == 0) {
            if (cwd.length() == home.length() || cwd[home.length()] == '/') {
                cwd = "~" + cwd.substr(home.length());
            }
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
        home_dir(cwd);
        return "\033[1;32m" + uname + "@" + host_name + "\033[0m:\033[1;34m" + cwd + "\033[0m$ ";
    }

public:
    static void run()
    {
        //Signal Handling
        struct sigaction ign_sgnl{};
        ign_sgnl.sa_handler = SIG_IGN;
        sigemptyset(&ign_sgnl.sa_mask);
        ign_sgnl.sa_flags = 0;

        sigaction(SIGINT,&ign_sgnl,nullptr);
        sigaction(SIGTSTP,&ign_sgnl,nullptr);
        //---------------
        //History
        const string history_file_path = []() {
            const char* home = getenv("HOME");
            return home ? string(home) + "/.myshell_history" : ".myshell_history";
        }();
        using_history(); // Initialized History
        read_history(history_file_path.c_str()); // Loaded history from the disk to ram
        stifle_history(1000); //Limited history to 1000 lines.
        while (true)
        {
            char* raw_input = readline(get_prompt().c_str());
            if (raw_input == nullptr) {
                cout << "\n";
                break;
            }
            string ip(raw_input);
            free(raw_input);
            if (ip.empty()) continue;
            HIST_ENTRY* last = history_get(history_length);
            if (last == nullptr || ip != last->line) {
                add_history(ip.c_str());
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
        write_history(history_file_path.c_str());
    }
};

int main()
{
    //Shell myShell;
    Shell::run();
    return 0;
}
