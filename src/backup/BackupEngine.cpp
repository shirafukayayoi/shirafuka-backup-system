#include "BackupEngine.h"
#include "BackupTask.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QRegularExpression> // QRegExp から QRegularExpression に変更

BackupEngine::BackupEngine(QObject *parent)
    : QObject(parent), m_currentTask(nullptr)
{
}

BackupEngine::~BackupEngine()
{
    if (m_currentTask)
    {
        m_currentTask->stop();
        m_currentTask->deleteLater();
    }
}

void BackupEngine::startBackup(const QString &sourcePath, const QString &destinationPath)
{
    if (m_currentTask)
    {
        qDebug() << "There is already a backup task running";
        return;
    }

    m_currentTask = new BackupTask(sourcePath, destinationPath, this);
    connect(m_currentTask, &BackupTask::progressUpdated, this, &BackupEngine::onBackupProgressUpdated);
    connect(m_currentTask, &BackupTask::finished, this, &BackupEngine::onBackupFinished);

    m_currentTask->start();
}

void BackupEngine::stopBackup()
{
    if (m_currentTask)
    {
        m_currentTask->stop();
    }
}

bool BackupEngine::isRunning() const
{
    return m_currentTask != nullptr && m_currentTask->isRunning();
}

void BackupEngine::onBackupProgressUpdated(int progress)
{
    emit backupProgress(progress);
}

void BackupEngine::onBackupFinished()
{
    // バックアップが完了したら、現在のタスクをクリーンアップ
    if (m_currentTask)
    {
        m_currentTask->deleteLater();
        m_currentTask = nullptr;
    }

    emit backupCompleted();
    emit backupComplete(); // 追加: どちらのシグナルも発火させる
}

void BackupEngine::runBackup(const BackupConfig &config)
{
    // バックアップ開始を記録
    emit backupProgress(0);

    // パスを取得
    QString sourcePath = config.sourcePath();
    QString destPath = config.destinationPath();

    // パスが存在するか確認
    QDir sourceDir(sourcePath);
    if (!sourceDir.exists())
    {
        emit backupError(tr("バックアップ元フォルダが存在しません: %1").arg(sourcePath));
        return;
    }

    QDir destDir(destPath);
    if (!destDir.exists())
    {
        // 保存先フォルダがなければ作成
        if (!destDir.mkpath("."))
        {
            emit backupError(tr("バックアップ先フォルダを作成できませんでした: %1").arg(destPath));
            return;
        }
    }

    // 除外パターンを準備
    QStringList excludedFiles = config.excludedFiles();
    QStringList excludedFolders = config.excludedFolders();
    QStringList excludedExtensions = config.excludedExtensions();

    // ファイルリストを取得
    QFileInfoList fileList = getFileList(sourceDir, excludedFiles, excludedFolders, excludedExtensions);

    // 総ファイル数
    int totalFiles = fileList.size();
    int copiedFiles = 0;

    if (totalFiles == 0)
    {
        emit backupProgress(100);
        emit backupComplete();
        return;
    }

    // バックアップ処理
    for (const QFileInfo &fileInfo : fileList)
    {
        QString relativePath = sourceDir.relativeFilePath(fileInfo.filePath());
        QString targetPath = destPath + QDir::separator() + relativePath;

        // ターゲットディレクトリがなければ作成
        QFileInfo targetFileInfo(targetPath);
        QDir targetDir = targetFileInfo.dir();
        if (!targetDir.exists())
        {
            targetDir.mkpath(".");
        }

        // コピー実行
        QFile sourceFile(fileInfo.filePath());
        QFile targetFile(targetPath);

        // 既存ファイルがある場合は削除
        if (targetFile.exists())
        {
            targetFile.remove();
        }

        // コピー実行
        sourceFile.copy(targetPath);

        // 進捗更新
        copiedFiles++;
        int progress = (copiedFiles * 100) / totalFiles;
        emit backupProgress(progress);
    }

    emit backupProgress(100);
    emit backupComplete();
}

QFileInfoList BackupEngine::getFileList(const QDir &sourceDir,
                                        const QStringList &excludedFiles,
                                        const QStringList &excludedFolders,
                                        const QStringList &excludedExtensions)
{
    QFileInfoList result;

    QFileInfoList entries = sourceDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);

    for (const QFileInfo &entry : entries)
    {
        QString relativePath = sourceDir.relativeFilePath(entry.filePath());

        if (entry.isDir())
        {
            bool exclude = false;

            for (const QString &pattern : excludedFolders)
            {
                // QRegExp から QRegularExpression に変更
                QString wildcardPattern = QRegularExpression::wildcardToRegularExpression(pattern);
                QRegularExpression regex(wildcardPattern, QRegularExpression::CaseInsensitiveOption);
                if (regex.match(entry.fileName()).hasMatch() || regex.match(relativePath).hasMatch())
                {
                    exclude = true;
                    break;
                }
            }

            if (!exclude)
            {
                QDir subDir(entry.filePath());
                result.append(getFileList(subDir, excludedFiles, excludedFolders, excludedExtensions));
            }
        }
        else if (entry.isFile())
        {
            bool exclude = false;

            for (const QString &pattern : excludedFiles)
            {
                // QRegExp から QRegularExpression に変更
                QString wildcardPattern = QRegularExpression::wildcardToRegularExpression(pattern);
                QRegularExpression regex(wildcardPattern, QRegularExpression::CaseInsensitiveOption);
                if (regex.match(entry.fileName()).hasMatch() || regex.match(relativePath).hasMatch())
                {
                    exclude = true;
                    break;
                }
            }

            if (!exclude)
            {
                QString extension = "." + entry.suffix().toLower();
                for (const QString &ext : excludedExtensions)
                {
                    if (ext.toLower() == extension)
                    {
                        exclude = true;
                        break;
                    }
                }
            }

            if (!exclude)
            {
                result.append(entry);
            }
        }
    }

    return result;
}