#pragma once

#include <QString>
#include <QVector>
#include "appmanager.hpp"

class ManagerLostFound
{
public:
    static QVector<LostFoundPost> getPosts();
    static void addPost(int posterId, const QString &posterName, const QString &posterRole,
                        const QString &type, const QString &itemName, const QString &description,
                        const QString &location);
    static void claimPost(int postId, const QString &claimerName);
    static void deletePost(int postId);
};
