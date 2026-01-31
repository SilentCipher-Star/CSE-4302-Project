#include "Input.hpp"
#include <iostream>
#include <limits>
#include <string>
#include <regex>

using namespace std;

int getInt()
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

int getIntInRange(int min, int max)
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

bool isValidCredential(const string &s)
{
    static const regex pattern("^[a-zA-Z0-9._\\-@#$!]+$");
    return regex_match(s, pattern);
}