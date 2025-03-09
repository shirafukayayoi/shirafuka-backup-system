#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <QString>
#include <QStringList>
#include <functional>

namespace FileSystem
{
    bool copyFile(const QString &source, const QString &destination);
    bool copyDirectory(const QString &sourceDir, const QString &destDir);
    bool deleteDirectory(const QString &dirPath);

    // 新しい関数の宣言を追加
    QStringList findSpecificFolders(const QString &rootDir, const QStringList &folderNames);
    QStringList findSpecificFolders(const QString &rootDir, const QStringList &folderNames, int maxDepth);

    // コールバック関数を受け取るバージョンを追加
    bool copyGameSaveData(const QStringList &sourceFolders, const QString &destRootDir);
    bool copyGameSaveData(
        const QStringList &sourceFolders,
        const QString &destRootDir,
        const std::function<void(const QString &)> &logCallback);
}

#endif // FILESYSTEM_H