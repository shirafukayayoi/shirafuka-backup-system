#include "MainWindow.h"
#include "ui/BackupDialog.h"
#include "ui/BackupCard.h"
#include "ui/SettingsDialog.h"
#include "utils/Logger.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QDir>
#include <QScrollArea>
#include <QJsonDocument>
#include <QScrollBar>
#include <QSettings>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPointer>
#include <QDebug>
#include <QApplication> // 追加: QApplicationクラスをインクルード

// MainWindowのコンストラクタで背景関連の初期化を削除
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      backupEngine(new BackupEngine(this)),
      isRunningBatchBackup(false),
      currentBackupIndex(0),
      totalBackupsInQueue(0),
      logDialog(nullptr),
      currentViewMode(CardView),
      isAutomaticBackup(false) // 追加
{
    // アイコンを設定 - パスを修正
    setWindowIcon(QIcon(":/src/icons/app_icon.png"));

    // 設定ファイルのパスを定義
    QString configPath = QDir::homePath() + "/.shirafuka_backup/config.json";

    // 設定ディレクトリが存在することを確認
    QDir configDir(QDir::homePath() + "/.shirafuka_backup");
    if (!configDir.exists())
    {
        configDir.mkpath(".");
    }

    // 設定マネージャーを作成
    configManager = new ConfigManager(configPath, this);
    configManager->loadConfig();

    // UI のセットアップ
    setupUI();

    // バックアップスケジューラを作成
    backupScheduler = new BackupScheduler(this);
    connect(backupScheduler, &BackupScheduler::backupTimerTriggered,
            this, &MainWindow::handleScheduledBackup);

    // バックアップリストを読み込む
    loadBackupConfigs();

    // スケジュール設定を読み込む
    loadSchedulerSettings();

    // シグナル/スロット接続
    connect(backupEngine, &BackupEngine::backupProgress, this, &MainWindow::updateBackupProgress);
    connect(backupEngine, &BackupEngine::backupCompleted, this, &MainWindow::backupComplete);

    // 新しいシグナル接続 - ファイル単位のログ記録用
    connect(backupEngine, &BackupEngine::fileProcessed, this, &MainWindow::onFileProcessed);
    connect(backupEngine, &BackupEngine::directoryProcessed, this, &MainWindow::onDirectoryProcessed);
    connect(backupEngine, &BackupEngine::backupLogMessage, this, &MainWindow::onBackupLogMessage);
}

MainWindow::~MainWindow()
{
    saveSchedulerSettings();
    saveBackupConfigs();
}

