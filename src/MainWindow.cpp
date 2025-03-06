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
#include <QTableWidget>     // 追加: QTableWidgetのヘッダー
#include <QTableWidgetItem> // 追加: QTableWidgetItemのヘッダー
#include <QHeaderView>      // 追加: QHeaderViewのヘッダー
#include <QPainter>
#include <QFileDialog>
#include <QStandardPaths>
#include <QGraphicsBlurEffect>
#include <QPointer>
#include <QDebug>

// 背景画像描画の根本的な問題を修正

// まず、カスタムセントラルウィジェットを作成して背景描画を担当させる
class BackgroundWidget : public QWidget
{
public:
    BackgroundWidget(QWidget *parent = nullptr) : QWidget(parent) {}

    void setBackgroundImage(const QPixmap &image, int opacity)
    {
        m_backgroundImage = image;
        m_opacity = opacity;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter(this);

        // 既存のウィジェットを描画
        QWidget::paintEvent(event);

        // その後、背景画像を下に描画
        if (!m_backgroundImage.isNull())
        {
            painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
            painter.setOpacity(m_opacity / 100.0);
            painter.drawPixmap(rect(), m_backgroundImage, m_backgroundImage.rect());
        }
    }

private:
    QPixmap m_backgroundImage;
    int m_opacity = 80;
};

