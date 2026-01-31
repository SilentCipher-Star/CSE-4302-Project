#pragma once
#include "../App/Person.hpp"
#include "../App/User.hpp"
#include <vector>
#include <memory>

#include <iostream>

class Admin : public Person
{
private:
    vector<shared_ptr<User>> &users;

public:
    Admin(vector<shared_ptr<User>> &users);
    ~Admin() = default;
    void showMenu() override;
    bool check_existing_user(string username, vector<shared_ptr<User>> &users);
    void saveUser(shared_ptr<User> user);
};