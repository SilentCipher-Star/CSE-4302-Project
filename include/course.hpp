#ifndef COURSE_H
#define COURSE_H

#include <QString>
#include <iostream>

class Course
{
private:
    int id;
    QString code;
    QString name;
    int teacherId;
    int semester;
    int credits;

public:
    Course(int id, QString code, QString name, int teacherId, int semester, int credits);

    int getId() const { return id; }
    QString getCode() const { return code; }
    QString getName() const { return name; }
    int getTeacherId() const { return teacherId; }
    int getSemester() const { return semester; }
    int getCredits() const { return credits; }

    friend std::ostream &operator<<(std::ostream &os, const Course &c);
};

#endif // COURSE_H