#include "delegates/eventdelegate.h"
#include "models/eventmodel.h"
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QApplication>
#include <QPalette>

EventDelegate::EventDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {
}

void EventDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const {
    painter->save();
    
    // Получаем данные события
    QString title = index.data(EventModel::TitleRole).toString();
    QString description = index.data(EventModel::DescriptionRole).toString();
    QDate startDate = index.data(EventModel::StartDateRole).toDate();
    int durationDays = index.data(EventModel::DurationDaysRole).toInt();
    QString eventType = index.data(EventModel::EventTypeRole).toString();
    QString category = index.data(EventModel::CategoryRole).toString();
    QByteArray iconData = index.data(EventModel::IconDataRole).toByteArray();
    
    // Определяем цвета в зависимости от типа события
    QColor bgColor;
    if (eventType == "holiday" || eventType == "Праздник") {
        bgColor = QColor(255, 235, 238);  // Светло-красный
    } else if (eventType == "task" || eventType == "Задача") {
        bgColor = QColor(232, 240, 254);  // Светло-синий
    } else if (eventType == "meeting" || eventType == "Встреча") {
        bgColor = QColor(230, 247, 231);  // Светло-зеленый
    } else {
        bgColor = option.palette.color(QPalette::Base);
    }
    
    // Рисуем фон
    drawEventBackground(painter, option, eventType);
    
    // Создаем прямоугольник для контента с отступами
    QRect contentRect = option.rect.adjusted(10, 8, -10, -8);
    
    // Рисуем иконку слева
    QRect iconRect(contentRect.left(), contentRect.top(), 40, 40);
    drawEventIcon(painter, iconRect, iconData);
    
    // Основная область текста (справа от иконки)
    QRect textRect(contentRect.left() + 50, contentRect.top(),
                   contentRect.width() - 60, contentRect.height());
    
    // Рисуем текст
    drawEventText(painter, textRect, title, description, startDate, durationDays);
    
    // Рисуем теги (тип и категория) внизу
    QRect tagsRect(textRect.left(), textRect.bottom() - 25,
                   textRect.width(), 25);
    drawEventTags(painter, tagsRect, eventType, category);
    
    // Рисуем рамку вокруг элемента
    painter->setPen(QPen(QColor(200, 200, 200), 1));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 5, 5);
    
    // Если элемент выбран, рисуем выделение
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, QColor(0, 120, 215, 40));
        painter->setPen(QPen(QColor(0, 120, 215), 2));
        painter->drawRoundedRect(option.rect.adjusted(2, 2, -2, -2), 5, 5);
    }
    
    painter->restore();
}

QSize EventDelegate::sizeHint(const QStyleOptionViewItem &option,
                              const QModelIndex &index) const {
    Q_UNUSED(index);
    
    // Фиксированная высота для каждого элемента
    return QSize(option.rect.width(), 90);
}

void EventDelegate::drawEventBackground(QPainter *painter, 
                                        const QStyleOptionViewItem &option,
                                        const QString& eventType) const {
    QColor bgColor;
    
    if (eventType == "holiday" || eventType == "Праздник") {
        bgColor = QColor(255, 235, 238);
    } else if (eventType == "task" || eventType == "Задача") {
        bgColor = QColor(232, 240, 254);
    } else if (eventType == "meeting" || eventType == "Встреча") {
        bgColor = QColor(230, 247, 231);
    } else if (eventType == "reminder" || eventType == "Напоминание") {
        bgColor = QColor(255, 249, 224);
    } else {
        bgColor = QColor(250, 250, 250);
    }
    
    painter->fillRect(option.rect, bgColor);
}

void EventDelegate::drawEventIcon(QPainter *painter, const QRect& rect,
                                  const QByteArray& iconData) const {
    Q_UNUSED(iconData);
    
    // Рисуем emoji иконку в зависимости от типа
    // В реальной реализации здесь можно загружать изображения из iconData
    
    painter->setFont(QFont("Segoe UI Emoji", 24));
    painter->setPen(Qt::black);
    painter->drawText(rect, Qt::AlignCenter, "📅");
}

