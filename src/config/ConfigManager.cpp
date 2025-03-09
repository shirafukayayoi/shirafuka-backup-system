#include "ConfigManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QStandardPaths>

ConfigManager::ConfigManager(const QString &filePath, QObject *parent)
    : QObject(parent), m_configFilePath(filePath)
{
    // デフォルトのバックアップ先をドキュメントフォルダに設定
    m_defaultBackupDestination = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Backups";
}

void ConfigManager::loadConfig()
{
    QFile file(m_configFilePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Could not open config file for reading:" << m_configFilePath;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull())
    {
        qDebug() << "Failed to parse JSON config file";
        return;
    }

    m_config = doc.object();

    // バックアップ設定を読み込み
    m_backupConfigs.clear();
    QJsonArray backupConfigsArray = m_config["backupConfigs"].toArray();
    for (const QJsonValue &value : backupConfigsArray)
    {
        BackupConfig config = BackupConfig::fromJson(value.toObject());
        m_backupConfigs.append(config);
    }

    // デフォルトバックアップ先を読み込み
    if (m_config.contains("defaultBackupDestination"))
    {
        m_defaultBackupDestination = m_config["defaultBackupDestination"].toString();
    }
}

void ConfigManager::saveConfig()
{
    // バックアップ設定を保存
    QJsonArray backupConfigsArray;
    for (const BackupConfig &config : m_backupConfigs)
    {
        backupConfigsArray.append(config.toJson());
    }
    m_config["backupConfigs"] = backupConfigsArray;

    // デフォルトバックアップ先を保存
    m_config["defaultBackupDestination"] = m_defaultBackupDestination;

    QJsonDocument doc(m_config);
    QFile file(m_configFilePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Could not open config file for writing:" << m_configFilePath;
        return;
    }

    file.write(doc.toJson());
    file.close();
}

QList<BackupConfig> ConfigManager::backupConfigs() const
{
    return m_backupConfigs;
}

void ConfigManager::setBackupConfigs(const QList<BackupConfig> &configs)
{
    m_backupConfigs = configs;
}

void ConfigManager::addBackupConfig(const BackupConfig &config)
{
    m_backupConfigs.append(config);
}

void ConfigManager::updateBackupConfig(int index, const BackupConfig &config)
{
    if (index >= 0 && index < m_backupConfigs.size())
    {
        m_backupConfigs[index] = config;
    }
}

void ConfigManager::removeBackupConfig(int index)
{
    if (index >= 0 && index < m_backupConfigs.size())
    {
        m_backupConfigs.removeAt(index);
    }
}

int ConfigManager::findConfigIndex(const BackupConfig &config) const
{
    for (int i = 0; i < m_backupConfigs.size(); ++i)
    {
        // IDでの比較は現在BackupConfigに実装されていないようなのでスキップ
        // そのかわり名前とパスで比較する
        if (m_backupConfigs[i].name() == config.name() &&
            m_backupConfigs[i].sourcePath() == config.sourcePath() &&
            m_backupConfigs[i].destinationPath() == config.destinationPath())
        {
            return i;
        }
    }
    return -1; // 見つからない場合
}

QString ConfigManager::defaultBackupDestination() const
{
    return m_defaultBackupDestination;
}

void ConfigManager::setDefaultBackupDestination(const QString &path)
{
    m_defaultBackupDestination = path;
}

QJsonObject ConfigManager::getConfig() const
{
    return m_config;
}

void ConfigManager::setConfig(const QJsonObject &config)
{
    m_config = config;
}
