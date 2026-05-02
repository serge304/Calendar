#ifndef EVENT_H
#define EVENT_H

#include <QDate>
#include <QString>
#include <QByteArray>
#include <QVariant>
#include <cstdint>

struct Event {
    int64_t id = -1;                    // ID события (для БД)
    QString title;                      // Название события
    QDate startDate;                    // Дата начала
    int durationDays = 1;               // Длительность в днях
    QString description;                // Текст описания
    QString eventType;                  // Тип события (встреча, задача, праздник и т.д.)
    QString category;                   // Категория события
    QByteArray iconData;                // Иконка события (бинарные данные)
    QList<QByteArray> imageDataList;    // Список изображений (бинарные данные)

    Event() = default;

    // Конструктор для создания нового события
    Event(const QString& t, const QDate& start, int days, 
          const QString& desc, const QString& type, const QString& cat)
        : title(t), startDate(start), durationDays(days), 
          description(desc), eventType(type), category(cat) {}

    // Преобразование в QVariantMap для использования в модели данных
    QVariantMap toMap() const {
        QVariantMap map;
        map["id"] = QVariant::fromValue(id);
        map["title"] = title;
        map["startDate"] = startDate;
        map["durationDays"] = durationDays;
        map["description"] = description;
        map["eventType"] = eventType;
        map["category"] = category;
        map["iconData"] = iconData;
        
        // Преобразуем список QByteArray в QVariantList
        QVariantList imageList;
        for (const auto& img : imageDataList) {
            imageList.append(img);
        }
        map["imageDataList"] = imageList;
        
        return map;
    }

    // Создание из QVariantMap (при загрузке из БД)
    static Event fromMap(const QVariantMap& map) {
        Event event;
        event.id = map["id"].toLongLong();
        event.title = map["title"].toString();
        event.startDate = map["startDate"].toDate();
        event.durationDays = map["durationDays"].toInt();
        event.description = map["description"].toString();
        event.eventType = map["eventType"].toString();
        event.category = map["category"].toString();
        event.iconData = map["iconData"].toByteArray();
        
        QVariantList imageList = map["imageDataList"].toList();
        for (const auto& img : imageList) {
            event.imageDataList.append(img.toByteArray());
        }
        
        return event;
    }
};

// Перегрузка оператора сравнения для фильтрации
bool operator==(const Event& lhs, const Event& rhs) {
    return lhs.id == rhs.id;
}

#endif // EVENT_H
