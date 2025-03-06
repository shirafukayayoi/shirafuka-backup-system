#ifndef BACKUPDIALOG_H
#define BACKUPDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QCheckBox>
#include <QProgressBar>
#include <QMessageBox>
#include <QCloseEvent> // 追加: QCloseEvent用
#include <QTimer>      // 追加: QTimer用
#include "../models/BackupConfig.h"
#include "FolderSelector.h" // FolderSelectorをインクルード

class BackupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BackupDialog(QWidget *parent = nullptr);
    explicit BackupDialog(const BackupConfig &config, QWidget *parent = nullptr);
    ~BackupDialog(); // デストラクタを追加

    QString title() const;
    QString sourcePath() const;
    QString destinationPath() const;

    void setTitle(const QString &title);
    void setSourcePath(const QString &path);
    void setDestinationPath(const QString &path);

    BackupConfig getBackupConfig() const;

public slots:
    void updateProgress(int value);
    void backupFinished();
    void accept() override;
    void reject() override;
    void done(int result) override; // 追加: done()をオーバーライド

signals:
    void backupRequested(const QString &source, const QString &destination);
    void backupCancelled();

private slots:
    void startBackup();
    void cancelBackup();
    void validateInput();
    void updateTitleFromSourcePath();
    void browseSourcePath();
    void browseDestinationPath();

protected:
    void closeEvent(QCloseEvent *event) override; // QEvent -> QCloseEvent に変更

private:
    void setupUI();
    void loadFromConfig(const BackupConfig &config);

    QLineEdit *titleEdit;
    FolderSelector *sourceSelector;
    FolderSelector *destSelector;
    QProgressBar *progressBar;
    QPushButton *startButton;
    QPushButton *cancelButton;

    QLineEdit *nameEdit;
    QLineEdit *sourcePathEdit;
    QLineEdit *destPathEdit;
    QPushButton *sourceBrowseButton;
    QPushButton *destBrowseButton;
    QPushButton *okButton;

    // 追加: 除外設定用のコントロール
    QTabWidget *tabWidget;                  // タブでUI整理
    QPlainTextEdit *excludedFilesEdit;      // 除外ファイル
    QPlainTextEdit *excludedFoldersEdit;    // 除外フォルダー
    QPlainTextEdit *excludedExtensionsEdit; // 除外拡張子
    QCheckBox *showHiddenFilesCheck;        // オプション：隠しファイルを表示するかのオプション
};

#endif // BACKUPDIALOG_H