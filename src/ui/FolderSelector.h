#ifndef FOLDERSELECTOR_H
#define FOLDERSELECTOR_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDir>

class FolderSelector : public QWidget
{
    Q_OBJECT

public:
    explicit FolderSelector(QWidget *parent = nullptr, bool isSaveMode = false)
        : QWidget(parent), m_isSaveMode(isSaveMode)
    {
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);

        m_lineEdit = new QLineEdit(this);
        m_browseButton = new QPushButton(tr("参照..."), this);

        layout->addWidget(m_lineEdit);
        layout->addWidget(m_browseButton);

        connect(m_browseButton, &QPushButton::clicked, this, &FolderSelector::browseFolder);
        connect(m_lineEdit, &QLineEdit::textChanged, this, &FolderSelector::pathChanged);
    }

    QString path() const { return m_lineEdit->text(); }
    void setPath(const QString &path) { m_lineEdit->setText(path); }
    QLineEdit *lineEdit() const { return m_lineEdit; }

signals:
    void pathChanged(const QString &path);

private slots:
    void browseFolder()
    {
        QString dir;
        if (m_isSaveMode)
        {
            dir = QFileDialog::getExistingDirectory(this, tr("保存先フォルダを選択"),
                                                    m_lineEdit->text().isEmpty() ? QDir::homePath() : m_lineEdit->text(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        }
        else
        {
            dir = QFileDialog::getExistingDirectory(this, tr("フォルダを選択"),
                                                    m_lineEdit->text().isEmpty() ? QDir::homePath() : m_lineEdit->text(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        }

        if (!dir.isEmpty())
        {
            m_lineEdit->setText(dir);
        }
    }

private:
    QLineEdit *m_lineEdit;
    QPushButton *m_browseButton;
    bool m_isSaveMode;
};

#endif // FOLDERSELECTOR_H