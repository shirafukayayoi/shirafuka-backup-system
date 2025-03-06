#include "BackupCard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyle>
#include <QApplication>
#include <QDateTime>

BackupCard::BackupCard(const BackupConfig &config, int index, QWidget *parent)
    : QWidget(parent), m_config(config), m_index(index)
{
    setupUI();
}

void BackupCard::setupUI()
{
    // スタイル設定の改善
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(248, 248, 252)); // 少し青みがかった背景
    setPalette(pal);

    // より良いフレーム効果を作成
    m_frame = new QFrame(this);
    m_frame->setFrameShape(QFrame::Box);
    m_frame->setFrameShadow(QFrame::Raised); // 立体的な効果を追加
    m_frame->setLineWidth(1);                // 細い線に
    m_frame->setMidLineWidth(0);             // 単線に変更

    // フレームの色を設定
    QPalette framePal = m_frame->palette();
    framePal.setColor(QPalette::WindowText, Qt::black); // 黒い枠線
    m_frame->setPalette(framePal);

    // フレームを最背面に配置
    m_frame->lower();

    // レイアウト
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(2);                  // スペーシングを小さく
    mainLayout->setContentsMargins(6, 6, 6, 6); // マージンを小さく

    // タイトル
    m_titleLabel = new QLabel(m_config.name(), this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(9); // フォントサイズを小さく
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_titleLabel);

    // パス情報（短縮表示）
    QString sourcePath = m_config.sourcePath();
    QString destPath = m_config.destinationPath();

    // 長いパスは省略
    if (sourcePath.length() > 20)
    {
        sourcePath = "..." + sourcePath.right(18);
    }
    if (destPath.length() > 20)
    {
        destPath = "..." + destPath.right(18);
    }

    QLabel *sourceLabel = new QLabel(tr("元: %1").arg(sourcePath), this);
    QLabel *destLabel = new QLabel(tr("先: %1").arg(destPath), this);

    QFont smallFont = sourceLabel->font();
    smallFont.setPointSize(8); // さらに小さいフォント
    sourceLabel->setFont(smallFont);
    destLabel->setFont(smallFont);

    mainLayout->addWidget(sourceLabel);
    mainLayout->addWidget(destLabel);

    // 最終バックアップ日時
    QString lastBackupInfo;
    if (m_config.lastBackupTime().isValid())
    {
        lastBackupInfo = tr("最終: %1").arg(m_config.lastBackupTime().toString("MM/dd hh:mm"));
    }
    else
    {
        lastBackupInfo = tr("未実行");
    }

    m_lastBackupLabel = new QLabel(lastBackupInfo, this);
    m_lastBackupLabel->setFont(smallFont);
    m_lastBackupLabel->setAlignment(Qt::AlignRight);
    mainLayout->addWidget(m_lastBackupLabel);

    // プログレスバー
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
    m_progressBar->setMaximumHeight(10); // より小さいプログレスバー
    mainLayout->addWidget(m_progressBar);

    // ボタン
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(3);

    m_backupButton = new QPushButton(tr("実行"), this);
    m_removeButton = new QPushButton(tr("削除"), this);

    // ボタンを小さく
    m_backupButton->setFixedHeight(22);
    m_removeButton->setFixedHeight(22);

    QFont buttonFont = m_backupButton->font();
    buttonFont.setPointSize(8);
    m_backupButton->setFont(buttonFont);
    m_removeButton->setFont(buttonFont);

    buttonLayout->addWidget(m_backupButton);
    buttonLayout->addWidget(m_removeButton);
    mainLayout->addLayout(buttonLayout);

    // シグナル/スロット接続
    connect(m_backupButton, &QPushButton::clicked, [this]()
            { emit runBackup(m_config); });

    connect(m_removeButton, &QPushButton::clicked, [this]()
            { emit removeBackup(m_index); });

    // 初期サイズに合わせて配置
    m_frame->setGeometry(0, 0, width(), height());
}

BackupConfig BackupCard::config() const
{
    return m_config;
}

int BackupCard::index() const
{
    return m_index;
}

QSize BackupCard::minimumSizeHint() const
{
    // カードのサイズを小さくする
    return QSize(200, 120); // 修正: 幅200, 高さ120に縮小
}

QSize BackupCard::sizeHint() const
{
    return minimumSizeHint();
}

void BackupCard::updateProgress(int progress)
{
    if (m_progressBar)
    {
        m_progressBar->setValue(progress);
        m_progressBar->setVisible(progress > 0 && progress < 100);
    }
}

void BackupCard::resetProgress()
{
    if (m_progressBar)
    {
        m_progressBar->setValue(0);
        m_progressBar->setVisible(false);
    }
}

void BackupCard::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_frame)
    {
        m_frame->setGeometry(0, 0, width(), height());
    }
}

// 進捗状況を設定するメソッド
void BackupCard::setProgress(int value)
{
    // 進捗バーを表示して値を設定
    if (!m_progressBar->isVisible())
    {
        m_progressBar->setVisible(true);
    }

    m_progressBar->setValue(value);

    // 100%完了したら非表示に戻す
    if (value >= 100)
    {
        // 少し遅延して非表示にすることも検討
        // QTimer::singleShot(1000, [this]() { m_progressBar->setVisible(false); });
        m_progressBar->setVisible(false);
    }
}