#ifndef EVENTDELEGATE_H
#define EVENTDELEGATE_H

#include <QStyledItemDelegate>

class EventDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit EventDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

private:
    // Вспомогательные методы для отрисовки
    void drawEventBackground(QPainter *painter, const QStyleOptionViewItem &option,
                             const QString& eventType) const;
    void drawEventIcon(QPainter *painter, const QRect& rect, 
                       const QByteArray& iconData) const;
    void drawEventText(QPainter *painter, const QRect& rect,
                       const QString& title, const QString& description,
                       const QDate& startDate, int durationDays) const;
    void drawEventTags(QPainter *painter, const QRect& rect,
                       const QString& eventType, const QString& category) const;
};

#endif // EVENTDELEGATE_H
