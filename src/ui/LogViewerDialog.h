#ifndef LOGVIEWERDIALOG_H
#define LOGVIEWERDIALOG_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QPushButton>

class LogViewerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LogViewerDialog(QWidget *parent = nullptr);

    // ログにエントリを追加（古い方法との互換性のため）
    void appendLogEntry(const QString &entry);

    // ログをすべて取得
    QString getAllLogs() const;

    // ログを更新 - 新機能
    void refreshLogs();

private slots:
    void saveLog();
    void clearLog();

private:
    void setupUI();

    QPlainTextEdit *m_logTextEdit;
    QPushButton *m_saveButton;
    QPushButton *m_clearButton;
    QPushButton *m_closeButton;
};

#endif // LOGVIEWERDIALOG_H