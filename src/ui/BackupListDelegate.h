#ifndef BACKUPLISTDELEGATE_H
#define BACKUPLISTDELEGATE_H

#include <QStyledItemDelegate>

class BackupListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit BackupListDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    // ヘッダー行かどうか判定
    bool isHeaderRow(const QModelIndex &index) const;
};

#endif // BACKUPLISTDELEGATE_H