// MainWindowのコンストラクタで初期化
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      backupEngine(new BackupEngine(this)),
      isRunningBatchBackup(false),
      currentBackupIndex(0),
      totalBackupsInQueue(0),
      logDialog(nullptr),
      currentViewMode(CardView),
      isAutomaticBackup(false),    // 追加
      m_useBackgroundImage(false), // 常にfalseに固定
      m_backgroundOpacity(80)
{
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

    // 背景画像の設定を読み込む - コメントアウトして無効化
    // loadBackgroundSettings();

    // メニューに背景設定を追加 - 背景設定メニューを無効化
    QMenu *viewMenu = menuBar()->addMenu(tr("表示"));
    QAction *backgroundAction = viewMenu->addAction(tr("背景画像設定..."));
    backgroundAction->setEnabled(false); // 無効化

    // ラムダ式を使用して無効化メッセージを表示
    connect(backgroundAction, &QAction::triggered, this, [this]()
            { QMessageBox::information(this, tr("背景画像機能は無効化されています"),
                                       tr("背景画像機能は現在無効化されています。\nパフォーマンスと安定性の向上のため、この機能は利用できません。")); });

    // テスト用メニュー追加 - 背景画像テストも無効化
    QMenu *debugMenu = menuBar()->addMenu("デバッグ");
    QAction *testBgAction = debugMenu->addAction("背景画像テスト...");
    testBgAction->setEnabled(false); // 無効化

    // 同じラムダ式を使用
    connect(testBgAction, &QAction::triggered, this, [this]()
            { QMessageBox::information(this, tr("背景画像機能は無効化されています"),
                                       tr("背景画像機能は現在無効化されています。\nパフォーマンスと安定性の向上のため、この機能は利用できません。")); });

    // 既存の背景がなければ、デフォルト背景を提供 - コメントアウト
    // if (!m_useBackgroundImage || m_backgroundImage.isNull())
    // {
    //     // デフォルト背景の提供（オプション）
    //     // setDefaultBackground();
    // }

    // カード部分を半透明にするスタイル設定
    setupTransparentStyle();
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

    // 表示スタックウィジェット（カードとリストの切り替え用）
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

    // 一括バックアップボタン
    runAllBackupsButton = new QPushButton("すべてバックアップ", this);

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
    buttonLayout->addWidget(runAllBackupsButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(viewModeButton);
    buttonLayout->addWidget(settingsButton);
    buttonLayout->addWidget(logButton); // 新規追加
    mainLayout->addLayout(buttonLayout);

    // ステータスバーの設定
    statusBar()->showMessage("準備完了");

    // 中央ウィジェットを設定
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // シグナル/スロット接続
    connect(addBackupButton, &QPushButton::clicked, this, &MainWindow::showBackupDialog);
    connect(runAllBackupsButton, &QPushButton::clicked, this, &MainWindow::runAllBackups);
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

    if (dialog.exec() == QDialog::Accepted)
    {
        // 設定ダイアログが受け入れられたら背景設定を再読み込み
        loadBackgroundSettings();
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
    clearBackupCards();

    QList<BackupConfig> configs = configManager->backupConfigs();
    for (const BackupConfig &config : configs)
    {
        addBackupCard(config);
    }

    // テーブルビューも更新
    if (currentViewMode == ListView)
    {
        updateTableView();
    }
}

void MainWindow::saveBackupConfigs()
{
    configManager->saveConfig();
}

// addBackupCardメソッドの更新
void MainWindow::addBackupCard(const BackupConfig &config)
{
    int cardIndex = backupCards.size();
    int row = cardIndex / 4; // 4列配置に変更
    int col = cardIndex % 4; // 列番号（0から3）

    BackupCard *card = new BackupCard(config, cardIndex, backupsContainer);

    // カードのサイズを最適化 - より小さく
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    card->setFixedHeight(120); // 高さを小さく (150→120)

    // カードをグリッドに追加する際に位置調整
    backupsLayout->addWidget(card, row, col, 1, 1, Qt::AlignTop);

    // カードのシグナルを接続
    connect(card, &BackupCard::runBackup, this, &MainWindow::runBackup);
    connect(card, &BackupCard::removeBackup, this, &MainWindow::removeBackup);

    backupCards.append(card);

    // テーブルビューも更新
    if (currentViewMode == ListView)
    {
        updateTableView();
    }
}

void MainWindow::clearBackupCards()
{
    // 既存のカードを削除
    for (BackupCard *card : backupCards)
    {
        backupsLayout->removeWidget(card);
        delete card;
    }
    backupCards.clear();
}

// showBackupDialogメソッドを完全に書き換え

void MainWindow::showBackupDialog()
{
    // 重要: QPointer を使用してダイアログの有効性を追跡
    QPointer<BackupDialog> dialog = new BackupDialog(this);

    // 既存の設定（必要に応じて）
    // dialog->setSourcePath(...);
    // dialog->setDestinationPath(...);

    // 重要: ダイアログをモードレス（非モーダル）で表示し、閉じた後に自動削除されるように設定
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowModality(Qt::NonModal);

    // 閉じるイベントをカスタム処理するためのフック
    QObject::connect(dialog, &QDialog::finished, [dialog]()
                     {
                         qDebug() << "ダイアログが閉じられました";
                         // 必要に応じて追加処理
                     });

    // バックアップ要求シグナルの処理（これは既存のコードを参考にしてください）
    QObject::connect(dialog, &BackupDialog::backupRequested,
                     this, [this, dialog](const QString &source, const QString &dest)
                     {
                         qDebug() << "バックアップ要求: " << source << " -> " << dest;
                         // 既存のバックアップ処理をここに記述
                     });

    // バックアップキャンセル処理
    QObject::connect(dialog, &BackupDialog::backupCancelled,
                     this, [this, dialog]()
                     {
                         qDebug() << "バックアップがキャンセルされました";
                         // 既存のキャンセル処理をここに記述
                     });

    // 非モーダルで表示（これが重要）
    dialog->show();
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
    backupEngine->startBackup(config.sourcePath(), config.destinationPath());

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

void MainWindow::backupComplete()
{
    // バッチバックアップ実行中の場合
    if (isRunningBatchBackup)
    {
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

// 以前の実装に加えて、新しい関数を追加

void MainWindow::runAllBackups()
{
    // 既に実行中の場合は何もしない
    if (isRunningBatchBackup)
    {
        statusBar()->showMessage(tr("バックアップが既に実行中です"));
        return;
    }

    // バックアップ設定のリストを取得
    QList<BackupConfig> configs = configManager->backupConfigs();
    if (configs.isEmpty())
    {
        QMessageBox::information(this, tr("バックアップ"), tr("バックアップ設定がありません。"));
        return;
    }

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

    // 自動バックアップの場合はログに記録
    if (isAutomaticBackup)
    {
        addLogEntry(QString("%1個のバックアップを自動実行します").arg(totalBackupsInQueue));
    }
    else
    {
        addLogEntry(QString("%1個のバックアップを実行します").arg(totalBackupsInQueue));
    }

    // バックアップを開始
    processNextBackup();
}

void MainWindow::processNextBackup()
{
    if (backupQueue.isEmpty())
    {
        isRunningBatchBackup = false;

        // 自動バックアップフラグをリセット
        bool wasAutomatic = isAutomaticBackup;
        isAutomaticBackup = false;

        statusBar()->showMessage(tr("すべてのバックアップが完了しました"));

        // 自動バックアップの場合は通知だけ表示して、ダイアログは表示しない
        if (!wasAutomatic)
        {
            QMessageBox::information(this, "完了", "すべてのバックアップが完了しました");
        }

        return;
    }

    // 残りはそのまま...
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
        updateTableView();
        viewStack->setCurrentWidget(backupTableWidget);
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
            runBackup(configManager->backupConfigs()[index]);
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
    QAction *removeAction = contextMenu.addAction(tr("削除"));

    QAction *selectedAction = contextMenu.exec(backupTableWidget->mapToGlobal(pos));

    if (selectedAction == runAction)
    {
        runBackup(configManager->backupConfigs()[index]);
    }
    else if (selectedAction == removeAction)
    {
        removeBackup(index);
    }
}

// paintEventメソッドを修正して透明度を確実に適用

void MainWindow::paintEvent(QPaintEvent *event)
{
    // 既存のウィジェット描画
    QMainWindow::paintEvent(event);

    // 背景画像を描画 - 完全に無効化
    // if (m_useBackgroundImage && !m_backgroundImage.isNull())
    // {
    //     QPainter painter(this);

    //     // 背景画像は他のウィジェットの下に描画
    //     painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);

    //     // 透明度を明示的に設定
    //     qreal opacity = static_cast<qreal>(m_backgroundOpacity) / 100.0;
    //     qDebug() << "背景画像の透明度: " << opacity << " (元の値: " << m_backgroundOpacity << ")";

    //     painter.setOpacity(opacity);
    //     painter.setRenderHint(QPainter::SmoothPixmapTransform);

    //     // ウィンドウ全体に画像を描画
    //     painter.drawPixmap(rect(), m_backgroundImage, m_backgroundImage.rect());
    // }
}

// 背景画像設定ダイアログを表示
void MainWindow::showBackgroundDialog()
{
    // ダイアログ作成
    QDialog dialog(this);
    dialog.setWindowTitle(tr("背景画像の設定"));
    dialog.setMinimumWidth(400);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    // 背景画像の使用有無を選択するチェックボックス
    QCheckBox *useBackground = new QCheckBox(tr("背景画像を使用する"));
    useBackground->setChecked(m_useBackgroundImage);
    layout->addWidget(useBackground);

    // 画像パス表示用のラベルとボタン
    QHBoxLayout *pathLayout = new QHBoxLayout();
    QLabel *pathLabel = new QLabel(tr("画像パス:"));
    QLineEdit *pathEdit = new QLineEdit(m_backgroundImagePath);
    pathEdit->setReadOnly(true);
    QPushButton *browseButton = new QPushButton(tr("参照..."));

    pathLayout->addWidget(pathLabel);
    pathLayout->addWidget(pathEdit);
    pathLayout->addWidget(browseButton);
    layout->addLayout(pathLayout);

    // 透明度設定用のスライダー
    QHBoxLayout *opacityLayout = new QHBoxLayout();
    QLabel *opacityLabel = new QLabel(tr("透明度:"));
    QSlider *opacitySlider = new QSlider(Qt::Horizontal);
    opacitySlider->setRange(0, 100);
    opacitySlider->setValue(m_backgroundOpacity);

    opacityLayout->addWidget(opacityLabel);
    opacityLayout->addWidget(opacitySlider);
    layout->addLayout(opacityLayout);

    // ボタン用レイアウト
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton(tr("OK"));
    QPushButton *cancelButton = new QPushButton(tr("キャンセル"));
    QPushButton *clearButton = new QPushButton(tr("背景をクリア"));

    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    // シグナル/スロット接続
    connect(browseButton, &QPushButton::clicked, [&]()
            {
        QString imagePath = QFileDialog::getOpenFileName(&dialog, 
            tr("背景画像を選択"), 
            QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
            tr("画像ファイル (*.png *.jpg *.jpeg *.bmp *.gif)"));
            
        if (!imagePath.isEmpty()) {
            pathEdit->setText(imagePath);
        } });

    connect(clearButton, &QPushButton::clicked, [&]()
            { pathEdit->clear(); });

    connect(okButton, &QPushButton::clicked, [&]()
            {
        bool useImage = useBackground->isChecked();
        QString path = pathEdit->text();
        
        // 背景画像を使用する設定だが画像が選択されていない場合
        if (useImage && path.isEmpty()) {
            QMessageBox::warning(&dialog, tr("警告"), tr("背景画像を使用する場合は、画像を選択してください。"));
            return;
        }
        
        // 設定を適用
        m_useBackgroundImage = useImage;
        
        if (useImage) {
            setBackgroundImage(path);
        } else {
            // 背景なしに設定
            m_backgroundImagePath = "";
            m_backgroundImage = QPixmap();
            repaint();
        }
        
        // 透明度を適用
        m_backgroundOpacity = opacitySlider->value();
        
        // 設定を保存
        saveBackgroundSettings();
        
        dialog.accept(); });

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    // ダイアログを表示
    dialog.exec();
}

// 背景画像を設定
void MainWindow::setBackgroundImage(const QString &imagePath)
{
    if (imagePath.isEmpty())
    {
        m_useBackgroundImage = false;
        m_backgroundImage = QPixmap();
        m_backgroundImagePath = "";
    }
    else
    {
        QPixmap image(imagePath);
        if (!image.isNull())
        {
            m_backgroundImage = image;
            m_backgroundImagePath = imagePath;
            m_useBackgroundImage = true;
        }
        else
        {
            QMessageBox::warning(this, tr("エラー"), tr("画像の読み込みに失敗しました: %1").arg(imagePath));
            m_useBackgroundImage = false;
        }
    }

    // 画面を再描画
    repaint();
}

// 背景画像をクリア
void MainWindow::clearBackgroundImage()
{
    m_useBackgroundImage = false;
    m_backgroundImage = QPixmap();
    m_backgroundImagePath = "";
    saveBackgroundSettings();
    repaint();
}

// 透明度の値を確実に読み込むよう修正
void MainWindow::loadBackgroundSettings()
{
    QString settingsPath = QDir::homePath() + "/shirafuka_settings.ini";
    QSettings settings(settingsPath, QSettings::IniFormat);

    m_useBackgroundImage = settings.value("Background/UseBackgroundImage", false).toBool();
    QString imagePath = settings.value("Background/BackgroundImagePath", "").toString();

    // 透明度の読み込みを確実に行う
    bool ok;
    int opacity = settings.value("Background/Opacity", 80).toInt(&ok);
    if (ok)
    {
        m_backgroundOpacity = opacity;
    }
    else
    {
        m_backgroundOpacity = 80; // デフォルト値
    }

    qDebug() << "背景設定読み込み: 使用=" << m_useBackgroundImage
             << ", パス=" << imagePath
             << ", 透明度=" << m_backgroundOpacity;

    // 既存のコード...
}

// 背景設定を保存する
void MainWindow::saveBackgroundSettings()
{
    QSettings settings;
    settings.beginGroup("Background");
    settings.setValue("UseBackgroundImage", m_useBackgroundImage);
    settings.setValue("BackgroundImagePath", m_backgroundImagePath);
    settings.setValue("Opacity", m_backgroundOpacity);
    settings.endGroup();
}

// テスト関数の設定保存部分も修正

void MainWindow::testBackgroundImage()
{
    // ファイル選択ダイアログを表示
    QString imagePath = QFileDialog::getOpenFileName(this,
                                                     tr("テスト背景画像を選択"),
                                                     QDir::homePath(),
                                                     tr("画像ファイル (*.png *.jpg *.jpeg *.bmp)"));

    if (imagePath.isEmpty())
    {
        return;
    }

    // 画像を直接設定
    QPixmap testImage(imagePath);
    if (!testImage.isNull())
    {
        m_backgroundImage = testImage;
        m_useBackgroundImage = true;
        m_backgroundOpacity = 80;

        qDebug() << "テスト背景画像を設定: " << imagePath;
        qDebug() << "画像サイズ: " << testImage.width() << "x" << testImage.height();

        // 同じ設定ファイルを使用
        QString settingsPath = QDir::homePath() + "/shirafuka_settings.ini";
        QSettings settings(settingsPath, QSettings::IniFormat);

        settings.setValue("Background/UseBackgroundImage", true);
        settings.setValue("Background/BackgroundImagePath", imagePath);
        settings.setValue("Opacity", m_backgroundOpacity);
        settings.sync();

        qDebug() << "テスト設定を保存: " << settings.fileName();

        // 強制的に再描画
        update();
    }
    else
    {
        QMessageBox::warning(this, tr("エラー"), tr("画像の読み込みに失敗しました: %1").arg(imagePath));
    }
}

// デフォルト背景を設定する補助メソッド
void MainWindow::setDefaultBackground()
{
    // リソースからデフォルト背景を読み込む例
    // QPixmap defaultBg(":/images/default_background.jpg");

    // またはシンプルなグラデーション背景を生成
    QLinearGradient gradient(0, 0, width(), height());
    gradient.setColorAt(0, QColor(240, 240, 255));
    gradient.setColorAt(1, QColor(210, 225, 240));

    QPixmap defaultBg(width(), height());
    defaultBg.fill(Qt::transparent);

    QPainter painter(&defaultBg);
    painter.fillRect(0, 0, width(), height(), gradient);

    m_backgroundImage = defaultBg;
    m_useBackgroundImage = true;
    m_backgroundOpacity = 60; // デフォルト背景は薄めに

    update();
}

// カードのスタイルを半透明に設定するメソッド
void MainWindow::setupTransparentStyle()
{
    // カード全体に適用するコンパクトなスタイル
    QString cardStyle = R"(
        /* コンパクトなカードスタイル */
        BackupCard {
            background-color: rgba(255, 255, 255, 250);
            border: 1px solid rgba(150, 150, 150, 200);
            border-radius: 6px;
            margin: 4px;
            padding: 6px 6px 12px 6px; /* 下部のパディングを増やす (6px→12px) */
        }
        
        /* ボタンコンテナの下部マージン */
        BackupCard QWidget#buttonContainer {
            margin-bottom: 8px;
        }
        
        /* ラベルをコンパクトに */
        BackupCard QLabel {
            color: #101010;
            font-weight: normal;
            background-color: transparent;
        }
        
        /* タイトルラベル - 小さめに */
        BackupCard QLabel#titleLabel {
            font-weight: bold;
            font-size: 11pt;
            color: #000000;
            margin-bottom: 4px;
        }
        
        /* パスラベル - さらに小さく */
        BackupCard QLabel#pathLabel {
            color: #404040;
            font-size: 8pt;
        }
        
        /* コンパクトなボタンスタイル */
        BackupCard QPushButton {
            background-color: #f0f0f0;
            color: #202020;
            border: 1px solid #c0c0c0;
            border-radius: 3px;
            padding: 4px 8px;
            font-weight: bold;
            min-width: 60px;
            min-height: 24px;
            font-size: 9pt;
            margin-bottom: 6px; /* ボタン下部にマージンを追加 */
        }
        
        /* 実行ボタン - コンパクトだが目立つように */
        BackupCard QPushButton#runButton {
            background-color: #e8f4e8;
            color: #006000;
            border: 1px solid #80c080;
            min-width: 70px;
            font-size: 10pt;
        }
        
        BackupCard QPushButton#runButton:hover {
            background-color: #d0f0d0;
        }
        
        /* その他のウィジェット - 変更なし */
    )";

    // バックアップカードの単純化とコンパクト化
    for (BackupCard *card : backupCards)
    {
        // 基本設定
        card->setAutoFillBackground(true);
        card->setContentsMargins(6, 6, 6, 12); // 下部の余白を増やす (6→12)

        // カードの高さを少し増やして余白を確保
        card->setFixedHeight(130); // 120→130に増加

        // 既存のボタンウィジェットを取得し、マージンを追加
        QList<QPushButton *> buttons = card->findChildren<QPushButton *>();
        for (QPushButton *btn : buttons)
        {
            // ボタン下部のマージンを設定
            QMargins margins = btn->contentsMargins();
            margins.setBottom(margins.bottom() + 6); // 下部に6pxのマージンを追加
            btn->setContentsMargins(margins);
        }

        // ボタンを含む親ウィジェット（通常はQFrame）を探してIDを設定
        QList<QFrame *> frames = card->findChildren<QFrame *>();
        for (QFrame *frame : frames)
        {
            if (frame->findChildren<QPushButton *>().count() > 0)
            {
                frame->setObjectName("buttonContainer");
                frame->setContentsMargins(frame->contentsMargins().left(),
                                          frame->contentsMargins().top(),
                                          frame->contentsMargins().right(),
                                          frame->contentsMargins().bottom() + 8);
            }
        }

        // その他の既存のスタイル設定（省略しています）
        // ...existing code...
    }

    // 全体のスタイルシートを設定
    setStyleSheet(cardStyle);
}

// MainWindow.cppでのaddBackupボタンの処理部分

void MainWindow::addBackup()
{
    // 親ウィジェットを明示的に指定してダイアログを作成
    BackupDialog *dialog = new BackupDialog(this);

    // モーダル（モードレス）動作を明示的に設定
    // モーダル：親ウィンドウを操作できなくなる
    dialog->setModal(true);

    // ダイアログを接続
    connect(dialog, &BackupDialog::accepted, this, [this, dialog]()
            {
                // ...existing code...
            });

    // 通常のshow()の代わりにexec()を使う場合は注意
    // exec()はモーダルダイアログを表示してユーザーの応答を待ちます
    dialog->exec(); // または dialog->show();
}

void MainWindow::showDisabledBackgroundDialog()
{
    QMessageBox::information(this, tr("背景画像機能は無効化されています"),
                             tr("背景画像機能は現在無効化されています。\nパフォーマンスと安定性の向上のため、この機能は利用できません。"));
}