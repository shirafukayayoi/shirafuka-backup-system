#ifndef SCHEDULESETTINGSDIALOG_H
#define SCHEDULESETTINGSDIALOG_H

#include <QDialog>
#include <QTimeEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>

class ScheduleSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScheduleSettingsDialog(QWidget *parent = nullptr);

    bool isScheduleEnabled() const;
    bool isFixedTimeSchedule() const;
    QTime scheduledTime() const;
    int intervalHours() const;

    void setScheduleEnabled(bool enabled);
    void setFixedTimeSchedule(bool isFixed);
    void setScheduledTime(const QTime &time);
    void setIntervalHours(int hours);

private:
    void setupUI();

    QCheckBox *enableScheduleCheckBox;

    QRadioButton *fixedTimeRadio;
    QRadioButton *intervalRadio;
    QButtonGroup *scheduleTypeBtnGroup; // 変数名要修正: scheduleTypeGroupではなく

    QTimeEdit *timeEdit;
    QSpinBox *intervalSpinBox;
};

#endif // SCHEDULESETTINGSDIALOG_H