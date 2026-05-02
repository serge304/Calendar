#include "views/eventlistview.h"
#include "delegates/eventdelegate.h"
#include "models/eventmodel.h"
#include <QMouseEvent>
#include <QMenu>
#include <QAction>

EventListView::EventListView(QWidget *parent)
    : QListView(parent)
    , m_delegate(nullptr)
{
    // Устанавливаем кастомный делегат для отрисовки событий
    m_delegate = new EventDelegate(this);
    setItemDelegate(m_delegate);
    
    // Настройки отображения
    setViewMode(QListView::ListMode);
    setMovement(QListView::Static);
    setUniformItemSizes(true);
    setWordWrap(true);
    
    // Контекстное меню
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QListView::customContextMenuRequested,
            this, [this](const QPoint& pos) {
        QModelIndex index = indexAt(pos);
        if (!index.isValid())
            return;
        
        QMenu menu(this);
        
        QAction* viewAction = menu.addAction("Просмотр");
        QAction* editAction = menu.addAction("Редактировать");
        menu.addSeparator();
        QAction* deleteAction = menu.addAction("Удалить");
        
        QAction* selectedAction = menu.exec(mapToGlobal(pos));
        
        if (selectedAction == viewAction || selectedAction == editAction) {
            Event event = model()->data(index, EventModel::TitleRole).toString().isEmpty() 
                ? Event() : Event();
            // В реальной реализации нужно получить полное событие
            emit eventDoubleClicked(event);
        } else if (selectedAction == deleteAction) {
            int64_t eventId = model()->data(index, EventModel::IdRole).toLongLong();
            emit deleteEventRequested(eventId);
        }
    });
}

EventListView::~EventListView() {
}

void EventListView::mouseDoubleClickEvent(QMouseEvent* event) {
    QModelIndex index = indexAt(event->pos());
    
    if (index.isValid()) {
        // Получаем данные события
        QVariantMap eventData;
        eventData["id"] = model()->data(index, EventModel::IdRole);
        eventData["title"] = model()->data(index, EventModel::TitleRole);
        eventData["startDate"] = model()->data(index, EventModel::StartDateRole);
        eventData["durationDays"] = model()->data(index, EventModel::DurationDaysRole);
        eventData["description"] = model()->data(index, EventModel::DescriptionRole);
        eventData["eventType"] = model()->data(index, EventModel::EventTypeRole);
        eventData["category"] = model()->data(index, EventModel::CategoryRole);
        
        Event event = Event::fromMap(eventData);
        emit eventDoubleClicked(event);
    }
    
    QListView::mouseDoubleClickEvent(event);
}
