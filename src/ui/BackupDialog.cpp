#include "BackupDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QDir>
#include <QLabel>
#include <QMessageBox>
#include <QGroupBox>
#include <QTimer>
#include <QApplication> // 追加: QApplication用
#include <QCloseEvent>  // 追加: QCloseEvent用（念のため）

BackupDialog::BackupDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setWindowTitle(tr("バックアップの追加"));
}

BackupDialog::BackupDialog(const BackupConfig &config, QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    loadFromConfig(config);
    setWindowTitle(tr("バックアップの編集"));
}

void BackupDialog::setupUI()
{
    // メインのレイアウト
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // タブウィジェット
    tabWidget = new QTabWidget(this);

    // 基本設定タブ
    QWidget *basicTab = new QWidget(tabWidget);
    QFormLayout *basicLayout = new QFormLayout(basicTab);

    // バックアップ名
    nameEdit = new QLineEdit(basicTab);
    nameEdit->setPlaceholderText(tr("バックアップの名前を入力"));
    basicLayout->addRow(tr("バックアップ名:"), nameEdit);

    // ソースパス
    QHBoxLayout *sourceLayout = new QHBoxLayout();
    sourcePathEdit = new QLineEdit(basicTab);
    sourcePathEdit->setPlaceholderText(tr("バックアップ元のパスを入力または選択"));
    sourceBrowseButton = new QPushButton(tr("参照..."), basicTab);
    sourceLayout->addWidget(sourcePathEdit);
    sourceLayout->addWidget(sourceBrowseButton);
    basicLayout->addRow(tr("バックアップ元:"), sourceLayout);

    // 保存先パス
    QHBoxLayout *destLayout = new QHBoxLayout();
    destPathEdit = new QLineEdit(basicTab);
    destPathEdit->setPlaceholderText(tr("バックアップ先のパスを入力または選択"));
    destBrowseButton = new QPushButton(tr("参照..."), basicTab);
    destLayout->addWidget(destPathEdit);
    destLayout->addWidget(destBrowseButton);
    basicLayout->addRow(tr("バックアップ先:"), destLayout);

    // 基本タブを追加
    tabWidget->addTab(basicTab, tr("基本設定"));

    // 除外設定タブ
    QWidget *exclusionTab = new QWidget(tabWidget);
    QVBoxLayout *exclusionLayout = new QVBoxLayout(exclusionTab);

    // 説明ラベル
    QLabel *helpLabel = new QLabel(tr("各項目は改行で区切って入力してください。ワイルドカード（*）が使用できます。"), exclusionTab);
    helpLabel->setWordWrap(true);
    exclusionLayout->addWidget(helpLabel);

    // 除外ファイル
    QGroupBox *filesGroup = new QGroupBox(tr("除外ファイル"), exclusionTab);
    QVBoxLayout *filesLayout = new QVBoxLayout(filesGroup);
    excludedFilesEdit = new QPlainTextEdit(filesGroup);
    excludedFilesEdit->setPlaceholderText(tr("例: secret.txt\nconfig.ini\ntemp*"));
    filesLayout->addWidget(excludedFilesEdit);
    exclusionLayout->addWidget(filesGroup);

    // 除外フォルダー
    QGroupBox *foldersGroup = new QGroupBox(tr("除外フォルダー"), exclusionTab);
    QVBoxLayout *foldersLayout = new QVBoxLayout(foldersGroup);
    excludedFoldersEdit = new QPlainTextEdit(foldersGroup);
    excludedFoldersEdit->setPlaceholderText(tr("例: temp\nlogs\ncache"));
    foldersLayout->addWidget(excludedFoldersEdit);
    exclusionLayout->addWidget(foldersGroup);

    // 除外拡張子
    QGroupBox *extGroup = new QGroupBox(tr("除外拡張子"), exclusionTab);
    QVBoxLayout *extLayout = new QVBoxLayout(extGroup);
    excludedExtensionsEdit = new QPlainTextEdit(extGroup);
    excludedExtensionsEdit->setPlaceholderText(tr("例: .tmp\n.log\n.bak"));
    extLayout->addWidget(excludedExtensionsEdit);
    exclusionLayout->addWidget(extGroup);

    // 隠しファイル表示オプション
    showHiddenFilesCheck = new QCheckBox(tr("バックアップに隠しファイルを含める"), exclusionTab);
    showHiddenFilesCheck->setChecked(true); // デフォルトはオン
    exclusionLayout->addWidget(showHiddenFilesCheck);

    // 除外タブを追加
    tabWidget->addTab(exclusionTab, tr("除外設定"));

    // メインレイアウトにタブを追加
    mainLayout->addWidget(tabWidget);

    // ボタンレイアウト
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    okButton = new QPushButton(tr("OK"), this);
    cancelButton = new QPushButton(tr("キャンセル"), this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    // シグナル/スロット接続
    connect(sourceBrowseButton, &QPushButton::clicked, this, &BackupDialog::browseSourcePath);
    connect(destBrowseButton, &QPushButton::clicked, this, &BackupDialog::browseDestinationPath);
    connect(nameEdit, &QLineEdit::textChanged, this, &BackupDialog::validateInput);
    connect(sourcePathEdit, &QLineEdit::textChanged, this, &BackupDialog::validateInput);
    connect(destPathEdit, &QLineEdit::textChanged, this, &BackupDialog::validateInput);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // 初期状態
    validateInput();
    setMinimumSize(500, 600);
}

void BackupDialog::loadFromConfig(const BackupConfig &config)
{
    nameEdit->setText(config.name());
    sourcePathEdit->setText(config.sourcePath());
    destPathEdit->setText(config.destinationPath());

    // 除外リストの設定
    excludedFilesEdit->setPlainText(config.excludedFiles().join("\n"));
    excludedFoldersEdit->setPlainText(config.excludedFolders().join("\n"));
    excludedExtensionsEdit->setPlainText(config.excludedExtensions().join("\n"));
}

BackupConfig BackupDialog::getBackupConfig() const
{
    BackupConfig config;
    config.setName(nameEdit->text().trimmed());
    config.setSourcePath(sourcePathEdit->text().trimmed());
    config.setDestinationPath(destPathEdit->text().trimmed());

    // テキストエディットから除外リストを取得して設定
    QStringList excludedFiles = excludedFilesEdit->toPlainText().split("\n", Qt::SkipEmptyParts);
    for (int i = 0; i < excludedFiles.size(); ++i)
    {
        excludedFiles[i] = excludedFiles[i].trimmed();
    }
    config.setExcludedFiles(excludedFiles);

    QStringList excludedFolders = excludedFoldersEdit->toPlainText().split("\n", Qt::SkipEmptyParts);
    for (int i = 0; i < excludedFolders.size(); ++i)
    {
        excludedFolders[i] = excludedFolders[i].trimmed();
    }
    config.setExcludedFolders(excludedFolders);

    QStringList excludedExtensions = excludedExtensionsEdit->toPlainText().split("\n", Qt::SkipEmptyParts);
    for (int i = 0; i < excludedExtensions.size(); ++i)
    {
        excludedExtensions[i] = excludedExtensions[i].trimmed();
        // 拡張子が.で始まっていなければ先頭に.を追加
        if (!excludedExtensions[i].startsWith('.'))
        {
            excludedExtensions[i] = "." + excludedExtensions[i];
        }
    }
    config.setExcludedExtensions(excludedExtensions);

    return config;
}

void BackupDialog::browseSourcePath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("バックアップ元フォルダーを選択"));
    if (!dir.isEmpty())
    {
        sourcePathEdit->setText(dir);
    }
}

