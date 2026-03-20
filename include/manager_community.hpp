#pragma once

#include <QString>
#include <QVector>
#include <QPair>
#include "models.hpp"

class ManagerCommunity
{
public:
    // Notices
    static QVector<Notice> getNotices();
    static void addNotice(const QString &content, const QString &author);
    static bool updateNotice(const QString &date, const QString &author, const QString &oldContent, const QString &newContent);
    static bool deleteNotice(const QString &date, const QString &author, const QString &content);

    // Messages
    static QVector<PersonalMessage> getMessages(int userId);
    static int getUnreadCount(int userId);
    static void sendMessage(int senderId, const QString &senderRole, int receiverId,
                            const QString &subject, const QString &content);
    static void markRead(int messageId);
    static void deleteMessage(int messageId);

    // Queries
    static QVector<Query> getQueries(int userId, QString role);
    static void addQuery(int userId, int teacherId, QString question);
    static void answerQuery(int queryId, QString answer);
    static QVector<QPair<int, QString>> getTeacherList();

    // Lost & Found
    static QVector<LostFoundPost> getPosts();
    static void addPost(int posterId, const QString &posterName, const QString &posterRole,
                        const QString &type, const QString &itemName, const QString &description,
                        const QString &location);
    static void claimPost(int postId, const QString &claimerName);
    static void deletePost(int postId);
};