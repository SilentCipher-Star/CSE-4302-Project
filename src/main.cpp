#include "../raylib.h"
#include <iostream>
#include <string>
#include <fstream>
#include <memory>
#include "App/App.hpp"
using namespace std;

int main()
{
    cout << "\033[2J\033[1;1H"; // clears terminal
    cout << "Hello, World!" << endl;
    App app;
    app.run();
}