#include "BackupTask.h"
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QApplication>
#include <QDebug>
#include <QDirIterator>

BackupTask::BackupTask(const QString &sourcePath, const QString &destinationPath, QObject *parent)
    : QObject(parent), m_sourcePath(sourcePath), m_destinationPath(destinationPath), m_running(false)
{
}

void BackupTask::start()
{
    m_running = true;

    qDebug() << "Starting backup from" << m_sourcePath << "to" << m_destinationPath;

    // 処理開始前に進捗0%を発行
    emit progressUpdated(0);

    // ソースディレクトリから最後のフォルダ名を取得
    QDir sourceDir(m_sourcePath);
    QString sourceDirName = sourceDir.dirName();

    // バックアップ先にソースフォルダ名と同じ名前のフォルダを作成
    QString actualDestPath = m_destinationPath + "/" + sourceDirName;
    QDir actualDestDir(actualDestPath);

    qDebug() << "Creating backup folder:" << actualDestPath;

    // デスティネーションフォルダが存在することを確認
    if (!actualDestDir.exists())
    {
        if (!QDir().mkpath(actualDestPath))
        {
            qDebug() << "Failed to create destination directory:" << actualDestPath;
            m_running = false;
            emit progressUpdated(100);
            emit finished();
            return;
        }
    }

    // ディレクトリとファイルの総数を計算（進捗表示用）
    int totalItems = countItems(m_sourcePath);
    int processedItems = 0;

    qDebug() << "Total items to backup:" << totalItems;

    if (totalItems == 0)
    {
        qDebug() << "No items to backup";
        m_running = false;
        emit progressUpdated(100);
        emit finished();
        return;
    }

    // フォルダ単位でバックアップを行う（ソースディレクトリの中身をコピー）
    bool success = copyDirectoryContents(sourceDir, actualDestDir, processedItems, totalItems);

    m_running = false;
    emit progressUpdated(100);
    emit finished();

    if (success)
    {
        qDebug() << "Backup completed successfully";
    }
    else
    {
        qDebug() << "Backup completed with errors";
    }
}

// ディレクトリの中身をコピーするメソッドを改修
bool BackupTask::copyDirectoryContents(const QDir &sourceDir, const QDir &destDir, int &processedItems, int totalItems)
{
    bool success = true;

    // ソースディレクトリ内の全アイテム（フォルダとファイル）を取得
    QFileInfoList entries = sourceDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);

    foreach (const QFileInfo &info, entries)
    {
        if (!m_running)
        {
            qDebug() << "Backup was stopped";
            return false;
        }

        QString srcItemPath = info.filePath();
        QString destItemPath = destDir.filePath(info.fileName());

        if (info.isDir())
        {
            // ディレクトリの場合、再帰的に処理
            QDir newDestDir(destItemPath);
            if (!newDestDir.exists())
            {
                qDebug() << "Creating directory:" << destItemPath;
                if (!destDir.mkdir(info.fileName()))
                {
                    qDebug() << "Failed to create directory:" << destItemPath;
                    success = false;
                    // ディレクトリ作成失敗のログとシグナル
                    emit directoryProcessed(destItemPath, false);
                    emit operationLog(tr("フォルダ作成失敗: %1").arg(destItemPath));
                    continue;
                }
                else
                {
                    // ディレクトリ作成成功のログとシグナル
                    emit directoryProcessed(destItemPath, true);
                    emit operationLog(tr("フォルダ作成: %1").arg(destItemPath));
                }
            }

            QDir newSrcDir(srcItemPath);
            if (!copyDirectory(newSrcDir, newDestDir, processedItems, totalItems))
            {
                success = false;
            }
        }
        else if (info.isFile())
        {
            // ファイルの場合、コピー
            qDebug() << "Copying file:" << srcItemPath << "to" << destItemPath;

            // 既にファイルが存在する場合は削除
            QFile destFile(destItemPath);
            if (destFile.exists())
            {
                destFile.remove();
            }

            QFile srcFile(srcItemPath);
            if (!srcFile.copy(destItemPath))
            {
                qDebug() << "Failed to copy file:" << srcFile.errorString();
                success = false;
                // ファイルコピー失敗のログとシグナル
                emit fileProcessed(srcItemPath, false);
                emit operationLog(tr("ファイルコピー失敗: %1 → %2 (%3)").arg(srcItemPath, destItemPath, srcFile.errorString()));
            }
            else
            {
                // ファイルコピー成功のログとシグナル
                emit fileProcessed(srcItemPath, true);
                emit operationLog(tr("ファイルコピー: %1").arg(info.fileName()));
            }

            // 進捗を更新
            processedItems++;
            int progress = (processedItems * 100) / (totalItems > 0 ? totalItems : 1);
            emit progressUpdated(progress);

            // UIを応答可能に保つために少し待機
            QApplication::processEvents();
        }
    }

    return success;
}

// 既存の copyDirectory メソッドはそのまま残す
bool BackupTask::copyDirectory(const QDir &sourceDir, const QDir &destDir, int &processedItems, int totalItems)
{
    return copyDirectoryContents(sourceDir, destDir, processedItems, totalItems);
}

int BackupTask::countItems(const QString &path)
{
    int count = 0;
    QDirIterator it(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();
        count++;
    }
    return count;
}

void BackupTask::stop()
{
    m_running = false;
}

bool BackupTask::isRunning() const
{
    return m_running;
}

QString BackupTask::source() const
{
    return m_sourcePath;
}

QString BackupTask::destination() const
{
    return m_destinationPath;
}