#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QMutex>

class Logger
{
public:
    // シングルトンインスタンス取得
    static Logger &instance();

    // ログエントリを追加
    void log(const QString &message);

    // すべてのログエントリを取得
    QStringList getAllLogs() const;

    // ログをクリア
    void clearLogs();

private:
    Logger();                                   // シングルトンなのでプライベートコンストラクタ
    Logger(const Logger &) = delete;            // コピーコンストラクタ禁止
    Logger &operator=(const Logger &) = delete; // 代入演算子禁止

    QStringList m_logEntries; // ログエントリの保存
    mutable QMutex m_mutex;   // マルチスレッド保護用
};

// 簡単に使えるようにするためのマクロ
#define LOG(message) Logger::instance().log(message)

#endif // LOGGER_H