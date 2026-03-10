#pragma once

#include <QString>
#include <QVector>
#include "appmanager.hpp"

class ManagerMessages
{
public:
    static QVector<PersonalMessage> getMessages(int userId);
    static int getUnreadCount(int userId);
    static void sendMessage(int senderId, const QString &senderRole, int receiverId,
                            const QString &subject, const QString &content);
    static void markRead(int messageId);
    static void deleteMessage(int messageId);
};
