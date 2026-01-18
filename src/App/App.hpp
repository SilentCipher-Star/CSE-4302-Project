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
    void saveUser(const std::shared_ptr<Person> &user);
    void saveAllUsers();
    void handleAdmin();

public:
    App();
    void run();
};