// setupUIメソッドでグリッドレイアウトの調整部分を修正
void MainWindow::setupUI()
{
    setWindowTitle("Shirafuka-Backup-System");
    resize(1600, 900);

    // メニューバーを非表示にする
    menuBar()->setVisible(false);

    // 表示スタックウィジェット（カードとリストの切り替え用）を作成
    viewStack = new QStackedWidget(this);

    // カードビュー
    scrollArea = new QScrollArea(viewStack);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    backupsContainer = new QWidget(scrollArea);
    backupsLayout = new QGridLayout(backupsContainer);

    // グリッドレイアウトの調整 - ここを修正
    backupsLayout->setSpacing(12);                     // 間隔を広げる
    backupsLayout->setVerticalSpacing(15);             // 垂直方向の間隔を広げる
    backupsLayout->setHorizontalSpacing(15);           // 水平方向も広げる
    backupsLayout->setContentsMargins(12, 12, 12, 12); // マージンを広げる
    backupsLayout->setAlignment(Qt::AlignTop);         // 上揃えで配置

    backupsContainer->setLayout(backupsLayout);

    scrollArea->setWidget(backupsContainer);
    viewStack->addWidget(scrollArea); // スタックに追加

    // リストビューをテーブルビューに変更
    backupTableWidget = new QTableWidget(viewStack);
    backupTableWidget->setColumnCount(4); // 4列: 名前、元パス、先パス、最終更新
    backupTableWidget->setHorizontalHeaderLabels(
        QStringList() << tr("バックアップ名") << tr("元パス") << tr("先パス") << tr("最終バックアップ"));

    // ヘッダースタイル設定
    backupTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    backupTableWidget->horizontalHeader()->setStretchLastSection(true);
    backupTableWidget->horizontalHeader()->setSectionsClickable(false);
    backupTableWidget->verticalHeader()->setVisible(false); // 行番号非表示

    // 枠線と選択スタイルの設定
    backupTableWidget->setShowGrid(true);
    backupTableWidget->setGridStyle(Qt::SolidLine);
    backupTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    backupTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    backupTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); // 編集不可
    viewStack->addWidget(backupTableWidget);                               // スタックに追加

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(viewStack);

    // ボタンレイアウト
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // バックアップ追加ボタン
    addBackupButton = new QPushButton("バックアップを追加", this);

    // 一括バックアップボタン - パスを修正
    batchBackupButton = new QPushButton("全てをバックアップ", this);
    batchBackupButton->setIcon(QIcon(":/src/icons/backup-all.png"));

    // runAllBackupsButtonは削除または使用しない
    // runAllBackupsButton = new QPushButton("すべてバックアップ", this);

    // 表示切替ボタン
    viewModeButton = new QToolButton(this);
    viewModeButton->setPopupMode(QToolButton::InstantPopup);
    viewModeButton->setText("表示方法");
    viewModeButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    // 表示切替メニュー
    QMenu *viewMenu = new QMenu(this);
    QAction *cardViewAction = viewMenu->addAction("カード表示");
    QAction *listViewAction = viewMenu->addAction("リスト表示");
    viewModeButton->setMenu(viewMenu);

    // 設定ボタン（追加）
    settingsButton = new QPushButton("設定", this);

    // ログボタン（新規追加）
    logButton = new QPushButton("ログ", this);

    buttonLayout->addWidget(addBackupButton);
    buttonLayout->addWidget(batchBackupButton); // これだけ追加
    // buttonLayout->addWidget(runAllBackupsButton); // この行は削除
    buttonLayout->addStretch();
    buttonLayout->addWidget(viewModeButton);
    buttonLayout->addWidget(settingsButton);
    buttonLayout->addWidget(logButton); // 新規追加

    // コネクト - runAllBackups に接続（ヘッダーですでに宣言されているため）
    connect(batchBackupButton, &QPushButton::clicked, this, &MainWindow::runAllBackups);
    // connect(runAllBackupsButton, &QPushButton::clicked, this, &MainWindow::runAllBackups);

    mainLayout->addLayout(buttonLayout);

    // ステータスバーの設定
    statusBar()->showMessage("準備完了");

    // 中央ウィジェットを設定
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // シグナル/スロット接続
    connect(addBackupButton, &QPushButton::clicked, this, &MainWindow::showBackupDialog);
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::showSettingsDialog);
    connect(logButton, &QPushButton::clicked, this, &MainWindow::showLogDialog); // 新規追加
    connect(cardViewAction, &QAction::triggered, this, &MainWindow::switchToCardView);
    connect(listViewAction, &QAction::triggered, this, &MainWindow::switchToListView);
    // connect(backupListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::handleListItemDoubleClicked);

    // デフォルトはカードビュー
    switchViewMode(CardView);
}

void MainWindow::showSettingsDialog()
{
    SettingsDialog dialog(this);

    // 現在のバックアップスケジュール設定をダイアログに渡す
    dialog.setScheduleEnabled(backupScheduler->isScheduleEnabled());
    dialog.setScheduledTime(backupScheduler->scheduledTime());
    dialog.setPeriodicBackupEnabled(backupScheduler->isPeriodicBackupEnabled());
    dialog.setPeriodicInterval(backupScheduler->periodicInterval());

    // 次回予定されているバックアップ時間を表示
    dialog.setNextBackupTime(backupScheduler->nextBackupTime());

    if (dialog.exec() == QDialog::Accepted)
    {
        // スケジューラ設定を更新
        backupScheduler->setScheduleEnabled(dialog.isScheduleEnabled());
        backupScheduler->setScheduledTime(dialog.scheduledTime());
        backupScheduler->setPeriodicBackupEnabled(dialog.isPeriodicBackupEnabled());
        backupScheduler->setPeriodicInterval(dialog.periodicInterval());

        // 設定を保存
        saveSchedulerSettings();

        // 次回バックアップ時間更新をログに記録
        QDateTime nextBackup = backupScheduler->nextBackupTime();
        if (nextBackup.isValid())
        {
            addLogEntry(QString("次回のバックアップが %1 に予定されています")
                            .arg(nextBackup.toString("yyyy/MM/dd HH:mm")));
        }
        else
        {
            addLogEntry("自動バックアップは無効になっています");
        }
    }
}

