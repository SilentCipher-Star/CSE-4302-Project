#pragma once

#include <QString>
#include <QVector>
#include <QDebug>
#include "routine.hpp"

class Notice
{
private:
    QString date;
    QString author;
    QString content;

public:
    Notice() = default;
    Notice(QString d, QString a, QString c) : date(d), author(a), content(c) {}
    QString getDate() const { return date; }
    QString getAuthor() const { return author; }
    QString getContent() const { return content; }
};

class Task
{
private:
    int id;
    QString description;
    bool isCompleted;

public:
    Task(int id, QString desc, bool completed) : id(id), description(desc), isCompleted(completed) {}
    int getId() const { return id; }
    QString getDescription() const { return description; }
    bool getIsCompleted() const { return isCompleted; }
    void setCompleted(bool val) { isCompleted = val; }

    bool operator==(const Task &other) const
    {
        return this->id == other.id && this->description == other.description &&
               this->isCompleted == other.isCompleted;
    }
};

class Assessment
{
private:
    int id;
    int courseId;
    QString courseName;
    QString title;
    QString type;
    QString date;
    int maxMarks;

public:
    Assessment(int id, int cid, QString cname, QString t, QString type, QString d, int max)
        : id(id), courseId(cid), courseName(cname), title(t), type(type), date(d), maxMarks(max) {}

    int getId() const { return id; }
    int getCourseId() const { return courseId; }
    QString getCourseName() const { return courseName; }
    QString getTitle() const { return title; }
    QString getType() const { return type; }
    QString getDate() const { return date; }
    int getMaxMarks() const { return maxMarks; }

    friend QDebug operator<<(QDebug dbg, const Assessment &a)
    {
        dbg.nospace() << "Assessment(id=" << a.id
                      << ", title=" << a.title
                      << ", course=" << a.courseName
                      << ", date=" << a.date
                      << ", maxMarks=" << a.maxMarks << ")";
        return dbg.space();
    }
};

class AttendanceRecord
{
private:
    QString courseName;
    int totalClasses;
    int attendedClasses;
    double totalMarksObtained;
    double totalMaxMarks;

public:
    AttendanceRecord(QString cname, int total, int attended, double marks, double max)
        : courseName(cname), totalClasses(total), attendedClasses(attended), totalMarksObtained(marks), totalMaxMarks(max) {}

    QString getCourseName() const { return courseName; }
    int getTotalClasses() const { return totalClasses; }
    int getAttendedClasses() const { return attendedClasses; }
    double getTotalMarksObtained() const { return totalMarksObtained; }
    double getTotalMaxMarks() const { return totalMaxMarks; }
};

struct AttendanceAnalytics
{
    int studentId;
    QString studentName;
    int courseId;
    QString courseName;
    int totalClasses;
    int attendedClasses;
    double percentage;
};

class Query
{
private:
    int id;
    int studentId;
    int teacherId;
    QString studentName;
    QString teacherName;
    QString question;
    QString answer;
    QString timestamp;

public:
    Query(int id, int sid, int tid, QString sname, QString tname, QString q, QString a, QString ts)
        : id(id), studentId(sid), teacherId(tid), studentName(sname), teacherName(tname), question(q), answer(a), timestamp(ts) {}

    int getId() const { return id; }
    int getStudentId() const { return studentId; }
    int getTeacherId() const { return teacherId; }
    QString getStudentName() const { return studentName; }
    QString getTeacherName() const { return teacherName; }
    QString getQuestion() const { return question; }
    QString getAnswer() const { return answer; }
    QString getTimestamp() const { return timestamp; }
};

class PersonalMessage
{
private:
    int id;
    int senderId;
    QString senderRole;
    QString senderName;
    int receiverId;
    QString subject;
    QString content;
    QString timestamp;
    bool isRead;

public:
    PersonalMessage(int id, int sid, QString srole, QString sname, int rid,
                    QString subj, QString cont, QString ts, bool read)
        : id(id), senderId(sid), senderRole(srole), senderName(sname),
          receiverId(rid), subject(subj), content(cont), timestamp(ts), isRead(read) {}

    int getId() const { return id; }
    int getSenderId() const { return senderId; }
    QString getSenderRole() const { return senderRole; }
    QString getSenderName() const { return senderName; }
    int getReceiverId() const { return receiverId; }
    QString getSubject() const { return subject; }
    QString getContent() const { return content; }
    QString getTimestamp() const { return timestamp; }
    bool getIsRead() const { return isRead; }
};

class LostFoundPost
{
private:
    int id;
    int posterId;
    QString posterName;
    QString posterRole;
    QString type; // "LOST" or "FOUND"
    QString itemName;
    QString description;
    QString location;
    QString date;
    QString status; // "OPEN" or "CLAIMED"
    QString claimedBy;

public:
    LostFoundPost(int id, int posterId, QString posterName, QString posterRole,
                  QString type, QString itemName, QString description,
                  QString location, QString date, QString status, QString claimedBy)
        : id(id), posterId(posterId), posterName(posterName), posterRole(posterRole),
          type(type), itemName(itemName), description(description),
          location(location), date(date), status(status), claimedBy(claimedBy) {}

    int getId() const { return id; }
    int getPosterId() const { return posterId; }
    QString getPosterName() const { return posterName; }
    QString getPosterRole() const { return posterRole; }
    QString getType() const { return type; }
    QString getItemName() const { return itemName; }
    QString getDescription() const { return description; }
    QString getLocation() const { return location; }
    QString getDate() const { return date; }
    QString getStatus() const { return status; }
    QString getClaimedBy() const { return claimedBy; }
};

struct RescheduleOption
{
    QString displayText;
    RoutineAdjustment adjustment;
    RoutineAdjustment secondaryAdjustment;
};