void EventDelegate::drawEventText(QPainter *painter, const QRect& rect,
                                   const QString& title, const QString& description,
                                   const QDate& startDate, int durationDays) const {
    QRect textRect = rect;
    
    // Заголовок (жирный шрифт)
    QFont titleFont = painter->font();
    titleFont.setBold(true);
    titleFont.setPointSize(12);
    painter->setFont(titleFont);
    painter->setPen(QColor(33, 33, 33));
    
    QString elidedTitle = painter->fontMetrics().elidedText(
        title, Qt::ElideRight, textRect.width());
    painter->drawText(textRect.topLeft(), elidedTitle);
    
    // Дата и длительность
    QFont dateFont = painter->font();
    dateFont.setPointSize(9);
    dateFont.setItalic(true);
    painter->setFont(dateFont);
    painter->setPen(QColor(100, 100, 100));
    
    QString dateStr = startDate.toString("dd.MM.yyyy");
    if (durationDays > 1) {
        dateStr += QString(" (%1 дн.)").arg(durationDays);
    }
    
    QRect dateRect(textRect.left(), textRect.top() + 18, textRect.width(), 20);
    painter->drawText(dateRect, dateStr);
    
    // Описание (если есть)
    if (!description.isEmpty()) {
        QFont descFont = painter->font();
        descFont.setPointSize(9);
        descFont.setItalic(false);
        painter->setFont(descFont);
        painter->setPen(QColor(80, 80, 80));
        
        QString elidedDesc = painter->fontMetrics().elidedText(
            description, Qt::ElideRight, textRect.width());
        
        QRect descRect(textRect.left(), textRect.top() + 38, textRect.width(), 20);
        painter->drawText(descRect, elidedDesc);
    }
}

void EventDelegate::drawEventTags(QPainter *painter, const QRect& rect,
                                   const QString& eventType, 
                                   const QString& category) const {
    const int tagHeight = 18;
    const int tagSpacing = 5;
    
    // Цвета для тегов
    QMap<QString, QColor> typeColors;
    typeColors["meeting"] = QColor(33, 150, 243);
    typeColors["task"] = QColor(76, 175, 80);
    typeColors["holiday"] = QColor(244, 67, 54);
    typeColors["reminder"] = QColor(255, 152, 0);
    typeColors["event"] = QColor(156, 39, 176);
    
    QMap<QString, QColor> catColors;
    catColors["work"] = QColor(33, 150, 243);
    catColors["personal"] = QColor(156, 39, 176);
    catColors["family"] = QColor(76, 175, 80);
    catColors["education"] = QColor(255, 152, 0);
    catColors["sports"] = QColor(244, 67, 54);
    
    int x = rect.left();
    int y = rect.top();
    
    // Рисуем тег типа
    if (!eventType.isEmpty()) {
        QColor tagColor = typeColors.value(eventType.toLower(), QColor(158, 158, 158));
        
        painter->setBrush(tagColor);
        painter->setPen(Qt::NoPen);
        
        QFont tagFont = painter->font();
        tagFont.setPointSize(8);
        painter->setFont(tagFont);
        
        QString displayType = eventType;
        int tagWidth = painter->fontMetrics().horizontalAdvance(displayType) + 16;
        
        painter->drawRoundedRect(x, y, tagWidth, tagHeight, 3, 3);
        
        painter->setPen(Qt::white);
        painter->setFont(tagFont);
        painter->drawText(QRect(x + 8, y, tagWidth - 16, tagHeight), 
                         Qt::AlignVCenter | Qt::AlignLeft, displayType);
        
        x += tagWidth + tagSpacing;
    }
    
    // Рисуем тег категории
    if (!category.isEmpty()) {
        QColor tagColor = catColors.value(category.toLower(), QColor(158, 158, 158));
        
        painter->setBrush(tagColor);
        painter->setPen(Qt::NoPen);
        
        QFont tagFont = painter->font();
        tagFont.setPointSize(8);
        painter->setFont(tagFont);
        
        QString displayCat = category;
        int tagWidth = painter->fontMetrics().horizontalAdvance(displayCat) + 16;
        
        painter->drawRoundedRect(x, y, tagWidth, tagHeight, 3, 3);
        
        painter->setPen(Qt::white);
        painter->setFont(tagFont);
        painter->drawText(QRect(x + 8, y, tagWidth - 16, tagHeight), 
                         Qt::AlignVCenter | Qt::AlignLeft, displayCat);
    }
}
