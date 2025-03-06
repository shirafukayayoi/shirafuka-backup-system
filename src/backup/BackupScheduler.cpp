#include "BackupScheduler.h"
#include <QDebug>

BackupScheduler::BackupScheduler(QObject *parent)
    : QObject(parent),
      m_scheduleEnabled(false),
      m_scheduledTime(QTime(3, 0)), // デフォルト：午前3時
      m_periodicEnabled(false),
      m_periodicInterval(24) // デフォルト：24時間
{
    m_dailyTimer = new QTimer(this);
    m_periodicTimer = new QTimer(this);

    connect(m_dailyTimer, &QTimer::timeout, this, &BackupScheduler::checkSchedule);
    connect(m_periodicTimer, &QTimer::timeout, this, &BackupScheduler::backupNeeded);

    // 初期設定
    calculateNextBackupTime();
}

BackupScheduler::~BackupScheduler()
{
    stop();
}

bool BackupScheduler::isScheduleEnabled() const
{
    return m_scheduleEnabled;
}

void BackupScheduler::setScheduleEnabled(bool enabled)
{
    m_scheduleEnabled = enabled;
    if (enabled)
    {
        calculateNextBackupTime();
    }
}

QTime BackupScheduler::scheduledTime() const
{
    return m_scheduledTime;
}

void BackupScheduler::setScheduledTime(const QTime &time)
{
    m_scheduledTime = time;
    calculateNextBackupTime();
}

bool BackupScheduler::isPeriodicBackupEnabled() const
{
    return m_periodicEnabled;
}

void BackupScheduler::setPeriodicBackupEnabled(bool enabled)
{
    m_periodicEnabled = enabled;
    if (enabled)
    {
        // 定期バックアップを有効にする
        m_periodicTimer->start(m_periodicInterval * 60 * 60 * 1000); // 時間をミリ秒に変換
    }
    else
    {
        m_periodicTimer->stop();
    }
}

int BackupScheduler::periodicInterval() const
{
    return m_periodicInterval;
}

void BackupScheduler::setPeriodicInterval(int hours)
{
    m_periodicInterval = hours;
    if (m_periodicEnabled)
    {
        // 既に有効なら、新しい間隔で再開
        m_periodicTimer->start(m_periodicInterval * 60 * 60 * 1000);
    }
}

QDateTime BackupScheduler::nextBackupTime() const
{
    return m_nextBackupTime;
}

void BackupScheduler::start()
{
    // 毎日のチェッカーは1分ごとに実行
    m_dailyTimer->start(60 * 1000); // 1分 = 60000ミリ秒

    // 定期バックアップが有効なら開始
    if (m_periodicEnabled)
    {
        m_periodicTimer->start(m_periodicInterval * 60 * 60 * 1000);
    }

    qDebug() << "BackupScheduler started. Next backup at:" << m_nextBackupTime.toString();
}

void BackupScheduler::stop()
{
    m_dailyTimer->stop();
    m_periodicTimer->stop();
    qDebug() << "BackupScheduler stopped";
}

void BackupScheduler::checkSchedule()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();

    // スケジュールが有効で、現在時刻が次回予定時刻を過ぎている場合
    if (m_scheduleEnabled && currentDateTime >= m_nextBackupTime)
    {
        qDebug() << "Scheduled backup time reached:" << currentDateTime.toString();
        emit backupNeeded();

        // 次回の予定時刻を計算
        calculateNextBackupTime();
    }
}

void BackupScheduler::calculateNextBackupTime()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QDateTime scheduledDateTime = QDateTime(currentDateTime.date(), m_scheduledTime);

    // 現在時刻が今日の予定時刻を過ぎている場合は、翌日に設定
    if (currentDateTime.time() >= m_scheduledTime)
    {
        scheduledDateTime = scheduledDateTime.addDays(1);
    }

    m_nextBackupTime = scheduledDateTime;
    emit nextBackupTimeChanged(m_nextBackupTime);

    qDebug() << "Next backup scheduled for:" << m_nextBackupTime.toString();
}