void MainWindow::handleScheduledBackup()
{
    qDebug() << "スケジュールによるバックアップ開始";

    // どのタイプの予定でトリガーされたかをログに記録
    if (backupScheduler->isPeriodicBackupEnabled())
    {
        addLogEntry(QString("定期バックアップ（%1時間ごと）を自動実行します").arg(backupScheduler->periodicInterval()));
    }
    else
    {
        addLogEntry(QString("定時バックアップ（%1）を自動実行します").arg(backupScheduler->scheduledTime().toString("HH:mm")));
    }

    // 自動バックアップフラグを設定
    isAutomaticBackup = true;

    // バックアップを実行
    runAllBackups();
}

void MainWindow::loadSchedulerSettings()
{
    // 設定マネージャーからスケジュール設定を読み込む
    QJsonObject config = configManager->getConfig();

    if (config.contains("backupScheduler"))
    {
        QJsonObject schedulerConfig = config["backupScheduler"].toObject();

        // 定時バックアップ設定
        if (schedulerConfig.contains("scheduleEnabled"))
        {
            backupScheduler->setScheduleEnabled(schedulerConfig["scheduleEnabled"].toBool());
        }

        if (schedulerConfig.contains("scheduledTime"))
        {
            QTime time = QTime::fromString(schedulerConfig["scheduledTime"].toString(), "hh:mm");
            if (time.isValid())
            {
                backupScheduler->setScheduledTime(time);
            }
        }

        // 定期バックアップ設定
        if (schedulerConfig.contains("periodicEnabled"))
        {
            backupScheduler->setPeriodicBackupEnabled(schedulerConfig["periodicEnabled"].toBool());
        }

        if (schedulerConfig.contains("periodicInterval"))
        {
            backupScheduler->setPeriodicInterval(schedulerConfig["periodicInterval"].toInt());
        }

        // より詳細なデバッグ情報を追加
        qDebug() << "スケジューラ設定を読み込み中:";
        qDebug() << "  定時バックアップ有効:" << schedulerConfig["scheduleEnabled"].toBool();
        qDebug() << "  定時バックアップ時刻:" << schedulerConfig["scheduledTime"].toString();
        qDebug() << "  定期バックアップ有効:" << schedulerConfig["periodicEnabled"].toBool();
        qDebug() << "  定期バックアップ間隔:" << schedulerConfig["periodicInterval"].toInt() << "時間";
    }
}

void MainWindow::saveSchedulerSettings()
{
    // スケジュール設定を構築
    QJsonObject schedulerConfig;
    schedulerConfig["scheduleEnabled"] = backupScheduler->isScheduleEnabled();
    schedulerConfig["scheduledTime"] = backupScheduler->scheduledTime().toString("hh:mm");
    schedulerConfig["periodicEnabled"] = backupScheduler->isPeriodicBackupEnabled();
    schedulerConfig["periodicInterval"] = backupScheduler->periodicInterval();

    // 設定マネージャーに保存
    QJsonObject config = configManager->getConfig();
    config["backupScheduler"] = schedulerConfig;
    configManager->setConfig(config);
    configManager->saveConfig();
}

// loadBackupConfigsメソッドの更新
void MainWindow::loadBackupConfigs()
{
    try
    {
        qDebug() << "Loading backup configs...";
        clearBackupCards();

        QList<BackupConfig> configs = configManager->backupConfigs();
        qDebug() << "Found" << configs.size() << "backup configs";

        for (const BackupConfig &config : configs)
        {
            qDebug() << "Adding card for config:" << config.name();
            addBackupCard(config);
            QApplication::processEvents(); // UIの応答性を維持
        }

        // テーブルビューも更新
        if (currentViewMode == ListView)
        {
            updateTableView();
        }

        qDebug() << "Backup configs loaded successfully";
    }
    catch (const std::exception &e)
    {
        qDebug() << "Exception in loadBackupConfigs: " << e.what();
        QMessageBox::critical(this, tr("エラー"),
                              tr("バックアップ設定の読み込み中にエラーが発生しました: %1").arg(e.what()));
    }
    catch (...)
    {
        qDebug() << "Unknown exception in loadBackupConfigs";
        QMessageBox::critical(this, tr("エラー"),
                              tr("バックアップ設定の読み込み中に不明なエラーが発生しました"));
    }
}