void BackupDialog::browseDestinationPath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("バックアップ先フォルダーを選択"));
    if (!dir.isEmpty())
    {
        destPathEdit->setText(dir);
    }
}

void BackupDialog::validateInput()
{
    bool isValid = !nameEdit->text().trimmed().isEmpty() &&
                   !sourcePathEdit->text().trimmed().isEmpty() &&
                   !destPathEdit->text().trimmed().isEmpty();
    okButton->setEnabled(isValid);
}

void BackupDialog::updateProgress(int progress)
{
    // プログレスバーが存在する場合は更新
    if (progressBar)
    {
        progressBar->setValue(progress);
    }
}

void BackupDialog::backupFinished()
{
    // バックアップ完了時の処理
    if (startButton)
    {
        startButton->setText(tr("バックアップを開始"));
        startButton->setEnabled(true);
    }

    if (cancelButton)
    {
        cancelButton->setText(tr("キャンセル"));
        cancelButton->setEnabled(false);
    }

    if (progressBar)
    {
        progressBar->setValue(100);
    }

    // 完了メッセージを表示
    QMessageBox::information(this, tr("バックアップ完了"), tr("バックアップが正常に完了しました。"));
}

void BackupDialog::startBackup()
{
    // バックアップ開始
    if (!sourcePathEdit->text().isEmpty() && !destPathEdit->text().isEmpty())
    {
        // ボタンの状態を変更
        startButton->setText(tr("実行中..."));
        startButton->setEnabled(false);
        cancelButton->setEnabled(true);

        // プログレスバーをリセット
        if (progressBar)
        {
            progressBar->setValue(0);
        }

        // バックアップ実行シグナルを発行
        emit backupRequested(sourcePathEdit->text(), destPathEdit->text());
    }
}

