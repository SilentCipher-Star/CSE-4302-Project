#include "../include/assetmanager.hpp"
#include "../include/csvhandler.hpp"
#include "../include/utils.hpp"
#include <QSoundEffect>
#include <QCoreApplication>
#include <QUrl>
#include <QDebug>

QMap<QString, QSoundEffect *> AssetManager::m_sounds;

void AssetManager::load()
{
    qDebug() << "AssetManager: Loading assets...";
    Utils::loadFonts();
    CsvHandler::initialize();  // Ensure files exist
    CsvHandler::loadAllData(); // Load data into cache

    // Load sounds
    loadSound("tick", "tick.wav");
    loadSound("end", "timer_end.wav");

    qDebug() << "AssetManager: Assets loaded.";
}

void AssetManager::unload()
{
    qDebug() << "AssetManager: Unloading assets...";
    CsvHandler::unloadAllData();

    qDeleteAll(m_sounds);
    m_sounds.clear();
    qDebug() << "AssetManager: Assets unloaded.";
}

void AssetManager::loadSound(const QString &name, const QString &filename)
{
    QString soundPath = QCoreApplication::applicationDirPath() + "/../assets/sounds/" + filename;
    QSoundEffect *sound = new QSoundEffect();
    sound->setSource(QUrl::fromLocalFile(soundPath));
    sound->setVolume(0.5f);
    m_sounds.insert(name, sound);
}

QSoundEffect *AssetManager::getSound(const QString &name)
{
    return m_sounds.value(name, nullptr);
}