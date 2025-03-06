#include "Logger.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>
#include <QStringList>

Logger &Logger::instance()
{
    static Logger instance;
    return instance;
}

// コンストラクタの修正（引数なし）
Logger::Logger()
{
    // ログ開始メッセージ
    log(QStringLiteral("アプリケーション起動"));
}

// デストラクタは必要ないので削除する

void Logger::log(const QString &message)
{
    QMutexLocker locker(&m_mutex);

    QString timestamp = QDateTime::currentDateTime().toString("[yyyy/MM/dd HH:mm:ss] ");
    m_logEntries.append(timestamp + message);

    // ログが多すぎる場合は古いものから削除（オプション）
    const int MAX_LOG_ENTRIES = 1000;
    while (m_logEntries.size() > MAX_LOG_ENTRIES)
    {
        m_logEntries.removeFirst();
    }
}

QStringList Logger::getAllLogs() const
{
    QMutexLocker locker(&m_mutex);
    return m_logEntries;
}

void Logger::clearLogs()
{
    QMutexLocker locker(&m_mutex);
    m_logEntries.clear();
    log(QStringLiteral("ログをクリア"));
}