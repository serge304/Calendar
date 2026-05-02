#ifndef EVENTLISTVIEW_H
#define EVENTLISTVIEW_H

#include <QListView>
#include "models/event.h"

class EventDelegate;

class EventListView : public QListView {
    Q_OBJECT

public:
    explicit EventListView(QWidget *parent = nullptr);
    ~EventListView();

signals:
    void eventDoubleClicked(const Event& event);
    void deleteEventRequested(int64_t eventId);

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    
private:
    EventDelegate* m_delegate;
};

#endif // EVENTLISTVIEW_H
