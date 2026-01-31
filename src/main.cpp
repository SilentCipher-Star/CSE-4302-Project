// #include "../raylib.h"
#include <iostream>
#include <string>
#include <fstream>
#include <memory>
#include "App/App.hpp"
#include "Core/Input.hpp"
using namespace std;

int main()
{
    clearScreen();
    cout << "Welcome to Acadence" << endl;
    App app;
    app.run();
}