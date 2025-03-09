#ifndef BACKUPCONFIG_H
#define BACKUPCONFIG_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QStringList>

class BackupConfig
{
public:
    BackupConfig();
    BackupConfig(const QString &name, const QString &sourcePath, const QString &destinationPath);

    QString name() const;
    void setName(const QString &name);

    QString sourcePath() const;
    void setSourcePath(const QString &path);

    QString destinationPath() const;
    void setDestinationPath(const QString &path);

    QDateTime lastBackupTime() const;
    void setLastBackupTime(const QDateTime &time);

    // 追加: 除外設定
    QStringList excludedFiles() const;
    void setExcludedFiles(const QStringList &files);

    QStringList excludedFolders() const;
    void setExcludedFolders(const QStringList &folders);

    QStringList excludedExtensions() const;
    void setExcludedExtensions(const QStringList &extensions);

    // JSON変換用メソッド
    QJsonObject toJson() const;
    static BackupConfig fromJson(const QJsonObject &json);

private:
    QString m_name;
    QString m_sourcePath;
    QString m_destinationPath;
    QDateTime m_lastBackupTime;

    // 追加: 除外リスト
    QStringList m_excludedFiles;
    QStringList m_excludedFolders;
    QStringList m_excludedExtensions;
};

#endif // BACKUPCONFIG_H