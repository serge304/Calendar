#include "database/dbmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

DbManager::DbManager(QObject *parent) : QObject(parent) {
}

DbManager::~DbManager() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DbManager::initDatabase(const QString& dbPath) {
    // Подключение к базе данных SQLite
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);
    
    if (!m_db.open()) {
        qCritical() << "Ошибка открытия базы данных:" << m_db.lastError().text();
        return false;
    }
    
    // Создание таблиц
    if (!createTables()) {
        return false;
    }
    
    qDebug() << "База данных успешно инициализирована";
    return true;
}

bool DbManager::createTables() {
    QSqlQuery query;
    
    // Таблица событий
    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            startDate TEXT NOT NULL,
            durationDays INTEGER DEFAULT 1,
            description TEXT,
            eventType TEXT,
            category TEXT,
            iconData BLOB,
            imageDataList BLOB
        )
    )";
    
    if (!query.exec(createTableQuery)) {
        qCritical() << "Ошибка создания таблицы events:" << query.lastError().text();
        return false;
    }
    
    // Индексы для ускорения фильтрации
    query.exec("CREATE INDEX IF NOT EXISTS idx_eventType ON events(eventType)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_category ON events(category)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_startDate ON events(startDate)");
    
    return true;
}

bool DbManager::addEvent(const Event& event) {
    QSqlQuery query;
    
    query.prepare(R"(
        INSERT INTO events (title, startDate, durationDays, description, 
                           eventType, category, iconData, imageDataList)
        VALUES (:title, :startDate, :durationDays, :description, 
                :eventType, :category, :iconData, :imageDataList)
    )");
    
    bindEventToQuery(query, event);
    
    if (!query.exec()) {
        qCritical() << "Ошибка добавления события:" << query.lastError().text();
        return false;
    }
    
    // Получаем ID нового события
    event.id = query.lastInsertId().toLongLong();
    return true;
}

