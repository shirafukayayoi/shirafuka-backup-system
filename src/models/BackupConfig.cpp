#include "BackupConfig.h"
#include <QJsonObject>
#include <QFileInfo>
#include <QJsonArray>

BackupConfig::BackupConfig()
    : m_lastBackupTime(QDateTime::currentDateTime())
{
}

BackupConfig::BackupConfig(const QString &name, const QString &sourcePath, const QString &destinationPath)
    : m_name(name), m_sourcePath(sourcePath), m_destinationPath(destinationPath), m_lastBackupTime(QDateTime::currentDateTime())
{
}

QString BackupConfig::name() const
{
    return m_name;
}

void BackupConfig::setName(const QString &name)
{
    m_name = name;
}

QString BackupConfig::sourcePath() const
{
    return m_sourcePath;
}

void BackupConfig::setSourcePath(const QString &path)
{
    m_sourcePath = path;
}

QString BackupConfig::destinationPath() const
{
    return m_destinationPath;
}

void BackupConfig::setDestinationPath(const QString &path)
{
    m_destinationPath = path;
}

QDateTime BackupConfig::lastBackupTime() const
{
    return m_lastBackupTime;
}

void BackupConfig::setLastBackupTime(const QDateTime &time)
{
    m_lastBackupTime = time;
}

QStringList BackupConfig::excludedFiles() const
{
    return m_excludedFiles;
}

void BackupConfig::setExcludedFiles(const QStringList &files)
{
    m_excludedFiles = files;
}

QStringList BackupConfig::excludedFolders() const
{
    return m_excludedFolders;
}

void BackupConfig::setExcludedFolders(const QStringList &folders)
{
    m_excludedFolders = folders;
}

QStringList BackupConfig::excludedExtensions() const
{
    return m_excludedExtensions;
}

void BackupConfig::setExcludedExtensions(const QStringList &extensions)
{
    m_excludedExtensions = extensions;
}

QJsonObject BackupConfig::extraData() const
{
    return m_extraData;
}

void BackupConfig::setExtraData(const QJsonObject &data)
{
    m_extraData = data;
}

QJsonObject BackupConfig::toJson() const
{
    QJsonObject json;
    json["name"] = m_name;
    json["sourcePath"] = m_sourcePath;
    json["destinationPath"] = m_destinationPath;
    json["lastBackupTime"] = m_lastBackupTime.toString(Qt::ISODate);

    if (!m_excludedFiles.isEmpty())
    {
        QJsonArray filesArray;
        for (const QString &file : m_excludedFiles)
        {
            filesArray.append(file);
        }
        json["excludedFiles"] = filesArray;
    }

    if (!m_excludedFolders.isEmpty())
    {
        QJsonArray foldersArray;
        for (const QString &folder : m_excludedFolders)
        {
            foldersArray.append(folder);
        }
        json["excludedFolders"] = foldersArray;
    }

    if (!m_excludedExtensions.isEmpty())
    {
        QJsonArray extensionsArray;
        for (const QString &ext : m_excludedExtensions)
        {
            extensionsArray.append(ext);
        }
        json["excludedExtensions"] = extensionsArray;
    }

    // 追加データを保存
    json["extraData"] = m_extraData;

    return json;
}

BackupConfig BackupConfig::fromJson(const QJsonObject &json)
{
    BackupConfig config;
    config.m_name = json["name"].toString();
    config.m_sourcePath = json["sourcePath"].toString();
    config.m_destinationPath = json["destinationPath"].toString();

    if (json.contains("lastBackupTime"))
    {
        config.m_lastBackupTime = QDateTime::fromString(json["lastBackupTime"].toString(), Qt::ISODate);
    }

    if (json.contains("excludedFiles"))
    {
        QStringList files;
        QJsonArray filesArray = json["excludedFiles"].toArray();
        for (const QJsonValue &value : filesArray)
        {
            files.append(value.toString());
        }
        config.setExcludedFiles(files);
    }

    if (json.contains("excludedFolders"))
    {
        QStringList folders;
        QJsonArray foldersArray = json["excludedFolders"].toArray();
        for (const QJsonValue &value : foldersArray)
        {
            folders.append(value.toString());
        }
        config.setExcludedFolders(folders);
    }

    if (json.contains("excludedExtensions"))
    {
        QStringList extensions;
        QJsonArray extensionsArray = json["excludedExtensions"].toArray();
        for (const QJsonValue &value : extensionsArray)
        {
            extensions.append(value.toString());
        }
        config.setExcludedExtensions(extensions);
    }

    // 追加データを読み込み
    if (json.contains("extraData"))
    {
        config.m_extraData = json["extraData"].toObject();
    }

    return config;
}