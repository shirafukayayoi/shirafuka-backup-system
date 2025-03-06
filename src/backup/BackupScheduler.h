#ifndef BACKUPSCHEDULER_H
#define BACKUPSCHEDULER_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QDateTime>
#include <QList>
#include "../models/BackupConfig.h"

class BackupScheduler : public QObject
{
    Q_OBJECT

public:
    explicit BackupScheduler(QObject *parent = nullptr);
    ~BackupScheduler();

    // スケジュール設定の取得・設定
    bool isScheduleEnabled() const;
    void setScheduleEnabled(bool enabled);

    QTime scheduledTime() const;
    void setScheduledTime(const QTime &time);

    // 定期バックアップの取得・設定
    bool isPeriodicBackupEnabled() const;
    void setPeriodicBackupEnabled(bool enabled);

    int periodicInterval() const; // 時間単位
    void setPeriodicInterval(int hours);

    // 次回のバックアップ時刻を計算
    QDateTime nextBackupTime() const;

public slots:
    void start();         // スケジューラーの開始
    void stop();          // スケジューラーの停止
    void checkSchedule(); // スケジュールチェック

signals:
    void backupNeeded(); // バックアップ実行のシグナル
    void nextBackupTimeChanged(const QDateTime &nextTime);

private:
    void setupTimers();
    void calculateNextBackupTime();

    bool m_scheduleEnabled;
    QTime m_scheduledTime;

    bool m_periodicEnabled;
    int m_periodicInterval; // 時間単位

    QTimer *m_dailyTimer;    // 毎日のスケジュールチェック用
    QTimer *m_periodicTimer; // 定期バックアップ用

    QDateTime m_nextBackupTime;
};

#endif // BACKUPSCHEDULER_H