void MainWindow::saveBackupConfigs()
{
    qDebug() << "Saving backup configs...";
    bool success = configManager->saveConfig();
    if (success)
    {
        qDebug() << "Backup configs saved successfully";
    }
    else
    {
        qDebug() << "Failed to save backup configs";
    }
}

// addBackupCardメソッドの更新
void MainWindow::addBackupCard(const BackupConfig &config)
{
    try
    {
        int cardIndex = backupCards.size();
        int row = cardIndex / 4; // 4列配置に変更
        int col = cardIndex % 4; // 列番号（0から3）

        qDebug() << "Creating card at position:" << row << "," << col << "for config:" << config.name();

        BackupCard *card = new BackupCard(config, cardIndex, backupsContainer);

        // カードのサイズを最適化 - より小さく
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        card->setFixedHeight(120); // 高さを小さく (150→120)

        // カードをグリッドに追加する際に位置調整
        backupsLayout->addWidget(card, row, col, 1, 1, Qt::AlignTop);

        // カードのシグナルを接続
        connect(card, &BackupCard::runBackup, this, &MainWindow::runBackup);
        connect(card, &BackupCard::editBackup, this, &MainWindow::editBackup); // 追加: 編集機能接続
        connect(card, &BackupCard::removeBackup, this, &MainWindow::removeBackup);

        backupCards.append(card);

        // テーブルビューも更新
        if (currentViewMode == ListView)
        {
            updateTableView();
        }
    }
    catch (const std::exception &e)
    {
        qDebug() << "Exception in addBackupCard:" << e.what();
    }
    catch (...)
    {
        qDebug() << "Unknown exception in addBackupCard";
    }
}

// シンプルにして設定ダイアログと同じスタイルに変更
void MainWindow::showBackupDialog()
{
    try
    {
        // スタック上にダイアログを作成し、スコープ終了時に自動的に破棄されるようにする
        BackupDialog dialog(this);

        // 標準的なモーダルダイアログとして実行
        if (dialog.exec() == QDialog::Accepted)
        {
            BackupConfig config = dialog.getBackupConfig();

            // 処理続行...
            configManager->addBackupConfig(config);
            saveBackupConfigs();
            loadBackupConfigs();
            addLogEntry(QString("新しいバックアップ '%1' を追加しました").arg(config.name()));
        }
    }
    catch (const std::exception &e)
    {
        qDebug() << "Exception in showBackupDialog: " << e.what();
        QMessageBox::critical(this, tr("エラー"),
                              tr("バックアップ設定の追加中にエラーが発生しました: %1").arg(e.what()));
    }
    catch (...)
    {
        qDebug() << "Unknown exception in showBackupDialog";
        QMessageBox::critical(this, tr("エラー"),
                              tr("バックアップ設定の追加中に不明なエラーが発生しました"));
    }
}

void MainWindow::runBackup(const BackupConfig &config)
{
    // 既に実行中のバックアップがあれば何もしない
    if (backupEngine->isRunning())
    {
        statusBar()->showMessage("バックアップが実行中です");
        addLogEntry("バックアップ要求を無視: 既に実行中のバックアップがあります");
        return;
    }

    addLogEntry(QString("バックアップ開始: %1 → %2").arg(config.sourcePath()).arg(config.destinationPath()));

    // startBackupの代わりにrunBackupを使用して設定情報を渡す
    backupEngine->runBackup(config);

    // ステータスバーメッセージを設定
    statusBar()->showMessage("バックアップ実行中...");
}

// removeBackupメソッドの更新
void MainWindow::removeBackup(int index)
{
    // 削除前に念のため確認
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "確認",
                                  "このバックアップを削除しますか？\n(バックアップファイルは削除されません)",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        configManager->removeBackupConfig(index);

        // 設定を保存して表示を更新
        saveBackupConfigs();
        loadBackupConfigs();

        // テーブルビューも更新
        if (currentViewMode == ListView)
        {
            updateTableView();
        }

        addLogEntry(tr("バックアップ設定が削除されました"));
    }
}

