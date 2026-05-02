#include "views/mainwindow.h"
#include "database/dbmanager.h"
#include "models/eventmodel.h"
#include "models/eventfilterproxymodel.h"
#include "views/eventdialog.h"
#include "views/eventlistview.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QToolBar>
#include <QMenuBar>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_dbManager(nullptr)
    , m_eventModel(nullptr)
    , m_filterProxyModel(nullptr)
    , m_titleFilterEdit(nullptr)
    , m_dateFromEdit(nullptr)
    , m_dateToEdit(nullptr)
    , m_typeFilterCombo(nullptr)
    , m_categoryFilterCombo(nullptr)
    , m_durationMinSpin(nullptr)
    , m_resetFilterButton(nullptr)
    , m_eventListView(nullptr)
    , m_addEventButton(nullptr)
{
    setWindowTitle("Календарь событий");
    resize(1000, 700);
    
    // Инициализация MVC компонентов
    initMvcComponents();
    
    // Настройка UI
    setupUi();
    
    // Загрузка данных
    m_eventModel->loadAllEvents();
}

MainWindow::~MainWindow() {
    delete m_dbManager;
}

void MainWindow::initMvcComponents() {
    // Model - создаем менеджер базы данных и модель
    m_dbManager = new DbManager(this);
    
    // Инициализируем БД (в памяти или файл)
    if (!m_dbManager->initDatabase("calendar_events.db")) {
        QMessageBox::critical(this, "Ошибка", "Не удалось инициализировать базу данных");
        return;
    }
    
    m_eventModel = new EventModel(m_dbManager, this);
    
    // Proxy Model для фильтрации
    m_filterProxyModel = new EventFilterProxyModel(this);
    m_filterProxyModel->setSourceModel(m_eventModel);
    m_filterProxyModel->setDynamicSortFilter(true);
    
    // Соединяем сигналы фильтров
    connect(m_filterProxyModel, &EventFilterProxyModel::filtersApplied,
            this, [this]() {
                qDebug() << "Фильтры применены. Найдено событий:" 
                         << m_filterProxyModel->rowCount();
            });
}

void MainWindow::setupUi() {
    // Создаем центральный виджет
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // Создаем панель инструментов
    createToolBar();
    
    // Создаем панель фильтров
    createFilterPanel();
    
    // Создаем список событий
    createEventList();
    
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
}

void MainWindow::createToolBar() {
    QToolBar* toolBar = addToolBar("Основная");
    toolBar->setMovable(false);
    
    m_addEventButton = new QPushButton("+ Добавить событие");
    m_addEventButton->setStyleSheet(
        "QPushButton { "
        "  background-color: #4CAF50; "
        "  color: white; "
        "  padding: 8px 16px; "
        "  border-radius: 4px; "
        "  font-weight: bold;"
        "} "
        "QPushButton:hover { background-color: #45a049; }"
    );
    
    connect(m_addEventButton, &QPushButton::clicked,
            this, &MainWindow::onAddEventClicked);
    
    toolBar->addWidget(m_addEventButton);
    toolBar->addSeparator();
    
    QLabel* infoLabel = new QLabel("Фильтруйте события для поиска");
    infoLabel->setStyleSheet("color: gray; font-style: italic;");
    toolBar->addWidget(infoLabel);
}

