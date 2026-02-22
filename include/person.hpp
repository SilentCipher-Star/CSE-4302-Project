#ifndef PERSON_H
#define PERSON_H

#include <QString>

class Person
{
protected:
    int id;
    QString name;
    QString email;
    QString username;
    QString password;

    static int personCount;

public:
    Person(int id, QString name, QString email, QString username = "", QString password = "");
    virtual ~Person();
    virtual QString getRole() const = 0;

    int getId() const { return id; }
    QString getName() const { return name; }
    QString getEmail() const { return email; }
    QString getUsername() const { return username; }
    QString getPassword() const { return password; }

    void setUsername(const QString &u) { username = u; }
    void setPassword(const QString &p) { password = p; }

    static int getPersonCount();
};

#endif // PERSON_H