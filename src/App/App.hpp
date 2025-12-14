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
    void saveUser(const std::string& type, const std::string& username, const std::string& password);

public:
    App();
    void run();
};