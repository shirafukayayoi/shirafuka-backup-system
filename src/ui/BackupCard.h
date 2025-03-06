#ifndef BACKUPCARD_H
#define BACKUPCARD_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QFrame>       // 追加
#include <QResizeEvent> // 追加
#include "../models/BackupConfig.h"

class BackupCard : public QWidget
{
    Q_OBJECT

public:
    explicit BackupCard(const BackupConfig &config, int index, QWidget *parent = nullptr);

    BackupConfig config() const;
    int index() const;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void updateProgress(int progress);
    void resetProgress();
    void setProgress(int value); // 追加：進捗を設定するメソッド

signals:
    void runBackup(const BackupConfig &config);
    void removeBackup(int index);

protected:
    void resizeEvent(QResizeEvent *event) override; // 追加

private:
    void setupUI();

    BackupConfig m_config;
    int m_index;
    QFrame *m_frame; // 追加
    QLabel *m_titleLabel;
    QLabel *m_pathLabel;
    QLabel *m_lastBackupLabel;
    QPushButton *m_backupButton;
    QPushButton *m_removeButton;
    QProgressBar *m_progressBar;
};

#endif // BACKUPCARD_H