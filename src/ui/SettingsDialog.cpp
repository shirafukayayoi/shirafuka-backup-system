#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include <QLineEdit>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDebug>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent),
      // 直接初期化リストでQSettingsを初期化
      m_settings(QDir::homePath() + "/shirafuka_settings.ini", QSettings::IniFormat),
      m_tabWidget(nullptr),
      m_buttonBox(nullptr)
// 背景関連の変数を削除
{
    setWindowTitle(tr("設定"));
    resize(500, 400);

    qDebug() << "設定ダイアログが使用する設定ファイル: " << m_settings.fileName();

    setupUI();
    loadSettings(); // 設定を読み込む
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::setupUI()
{
    // メインレイアウト
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // タブウィジェット
    m_tabWidget = new QTabWidget(this);

    // --- 外観設定タブ削除 ---
    // 背景画像設定のグループボックスとすべての関連コードを削除

    // --- スケジュール設定タブ ---
    QWidget *scheduleTab = new QWidget(m_tabWidget);
    QVBoxLayout *scheduleLayout = new QVBoxLayout(scheduleTab);

    // 定時バックアップのグループボックス
    QGroupBox *dailyGroup = new QGroupBox(tr("定時バックアップ"), scheduleTab);
    QVBoxLayout *dailyLayout = new QVBoxLayout(dailyGroup);

    // 定時バックアップ有効/無効チェックボックス
    m_scheduleEnabledCheckBox = new QCheckBox(tr("指定した時刻にバックアップを実行する"));
    dailyLayout->addWidget(m_scheduleEnabledCheckBox);

    // 時刻設定
    QHBoxLayout *timeLayout = new QHBoxLayout();
    QLabel *timeLabel = new QLabel(tr("実行時刻:"));
    m_scheduledTimeEdit = new QTimeEdit(QTime(23, 0)); // デフォルトは23:00
    m_scheduledTimeEdit->setDisplayFormat("HH:mm");

    timeLayout->addWidget(timeLabel);
    timeLayout->addWidget(m_scheduledTimeEdit);
    timeLayout->addStretch();
    dailyLayout->addLayout(timeLayout);

    // 定期バックアップのグループボックス
    QGroupBox *periodicGroup = new QGroupBox(tr("定期バックアップ"), scheduleTab);
    QVBoxLayout *periodicLayout = new QVBoxLayout(periodicGroup);

    // 定期バックアップ有効/無効チェックボックス
    m_periodicEnabledCheckBox = new QCheckBox(tr("一定時間ごとにバックアップを実行する"));
    periodicLayout->addWidget(m_periodicEnabledCheckBox);

    // 間隔設定
    QHBoxLayout *intervalLayout = new QHBoxLayout();
    QLabel *intervalLabel = new QLabel(tr("実行間隔:"));
    m_periodicIntervalSpinBox = new QSpinBox();
    m_periodicIntervalSpinBox->setRange(1, 24);
    m_periodicIntervalSpinBox->setValue(8); // デフォルトは8時間
    m_periodicIntervalSpinBox->setSuffix(tr(" 時間"));

    intervalLayout->addWidget(intervalLabel);
    intervalLayout->addWidget(m_periodicIntervalSpinBox);
    intervalLayout->addStretch();
    periodicLayout->addLayout(intervalLayout);

    // 次回バックアップ時間表示
    QGroupBox *nextBackupGroup = new QGroupBox(tr("次回のバックアップ"), scheduleTab);
    QVBoxLayout *nextBackupLayout = new QVBoxLayout(nextBackupGroup);

    m_nextBackupTimeLabel = new QLabel(tr("バックアップが設定されていません"));
    m_nextBackupTimeLabel->setAlignment(Qt::AlignCenter);
    QFont font = m_nextBackupTimeLabel->font();
    font.setBold(true);
    m_nextBackupTimeLabel->setFont(font);

    nextBackupLayout->addWidget(m_nextBackupTimeLabel);

    // タブにグループボックスを追加
    scheduleLayout->addWidget(dailyGroup);
    scheduleLayout->addWidget(periodicGroup);
    scheduleLayout->addWidget(nextBackupGroup);
    scheduleLayout->addStretch();

    // シグナル/スロット接続（スケジュール関連）
    connect(m_scheduleEnabledCheckBox, &QCheckBox::toggled, [=](bool checked)
            {
        m_scheduledTimeEdit->setEnabled(checked);
        updateNextBackupDisplay(); });

    connect(m_periodicEnabledCheckBox, &QCheckBox::toggled, [=](bool checked)
            {
        m_periodicIntervalSpinBox->setEnabled(checked);
        updateNextBackupDisplay(); });

    connect(m_scheduledTimeEdit, &QTimeEdit::timeChanged, this, &SettingsDialog::updateNextBackupDisplay);
    connect(m_periodicIntervalSpinBox, &QSpinBox::valueChanged, this, &SettingsDialog::updateNextBackupDisplay);

    // 背景設定関連のコード削除

    // タブに追加 (外観タブを削除して、スケジュールタブのみに)
    m_tabWidget->addTab(scheduleTab, tr("スケジュール"));

    mainLayout->addWidget(m_tabWidget);

    // ダイアログボタン
    m_buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal,
        this);

    mainLayout->addWidget(m_buttonBox);

    // OKボタン接続
    connect(m_buttonBox, &QDialogButtonBox::accepted, [this]()
            {
        saveSettings();
        emit settingsChanged();
        accept(); });

    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // 初期状態の設定
    m_scheduledTimeEdit->setEnabled(m_scheduleEnabledCheckBox->isChecked());
    m_periodicIntervalSpinBox->setEnabled(m_periodicEnabledCheckBox->isChecked());
}

