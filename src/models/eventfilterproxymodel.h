#ifndef EVENTFILTERPROXYMODEL_H
#define EVENTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QDate>
#include <QVariantMap>

class EventFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

    Q_PROPERTY(QString filterTitle READ filterTitle WRITE setFilterTitle NOTIFY filterTitleChanged)
    Q_PROPERTY(QDate filterStartDateFrom READ filterStartDateFrom WRITE setFilterStartDateFrom NOTIFY filterStartDateFromChanged)
    Q_PROPERTY(QDate filterStartDateTo READ filterStartDateTo WRITE setFilterStartDateTo NOTIFY filterStartDateToChanged)
    Q_PROPERTY(QString filterEventType READ filterEventType WRITE setFilterEventType NOTIFY filterEventTypeChanged)
    Q_PROPERTY(QString filterCategory READ filterCategory WRITE setFilterCategory NOTIFY filterCategoryChanged)
    Q_PROPERTY(int filterDurationMin READ filterDurationMin WRITE setFilterDurationMin NOTIFY filterDurationMinChanged)

public:
    explicit EventFilterProxyModel(QObject *parent = nullptr);

    // Геттеры и сеттеры для свойств фильтрации
    QString filterTitle() const;
    void setFilterTitle(const QString& title);

    QDate filterStartDateFrom() const;
    void setFilterStartDateFrom(const QDate& date);

    QDate filterStartDateTo() const;
    void setFilterStartDateTo(const QDate& date);

    QString filterEventType() const;
    void setFilterEventType(const QString& type);

    QString filterCategory() const;
    void setFilterCategory(const QString& category);

    int filterDurationMin() const;
    void setFilterDurationMin(int days);

    // Применение всех фильтров сразу из QVariantMap
    Q_INVOKABLE void setFilterCriteria(const QVariantMap& criteria);
    
    // Сброс всех фильтров
    Q_INVOKABLE void resetFilters();

signals:
    void filterTitleChanged();
    void filterStartDateFromChanged();
    void filterStartDateToChanged();
    void filterEventTypeChanged();
    void filterCategoryChanged();
    void filterDurationMinChanged();
    void filtersApplied();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QString m_filterTitle;
    QDate m_filterStartDateFrom;
    QDate m_filterStartDateTo;
    QString m_filterEventType;
    QString m_filterCategory;
    int m_filterDurationMin = 0;
};

#endif // EVENTFILTERPROXYMODEL_H
