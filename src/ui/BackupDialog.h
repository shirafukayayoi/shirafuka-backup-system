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
#include <QCloseEvent>
#include <QTimer>
#include <QRadioButton> // 追加: QRadioButtonのヘッダー
#include "../models/BackupConfig.h"
#include "FolderSelector.h" // FolderSelectorをインクルード

class BackupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BackupDialog(QWidget *parent = nullptr);
    explicit BackupDialog(const BackupConfig &config, QWidget *parent = nullptr);
    ~BackupDialog(); // デストラクタの宣言のみ、実装は削除

    QString title() const;
    QString sourcePath() const;
    QString destinationPath() const;

    void setTitle(const QString &title);
    void setSourcePath(const QString &path);
    void setDestinationPath(const QString &path);

    BackupConfig getBackupConfig() const;

    // バックアップモード追加
    enum BackupMode
    {
        StandardBackup,
        GameSaveBackup
    };

    BackupMode backupMode() const;
    void setBackupMode(BackupMode mode);
    QStringList saveDataFolderNames() const;

protected:
    // 重複宣言を削除して一つに統一
    void closeEvent(QCloseEvent *event) override;
    void reject() override;

private slots:
    void startBackup();
    void cancelBackup();
    void validateInput();
    void updateTitleFromSourcePath();
    void browseSourcePath();
    void browseDestinationPath();

public slots:
    void updateProgress(int value);
    void backupFinished();
    void accept() override;
    // 重複している reject() の宣言をここから削除
    void done(int result) override;

signals:
    void backupRequested(const QString &source, const QString &destination);
    void backupCancelled();

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

    // セーブデータバックアップ関連のUI要素
    QRadioButton *standardBackupRadio;
    QRadioButton *saveDataBackupRadio;
    QPlainTextEdit *saveDataFoldersEdit;
    BackupMode m_backupMode;
    QStringList m_saveDataFolderNames;
};

#endif // BACKUPDIALOG_H