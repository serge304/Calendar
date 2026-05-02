#include "models/eventmodel.h"
#include <QDebug>

EventModel::EventModel(DbManager* dbManager, QObject *parent)
    : QAbstractListModel(parent), m_dbManager(dbManager) {
}

int EventModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return m_events.count();
}

QVariant EventModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_events.count())
        return QVariant();

    const Event& event = m_events.at(index.row());

    switch (role) {
    case IdRole:
        return QVariant::fromValue(event.id);
    case TitleRole:
        return event.title;
    case StartDateRole:
        return event.startDate;
    case DurationDaysRole:
        return event.durationDays;
    case DescriptionRole:
        return event.description;
    case EventTypeRole:
        return event.eventType;
    case CategoryRole:
        return event.category;
    case IconDataRole:
        return event.iconData;
    case ImageDataListRole: {
        QVariantList imageList;
        for (const auto& img : event.imageDataList) {
            imageList.append(img);
        }
        return imageList;
    }
    case EndDateRole:
        return event.startDate.addDays(event.durationDays - 1);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> EventModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "eventId";
    roles[TitleRole] = "title";
    roles[StartDateRole] = "startDate";
    roles[DurationDaysRole] = "durationDays";
    roles[DescriptionRole] = "description";
    roles[EventTypeRole] = "eventType";
    roles[CategoryRole] = "category";
    roles[IconDataRole] = "iconData";
    roles[ImageDataListRole] = "imageDataList";
    roles[EndDateRole] = "endDate";
    return roles;
}

bool EventModel::addEvent(const Event& event) {
    Event newEvent = event;
    if (!m_dbManager->addEvent(newEvent)) {
        return false;
    }
    
    beginInsertRows(QModelIndex(), m_events.count(), m_events.count());
    m_events.append(newEvent);
    endInsertRows();
    
    return true;
}

bool EventModel::updateEvent(const Event& event) {
    if (!m_dbManager->updateEvent(event)) {
        return false;
    }
    
    int index = findEventIndexById(event.id);
    if (index >= 0) {
        m_events[index] = event;
        emit dataChanged(createIndex(index, 0), createIndex(index, 0));
        return true;
    }
    
    return false;
}

bool EventModel::deleteEvent(int64_t id) {
    if (!m_dbManager->deleteEvent(id)) {
        return false;
    }
    
    int index = findEventIndexById(id);
    if (index >= 0) {
        beginRemoveRows(QModelIndex(), index, index);
        m_events.removeAt(index);
        endRemoveRows();
        return true;
    }
    
    return false;
}

void EventModel::loadAllEvents() {
    beginResetModel();
    m_events = m_dbManager->getAllEvents();
    endResetModel();
    
    emit eventsLoaded(m_events.count());
}

void EventModel::applyFilter(const QVariantMap& filterCriteria) {
    // Эта функция будет использоваться вместе с FilterProxyModel
    // В данной реализации мы просто загружаем отфильтрованные данные
    beginResetModel();
    m_events = m_dbManager->searchEvents(filterCriteria);
    endResetModel();
    
    emit filterApplied(m_events.count());
}

Event EventModel::getEventAt(int row) const {
    if (row >= 0 && row < m_events.count()) {
        return m_events.at(row);
    }
    return Event();
}

void EventModel::clear() {
    beginResetModel();
    m_events.clear();
    endResetModel();
}

int EventModel::findEventIndexById(int64_t id) const {
    for (int i = 0; i < m_events.count(); ++i) {
        if (m_events.at(i).id == id) {
            return i;
        }
    }
    return -1;
}
