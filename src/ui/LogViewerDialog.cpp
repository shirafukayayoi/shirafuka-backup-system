#include "LogViewerDialog.h"
#include "../utils/Logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

LogViewerDialog::LogViewerDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();

    // 既存のログをロード
    refreshLogs();
}

void LogViewerDialog::setupUI()
{
    setWindowTitle(tr("バックアップログ"));
    resize(700, 500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // ログテキストエディタ
    m_logTextEdit = new QPlainTextEdit(this);
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    m_logTextEdit->setFont(QFont("Consolas", 9)); // モノスペースフォントを使用

    mainLayout->addWidget(m_logTextEdit);

    // ボタンレイアウト
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *refreshButton = new QPushButton(tr("更新"), this);
    m_saveButton = new QPushButton(tr("ログを保存"), this);
    m_clearButton = new QPushButton(tr("ログをクリア"), this);
    m_closeButton = new QPushButton(tr("閉じる"), this);

    buttonLayout->addWidget(refreshButton);
    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeButton);

    mainLayout->addLayout(buttonLayout);

    // シグナル/スロット接続
    connect(refreshButton, &QPushButton::clicked, this, &LogViewerDialog::refreshLogs);
    connect(m_saveButton, &QPushButton::clicked, this, &LogViewerDialog::saveLog);
    connect(m_clearButton, &QPushButton::clicked, this, &LogViewerDialog::clearLog);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

void LogViewerDialog::refreshLogs()
{
    // ログテキストをクリアして再ロード
    m_logTextEdit->clear();

    // Loggerからすべてのログを取得
    QStringList logs = Logger::instance().getAllLogs();

    // テキストエディタに追加
    for (const QString &log : logs)
    {
        m_logTextEdit->appendPlainText(log);
    }

    // スクロールを最下部に移動
    QTextCursor cursor = m_logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_logTextEdit->setTextCursor(cursor);
}

void LogViewerDialog::appendLogEntry(const QString &entry)
{
    // このメソッドは、古いメソッドとの互換性のために保持
    // 実際にはここでログに追加せず、Loggerを使用
    Logger::instance().log(entry);
    refreshLogs();
}

QString LogViewerDialog::getAllLogs() const
{
    return m_logTextEdit->toPlainText();
}

void LogViewerDialog::clearLog()
{
    if (m_logTextEdit->document()->isEmpty())
        return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("ログの消去"),
        tr("すべてのログエントリを消去しますか？"),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        Logger::instance().clearLogs();
        refreshLogs();
    }
}

void LogViewerDialog::saveLog()
{
    if (m_logTextEdit->document()->isEmpty())
    {
        QMessageBox::information(this, tr("保存できません"), tr("ログが空です。"));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this, tr("ログの保存"),
        QDir::homePath() + "/backup_log_" +
            QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".txt",
        tr("テキストファイル (*.txt)"));

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("エラー"),
                             tr("ファイルを保存できませんでした: %1").arg(file.errorString()));
        return;
    }

    QTextStream out(&file);
    out << m_logTextEdit->toPlainText();
    file.close();

    QMessageBox::information(this, tr("保存完了"),
                             tr("ログを保存しました: %1").arg(fileName));
}