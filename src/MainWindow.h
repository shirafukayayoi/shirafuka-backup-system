#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScrollArea>
#include <QGridLayout>
#include <QListWidget>
#include <QPushButton>
#include <QQueue>
#include <QTimer>
#include <QTime>
#include <QDateTime>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QToolButton>
#include <QTableWidget>
#include <QPixmap>
#include <QBrush>

#include "backup/BackupEngine.h"
#include "config/ConfigManager.h"
#include "models/BackupConfig.h"
#include "ui/BackupCard.h"
#include "scheduler/BackupScheduler.h"
#include "ui/LogViewerDialog.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    // 背景画像テスト関数を削除

private slots:
    void showBackupDialog();
    void updateBackupProgress(int progress);
    void backupComplete();
    void runBackup(const BackupConfig &config);
    void removeBackup(int index);
    void editBackup(int index); // 追加: 編集スロット
    void runAllBackups();
    void showSettingsDialog();
    void handleScheduledBackup();
    void showLogDialog();
    void switchToCardView();
    void switchToListView();
    void handleTableItemDoubleClicked(int row, int column);
    void showTableContextMenu(const QPoint &pos);
    // 背景画像関連のスロットを削除
    void addBackup();

    // 新しいスロット - ファイル単位のバックアップ状況をログに記録
    void onFileProcessed(const QString &filePath, bool success);
    void onDirectoryProcessed(const QString &dirPath, bool created);
    void onBackupLogMessage(const QString &message);

protected:
    // paintEvent を削除（背景描画に使用していたため）

public:
    void addLogEntry(const QString &entry);

private:
    enum ViewMode
    {
        CardView,
        ListView
    };

    void setupUI();
    void loadBackupConfigs();
    void saveBackupConfigs();
    void addBackupCard(const BackupConfig &config);
    void updateTableView();
    void clearBackupCards();
    void processNextBackup();
    void loadSchedulerSettings();
    void saveSchedulerSettings();
    void switchViewMode(ViewMode mode);

    BackupEngine *backupEngine;
    ConfigManager *configManager;
    BackupScheduler *backupScheduler;

    // UI要素
    QScrollArea *scrollArea;
    QWidget *backupsContainer;
    QGridLayout *backupsLayout;
    QTableWidget *backupTableWidget;
    QStackedWidget *viewStack;

    // ボタン
    QPushButton *addBackupButton;
    QPushButton *runAllBackupsButton;
    QPushButton *settingsButton;
    QPushButton *logButton;
    QToolButton *viewModeButton;
    QPushButton *batchBackupButton;

    // バックアップ管理
    QList<BackupCard *> backupCards;

    // 一括バックアップのキュー管理
    QQueue<BackupConfig> backupQueue;
    int totalBackupsInQueue;
    int currentBackupIndex;
    bool isRunningBatchBackup;

    // ログダイアログ
    LogViewerDialog *logDialog;

    // 現在の表示モード
    ViewMode currentViewMode;

    bool isAutomaticBackup; // 自動バックアップフラグ

    // 背景画像関連のメンバ変数を削除

    // 背景画像設定の読み込み/保存メソッドを削除
};

#endif // MAINWINDOW_H