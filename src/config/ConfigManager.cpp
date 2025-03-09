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

bool ConfigManager::saveConfig()
{
    try
    {
        qDebug() << "ConfigManager::saveConfig - Saving config with " << m_backupConfigs.size() << " backup configs";

        // バックアップ設定を保存
        QJsonArray backupConfigsArray;
        for (const BackupConfig &config : m_backupConfigs)
        {
            QJsonObject configJson = config.toJson();
            if (configJson.isEmpty())
            {
                qDebug() << "Warning: Empty JSON object for config: " << config.name();
                continue;
            }
            backupConfigsArray.append(configJson);
        }
        m_config["backupConfigs"] = backupConfigsArray;

        // デフォルトバックアップ先を保存
        m_config["defaultBackupDestination"] = m_defaultBackupDestination;

        QJsonDocument doc(m_config);
        QFile file(m_configFilePath);
        if (!file.open(QIODevice::WriteOnly))
        {
            qDebug() << "Could not open config file for writing:" << m_configFilePath;
            return false;
        }

        qint64 bytesWritten = file.write(doc.toJson());
        file.close();

        if (bytesWritten <= 0)
        {
            qDebug() << "Failed to write config data to file";
            return false;
        }

        qDebug() << "Config saved successfully, wrote" << bytesWritten << "bytes";
        return true;
    }
    catch (const std::exception &e)
    {
        qDebug() << "Exception in saveConfig: " << e.what();
        return false;
    }
    catch (...)
    {
        qDebug() << "Unknown exception in saveConfig";
        return false;
    }
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
    try
    {
        qDebug() << "ConfigManager::addBackupConfig - Adding backup config: " << config.name();

        // 設定の最低限の妥当性をチェック
        if (config.name().isEmpty())
        {
            qDebug() << "Warning: Trying to add config with empty name";
        }

        if (config.sourcePath().isEmpty())
        {
            qDebug() << "Warning: Trying to add config with empty source path";
        }

        if (config.destinationPath().isEmpty())
        {
            qDebug() << "Warning: Trying to add config with empty destination path";
        }

        // 設定を追加
        m_backupConfigs.append(config);
        qDebug() << "Config added successfully, current count: " << m_backupConfigs.size();
    }
    catch (const std::exception &e)
    {
        qDebug() << "Exception in addBackupConfig: " << e.what();
        throw; // より上位のレイヤーで処理できるように再スロー
    }
    catch (...)
    {
        qDebug() << "Unknown exception in addBackupConfig";
        throw; // より上位のレイヤーで処理できるように再スロー
    }
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
