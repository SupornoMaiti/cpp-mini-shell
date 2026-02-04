#include <string>
#include <iostream>
#include <sstream>
#include <vector>

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
            for (auto it : tokens)
            {
                cout << it << "^";
            }
            cout << endl;
        }
    }
    return 0;
}