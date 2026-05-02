#include "models/eventfilterproxymodel.h"
#include "models/eventmodel.h"
#include <QDebug>

EventFilterProxyModel::EventFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent) {
}

QString EventFilterProxyModel::filterTitle() const {
    return m_filterTitle;
}

void EventFilterProxyModel::setFilterTitle(const QString& title) {
    if (m_filterTitle != title) {
        m_filterTitle = title;
        emit filterTitleChanged();
        invalidateFilter();
        emit filtersApplied();
    }
}

QDate EventFilterProxyModel::filterStartDateFrom() const {
    return m_filterStartDateFrom;
}

void EventFilterProxyModel::setFilterStartDateFrom(const QDate& date) {
    if (m_filterStartDateFrom != date) {
        m_filterStartDateFrom = date;
        emit filterStartDateFromChanged();
        invalidateFilter();
        emit filtersApplied();
    }
}

QDate EventFilterProxyModel::filterStartDateTo() const {
    return m_filterStartDateTo;
}

void EventFilterProxyModel::setFilterStartDateTo(const QDate& date) {
    if (m_filterStartDateTo != date) {
        m_filterStartDateTo = date;
        emit filterStartDateToChanged();
        invalidateFilter();
        emit filtersApplied();
    }
}

QString EventFilterProxyModel::filterEventType() const {
    return m_filterEventType;
}

void EventFilterProxyModel::setFilterEventType(const QString& type) {
    if (m_filterEventType != type) {
        m_filterEventType = type;
        emit filterEventTypeChanged();
        invalidateFilter();
        emit filtersApplied();
    }
}

QString EventFilterProxyModel::filterCategory() const {
    return m_filterCategory;
}

void EventFilterProxyModel::setFilterCategory(const QString& category) {
    if (m_filterCategory != category) {
        m_filterCategory = category;
        emit filterCategoryChanged();
        invalidateFilter();
        emit filtersApplied();
    }
}

int EventFilterProxyModel::filterDurationMin() const {
    return m_filterDurationMin;
}

void EventFilterProxyModel::setFilterDurationMin(int days) {
    if (m_filterDurationMin != days) {
        m_filterDurationMin = days;
        emit filterDurationMinChanged();
        invalidateFilter();
        emit filtersApplied();
    }
}

void EventFilterProxyModel::setFilterCriteria(const QVariantMap& criteria) {
    bool filterChanged = false;
    
    if (criteria.contains("title")) {
        QString title = criteria["title"].toString();
        if (m_filterTitle != title) {
            m_filterTitle = title;
            emit filterTitleChanged();
            filterChanged = true;
        }
    }
    
    if (criteria.contains("startDateFrom")) {
        QDate dateFrom = criteria["startDateFrom"].toDate();
        if (m_filterStartDateFrom != dateFrom) {
            m_filterStartDateFrom = dateFrom;
            emit filterStartDateFromChanged();
            filterChanged = true;
        }
    }
    
    if (criteria.contains("startDateTo")) {
        QDate dateTo = criteria["startDateTo"].toDate();
        if (m_filterStartDateTo != dateTo) {
            m_filterStartDateTo = dateTo;
            emit filterStartDateToChanged();
            filterChanged = true;
        }
    }
    
    if (criteria.contains("eventType")) {
        QString type = criteria["eventType"].toString();
        if (m_filterEventType != type) {
            m_filterEventType = type;
            emit filterEventTypeChanged();
            filterChanged = true;
        }
    }
    
    if (criteria.contains("category")) {
        QString category = criteria["category"].toString();
        if (m_filterCategory != category) {
            m_filterCategory = category;
            emit filterCategoryChanged();
            filterChanged = true;
        }
    }
    
    if (criteria.contains("durationMin")) {
        int duration = criteria["durationMin"].toInt();
        if (m_filterDurationMin != duration) {
            m_filterDurationMin = duration;
            emit filterDurationMinChanged();
            filterChanged = true;
        }
    }
    
    if (filterChanged) {
        invalidateFilter();
        emit filtersApplied();
    }
}

void EventFilterProxyModel::resetFilters() {
    m_filterTitle.clear();
    m_filterStartDateFrom = QDate();
    m_filterStartDateTo = QDate();
    m_filterEventType.clear();
    m_filterCategory.clear();
    m_filterDurationMin = 0;
    
    emit filterTitleChanged();
    emit filterStartDateFromChanged();
    emit filterStartDateToChanged();
    emit filterEventTypeChanged();
    emit filterCategoryChanged();
    emit filterDurationMinChanged();
    
    invalidateFilter();
    emit filtersApplied();
}

bool EventFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    
    // Фильтр по названию (поиск подстроки)
    if (!m_filterTitle.isEmpty()) {
        QString title = sourceModel()->data(index, EventModel::TitleRole).toString();
        if (!title.contains(m_filterTitle, Qt::CaseInsensitive)) {
            return false;
        }
    }
    
    // Фильтр по дате начала (от)
    if (m_filterStartDateFrom.isValid()) {
        QDate startDate = sourceModel()->data(index, EventModel::StartDateRole).toDate();
        if (startDate < m_filterStartDateFrom) {
            return false;
        }
    }
    
    // Фильтр по дате начала (до)
    if (m_filterStartDateTo.isValid()) {
        QDate startDate = sourceModel()->data(index, EventModel::StartDateRole).toDate();
        if (startDate > m_filterStartDateTo) {
            return false;
        }
    }
    
    // Фильтр по типу события
    if (!m_filterEventType.isEmpty()) {
        QString eventType = sourceModel()->data(index, EventModel::EventTypeRole).toString();
        if (eventType != m_filterEventType) {
            return false;
        }
    }
    
    // Фильтр по категории
    if (!m_filterCategory.isEmpty()) {
        QString category = sourceModel()->data(index, EventModel::CategoryRole).toString();
        if (category != m_filterCategory) {
            return false;
        }
    }
    
    // Фильтр по минимальной длительности
    if (m_filterDurationMin > 0) {
        int duration = sourceModel()->data(index, EventModel::DurationDaysRole).toInt();
        if (duration < m_filterDurationMin) {
            return false;
        }
    }
    
    return true;
}
