#pragma once
#include <vector>
#include <memory>
#include "Person.hpp";
#include "User.hpp"
using namespace std;

class App
{
private:
    shared_ptr<Person> admin;
    vector<shared_ptr<User>> users;
    bool isRunning;

    void loadUsers();

public:
    App();
    void run();
};