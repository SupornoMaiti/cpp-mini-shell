#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace std;

int main()
{
    string ip;
    vector<string> tokens;

    while (true)
    {
        cout << "user@myShell> ";
        getline(cin, ip);
        if (ip == "exit")
            break;
        else
        {
            stringstream ss(ip);
            string token;
            tokens.clear();
            while (ss >> token)
            {
                tokens.push_back(token);
            }

            // Solving the "cd" problem
            if (tokens[0] == "cd")
            {
                if (tokens.size() < 2)
                {
                    perror("Error: Provide a valid path");
                }
                else
                {
                    if (chdir(tokens[1].data()) != 0)
                    {
                        perror("Error: cd failed");
                    }
                }
                continue; // path successfully changed, Don't need to run the loop anymore.
            }

            // Output Redirection
            string redirected_file_name = "";
            bool flag_to_continue = false;
            for (int i = 0; i < tokens.size(); ++i)
            {
                if (tokens[i] == ">")
                {
                    if (i + 1 < tokens.size() && (tokens.size() - (i + 1)) == 1)
                    {
                        redirected_file_name = tokens[i + 1];
                        tokens.resize(i);
                        break;
                    }
                    else
                    {
                        cout << "Error: No output file specified !" << endl;
                        flag_to_continue = true;
                        break;
                    }
                }
            }
            if (flag_to_continue)
            {
                continue;
            }

            // converting the tokens vector int a list of pointers storing the same token/s.
            vector<char *> c_pointers;
            for (auto &it : tokens)
            {
                c_pointers.push_back(it.data());
            }
            c_pointers.push_back(nullptr); // pushing the null pointer.

            // creating parent and child process
            pid_t pid = fork(); // making the parent and child process
            if (pid < 0)
            {
                cout << "Error: Process not created.";
            }
            else if (pid == 0)
            {
                // This is child process
                // Redirection
                if (redirected_file_name != "")
                {
                    int fd = open(redirected_file_name.data(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd < 0)
                    {
                        perror("Error: Cannot open redirection file");
                        exit(1);
                    }
                    else
                    {
                        dup2(fd, 1);
                        close(fd);
                    }
                }
                execvp(c_pointers[0], c_pointers.data());
                // if the execvp fails....
                perror("Error: Command not found");
                exit(1);
            }
            else
            {
                // waiting for the child process to be dead.
                wait(NULL);
            }
            // cout << endl;
        }
    }
    return 0;
}