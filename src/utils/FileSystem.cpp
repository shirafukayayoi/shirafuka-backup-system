#include "FileSystem.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QApplication>

namespace FileSystem
{

    bool copyDirectory(const QString &sourceDir, const QString &destDir)
    {
        QDir source(sourceDir);
        QDir destination(destDir);

        if (!source.exists())
        {
            qWarning() << "Source directory does not exist:" << sourceDir;
            return false;
        }

        if (!destination.exists())
        {
            destination.mkpath(destDir);
        }

        foreach (QString file, source.entryList(QDir::Files))
        {
            QString srcFilePath = source.filePath(file);
            QString destFilePath = destination.filePath(file);
            if (!QFile::copy(srcFilePath, destFilePath))
            {
                qWarning() << "Failed to copy file:" << srcFilePath << "to" << destFilePath;
                return false;
            }
        }

        foreach (QString dir, source.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
        {
            QString srcSubDirPath = source.filePath(dir);
            QString destSubDirPath = destination.filePath(dir);
            if (!copyDirectory(srcSubDirPath, destSubDirPath))
            {
                return false;
            }
        }

        return true;
    }

    bool deleteDirectory(const QString &dirPath)
    {
        QDir dir(dirPath);
        if (!dir.exists())
        {
            return true; // Directory does not exist, nothing to delete
        }

        foreach (QString file, dir.entryList(QDir::Files))
        {
            QFile::remove(dir.filePath(file));
        }

        foreach (QString subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
        {
            deleteDirectory(dir.filePath(subDir));
        }

        return dir.rmdir(dirPath);
    }

    // 特定の名前を持つフォルダーを再帰的に検索する関数
    QStringList findSpecificFolders(const QString &rootDir, const QStringList &folderNames, int maxDepth)
    {
        QStringList result;
        QDir dir(rootDir);

        if (!dir.exists())
        {
            qWarning() << "Directory does not exist:" << rootDir;
            return result;
        }

        // 最大深度に達したら検索を中止
        if (maxDepth <= 0)
        {
            return result;
        }

        // 直下の一致するフォルダーをチェック
        foreach (QString folderName, folderNames)
        {
            if (dir.exists(folderName))
            {
                QString fullPath = dir.filePath(folderName);
                result.append(fullPath);
                qDebug() << "Found matching folder:" << fullPath;
            }
        }

        // サブディレクトリを再帰的にチェック
        foreach (QString subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
        {
            QString subDirPath = dir.filePath(subDir);

            // 特殊パターンのためのログ出力
            if (subDir == "www")
            {
                qDebug() << "Checking special pattern www/save in:" << subDirPath;
            }

            result.append(findSpecificFolders(subDirPath, folderNames, maxDepth - 1));

            // www/save のような特殊パターンもチェック
            if (subDir == "www")
            {
                QString wwwSavePath = subDirPath + "/save";
                if (QDir(wwwSavePath).exists())
                {
                    result.append(wwwSavePath);
                    qDebug() << "Found special www/save pattern:" << wwwSavePath;
                }
            }

            // UI応答性を維持するためのイベント処理
            QApplication::processEvents();
        }

        return result;
    }

    // オーバーロードした関数 - 互換性のため
    QStringList findSpecificFolders(const QString &rootDir, const QStringList &folderNames)
    {
        // デフォルトの深さを10に制限して検索
        return findSpecificFolders(rootDir, folderNames, 10);
    }

    // 見つかったセーブデータフォルダーを指定先にコピーする関数
    bool copyGameSaveData(const QStringList &sourceFolders, const QString &destRootDir)
    {
        // 引数なしのコールバックを渡すバージョンを呼び出し
        return copyGameSaveData(sourceFolders, destRootDir, [](const QString &) {});
    }

    // 詳細ログ出力を実装したバージョン
    bool copyGameSaveData(
        const QStringList &sourceFolders,
        const QString &destRootDir,
        const std::function<void(const QString &)> &logCallback)
    {
        bool success = true;
        QDir destRoot(destRootDir);

        if (!destRoot.exists())
        {
            destRoot.mkpath(destRootDir);
            logCallback(QString("バックアップ先ディレクトリを作成しました: %1").arg(destRootDir));
        }

        int totalFolders = sourceFolders.size();
        int processedFolders = 0;

        logCallback(QString("合計 %1 個のセーブデータフォルダをコピーします").arg(totalFolders));

        foreach (QString sourceFolder, sourceFolders)
        {
            QFileInfo sourceInfo(sourceFolder);
            QString gameFolderName = sourceInfo.dir().dirName(); // ゲームフォルダ名を取得
            QString saveDataFolderName = sourceInfo.fileName();  // セーブデータフォルダ名を取得

            logCallback(QString("処理中 (%1/%2): %3").arg(++processedFolders).arg(totalFolders).arg(sourceFolder));

            // 特殊なケースを処理（www/save など）
            QString relativePath;
            if (sourceInfo.dir().dirName() == "www")
            {
                QString parentName = sourceInfo.dir().path();
                QFileInfo parentInfo(parentName);
                gameFolderName = parentInfo.dir().dirName(); // ゲームフォルダ名
                relativePath = "www/save";                   // 相対パス
                logCallback(QString("特殊パス検出: www/save パターン"));
            }
            else
            {
                relativePath = saveDataFolderName; // 通常のセーブフォルダ
            }

            // ゲームごとにサブフォルダを作成
            QString destGameDir = destRootDir + "/" + gameFolderName;
            QString destFullPath = destGameDir;

            if (!relativePath.isEmpty())
            {
                destFullPath = destGameDir + "/" + relativePath;
            }

            // ディレクトリ作成前にログ
            logCallback(QString("コピー先ディレクトリを準備中: %1").arg(destFullPath));

            QDir().mkpath(QFileInfo(destFullPath).path()); // 親ディレクトリを作成

            // フォルダをコピー
            logCallback(QString("コピー開始: %1 → %2").arg(sourceFolder).arg(destFullPath));

            QDir sourceDir(sourceFolder);
            QStringList files = sourceDir.entryList(QDir::Files | QDir::Hidden | QDir::System);
            logCallback(QString("ファイル数: %1 個").arg(files.count()));

            if (!copyDirectory(sourceFolder, destFullPath))
            {
                logCallback(QString("エラー: コピーに失敗しました: %1").arg(sourceFolder));
                success = false;
            }
            else
            {
                logCallback(QString("成功: %1 のコピーが完了しました").arg(sourceFolder));
            }

            // 進捗を示し、UI応答性を維持
            QApplication::processEvents();
        }

        if (success)
        {
            logCallback(QString("すべてのセーブデータを %1 にコピーしました").arg(destRootDir));
        }
        else
        {
            logCallback(QString("一部のコピー処理に失敗しました"));
        }

        return success;
    }

} // namespace FileSystem