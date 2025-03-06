#include "BackupListDelegate.h"
#include <QPainter>
#include <QApplication>
#include "ui/BackupListDelegate.h"

BackupListDelegate::BackupListDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void BackupListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    // ヘッダー行の特別な描画
    if (isHeaderRow(index))
    {
        // ヘッダー背景を描画
        painter->fillRect(option.rect, QColor(230, 230, 240));

        // ヘッダーテキストを描画
        QPen pen(QColor(60, 60, 80));
        painter->setPen(pen);

        QFont headerFont = option.font;
        headerFont.setBold(true);
        painter->setFont(headerFont);

        painter->drawText(option.rect.adjusted(4, 4, -4, -4),
                          Qt::AlignLeft | Qt::AlignVCenter,
                          index.data().toString());
    }
    else
    {
        // 通常の行の描画
        if (option.state & QStyle::State_Selected)
        {
            painter->fillRect(option.rect, option.palette.highlight());
            painter->setPen(option.palette.highlightedText().color());
        }
        else if (option.state & QStyle::State_MouseOver)
        {
            painter->fillRect(option.rect, QColor(240, 240, 245)); // ホバー色
            painter->setPen(option.palette.text().color());
        }
        else
        {
            painter->fillRect(option.rect, option.palette.base());
            painter->setPen(option.palette.text().color());
        }

        // 区切り線
        painter->drawLine(
            option.rect.left(), option.rect.bottom(),
            option.rect.right(), option.rect.bottom());

        // テキスト描画
        QString text = index.data().toString();
        painter->drawText(option.rect.adjusted(4, 4, -4, -4),
                          Qt::AlignLeft | Qt::AlignVCenter, text);
    }

    painter->restore();
}

QSize BackupListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);

    // ヘッダー行は通常より少し高くする
    if (isHeaderRow(index))
    {
        size.setHeight(30);
    }
    else
    {
        size.setHeight(40);
    }

    return size;
}

bool BackupListDelegate::isHeaderRow(const QModelIndex &index) const
{
    // インデックスが0ならヘッダー行
    return index.row() == 0;
}
