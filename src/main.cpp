#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

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
            // converting the tokens vector int a list of pointers storing the same token/s.
            vector<char *> c_pointers;
            for (auto &it : tokens)
            {
                c_pointers.push_back(it.data());
            }
            // pushing the null pointer.
            c_pointers.push_back(nullptr);
            // creating parent and child process
            pid_t pid = fork(); // making the parent and child process
            if (pid < 0)
            {
                cout << "Error: Process not created.";
            }
            else if (pid == 0)
            {
                // This is child process
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
            cout << endl;
        }
    }
    return 0;
}