bool DbManager::updateEvent(const Event& event) {
    if (event.id < 0) {
        qWarning() << "Нельзя обновить событие без ID";
        return false;
    }
    
    QSqlQuery query;
    
    query.prepare(R"(
        UPDATE events 
        SET title = :title,
            startDate = :startDate,
            durationDays = :durationDays,
            description = :description,
            eventType = :eventType,
            category = :category,
            iconData = :iconData,
            imageDataList = :imageDataList
        WHERE id = :id
    )");
    
    bindEventToQuery(query, event);
    query.bindValue(":id", event.id);
    
    if (!query.exec()) {
        qCritical() << "Ошибка обновления события:" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

bool DbManager::deleteEvent(int64_t eventId) {
    QSqlQuery query;
    
    query.prepare("DELETE FROM events WHERE id = :id");
    query.bindValue(":id", eventId);
    
    if (!query.exec()) {
        qCritical() << "Ошибка удаления события:" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

Event DbManager::getEvent(int64_t eventId) const {
    QSqlQuery query;
    
    query.prepare("SELECT * FROM events WHERE id = :id");
    query.bindValue(":id", eventId);
    
    if (!query.exec() || !query.next()) {
        qWarning() << "Событие не найдено или ошибка запроса";
        return Event();
    }
    
    return queryToEvent(const_cast<QSqlQuery&>(query));
}

QList<Event> DbManager::getAllEvents() const {
    QList<Event> events;
    
    QSqlQuery query("SELECT * FROM events ORDER BY startDate");
    
    while (query.next()) {
        events.append(queryToEvent(const_cast<QSqlQuery&>(query)));
    }
    
    return events;
}

QList<Event> DbManager::searchEvents(const QVariantMap& filterCriteria) const {
    QList<Event> events;
    
    QStringList conditions;
    QStringList bindings;
    
    // Построение динамического запроса на основе критериев фильтрации
    if (filterCriteria.contains("title") && !filterCriteria["title"].toString().isEmpty()) {
        conditions.append("title LIKE :title");
        bindings.append("title");
    }
    
    if (filterCriteria.contains("startDateFrom") && filterCriteria["startDateFrom"].toDate().isValid()) {
        conditions.append("startDate >= :startDateFrom");
        bindings.append("startDateFrom");
    }
    
    if (filterCriteria.contains("startDateTo") && filterCriteria["startDateTo"].toDate().isValid()) {
        conditions.append("startDate <= :startDateTo");
        bindings.append("startDateTo");
    }
    
    if (filterCriteria.contains("eventType") && !filterCriteria["eventType"].toString().isEmpty()) {
        conditions.append("eventType = :eventType");
        bindings.append("eventType");
    }
    
    if (filterCriteria.contains("category") && !filterCriteria["category"].toString().isEmpty()) {
        conditions.append("category = :category");
        bindings.append("category");
    }
    
    if (filterCriteria.contains("durationMin") && filterCriteria["durationMin"].toInt() > 0) {
        conditions.append("durationDays >= :durationMin");
        bindings.append("durationMin");
    }
    
    QString queryString = "SELECT * FROM events";
    if (!conditions.isEmpty()) {
        queryString += " WHERE " + conditions.join(" AND ");
    }
    queryString += " ORDER BY startDate";
    
    QSqlQuery query;
    query.prepare(queryString);
    
    // Привязка параметров
    for (const auto& binding : bindings) {
        query.bindValue(":" + binding, filterCriteria[binding]);
    }
    
    // Особая обработка для LIKE
    if (filterCriteria.contains("title") && !filterCriteria["title"].toString().isEmpty()) {
        query.bindValue(":title", "%" + filterCriteria["title"].toString() + "%");
    }
    
    if (!query.exec()) {
        qCritical() << "Ошибка поиска событий:" << query.lastError().text();
        return events;
    }
    
    while (query.next()) {
        events.append(queryToEvent(const_cast<QSqlQuery&>(query)));
    }
    
    return events;
}

QStringList DbManager::getEventTypes() const {
    QStringList types;
    
    QSqlQuery query("SELECT DISTINCT eventType FROM events WHERE eventType IS NOT NULL ORDER BY eventType");
    
    while (query.next()) {
        types.append(query.value(0).toString());
    }
    
    return types;
}

QStringList DbManager::getCategories() const {
    QStringList categories;
    
    QSqlQuery query("SELECT DISTINCT category FROM events WHERE category IS NOT NULL ORDER BY category");
    
    while (query.next()) {
        categories.append(query.value(0).toString());
    }
    
    return categories;
}

void DbManager::bindEventToQuery(QSqlQuery& query, const Event& event) const {
    query.bindValue(":title", event.title);
    query.bindValue(":startDate", event.startDate.toString(Qt::ISODate));
    query.bindValue(":durationDays", event.durationDays);
    query.bindValue(":description", event.description);
    query.bindValue(":eventType", event.eventType);
    query.bindValue(":category", event.category);
    query.bindValue(":iconData", event.iconData);
    
    // Сериализация списка изображений в JSON + base64
    QJsonArray imageArray;
    for (const auto& img : event.imageDataList) {
        imageArray.append(QString::fromLatin1(img.toBase64()));
    }
    QJsonDocument doc(imageArray);
    query.bindValue(":imageDataList", doc.toJson(QJsonDocument::Compact));
}

Event DbManager::queryToEvent(QSqlQuery& query) const {
    Event event;
    
    event.id = query.value("id").toLongLong();
    event.title = query.value("title").toString();
    event.startDate = QDate::fromString(query.value("startDate").toString(), Qt::ISODate);
    event.durationDays = query.value("durationDays").toInt();
    event.description = query.value("description").toString();
    event.eventType = query.value("eventType").toString();
    event.category = query.value("category").toString();
    event.iconData = query.value("iconData").toByteArray();
    
    // Десериализация списка изображений
    QByteArray imageData = query.value("imageDataList").toByteArray();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(imageData, &parseError);
    
    if (parseError.error == QJsonParseError::NoError && doc.isArray()) {
        QJsonArray imageArray = doc.array();
        for (const auto& imgVal : imageArray) {
            QString base64Str = imgVal.toString();
            event.imageDataList.append(QByteArray::fromBase64(base64Str.toLatin1()));
        }
    }
    
    return event;
}
