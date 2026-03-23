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
    CsvHandler::initialize();
    CsvHandler::loadAllData();

    // Use the virtual paths defined in your assets.qrc
    loadSound("tick", "qrc:/assets/sounds/tick.wav");
    loadSound("end", "qrc:/assets/sounds/timer_end.wav");

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

void AssetManager::loadSound(const QString &name, const QString &path)
{
    QSoundEffect *sound = new QSoundEffect();
    
    // Switch from local file to a standard QUrl for the resource path
    sound->setSource(QUrl(path));
    
    sound->setVolume(0.5f);
    m_sounds.insert(name, sound);
    
    if (!sound->isLoaded()) {
        qWarning() << "AssetManager: Failed to load sound:" << name << "from path:" << path;
    }
}

QSoundEffect *AssetManager::getSound(const QString &name)
{
    return m_sounds.value(name, nullptr);
}