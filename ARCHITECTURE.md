# Архитектура приложения "Календарь событий" на Qt (C++)

## Общая архитектура в парадигме MVC

Приложение построено по паттерну **Model-View-Controller (MVC)** с использованием возможностей фреймворка Qt:

```
┌─────────────────────────────────────────────────────────────────┐
│                        VIEW (Представление)                      │
│  ┌─────────────┐  ┌──────────────┐  ┌─────────────────────────┐ │
│  │ MainWindow  │  │ EventDialog  │  │    EventListView        │ │
│  │ - Панель    │  │ - Диалог     │  │ - Список событий с      │ │
│  │   фильтров  │  │   добавления │  │   кастомным делегатом   │ │
│  │ - Список    │  │ /редактирова│  │ - Контекстное меню      │ │
│  │   событий   │  │   ния        │  │ - Двойной клик          │ │
│  └──────┬──────┘  └──────┬───────┘  └───────────┬─────────────┘ │
└─────────┼────────────────┼──────────────────────┼───────────────┘
          │                │                      │
          ▼                ▼                      ▼
┌─────────────────────────────────────────────────────────────────┐
│                    CONTROLLER (Логика управления)                │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │  MainWindow (координирует взаимодействие View и Model)      ││
│  │  - Обработка сигналов от UI                                 ││
│  │  - Применение фильтров                                      ││
│  │  - Открытие диалогов                                        ││
│  └─────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘
          │
          ▼
┌─────────────────────────────────────────────────────────────────┐
│                         MODEL (Данные)                           │
│  ┌─────────────┐  ┌──────────────────┐  ┌────────────────────┐  │
│  │  EventModel │  │EventFilterProxy  │  │    DbManager       │  │
│  │ - QAbstract │  │ - QSortFilter    │  │ - QSqlDatabase     │  │
│  │   ListModel │  │   ProxyModel     │  │ - CRUD операции    │  │
│  │ - Список    │  │ - Фильтрация     │  │ - Поиск по         │  │
│  │   событий   │  │   по критериям   │  │   критериям        │  │
│  └─────────────┘  └──────────────────┘  └────────────────────┘  │
│                                                                  │
│  ┌─────────────────────────────────────────────────────────────┐│
│  │  Event (struct) - Модель данных события                     ││
│  │  - id, title, startDate, durationDays                       ││
│  │  - description, eventType, category                         ││
│  │  - iconData, imageDataList                                  ││
│  └─────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘
```

## Структура файлов проекта

```
CalendarApp/
├── CMakeLists.txt              # Конфигурация сборки CMake
├── src/
│   ├── main.cpp                # Точка входа
│   ├── database/
│   │   ├── dbmanager.h         # Менеджер базы данных
│   │   └── dbmanager.cpp
│   ├── models/
│   │   ├── event.h             # Структура данных события
│   │   ├── eventmodel.h        # Модель данных (QAbstractListModel)
│   │   ├── eventmodel.cpp
│   │   ├── eventfilterproxymodel.h  # Proxy-модель для фильтрации
│   │   └── eventfilterproxymodel.cpp
│   ├── views/
│   │   ├── mainwindow.h        # Главное окно
│   │   ├── mainwindow.cpp
│   │   ├── eventdialog.h       # Диалог добавления/редактирования
│   │   ├── eventdialog.cpp
│   │   ├── eventlistview.h     # Кастомный список событий
│   │   └── eventlistview.cpp
│   └── delegates/
│       ├── eventdelegate.h     # Делегат для отрисовки событий
│       └── eventdelegate.cpp
└── README.md                   # Документация
```

## Механизм получения и сохранения данных события

### 1. Добавление нового события