// 設定保存処理から背景設定を削除
void SettingsDialog::saveSettings()
{
    // 背景画像設定の保存コードを削除

    // 明示的なファイルパスで保存
    QString settingsPath = QDir::homePath() + "/shirafuka_settings.ini";
    QSettings settings(settingsPath, QSettings::IniFormat);

    // スケジュール設定の保存
    settings.setValue("Schedule/ScheduleEnabled", m_scheduleEnabledCheckBox->isChecked());
    settings.setValue("Schedule/ScheduledTime", m_scheduledTimeEdit->time().toString("hh:mm"));
    settings.setValue("Schedule/PeriodicEnabled", m_periodicEnabledCheckBox->isChecked());
    settings.setValue("Schedule/PeriodicInterval", m_periodicIntervalSpinBox->value());

    // 設定を即時に反映させる
    settings.sync();

    // 設定が保存されたことを確認
    qDebug() << "設定ファイル: " << settings.fileName();

    // 同じ設定ファイルを使うように他のクラスに知らせる
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QDir::homePath());
}

bool SettingsDialog::isScheduleEnabled() const
{
    return m_scheduleEnabledCheckBox->isChecked();
}

void SettingsDialog::setScheduleEnabled(bool enabled)
{
    m_scheduleEnabledCheckBox->setChecked(enabled);
    m_scheduledTimeEdit->setEnabled(enabled);
}

QTime SettingsDialog::scheduledTime() const
{
    return m_scheduledTimeEdit->time();
}

void SettingsDialog::setScheduledTime(const QTime &time)
{
    m_scheduledTimeEdit->setTime(time);
}

bool SettingsDialog::isPeriodicBackupEnabled() const
{
    return m_periodicEnabledCheckBox->isChecked();
}

void SettingsDialog::setPeriodicBackupEnabled(bool enabled)
{
    m_periodicEnabledCheckBox->setChecked(enabled);
    m_periodicIntervalSpinBox->setEnabled(enabled);
}

int SettingsDialog::periodicInterval() const
{
    return m_periodicIntervalSpinBox->value();
}

void SettingsDialog::setPeriodicInterval(int hours)
{
    m_periodicIntervalSpinBox->setValue(hours);
}

void SettingsDialog::setNextBackupTime(const QDateTime &time)
{
    if (time.isValid())
    {
        m_nextBackupTimeLabel->setText(tr("次回のバックアップ: %1").arg(time.toString("yyyy/MM/dd HH:mm")));
    }
    else
    {
        m_nextBackupTimeLabel->setText(tr("バックアップが設定されていません"));
    }
}

void SettingsDialog::updateNextBackupDisplay()
{
    if (isScheduleEnabled() || isPeriodicBackupEnabled())
    {
        // 次回バックアップ時刻の計算ロジックはここでは単純化
        QDateTime now = QDateTime::currentDateTime();
        QDateTime nextTime;

        if (isScheduleEnabled())
        {
            QDateTime scheduledDateTime(now.date(), scheduledTime());
            if (scheduledDateTime <= now)
            {
                scheduledDateTime = scheduledDateTime.addDays(1);
            }
            nextTime = scheduledDateTime;
        }

        if (isPeriodicBackupEnabled())
        {
            QDateTime periodicTime = now.addSecs(periodicInterval() * 3600);
            if (!nextTime.isValid() || periodicTime < nextTime)
            {
                nextTime = periodicTime;
            }
        }

        setNextBackupTime(nextTime);
    }
    else
    {
        setNextBackupTime(QDateTime());
    }
}

// 設定読み込みメソッドから背景設定を削除
void SettingsDialog::loadSettings()
{
    // 背景設定を読み込むコードを削除

    // スケジュール設定を読み込む
    bool scheduleEnabled = m_settings.value("Schedule/ScheduleEnabled", false).toBool();
    QString timeStr = m_settings.value("Schedule/ScheduledTime", "23:00").toString();
    bool periodicEnabled = m_settings.value("Schedule/PeriodicEnabled", false).toBool();
    int periodicInterval = m_settings.value("Schedule/PeriodicInterval", 8).toInt();

    // UIに反映
    m_scheduleEnabledCheckBox->setChecked(scheduleEnabled);
    m_scheduledTimeEdit->setTime(QTime::fromString(timeStr, "hh:mm"));
    m_periodicEnabledCheckBox->setChecked(periodicEnabled);
    m_periodicIntervalSpinBox->setValue(periodicInterval);

    // 次回バックアップ表示を更新
    updateNextBackupDisplay();
}