#ifndef BACKUPSCHEDULER_H
#define BACKUPSCHEDULER_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QDateTime>

class BackupScheduler : public QObject
{
    Q_OBJECT

public:
    explicit BackupScheduler(QObject *parent = nullptr);

    // 定時バックアップ
    bool isScheduleEnabled() const;
    void setScheduleEnabled(bool enabled);
    QTime scheduledTime() const;
    void setScheduledTime(const QTime &time);

    // 定期バックアップ
    bool isPeriodicBackupEnabled() const;
    void setPeriodicBackupEnabled(bool enabled);
    int periodicInterval() const;
    void setPeriodicInterval(int hours);

    // スケジュール計算
    QDateTime calculateNextBackupTime() const;
    QDateTime nextBackupTime() const; // 追加: 次回バックアップ時間を取得するメソッド

signals:
    void backupTimerTriggered();                           // 追加: バックアップのトリガーシグナル
    void nextBackupTimeChanged(const QDateTime &nextTime); // 追加: 次回バックアップ時間変更シグナル

private slots:
    void dailyTimerTimeout();    // 追加: 日次タイマータイムアウト
    void periodicTimerTimeout(); // 追加: 定期タイマータイムアウト

private:
    void updateTimers(); // 追加: タイマー更新メソッド

    // 追加: タイマーメンバー変数
    QTimer m_dailyTimer;    // 日次バックアップ用タイマー
    QTimer m_periodicTimer; // 定期バックアップ用タイマー

    // 既存のメンバー変数
    bool m_scheduleEnabled;
    QTime m_scheduledTime;
    bool m_periodicEnabled;
    int m_periodicInterval;
    QDateTime m_nextBackupTime; // 追加: 次回バックアップ時間を保存するメンバ変数
};

#endif // BACKUPSCHEDULER_H