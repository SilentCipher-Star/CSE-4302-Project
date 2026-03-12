#pragma once
#include <QString>
#include <QMap>

class QSoundEffect;

class AssetManager
{
public:
    // Loads all assets (fonts, data caches, sounds) into memory
    static void load();

    // Clears all assets from memory
    static void unload();

    static QSoundEffect *getSound(const QString &name);

private:
    static void loadSound(const QString &name, const QString &filename);
    static QMap<QString, QSoundEffect *> m_sounds;
};