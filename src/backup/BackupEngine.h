#ifndef BACKUPENGINE_H
#define BACKUPENGINE_H

#include <QObject>
#include <QFileInfoList>
#include <QDir>
#include <QRegularExpression> // QRegExp から QRegularExpression に変更
#include "../models/BackupConfig.h"

class BackupTask;

class BackupEngine : public QObject
{
    Q_OBJECT

public:
    explicit BackupEngine(QObject *parent = nullptr);
    ~BackupEngine();

    void startBackup(const QString &sourcePath, const QString &destinationPath);
    void stopBackup();
    bool isRunning() const;
    void runBackup(const BackupConfig &config);

signals:
    void backupProgress(int progress);
    void backupCompleted();
    void backupComplete();
    void backupError(const QString &message);

private slots:
    void onBackupProgressUpdated(int progress);
    void onBackupFinished();

private:
    BackupTask *m_currentTask;

    QFileInfoList getFileList(const QDir &sourceDir,
                              const QStringList &excludedFiles,
                              const QStringList &excludedFolders,
                              const QStringList &excludedExtensions);
};

#endif // BACKUPENGINE_H