void MainWindow::updateBackupProgress(int progress)
{
    // 進捗をタイトルバーに表示
    if (isRunningBatchBackup)
    {
        setWindowTitle(QString("%1 - バックアップ中 %2/%3 (%4%)").arg(tr("しらふか・バックアップ"), QString::number(currentBackupIndex), QString::number(totalBackupsInQueue), QString::number(progress)));
    }

    // 進捗バーの更新
    if (isRunningBatchBackup && currentBackupIndex - 1 < backupCards.size())
    {
        BackupCard *card = backupCards[currentBackupIndex - 1];
        card->setProgress(progress);
    }
}

// バックアップ完了時のステータス更新処理を改善
void MainWindow::backupComplete()
{
    // バッチバックアップ実行中の場合
    if (isRunningBatchBackup)
    {
        qDebug() << "バックアップ完了 - 次のバックアップを開始します";

        // 次のバックアップを処理
        processNextBackup();
    }
    else
    {
        // 単体バックアップ完了メッセージ
        statusBar()->showMessage(tr("バックアップが完了しました"), 5000);
        addLogEntry("バックアップが完了しました");

        // ウィンドウタイトルを元に戻す
        setWindowTitle(tr("しらふか・バックアップ"));
    }
}

// processNextBackupメソッド内のカード進捗表示部分を修正
void MainWindow::processNextBackup()
{
    qDebug() << "processNextBackup called, queue size: " << backupQueue.size();
    if (backupQueue.isEmpty())
    {
        // すべてのバックアップが完了
        isRunningBatchBackup = false;
        statusBar()->showMessage(tr("すべてのバックアップが完了しました"), 5000);

        // 自動バックアップフラグをリセット
        isAutomaticBackup = false;

        QMessageBox::information(this, tr("バックアップ完了"), tr("すべてのバックアップが完了しました。"));
        qDebug() << "すべてのバックアップが完了しました";
        return;
    }

    // キューから次のバックアップ設定を取得
    BackupConfig config = backupQueue.dequeue();
    currentBackupIndex++;

    qDebug() << "バックアップ実行: " << config.name() << " (" << currentBackupIndex
             << "/" << totalBackupsInQueue << ")";

    // 設定を直接渡して、セーブデータモードを尊重
    backupEngine->runBackup(config);

    // カードの進捗表示を更新（オプション）
    int cardIndex = configManager->findConfigIndex(config);
    if (cardIndex >= 0 && cardIndex < backupCards.size())
    {
        // startProgress()ではなくsetProgress()を使用
        backupCards[cardIndex]->setProgress(0); // 進捗を0%から開始
    }
}

// MainWindow.cppでのaddBackupボタンの処理部分
void MainWindow::addBackup()
{
    // 親ウィジェットを明示的に指定してダイアログを作成
    BackupDialog *dialog = new BackupDialog(this);

    // モーダルダイアログに設定
    dialog->setModal(true);

    // ダイアログが閉じられたときのメモリ管理
    connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);

    // ダイアログを接続
    connect(dialog, &QDialog::accepted, this, [this, dialog]()
            {
                try {
                    // ダイアログから設定を取得
                    BackupConfig config = dialog->getBackupConfig();
                    
                    qDebug() << "Adding backup config from dialog: " << config.name();
                    
                    // 設定マネージャーに追加
                    configManager->addBackupConfig(config);
                    saveBackupConfigs();
                    
                    // カードとリストを更新
                    loadBackupConfigs();
                    
                    // ログに表示
                    addLogEntry(QString("新しいバックアップ '%1' を追加しました").arg(config.name()));
                    
                    qDebug() << "New backup config added successfully: " << config.name();
                    
                } catch (const std::exception &e) {
                    qDebug() << "Exception during backup config processing: " << e.what();
                    QMessageBox::critical(this, tr("エラー"),
                        tr("バックアップ設定の追加中にエラーが発生しました: %1").arg(e.what()));
                } catch (...) {
                    qDebug() << "Unknown exception during backup config processing";
                    QMessageBox::critical(this, tr("エラー"),
                        tr("バックアップ設定の追加中に不明なエラーが発生しました"));
                } });

    // モーダルダイアログとして実行
    dialog->exec();
}

