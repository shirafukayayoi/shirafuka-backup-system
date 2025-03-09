#include "BackupEngine.h"
#include "BackupTask.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QRegularExpression>    // QRegExp から QRegularExpression に変更
#include <QJsonArray>            // 追加: QJsonArrayのヘッダー
#include "../utils/FileSystem.h" // FileSystemを追加
#include <QApplication>          // 追加: QApplicationのヘッダー

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
    connect(m_currentTask, &BackupTask::fileProcessed, this, &BackupEngine::onFileProcessed);
    connect(m_currentTask, &BackupTask::directoryProcessed, this, &BackupEngine::onDirectoryProcessed);
    connect(m_currentTask, &BackupTask::operationLog, this, &BackupEngine::onOperationLog);

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

    // 完了メッセージをログに記録
    emit backupLogMessage(tr("バックアップ処理が正常に完了しました"));

    // 完了シグナルを発行
    emit backupCompleted();
    emit backupComplete(); // 両方のシグナルを発行
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

    // バックアップモードを確認
    bool isGameSaveBackup = false;
    QStringList saveDataFolders;

    if (config.extraData().contains("backupMode"))
    {
        int mode = config.extraData()["backupMode"].toInt();
        isGameSaveBackup = (mode == 1); // GameSaveBackupは値1

        if (isGameSaveBackup && config.extraData().contains("saveDataFolders"))
        {
            QJsonArray foldersArray = config.extraData()["saveDataFolders"].toArray();
            for (const QJsonValue &value : foldersArray)
            {
                saveDataFolders.append(value.toString());
            }
        }
    }

    // セーブデータバックアップモードの場合
    if (isGameSaveBackup && !saveDataFolders.isEmpty())
    {
        emit backupProgress(10);
        emit backupLogMessage(tr("セーブデータバックアップモードを使用します"));
        emit backupLogMessage(tr("検索対象フォルダ: %1").arg(saveDataFolders.join(", ")));
        emit backupLogMessage(tr("セーブデータフォルダの検索を開始します..."));

        // UI応答性のために処理
        QApplication::processEvents();

        // セーブデータフォルダを検索 - 最大深度10で検索
        QStringList foundFolders = FileSystem::findSpecificFolders(sourcePath, saveDataFolders, 10);

        // 検索完了後にも応答性を維持
        QApplication::processEvents();

        if (foundFolders.isEmpty())
        {
            emit backupLogMessage(tr("セーブデータフォルダが見つかりませんでした"));
            emit backupLogMessage(tr("バックアップ元: %1").arg(sourcePath));
            emit backupLogMessage(tr("バックアップを中断します"));
            emit backupProgress(100);
            emit backupComplete();
            return;
        }

        // 見つかったフォルダの一覧をログに表示
        emit backupLogMessage(tr("セーブデータフォルダを %1 個見つけました:").arg(foundFolders.size()));
        for (const QString &folder : foundFolders)
        {
            emit backupLogMessage(tr("  - %1").arg(folder));
        }

        emit backupProgress(30);
        emit backupLogMessage(tr("セーブデータのコピーを開始します..."));

        // UI応答性を維持
        QApplication::processEvents();

        // セーブデータフォルダをコピー
        bool success = FileSystem::copyGameSaveData(foundFolders, destPath, [this](const QString &message)
                                                    {
                                                        // コールバックでログメッセージを受け取る
                                                        emit backupLogMessage(message);
                                                        QApplication::processEvents(); // UIの応答性を維持
                                                    });

        // コピー完了後にも応答性を維持
        QApplication::processEvents();

        if (success)
        {
            emit backupLogMessage(tr("すべてのセーブデータのバックアップが完了しました"));
            emit backupLogMessage(tr("バックアップ先: %1").arg(destPath));
        }
        else
        {
            emit backupLogMessage(tr("一部のセーブデータのバックアップに失敗しました"));
            emit backupLogMessage(tr("詳細はログを確認してください"));
        }

        emit backupProgress(100);
        emit backupComplete();
        return;
    }

    // 通常バックアップの場合は既存のコード
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

    // バックアップ処理が完了したら、明示的に進捗100%を設定してから完了シグナルを発行
    emit backupProgress(100);
    emit backupLogMessage(tr("バックアップ処理が完了しました"));
    emit backupComplete();
    emit backupCompleted(); // 両方のシグナルを発行（互換性のため）
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

void BackupEngine::onFileProcessed(const QString &filePath, bool success)
{
    // シグナルを転送
    emit fileProcessed(filePath, success);
}

void BackupEngine::onDirectoryProcessed(const QString &dirPath, bool created)
{
    // シグナルを転送
    emit directoryProcessed(dirPath, created);
}

void BackupEngine::onOperationLog(const QString &message)
{
    // シグナルを転送
    emit backupLogMessage(message);
}