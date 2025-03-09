#ifndef BACKUPENGINE_H
#define BACKUPENGINE_H

#include <QObject>
#include <QFileInfoList>
#include <QDir>
#include <QRegularExpression>       // QRegExp から QRegularExpression に変更
#include "../models/BackupConfig.h" // 明示的にBackupConfigをインクルード

class BackupTask;

class BackupEngine : public QObject
{
    Q_OBJECT

public:
    explicit BackupEngine(QObject *parent = nullptr);
    ~BackupEngine();

    void startBackup(const QString &sourcePath, const QString &destinationPath);
    void runBackup(const BackupConfig &config); // 既存のメソッドをヘッダーに追加
    void stopBackup();
    bool isRunning() const;

signals:
    void backupProgress(int progress);
    void backupCompleted();
    void backupComplete(); // 両方のシグナル名をサポート
    void backupError(const QString &errorMessage);
    void fileProcessed(const QString &filePath, bool success);
    void directoryProcessed(const QString &dirPath, bool created);
    void backupLogMessage(const QString &message);

private slots:
    void onBackupProgressUpdated(int progress);
    void onBackupFinished();
    void onFileProcessed(const QString &filePath, bool success);
    void onDirectoryProcessed(const QString &dirPath, bool created);
    void onOperationLog(const QString &message);

private:
    BackupTask *m_currentTask;

    QFileInfoList getFileList(const QDir &sourceDir,
                              const QStringList &excludedFiles,
                              const QStringList &excludedFolders,
                              const QStringList &excludedExtensions);
};

#endif // BACKUPENGINE_H