void MainWindow::createFilterPanel() {
    QGroupBox* filterGroup = new QGroupBox("Фильтры событий");
    
    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(10);
    
    // Фильтр по названию
    m_titleFilterEdit = new QLineEdit();
    m_titleFilterEdit->setPlaceholderText("Поиск по названию...");
    connect(m_titleFilterEdit, &QLineEdit::textChanged,
            this, &MainWindow::onFilterChanged);
    formLayout->addRow("Название:", m_titleFilterEdit);
    
    // Фильтр по дате (от)
    m_dateFromEdit = new QDateEdit();
    m_dateFromEdit->setCalendarPopup(true);
    m_dateFromEdit->setDisplayFormat("dd.MM.yyyy");
    m_dateFromEdit->setDate(QDate::currentDate().addMonths(-1));
    connect(m_dateFromEdit, &QDateEdit::dateChanged,
            this, &MainWindow::onFilterChanged);
    formLayout->addRow("Дата от:", m_dateFromEdit);
    
    // Фильтр по дате (до)
    m_dateToEdit = new QDateEdit();
    m_dateToEdit->setCalendarPopup(true);
    m_dateToEdit->setDisplayFormat("dd.MM.yyyy");
    m_dateToEdit->setDate(QDate::currentDate().addMonths(1));
    connect(m_dateToEdit, &QDateEdit::dateChanged,
            this, &MainWindow::onFilterChanged);
    formLayout->addRow("Дата до:", m_dateToEdit);
    
    // Фильтр по типу
    m_typeFilterCombo = new QComboBox();
    m_typeFilterCombo->addItem("Все типы", "");
    m_typeFilterCombo->addItems(QStringList() 
        << "Встреча" << "Задача" << "Праздник" << "Напоминание" << "Другое");
    connect(m_typeFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFilterChanged);
    formLayout->addRow("Тип:", m_typeFilterCombo);
    
    // Фильтр по категории
    m_categoryFilterCombo = new QComboBox();
    m_categoryFilterCombo->addItem("Все категории", "");
    m_categoryFilterCombo->addItems(QStringList() 
        << "Работа" << "Личное" << "Семья" << "Учеба" << "Спорт" << "Другое");
    connect(m_categoryFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFilterChanged);
    formLayout->addRow("Категория:", m_categoryFilterCombo);
    
    // Фильтр по длительности
    m_durationMinSpin = new QSpinBox();
    m_durationMinSpin->setRange(0, 365);
    m_durationMinSpin->setSuffix(" дн.");
    m_durationMinSpin->setValue(0);
    connect(m_durationMinSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onFilterChanged);
    formLayout->addRow("Длительность от:", m_durationMinSpin);
    
    // Кнопка сброса фильтров
    m_resetFilterButton = new QPushButton("Сбросить фильтры");
    connect(m_resetFilterButton, &QPushButton::clicked,
            this, &MainWindow::onResetFiltersClicked);
    
    QVBoxLayout* groupLayout = new QVBoxLayout(filterGroup);
    groupLayout->addLayout(formLayout);
    groupLayout->addWidget(m_resetFilterButton);
    
    filterGroup->setMaximumHeight(250);
    
    // Добавляем в основной layout
    centralWidget()->layout()->addWidget(filterGroup);
}

void MainWindow::createEventList() {
    // Создаем кастомный view для отображения событий
    m_eventListView = new EventListView();
    m_eventListView->setModel(m_filterProxyModel);
    m_eventListView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_eventListView->setResizeMode(QListView::Adjust);
    m_eventListView->setSpacing(5);
    
    // Стилизация
    m_eventListView->setStyleSheet(
        "QListView { "
        "  border: 1px solid #ddd; "
        "  border-radius: 4px; "
        "  background-color: white;"
        "} "
        "QListView::item { "
        "  border: 1px solid #eee; "
        "  border-radius: 4px; "
        "  padding: 8px;"
        "} "
        "QListView::item:selected { "
        "  background-color: #e3f2fd; "
        "  border-color: #2196F3;"
        "} "
        "QListView::item:hover { "
        "  background-color: #f5f5f5;"
        "}"
    );
    
    connect(m_eventListView, &QListView::clicked,
            this, &MainWindow::onEventSelected);
    
    centralWidget()->layout()->addWidget(m_eventListView);
}

void MainWindow::onAddEventClicked() {
    EventDialog dialog(this);
    
    if (dialog.exec() == QDialog::Accepted) {
        Event newEvent = dialog.getEvent();
        
        if (m_eventModel->addEvent(newEvent)) {
            qDebug() << "Событие успешно добавлено:" << newEvent.title;
            
            // Применяем текущие фильтры, чтобы показать новое событие если оно подходит
            applyFilters();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось добавить событие");
        }
    }
}

void MainWindow::onFilterChanged() {
    applyFilters();
}

void MainWindow::onResetFiltersClicked() {
    m_filterProxyModel->resetFilters();
    
    // Сбрасываем UI элементы
    m_titleFilterEdit->clear();
    m_dateFromEdit->setDate(QDate::currentDate().addMonths(-1));
    m_dateToEdit->setDate(QDate::currentDate().addMonths(1));
    m_typeFilterCombo->setCurrentIndex(0);
    m_categoryFilterCombo->setCurrentIndex(0);
    m_durationMinSpin->setValue(0);
}

void MainWindow::onEventSelected(const QModelIndex& index) {
    if (!index.isValid())
        return;
    
    // Получаем событие из proxy модели
    QModelIndex sourceIndex = m_filterProxyModel->mapToSource(index);
    Event event = m_eventModel->getEventAt(sourceIndex.row());
    
    qDebug() << "Выбрано событие:" << event.title 
             << "Дата:" << event.startDate.toString()
             << "Тип:" << event.eventType;
    
    // Здесь можно открыть диалог редактирования или детального просмотра
    // EventDialog dialog(this, event);
    // if (dialog.exec() == QDialog::Accepted) { ... }
}

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
