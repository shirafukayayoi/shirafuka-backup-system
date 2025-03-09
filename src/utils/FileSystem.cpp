#include "FileSystem.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

namespace FileSystem {

bool copyDirectory(const QString &sourceDir, const QString &destDir) {
    QDir source(sourceDir);
    QDir destination(destDir);

    if (!source.exists()) {
        qWarning() << "Source directory does not exist:" << sourceDir;
        return false;
    }

    if (!destination.exists()) {
        destination.mkpath(destDir);
    }

    foreach (QString file, source.entryList(QDir::Files)) {
        QString srcFilePath = source.filePath(file);
        QString destFilePath = destination.filePath(file);
        if (!QFile::copy(srcFilePath, destFilePath)) {
            qWarning() << "Failed to copy file:" << srcFilePath << "to" << destFilePath;
            return false;
        }
    }

    foreach (QString dir, source.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString srcSubDirPath = source.filePath(dir);
        QString destSubDirPath = destination.filePath(dir);
        if (!copyDirectory(srcSubDirPath, destSubDirPath)) {
            return false;
        }
    }

    return true;
}

bool deleteDirectory(const QString &dirPath) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        return true; // Directory does not exist, nothing to delete
    }

    foreach (QString file, dir.entryList(QDir::Files)) {
        QFile::remove(dir.filePath(file));
    }

    foreach (QString subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        deleteDirectory(dir.filePath(subDir));
    }

    return dir.rmdir(dirPath);
}

} // namespace FileSystem