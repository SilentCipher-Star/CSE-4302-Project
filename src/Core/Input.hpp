#pragma once
#include <iostream>
#include <limits>
#include <string>
#include <regex>
#include <functional>

#define min_password_length 5
#define max_password_length 20

using namespace std;

inline void clearScreen()
{
    cout << "\033[2J\033[1;1H";
}

inline int getInt()
{
    int x;
    while (!(cin >> x))
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! Choice must be an Integer.\nEnter choice: ";
    }
    return x;
}

inline int getIntInRange(int min, int max)
{
    int x;
    while (true)
    {
        x = getInt();
        if (x >= min && x <= max)
            return x;
        cout << "Enter a value between " << min << " and " << max << ": ";
    }
}

inline bool isValidInput(const string &s)
{
    static const regex pattern("^[a-zA-Z0-9._\\-@#$!]+$");
    return regex_match(s, pattern);
}

inline void getValidInput(string &input, const string &prompt, const function<bool(const string &)> &validator)
{
    while (true)
    {
        cout << prompt;
        cin >> input;
        if (validator(input))
        {
            break;
        }
    }
}

inline void inputPassword(string &password)
{
    getValidInput(password, "Enter password: ", [](const string &input) -> bool
                  {
        if (!isValidInput(input))
        {
            cout << "Invalid Characters present" << endl;
            return false;
        }
        if (input.length() < min_password_length)
        {
            cout << "Password must be at least " << min_password_length << " characters long" << endl;
            return false;
        }
        if (input.length() > max_password_length)
        {
            cout << "Password must be at most " << max_password_length << " characters long" << endl;
            return false;
        }
        return true; });
}