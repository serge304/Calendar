#ifndef EVENTMODEL_H
#define EVENTMODEL_H

#include <QAbstractListModel>
#include <QList>
#include "models/event.h"
#include "database/dbmanager.h"

class EventModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum EventRoles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        StartDateRole,
        DurationDaysRole,
        DescriptionRole,
        EventTypeRole,
        CategoryRole,
        IconDataRole,
        ImageDataListRole,
        EndDateRole  // Вычисляемое поле
    };

    explicit EventModel(DbManager* dbManager, QObject *parent = nullptr);
    
    // Базовые методы модели
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Добавление/удаление/обновление событий
    Q_INVOKABLE bool addEvent(const Event& event);
    Q_INVOKABLE bool updateEvent(const Event& event);
    Q_INVOKABLE bool deleteEvent(int64_t id);
    
    // Загрузка данных из БД
    Q_INVOKABLE void loadAllEvents();
    
    // Фильтрация (использует FilterProxyModel)
    Q_INVOKABLE void applyFilter(const QVariantMap& filterCriteria);
    
    // Получение события по индексу
    Q_INVOKABLE Event getEventAt(int row) const;
    
    // Очистка модели
    Q_INVOKABLE void clear();

signals:
    void eventsLoaded(int count);
    void filterApplied(int count);

private:
    DbManager* m_dbManager;
    QList<Event> m_events;
    
    // Вспомогательные методы
    int findEventIndexById(int64_t id) const;
};

#endif // EVENTMODEL_H
