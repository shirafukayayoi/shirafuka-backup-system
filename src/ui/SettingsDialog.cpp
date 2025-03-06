#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include <QLineEdit>
#include <QFileDialog>
#include <QStandardPaths>
#include <QSlider>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDebug>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent),
      // 直接初期化リストでQSettingsを初期化
      m_settings(QDir::homePath() + "/shirafuka_settings.ini", QSettings::IniFormat),
      m_tabWidget(nullptr),
      m_buttonBox(nullptr),
      m_useBackgroundCheck(nullptr),
      m_backgroundPathEdit(nullptr),
      m_opacitySlider(nullptr),
      m_imagePreview(nullptr)
{
    setWindowTitle(tr("設定"));
    resize(500, 400);

    qDebug() << "設定ダイアログが使用する設定ファイル: " << m_settings.fileName();

    setupUI();
    loadSettings(); // 設定を読み込む
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::setupUI()
{
    // メインレイアウト
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // タブウィジェット
    m_tabWidget = new QTabWidget(this);

    // --- 外観設定タブ ---
    QWidget *appearanceTab = new QWidget(m_tabWidget);
    QVBoxLayout *appearanceLayout = new QVBoxLayout(appearanceTab);

    // 背景画像設定のグループボックス
    QGroupBox *backgroundGroup = new QGroupBox(tr("背景画像"), appearanceTab);
    QVBoxLayout *bgLayout = new QVBoxLayout(backgroundGroup);

    // 背景画像の使用有無を選択するチェックボックス
    m_useBackgroundCheck = new QCheckBox(tr("背景画像を使用する"));
    m_useBackgroundCheck->setChecked(m_settings.value("Background/UseBackgroundImage", false).toBool());
    bgLayout->addWidget(m_useBackgroundCheck);

    // 画像パス設定
    QHBoxLayout *pathLayout = new QHBoxLayout();
    QLabel *pathLabel = new QLabel(tr("画像パス:"));
    m_backgroundPathEdit = new QLineEdit(m_settings.value("Background/BackgroundImagePath", "").toString());
    QPushButton *browseButton = new QPushButton(tr("参照..."));

    pathLayout->addWidget(pathLabel);
    pathLayout->addWidget(m_backgroundPathEdit);
    pathLayout->addWidget(browseButton);
    bgLayout->addLayout(pathLayout);

    // 現在の背景のプレビュー
    QLabel *previewLabel = new QLabel(tr("プレビュー:"));
    m_imagePreview = new QLabel();
    m_imagePreview->setMinimumSize(200, 120);
    m_imagePreview->setAlignment(Qt::AlignCenter);
    m_imagePreview->setFrameShape(QFrame::StyledPanel);

    QString currentPath = m_settings.value("Background/BackgroundImagePath", "").toString();
    if (!currentPath.isEmpty())
    {
        QPixmap preview = QPixmap(currentPath).scaled(200, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        if (!preview.isNull())
        {
            m_imagePreview->setPixmap(preview);
        }
        else
        {
            m_imagePreview->setText(tr("画像を読み込めません"));
        }
    }
    else
    {
        m_imagePreview->setText(tr("画像なし"));
    }

    bgLayout->addWidget(previewLabel);
    bgLayout->addWidget(m_imagePreview);

    // 透明度スライダー
    QHBoxLayout *opacityLayout = new QHBoxLayout();
    QLabel *opacityLabel = new QLabel(tr("透明度:"));
    m_opacitySlider = new QSlider(Qt::Horizontal);
    m_opacitySlider->setRange(0, 100);
    m_opacitySlider->setValue(m_settings.value("Background/Opacity", 80).toInt());
    QLabel *opacityValue = new QLabel(QString("%1%").arg(m_opacitySlider->value()));

    opacityLayout->addWidget(opacityLabel);
    opacityLayout->addWidget(m_opacitySlider);
    opacityLayout->addWidget(opacityValue);
    bgLayout->addLayout(opacityLayout);

    // 背景グループを外観タブに追加
    appearanceLayout->addWidget(backgroundGroup);
    appearanceLayout->addStretch();

    // 参照ボタンのシグナル接続
    connect(browseButton, &QPushButton::clicked, [this]()
            {
        QString imagePath = QFileDialog::getOpenFileName(this, 
            tr("背景画像を選択"), 
            QDir::homePath(), // ホームディレクトリから開始
            tr("画像ファイル (*.png *.jpg *.jpeg *.bmp *.gif)"));
            
        if (!imagePath.isEmpty()) {
            m_backgroundPathEdit->setText(imagePath);
            
            // プレビューを更新
            QPixmap preview = QPixmap(imagePath).scaled(
                200, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            if (!preview.isNull()) {
                m_imagePreview->setPixmap(preview);
                qDebug() << "プレビュー画像を読み込みました: " << imagePath;
            } else {
                m_imagePreview->setText(tr("画像を読み込めません"));
                qDebug() << "プレビュー画像の読み込みに失敗: " << imagePath;
            }
        } });

    // スライダーの値が変更されたときに、ラベルを更新
    connect(m_opacitySlider, &QSlider::valueChanged, [opacityValue](int value)
            { opacityValue->setText(QString("%1%").arg(value)); });

    // タブに追加
    m_tabWidget->addTab(appearanceTab, tr("外観"));

    // 既存のタブがあれば、それらも追加
    // 例: m_tabWidget->addTab(generalTab, tr("一般"));

    mainLayout->addWidget(m_tabWidget);

    // ダイアログボタン
    m_buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal,
        this);

    mainLayout->addWidget(m_buttonBox);

    // OKボタン接続
    connect(m_buttonBox, &QDialogButtonBox::accepted, [this]()
            {
        saveSettings();
        emit settingsChanged();
        accept(); });

    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

// 設定保存部分を強化

// 明示的なファイルパスでQSettingsを初期化

void SettingsDialog::saveSettings()
{
    // 背景画像設定の保存
    bool useBackground = m_useBackgroundCheck->isChecked();
    QString path = m_backgroundPathEdit->text().trimmed(); // 余分なスペースを削除
    int opacity = m_opacitySlider->value();

    // 設定を明示的なファイルパスで保存
    QString settingsPath = QDir::homePath() + "/shirafuka_settings.ini";
    QSettings settings(settingsPath, QSettings::IniFormat);

    settings.setValue("Background/UseBackgroundImage", useBackground);
    settings.setValue("Background/BackgroundImagePath", path);
    settings.setValue("Background/Opacity", opacity);

    // 設定を即時に反映させる
    settings.sync();

    // 設定が保存されたことを確認
    qDebug() << "設定ファイル: " << settings.fileName();
    qDebug() << "保存後の確認読み取り: " << settings.value("Background/BackgroundImagePath").toString();

    // 同じ設定ファイルを使うように他のクラスに知らせる
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QDir::homePath());
}

bool SettingsDialog::isScheduleEnabled() const
{
    return m_scheduleEnabledCheckBox->isChecked();
}

void SettingsDialog::setScheduleEnabled(bool enabled)
{
    m_scheduleEnabledCheckBox->setChecked(enabled);
    m_scheduledTimeEdit->setEnabled(enabled);
}

QTime SettingsDialog::scheduledTime() const
{
    return m_scheduledTimeEdit->time();
}

void SettingsDialog::setScheduledTime(const QTime &time)
{
    m_scheduledTimeEdit->setTime(time);
}

bool SettingsDialog::isPeriodicBackupEnabled() const
{
    return m_periodicEnabledCheckBox->isChecked();
}

void SettingsDialog::setPeriodicBackupEnabled(bool enabled)
{
    m_periodicEnabledCheckBox->setChecked(enabled);
    m_periodicIntervalSpinBox->setEnabled(enabled);
}

int SettingsDialog::periodicInterval() const
{
    return m_periodicIntervalSpinBox->value();
}

void SettingsDialog::setPeriodicInterval(int hours)
{
    m_periodicIntervalSpinBox->setValue(hours);
}

void SettingsDialog::setNextBackupTime(const QDateTime &time)
{
    if (time.isValid())
    {
        m_nextBackupTimeLabel->setText(tr("次回のバックアップ: %1").arg(time.toString("yyyy/MM/dd HH:mm")));
    }
    else
    {
        m_nextBackupTimeLabel->setText(tr("バックアップが設定されていません"));
    }
}

void SettingsDialog::updateNextBackupDisplay()
{
    if (isScheduleEnabled() || isPeriodicBackupEnabled())
    {
        // 次回バックアップ時刻の計算ロジックはここでは単純化
        QDateTime now = QDateTime::currentDateTime();
        QDateTime nextTime;

        if (isScheduleEnabled())
        {
            QDateTime scheduledDateTime(now.date(), scheduledTime());
            if (scheduledDateTime <= now)
            {
                scheduledDateTime = scheduledDateTime.addDays(1);
            }
            nextTime = scheduledDateTime;
        }

        if (isPeriodicBackupEnabled())
        {
            QDateTime periodicTime = now.addSecs(periodicInterval() * 3600);
            if (!nextTime.isValid() || periodicTime < nextTime)
            {
                nextTime = periodicTime;
            }
        }

        setNextBackupTime(nextTime);
    }
    else
    {
        setNextBackupTime(QDateTime());
    }
}

// 設定読み込みメソッドを追加
void SettingsDialog::loadSettings()
{
    // 設定を読み込む
    bool useBackground = m_settings.value("Background/UseBackgroundImage", false).toBool();
    QString imagePath = m_settings.value("Background/BackgroundImagePath", "").toString();
    int opacity = m_settings.value("Background/Opacity", 80).toInt();

    qDebug() << "設定ダイアログが読み込んだ設定: 使用=" << useBackground
             << ", パス=" << imagePath
             << ", 透明度=" << opacity;

    // UI要素に反映
    if (m_useBackgroundCheck)
    {
        m_useBackgroundCheck->setChecked(useBackground);
    }

    if (m_backgroundPathEdit)
    {
        m_backgroundPathEdit->setText(imagePath);
    }

    if (m_opacitySlider)
    {
        m_opacitySlider->setValue(opacity);
    }

    // プレビュー画像を更新
    if (m_imagePreview && !imagePath.isEmpty())
    {
        QPixmap preview = QPixmap(imagePath).scaled(200, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        if (!preview.isNull())
        {
            m_imagePreview->setPixmap(preview);
        }
        else
        {
            m_imagePreview->setText(tr("画像を読み込めません"));
        }
    }
}