```cpp
// В EventDialog пользователь заполняет форму
Event event;
event.title = "Совещание";
event.startDate = QDate(2024, 1, 15);
event.durationDays = 1;
event.description = "Еженедельное совещание команды";
event.eventType = "meeting";
event.category = "work";
event.iconData = ...;           // Бинарные данные иконки
event.imageDataList = {...};    // Список изображений (QByteArray)

// В MainWindow при нажатии OK
m_eventModel->addEvent(event);

// В EventModel::addEvent()
bool EventModel::addEvent(const Event& event) {
    Event newEvent = event;
    if (!m_dbManager->addEvent(newEvent)) {  // Сохранение в БД
        return false;
    }
    
    beginInsertRows(QModelIndex(), m_events.count(), m_events.count());
    m_events.append(newEvent);  // Добавление в память
    endInsertRows();
    
    return true;
}

// В DbManager::addEvent()
bool DbManager::addEvent(const Event& event) {
    QSqlQuery query;
    query.prepare("INSERT INTO events (...) VALUES (...)");
    
    // Сериализация списка изображений в JSON + base64
    QJsonArray imageArray;
    for (const auto& img : event.imageDataList) {
        imageArray.append(QString::fromLatin1(img.toBase64()));
    }
    QJsonDocument doc(imageArray);
    query.bindValue(":imageDataList", doc.toJson());
    
    query.exec();
    event.id = query.lastInsertId().toLongLong();
    return true;
}
```

### 2. Загрузка событий из БД

```cpp
// При запуске приложения
m_eventModel->loadAllEvents();

// В EventModel::loadAllEvents()
void EventModel::loadAllEvents() {
    beginResetModel();
    m_events = m_dbManager->getAllEvents();
    endResetModel();
    emit eventsLoaded(m_events.count());
}

// В DbManager::getAllEvents()
QList<Event> DbManager::getAllEvents() const {
    QList<Event> events;
    QSqlQuery query("SELECT * FROM events ORDER BY startDate");
    
    while (query.next()) {
        events.append(queryToEvent(query));
    }
    return events;
}

// Десериализация изображений
Event DbManager::queryToEvent(QSqlQuery& query) const {
    Event event;
    // ... загрузка полей ...
    
    QByteArray imageData = query.value("imageDataList").toByteArray();
    QJsonDocument doc = QJsonDocument::fromJson(imageData);
    
    if (doc.isArray()) {
        for (const auto& imgVal : doc.array()) {
            event.imageDataList.append(
                QByteArray::fromBase64(imgVal.toString().toLatin1())
            );
        }
    }
    return event;
}
```

## Механизм фильтрации событий

### Архитектура фильтрации

```
┌─────────────────────────────────────────────────────────────┐
│                    Панель фильтров (UI)                      │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌────────────────┐  │
│  │ Название │ │Дата от-до│ │   Тип    │ │   Категория    │  │
│  └────┬─────┘ └────┬─────┘ └────┬─────┘ └───────┬────────┘  │
└───────┼────────────┼────────────┼───────────────┼───────────┘
        │            │            │               │
        └────────────┴────────────┴───────────────┘
                         │
                         ▼
        ┌────────────────────────────────┐
        │   applyFilters() в MainWindow  │
        │   Создание QVariantMap с       │
        │   критериями фильтрации        │
        └───────────────┬────────────────┘
                        │
                        ▼
        ┌───────────────────────────────────────────────────┐
        │      EventFilterProxyModel (QSortFilterProxyModel)│
        │  Свойства:                                         │
        │  - filterTitle                                     │
        │  - filterStartDateFrom / filterStartDateTo         │
        │  - filterEventType                                 │
        │  - filterCategory                                  │
        │  - filterDurationMin                               │
        │                                                    │
        │  setFilterCriteria(QVariantMap) - установка всех   │
        │                                    критериев сразу │
        └───────────────────┬───────────────────────────────┘
                            │
                            ▼
        ┌───────────────────────────────────────────────────┐
        │  filterAcceptsRow() - проверка каждой строки      │
        │  - Поиск подстроки в названии                     │
        │  - Проверка диапазона дат                         │
        │  - Сравнение типа и категории                     │
        │  - Проверка минимальной длительности              │
        └───────────────────┬───────────────────────────────┘
                            │
                            ▼
        ┌───────────────────────────────────────────────────┐
        │              EventListView                        │
        │  Отображает только отфильтрованные события        │
        └───────────────────────────────────────────────────┘
```

### Код фильтрации

