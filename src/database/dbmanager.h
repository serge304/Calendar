#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QList>
#include <QVariantMap>
#include "models/event.h"

class DbManager : public QObject {
    Q_OBJECT

public:
    explicit DbManager(QObject *parent = nullptr);
    ~DbManager();

    // Инициализация базы данных
    bool initDatabase(const QString& dbPath = ":memory:");
    
    // CRUD операции для событий
    bool addEvent(Event& event);  // Не const, т.к. меняется ID после вставки
    bool updateEvent(const Event& event);
    bool deleteEvent(int64_t eventId);
    Event getEvent(int64_t eventId) const;
    QList<Event> getAllEvents() const;
    
    // Поиск событий по фильтру
    QList<Event> searchEvents(const QVariantMap& filterCriteria) const;
    
    // Получение уникальных типов и категорий для фильтров
    QStringList getEventTypes() const;
    QStringList getCategories() const;

private:
    QSqlDatabase m_db;
    
    // Создание таблицы событий
    bool createTables();
    
    // Преобразование Event в значения для запроса
    void bindEventToQuery(QSqlQuery& query, const Event& event) const;
    
    // Преобразование результата запроса в Event
    Event queryToEvent(QSqlQuery& query) const;
};

#endif // DBMANAGER_H
