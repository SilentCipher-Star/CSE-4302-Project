#pragma once
#include <vector>
#include <memory>
#include "Person.hpp"

class App
{
private:
    std::vector<std::shared_ptr<Person>> users;
    bool isRunning;

    void loadUsers();

public:
    App();
    void run();
};