#include "BackupScheduler.h"
#include <QDebug>

BackupScheduler::BackupScheduler(QObject *parent)
    : QObject(parent),
      m_scheduleEnabled(false),
      m_periodicEnabled(false),
      m_periodicInterval(8) // デフォルトは8時間
{
    // タイマー設定
    m_dailyTimer.setSingleShot(true);     // 一日一回
    m_periodicTimer.setSingleShot(false); // 繰り返し

    // シグナル/スロット接続
    connect(&m_dailyTimer, &QTimer::timeout, this, &BackupScheduler::dailyTimerTimeout);
    connect(&m_periodicTimer, &QTimer::timeout, this, &BackupScheduler::periodicTimerTimeout);

    // 次回バックアップ時間を無効な状態で初期化
    m_nextBackupTime = QDateTime();

    // 初期化時に一度タイマーを設定
    updateTimers();
}

bool BackupScheduler::isScheduleEnabled() const
{
    return m_scheduleEnabled;
}

void BackupScheduler::setScheduleEnabled(bool enabled)
{
    m_scheduleEnabled = enabled;
    updateTimers();
}

QTime BackupScheduler::scheduledTime() const
{
    return m_scheduledTime;
}

void BackupScheduler::setScheduledTime(const QTime &time)
{
    if (time.isValid() && m_scheduledTime != time)
    {
        m_scheduledTime = time;
        updateTimers();
    }
}

bool BackupScheduler::isPeriodicBackupEnabled() const
{
    return m_periodicEnabled;
}

void BackupScheduler::setPeriodicBackupEnabled(bool enabled)
{
    m_periodicEnabled = enabled;
    updateTimers();
}

int BackupScheduler::periodicInterval() const
{
    return m_periodicInterval;
}

void BackupScheduler::setPeriodicInterval(int hours)
{
    if (hours > 0 && m_periodicInterval != hours)
    {
        m_periodicInterval = hours;
        updateTimers();
    }
}

QDateTime BackupScheduler::calculateNextBackupTime() const
{
    QDateTime nextBackupTime;

    // 定時バックアップの次回時刻を計算
    if (m_scheduleEnabled && m_scheduledTime.isValid())
    {
        QDateTime scheduledDateTime = QDateTime::currentDateTime();
        scheduledDateTime.setTime(m_scheduledTime);

        // 既に今日の指定時刻を過ぎていたら明日の同時刻に設定
        if (scheduledDateTime <= QDateTime::currentDateTime())
        {
            scheduledDateTime = scheduledDateTime.addDays(1);
        }

        nextBackupTime = scheduledDateTime;
    }

    // 定期バックアップの次回時刻を計算
    if (m_periodicEnabled && m_periodicInterval > 0)
    {
        QDateTime periodicDateTime = QDateTime::currentDateTime().addSecs(m_periodicInterval * 60 * 60);

        // 定時バックアップがない、または定期バックアップの方が早い場合
        if (!nextBackupTime.isValid() || periodicDateTime < nextBackupTime)
        {
            nextBackupTime = periodicDateTime;
        }
    }

    return nextBackupTime;
}

QDateTime BackupScheduler::nextBackupTime() const
{
    return m_nextBackupTime;
}

void BackupScheduler::updateTimers()
{
    // 両方のタイマーを一度停止
    m_dailyTimer.stop();
    m_periodicTimer.stop();

    // 定時バックアップタイマー設定
    if (m_scheduleEnabled && m_scheduledTime.isValid())
    {
        QTime currentTime = QTime::currentTime();
        QTime targetTime = m_scheduledTime;

        int secsToTarget;
        if (currentTime < targetTime)
        {
            // 今日の指定時刻まで待機
            secsToTarget = currentTime.secsTo(targetTime);
        }
        else
        {
            // 明日の指定時刻まで待機
            secsToTarget = currentTime.secsTo(QTime(23, 59, 59)) + 1 + QTime(0, 0, 0).secsTo(targetTime);
        }

        qDebug() << "定時バックアップを設定: " << m_scheduledTime.toString("HH:mm")
                 << " (" << secsToTarget << "秒後)";

        m_dailyTimer.start(secsToTarget * 1000);
    }

    // 定期バックアップタイマー設定
    if (m_periodicEnabled && m_periodicInterval > 0)
    {
        int intervalMs = m_periodicInterval * 60 * 60 * 1000; // 時間→ミリ秒変換

        qDebug() << "定期バックアップを設定: " << m_periodicInterval << "時間ごと ("
                 << intervalMs << "ミリ秒)";

        m_periodicTimer.start(intervalMs);
    }

    // 次回バックアップ時間の計算と通知
    m_nextBackupTime = calculateNextBackupTime(); // 保存するように変更
    if (m_nextBackupTime.isValid())
    {
        emit nextBackupTimeChanged(m_nextBackupTime);
    }
}

void BackupScheduler::dailyTimerTimeout()
{
    qDebug() << "定時バックアップがトリガーされました: " << m_scheduledTime.toString("HH:mm");

    // バックアップをトリガー
    emit backupTimerTriggered();

    // 次の日のタイマーを設定
    updateTimers();
}

void BackupScheduler::periodicTimerTimeout()
{
    qDebug() << "定期バックアップがトリガーされました: " << m_periodicInterval << "時間ごと";

    // バックアップをトリガー
    emit backupTimerTriggered();

    // periodicTimerはsingleShotではないので自動的に再開します
    // ただし設定を更新するために一応updateTimersを呼ぶ
    updateTimers();
}