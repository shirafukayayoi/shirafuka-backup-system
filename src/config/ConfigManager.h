#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QList>
#include "../models/BackupConfig.h"

class ConfigManager : public QObject
{
    Q_OBJECT

public:
    ConfigManager(const QString &filePath, QObject *parent = nullptr);

    void loadConfig();
    void saveConfig();

    QList<BackupConfig> backupConfigs() const;
    void setBackupConfigs(const QList<BackupConfig> &configs);

    void addBackupConfig(const BackupConfig &config);
    void updateBackupConfig(int index, const BackupConfig &config);
    void removeBackupConfig(int index);

    QString defaultBackupDestination() const;
    void setDefaultBackupDestination(const QString &path);

    QJsonObject getConfig() const;             // 設定全体を取得
    void setConfig(const QJsonObject &config); // 設定全体を設定

private:
    QString m_configFilePath;
    QJsonObject m_config;
    QList<BackupConfig> m_backupConfigs;
    QString m_defaultBackupDestination;
};

#endif // CONFIGMANAGER_H