// 新しいスロット実装
void MainWindow::onFileProcessed(const QString &filePath, bool success)
{
    // ファイルのバックアップ状況をログに記録
    if (success)
    {
        QFileInfo fileInfo(filePath);
        addLogEntry(tr("ファイルをバックアップしました: %1").arg(fileInfo.fileName()));
    }
    else
    {
        QFileInfo fileInfo(filePath);
        addLogEntry(tr("ファイルのバックアップに失敗しました: %1").arg(fileInfo.fileName()));
    }
}

void MainWindow::onDirectoryProcessed(const QString &dirPath, bool created)
{
    // ディレクトリ作成状況をログに記録
    if (created)
    {
        QFileInfo dirInfo(dirPath);
        addLogEntry(tr("フォルダを作成しました: %1").arg(dirInfo.fileName()));
    }
    else
    {
        QFileInfo dirInfo(dirPath);
        addLogEntry(tr("フォルダの作成に失敗しました: %1").arg(dirInfo.fileName()));
    }
}

void MainWindow::onBackupLogMessage(const QString &message)
{
    addLogEntry(message);
}

void MainWindow::clearBackupCards()
{
    qDebug() << "Clearing all backup cards...";

    // 既存のカードを削除
    for (BackupCard *card : backupCards)
    {
        if (card)
        {
            backupsLayout->removeWidget(card);
            card->deleteLater(); // deleteより安全なdeleteLaterを使用
        }
    }
    backupCards.clear();

    qDebug() << "All backup cards cleared";
}

void MainWindow::showLogDialog()
{
    if (!logDialog)
    {
        logDialog = new LogViewerDialog(this);
    }

    // 現在のログをリフレッシュして表示
    logDialog->refreshLogs();
    logDialog->show();
    logDialog->raise();
    logDialog->activateWindow();
}

void MainWindow::addLogEntry(const QString &entry)
{
    // Loggerにエントリを追加（ダイアログが表示されていなくても記録される）
    Logger::instance().log(entry);

    // ログダイアログが表示されていれば更新
    if (logDialog && logDialog->isVisible())
    {
        logDialog->refreshLogs();
    }
}

// 表示モード切替メソッド
void MainWindow::switchViewMode(ViewMode mode)
{
    currentViewMode = mode;
    if (mode == CardView)
    {
        viewStack->setCurrentWidget(scrollArea);
    }
    else
    {
        // テーブルビューに切り替え
        viewStack->setCurrentWidget(backupTableWidget);
        updateTableView();
    }
}

// カードビューに切り替え
void MainWindow::switchToCardView()
{
    switchViewMode(CardView);
}

// リストビューに切り替え
void MainWindow::switchToListView()
{
    switchViewMode(ListView);
}

// テーブルビューを更新するメソッド（updateListViewの代わり）
void MainWindow::updateTableView()
{
    // テーブルをクリア
    backupTableWidget->setRowCount(0);

    // バックアップ設定を取得
    QList<BackupConfig> configs = configManager->backupConfigs();

    // 行数を設定
    backupTableWidget->setRowCount(configs.size());

    // データを設定
    for (int i = 0; i < configs.size(); i++)
    {
        const BackupConfig &config = configs[i];

        // バックアップ名
        QTableWidgetItem *nameItem = new QTableWidgetItem(config.name());
        nameItem->setData(Qt::UserRole, i); // インデックスを保存
        backupTableWidget->setItem(i, 0, nameItem);

        // 元パス
        QString srcPath = config.sourcePath();
        if (srcPath.length() > 30)
        {
            srcPath = "..." + srcPath.right(28);
        }
        backupTableWidget->setItem(i, 1, new QTableWidgetItem(srcPath));

        // 先パス
        QString dstPath = config.destinationPath();
        if (dstPath.length() > 30)
        {
            dstPath = "..." + dstPath.right(28);
        }
        backupTableWidget->setItem(i, 2, new QTableWidgetItem(dstPath));

        // 最終バックアップ
        QString lastBackupTime;
        if (config.lastBackupTime().isValid())
        {
            lastBackupTime = config.lastBackupTime().toString("yyyy/MM/dd HH:mm");
        }
        else
        {
            lastBackupTime = tr("未実行");
        }
        backupTableWidget->setItem(i, 3, new QTableWidgetItem(lastBackupTime));
    }

    // 列幅を内容に合わせて調整（オプション）
    // backupTableWidget->resizeColumnsToContents();
}

