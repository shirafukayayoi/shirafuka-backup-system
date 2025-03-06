#ifndef BACKUPTASK_H
#define BACKUPTASK_H

#include <QObject>
#include <QString>
#include <QDir>

class BackupTask : public QObject
{
    Q_OBJECT

public:
    BackupTask(const QString &sourcePath, const QString &destinationPath, QObject *parent = nullptr);
    void start();
    void stop();
    bool isRunning() const;
    QString source() const;
    QString destination() const;

signals:
    void progressUpdated(int progress);
    void finished();

private:
    bool copyDirectory(const QDir &sourceDir, const QDir &destDir, int &processedItems, int totalItems);
    bool copyDirectoryContents(const QDir &sourceDir, const QDir &destDir, int &processedItems, int totalItems);
    int countItems(const QString &path);

    QString m_sourcePath;
    QString m_destinationPath;
    bool m_running;
};

#endif // BACKUPTASK_H