void BackupDialog::cancelBackup()
{
    // cancelRequested() から backupCancelled() へ変更
    emit backupCancelled();

    // UI状態の復元
    if (startButton)
    {
        startButton->setText(tr("バックアップを開始"));
        startButton->setEnabled(true);
    }
    if (cancelButton)
    {
        cancelButton->setEnabled(false);
    }
}

void BackupDialog::updateTitleFromSourcePath()
{
    // ソースパスからタイトルを自動的に設定
    QString path = sourcePathEdit->text();
    if (!path.isEmpty())
    {
        QFileInfo fileInfo(path);
        QString dirName = fileInfo.fileName();
        if (!dirName.isEmpty())
        {
            nameEdit->setText(dirName);
        }
    }
}

// accept()メソッドのオーバーライド
void BackupDialog::accept()
{
    // 入力検証など、必要な処理を行った後にダイアログを受け入れる
    QDialog::accept();
}

// reject()メソッド - 単純化と明示的なスコープ制限
void BackupDialog::reject()
{
    qDebug() << "BackupDialog::reject() called"; // デバッグログを追加

    // バックアップ実行中なら確認
    if (startButton && !startButton->isEnabled())
    {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("確認"),
            tr("バックアップが進行中です。キャンセルしますか？"),
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            // バックアップをキャンセル
            emit backupCancelled();

            // UIの状態を更新
            if (startButton)
            {
                startButton->setText(tr("バックアップを開始"));
                startButton->setEnabled(true);
            }
            if (cancelButton)
            {
                cancelButton->setEnabled(false);
            }

            // 親クラスのrejectを呼び出してダイアログを閉じる
            QDialog::reject();
        }
        // Noの場合は何もしない（ダイアログを開いたままにする）
    }
    else
    {
        // バックアップ実行中でなければ普通にダイアログを閉じる
        QDialog::reject();
    }

    qDebug() << "BackupDialog::reject() finished"; // デバッグログを追加
}

// closeEvent - 単純化
void BackupDialog::closeEvent(QCloseEvent *event)
{
    qDebug() << "BackupDialog::closeEvent() called"; // デバッグログを追加

    // バックアップ実行中なら確認
    if (startButton && !startButton->isEnabled())
    {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("確認"),
            tr("バックアップが進行中です。キャンセルしますか？"),
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            // バックアップをキャンセル
            emit backupCancelled();

            // UIの状態を更新
            if (startButton)
            {
                startButton->setText(tr("バックアップを開始"));
                startButton->setEnabled(true);
            }
            if (cancelButton)
            {
                cancelButton->setEnabled(false);
            }

            // イベントを受け入れてダイアログのみを閉じる
            event->accept();
        }
        else
        {
            // キャンセルしない場合はダイアログを開いたままにする
            event->ignore();
        }
    }
    else
    {
        // バックアップ実行中でなければ、通常通りダイアログを閉じる
        event->accept();
    }

    qDebug() << "BackupDialog::closeEvent() finished with accept:"
             << (event->isAccepted() ? "true" : "false"); // デバッグログを追加
}

// デストラクタの実装を追加
BackupDialog::~BackupDialog()
{
    // 進行中のバックアップ処理があれば停止
    if (startButton && !startButton->isEnabled())
    {
        emit backupCancelled();
    }
}

// ソースパス取得メソッド
QString BackupDialog::sourcePath() const
{
    return sourcePathEdit->text();
}

// 宛先パス取得メソッド
QString BackupDialog::destinationPath() const
{
    return destPathEdit->text();
}

// タイトル取得メソッド
QString BackupDialog::title() const
{
    return nameEdit->text();
}

// 宛先パス設定メソッド
void BackupDialog::setDestinationPath(const QString &path)
{
    if (destPathEdit)
    {
        destPathEdit->setText(path);
    }
}

// done()メソッドをオーバーライドしてアプリ終了を防止
void BackupDialog::done(int result)
{
    qDebug() << "BackupDialog::done() called with result:" << result;

    // 明示的に親クラスの実装を呼び出し、予期せぬ動作を防止
    QDialog::done(result);

    qDebug() << "BackupDialog::done() finished";
}