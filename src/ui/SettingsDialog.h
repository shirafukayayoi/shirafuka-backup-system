#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QSlider>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTimeEdit>
#include <QSpinBox>
#include <QDateTime>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    // 定時バックアップ設定
    bool isScheduleEnabled() const;
    void setScheduleEnabled(bool enabled);
    QTime scheduledTime() const;
    void setScheduledTime(const QTime &time);

    // 定期バックアップ設定
    bool isPeriodicBackupEnabled() const;
    void setPeriodicBackupEnabled(bool enabled);
    int periodicInterval() const;
    void setPeriodicInterval(int hours);

    // 次回バックアップ表示
    void setNextBackupTime(const QDateTime &time);

signals:
    // 設定変更シグナルを追加
    void settingsChanged();

private:
    void setupUI();
    void saveSettings();
    void updateNextBackupDisplay(); // この行を追加
    void loadSettings();            // 追加: 設定読み込みメソッド

    // QSettings変数を追加
    QSettings m_settings;

    // UI要素のポインタ（必要に応じて）
    QTabWidget *m_tabWidget;
    QDialogButtonBox *m_buttonBox;

    // 背景設定用のUI変数
    QCheckBox *m_useBackgroundCheck;
    QLineEdit *m_backgroundPathEdit;
    QSlider *m_opacitySlider;
    QLabel *m_imagePreview;

private:
    QCheckBox *m_scheduleEnabledCheckBox;
    QTimeEdit *m_scheduledTimeEdit;

    QCheckBox *m_periodicEnabledCheckBox;
    QSpinBox *m_periodicIntervalSpinBox;

    QLabel *m_nextBackupTimeLabel;
};

#endif // SETTINGSDIALOG_H