#include "ScheduleSettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

ScheduleSettingsDialog::ScheduleSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

void ScheduleSettingsDialog::setupUI()
{
    setWindowTitle(tr("バックアップスケジュール設定"));
    setMinimumWidth(350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // スケジュール有効/無効チェックボックス
    enableScheduleCheckBox = new QCheckBox(tr("自動バックアップを有効にする"), this);
    mainLayout->addWidget(enableScheduleCheckBox);

    // スケジュールタイプの選択
    QGroupBox *scheduleTypeGroup = new QGroupBox(tr("スケジュール種別"), this);
    QVBoxLayout *scheduleTypeLayout = new QVBoxLayout(scheduleTypeGroup);

    fixedTimeRadio = new QRadioButton(tr("指定時刻に実行"), this);
    intervalRadio = new QRadioButton(tr("定期的に実行"), this);

    // ボタングループを作成
    scheduleTypeBtnGroup = new QButtonGroup(this);
    scheduleTypeBtnGroup->addButton(fixedTimeRadio, 0);
    scheduleTypeBtnGroup->addButton(intervalRadio, 1);
    fixedTimeRadio->setChecked(true);

    scheduleTypeLayout->addWidget(fixedTimeRadio);
    scheduleTypeLayout->addWidget(intervalRadio);

    mainLayout->addWidget(scheduleTypeGroup);

    // 時刻指定
    QGroupBox *timeSettingsGroup = new QGroupBox(tr("時刻設定"), this);
    QVBoxLayout *timeSettingsLayout = new QVBoxLayout(timeSettingsGroup);

    QHBoxLayout *fixedTimeLayout = new QHBoxLayout();
    QLabel *fixedTimeLabel = new QLabel(tr("実行時刻:"), this);
    timeEdit = new QTimeEdit(QTime(23, 00), this); // デフォルトは23:00
    timeEdit->setDisplayFormat("HH:mm");
    fixedTimeLayout->addWidget(fixedTimeLabel);
    fixedTimeLayout->addWidget(timeEdit);

    QHBoxLayout *intervalLayout = new QHBoxLayout();
    QLabel *intervalLabel = new QLabel(tr("実行間隔:"), this);
    intervalSpinBox = new QSpinBox(this);
    intervalSpinBox->setMinimum(1);
    intervalSpinBox->setMaximum(24);
    intervalSpinBox->setValue(6); // デフォルトは6時間
    QLabel *hoursLabel = new QLabel(tr("時間"), this);
    intervalLayout->addWidget(intervalLabel);
    intervalLayout->addWidget(intervalSpinBox);
    intervalLayout->addWidget(hoursLabel);

    timeSettingsLayout->addLayout(fixedTimeLayout);
    timeSettingsLayout->addLayout(intervalLayout);

    mainLayout->addWidget(timeSettingsGroup);

    // OKキャンセルボタン
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton(tr("OK"), this);
    QPushButton *cancelButton = new QPushButton(tr("キャンセル"), this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    // シグナルスロット接続
    connect(enableScheduleCheckBox, &QCheckBox::toggled, scheduleTypeGroup, &QGroupBox::setEnabled);
    connect(enableScheduleCheckBox, &QCheckBox::toggled, timeSettingsGroup, &QGroupBox::setEnabled);

    connect(fixedTimeRadio, &QRadioButton::toggled, [=](bool checked)
            { timeEdit->setEnabled(checked); });

    connect(intervalRadio, &QRadioButton::toggled, [=](bool checked)
            { intervalSpinBox->setEnabled(checked); });

    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // 初期状態
    enableScheduleCheckBox->setChecked(false);
    scheduleTypeGroup->setEnabled(false);
    timeSettingsGroup->setEnabled(false);
    intervalSpinBox->setEnabled(false);
}

bool ScheduleSettingsDialog::isScheduleEnabled() const
{
    return enableScheduleCheckBox->isChecked();
}

bool ScheduleSettingsDialog::isFixedTimeSchedule() const
{
    return fixedTimeRadio->isChecked();
}

QTime ScheduleSettingsDialog::scheduledTime() const
{
    return timeEdit->time();
}

int ScheduleSettingsDialog::intervalHours() const
{
    return intervalSpinBox->value();
}

void ScheduleSettingsDialog::setScheduleEnabled(bool enabled)
{
    enableScheduleCheckBox->setChecked(enabled);
}

void ScheduleSettingsDialog::setFixedTimeSchedule(bool isFixed)
{
    if (isFixed)
    {
        fixedTimeRadio->setChecked(true);
    }
    else
    {
        intervalRadio->setChecked(true);
    }
}

void ScheduleSettingsDialog::setScheduledTime(const QTime &time)
{
    timeEdit->setTime(time);
}

void ScheduleSettingsDialog::setIntervalHours(int hours)
{
    intervalSpinBox->setValue(hours);
}