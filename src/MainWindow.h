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
#include <QTableWidget> // 追加: QTableWidgetのインクルード
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
    void testBackgroundImage(); // 追加: テスト用背景画像選択関数

private slots:
    void showBackupDialog();
    void updateBackupProgress(int progress);
    void backupComplete();
    void runBackup(const BackupConfig &config);
    void removeBackup(int index);
    void runAllBackups();
    void showSettingsDialog();
    void handleScheduledBackup();
    void showLogDialog();
    void switchToCardView();                                // カードビューに切り替え
    void switchToListView();                                // リストビューに切り替え
    void handleTableItemDoubleClicked(int row, int column); // テーブルアイテムダブルクリック
    void showTableContextMenu(const QPoint &pos);           // テーブルのコンテキストメニュー
    void showBackgroundDialog();                            // 背景画像設定ダイアログを表示するスロット
    void setBackgroundImage(const QString &imagePath);      // 背景画像を設定するスロット
    void clearBackgroundImage();                            // 背景画像をクリアするスロット
    void addBackup();                                       // 追加
    void showDisabledBackgroundDialog();                    // 追加

protected:
    void paintEvent(QPaintEvent *event) override; // 背景を描画するために必要

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
    void updateTableView(); // テーブルビューを更新
    void clearBackupCards();
    void processNextBackup();
    void loadSchedulerSettings();
    void saveSchedulerSettings();
    void switchViewMode(ViewMode mode); // 表示モード切替

    BackupEngine *backupEngine;
    ConfigManager *configManager;
    BackupScheduler *backupScheduler;

    // UI要素
    QScrollArea *scrollArea;
    QWidget *backupsContainer;
    QGridLayout *backupsLayout;
    QTableWidget *backupTableWidget; // QListWidgetからQTableWidgetに変更
    QStackedWidget *viewStack;

    // ボタン
    QPushButton *addBackupButton;
    QPushButton *runAllBackupsButton;
    QPushButton *settingsButton;
    QPushButton *logButton;
    QToolButton *viewModeButton;

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

    // 背景画像関連
    QPixmap m_backgroundImage;
    QString m_backgroundImagePath;
    bool m_useBackgroundImage;
    int m_backgroundOpacity;

    // 設定の読み込み/保存
    void loadBackgroundSettings();
    void saveBackgroundSettings();
    void setDefaultBackground();  // 追加: デフォルト背景設定関数
    void setupTransparentStyle(); // 追加: 透明スタイル設定メソッド
};

#endif // MAINWINDOW_H