```cpp
// В MainWindow::applyFilters()
void MainWindow::applyFilters() {
    QVariantMap filterCriteria;
    
    filterCriteria["title"] = m_titleFilterEdit->text();
    filterCriteria["startDateFrom"] = m_dateFromEdit->date();
    filterCriteria["startDateTo"] = m_dateToEdit->date();
    filterCriteria["eventType"] = m_typeFilterCombo->currentData().toString();
    filterCriteria["category"] = m_categoryFilterCombo->currentData().toString();
    filterCriteria["durationMin"] = m_durationMinSpin->value();
    
    m_filterProxyModel->setFilterCriteria(filterCriteria);
}

// В EventFilterProxyModel::setFilterCriteria()
void EventFilterProxyModel::setFilterCriteria(const QVariantMap& criteria) {
    bool filterChanged = false;
    
    if (criteria.contains("title")) {
        m_filterTitle = criteria["title"].toString();
        emit filterTitleChanged();
        filterChanged = true;
    }
    // ... обработка остальных критериев ...
    
    if (filterChanged) {
        invalidateFilter();  // Пересчет фильтра
        emit filtersApplied();
    }
}

// В EventFilterProxyModel::filterAcceptsRow()
bool EventFilterProxyModel::filterAcceptsRow(int source_row, 
                                              const QModelIndex& parent) const {
    QModelIndex index = sourceModel()->index(source_row, 0, parent);
    
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
    
    // Фильтр по типу события
    if (!m_filterEventType.isEmpty()) {
        QString eventType = sourceModel()->data(index, EventModel::EventTypeRole).toString();
        if (eventType != m_filterEventType) {
            return false;
        }
    }
    
    // ... остальные проверки ...
    
    return true;  // Все условия выполнены
}
```

## Механизм отображения событий

### Кастомный делегат для отрисовки

```cpp
// В EventDelegate::paint()
void EventDelegate::paint(QPainter* painter, 
                          const QStyleOptionViewItem& option,
                          const QModelIndex& index) const {
    painter->save();
    
    // Получение данных из модели
    QString title = index.data(EventModel::TitleRole).toString();
    QDate startDate = index.data(EventModel::StartDateRole).toDate();
    QString eventType = index.data(EventModel::EventTypeRole).toString();
    QString category = index.data(EventModel::CategoryRole).toString();
    
    // Цвет фона в зависимости от типа
    QColor bgColor = getTypeColor(eventType);
    painter->fillRect(option.rect, bgColor);
    
    // Отрисовка иконки
    drawEventIcon(painter, iconRect, iconData);
    
    // Отрисовка текста (заголовок, дата, описание)
    drawEventText(painter, textRect, title, description, startDate, durationDays);
    
    // Отрисовка тегов (тип и категория)
    drawEventTags(painter, tagsRect, eventType, category);
    
    // Рамка и выделение
    painter->drawRoundedRect(option.rect, 5, 5);
    
    painter->restore();
}
```

## Схема базы данных SQLite

```sql
CREATE TABLE IF NOT EXISTS events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    startDate TEXT NOT NULL,              -- ISO формат (YYYY-MM-DD)
    durationDays INTEGER DEFAULT 1,
    description TEXT,
    eventType TEXT,
    category TEXT,
    iconData BLOB,                        -- Бинарные данные иконки
    imageDataList BLOB                    -- JSON массив base64 строк
);

-- Индексы для ускорения фильтрации
CREATE INDEX idx_eventType ON events(eventType);
CREATE INDEX idx_category ON events(category);
CREATE INDEX idx_startDate ON events(startDate);
```

## Ключевые особенности архитектуры

1. **Разделение ответственности**: Model хранит данные, View отображает, Controller управляет потоком
2. **Proxy Model для фильтрации**: Используется стандартный механизм Qt для фильтрации без изменения исходной модели
3. **Сериализация бинарных данных**: Изображения хранятся как base64 в JSON формате
4. **Кастомная отрисовка**: Делегат обеспечивает гибкое визуальное представление
5. **Реактивность**: Сигналы и слоты Qt обеспечивают автоматическое обновление UI при изменении данных

## Сборка проекта

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt
make -j$(nproc)
./CalendarApp
```

## Зависимости

- Qt 6.x (Core, Gui, Widgets, Sql)
- CMake 3.16+
- C++17 или выше
