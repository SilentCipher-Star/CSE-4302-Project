#pragma once
#include <QVector>
#include "student.hpp"

class StudentIterator
{
public:
    explicit StudentIterator(const QVector<Student *> &students)
        : m_students(students), m_index(0) {}

    bool hasNext() const { return m_index < m_students.size(); }
    Student *next() { return m_students[m_index++]; }
    void reset() { m_index = 0; }
    int count() const { return m_students.size(); }

private:
    const QVector<Student *> &m_students;
    int m_index;
};

class StudentCollection
{
public:
    void add(Student *s) { m_students.append(s); }
    StudentIterator createIterator() const { return StudentIterator(m_students); }
    int size() const { return m_students.size(); }

private:
    QVector<Student *> m_students;
};