// テーブルアイテムのダブルクリック処理
void MainWindow::handleTableItemDoubleClicked(int row, int column)
{
    QTableWidgetItem *item = backupTableWidget->item(row, 0); // 最初の列からインデックスを取得
    if (item)
    {
        int index = item->data(Qt::UserRole).toInt();
        if (index >= 0 && index < configManager->backupConfigs().size())
        {
            // バックアップ実行の代わりに編集を呼び出す
            editBackup(index);
        }
    }
}

// テーブルのコンテキストメニュー
void MainWindow::showTableContextMenu(const QPoint &pos)
{
    QTableWidgetItem *item = backupTableWidget->itemAt(pos);
    if (!item)
        return;

    int row = item->row();
    QTableWidgetItem *firstColItem = backupTableWidget->item(row, 0);
    if (!firstColItem)
        return;

    int index = firstColItem->data(Qt::UserRole).toInt();
    if (index < 0 || index >= configManager->backupConfigs().size())
        return;

    QMenu contextMenu(this);
    QAction *runAction = contextMenu.addAction(tr("バックアップ実行"));
    QAction *editAction = contextMenu.addAction(tr("編集")); // 追加: 編集アクション
    QAction *removeAction = contextMenu.addAction(tr("削除"));

    QAction *selectedAction = contextMenu.exec(backupTableWidget->mapToGlobal(pos));
    if (selectedAction == runAction)
    {
        runBackup(configManager->backupConfigs()[index]);
    }
    else if (selectedAction == editAction) // 追加: 編集アクション処理
    {
        editBackup(index);
    }
    else if (selectedAction == removeAction)
    {
        removeBackup(index);
    }
}

// runAllBackupsメソッドの完全な実装を追加
void MainWindow::runAllBackups()
{
    qDebug() << "runAllBackups called";

    // 既に実行中の場合は何もしない
    if (isRunningBatchBackup)
    {
        statusBar()->showMessage(tr("バックアップが既に実行中です"));
        qDebug() << "バックアップが既に実行中です";
        return;
    }

    // バックアップ設定のリストを取得
    QVector<BackupConfig> configs = configManager->backupConfigs();
    if (configs.isEmpty())
    {
        QMessageBox::information(this, tr("バックアップ"), tr("バックアップ設定がありません。"));
        qDebug() << "バックアップ設定がありません";
        return;
    }

    qDebug() << "バックアップ設定数: " << configs.size();

    // キューをクリアして設定を追加
    backupQueue.clear();
    for (const BackupConfig &config : configs)
    {
        backupQueue.enqueue(config);
    }

    // カウンターを初期化
    totalBackupsInQueue = backupQueue.size();
    currentBackupIndex = 0;
    isRunningBatchBackup = true;

    // ステータスバー表示の更新
    statusBar()->showMessage(tr("一括バックアップを開始します..."));

    // 最初のバックアップを開始
    processNextBackup();
}

// 追加: 編集機能の実装
void MainWindow::editBackup(int index)
{
    if (index < 0 || index >= configManager->backupConfigs().size())
    {
        qDebug() << "Invalid backup index for edit:" << index;
        return;
    }

    try
    {
        // 編集するバックアップ設定を取得
        BackupConfig config = configManager->backupConfigs()[index];

        // スタック上にダイアログを作成
        BackupDialog dialog(config, this);

        // 標準的なモーダルダイアログとして実行
        if (dialog.exec() == QDialog::Accepted)
        {
            // 更新された設定を取得
            BackupConfig updatedConfig = dialog.getBackupConfig();
            configManager->updateBackupConfig(index, updatedConfig);
            saveBackupConfigs();
            loadBackupConfigs();
            addLogEntry(QString("バックアップ設定 '%1' を更新しました").arg(updatedConfig.name()));
        }
    }
    catch (const std::exception &e)
    {
        qDebug() << "Exception in editBackup: " << e.what();
        QMessageBox::critical(this, tr("エラー"),
                              tr("バックアップ設定の編集中にエラーが発生しました: %1").arg(e.what()));
    }
    catch (...)
    {
        qDebug() << "Unknown exception in editBackup";
        QMessageBox::critical(this, tr("エラー"),
                              tr("バックアップ設定の編集中に不明なエラーが発生しました"));
    }
}