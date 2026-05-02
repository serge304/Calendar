#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVariantMap>

class DbManager;
class EventModel;
class EventFilterProxyModel;
class QListView;
class QLineEdit;
class QDateEdit;
class QComboBox;
class QPushButton;
class QSpinBox;
class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Обработчики событий UI
    void onAddEventClicked();
    void onFilterChanged();
    void onResetFiltersClicked();
    void onEventSelected(const QModelIndex& index);
    
private:
    // Инициализация UI компонентов
    void setupUi();
    void createFilterPanel();
    void createEventList();
    void createToolBar();
    
    // Инициализация компонентов MVC
    void initMvcComponents();
    
    // Применение фильтров
    void applyFilters();

    // Компоненты MVC
    DbManager* m_dbManager;
    EventModel* m_eventModel;
    EventFilterProxyModel* m_filterProxyModel;
    
    // UI компоненты фильтрации
    QLineEdit* m_titleFilterEdit;
    QDateEdit* m_dateFromEdit;
    QDateEdit* m_dateToEdit;
    QComboBox* m_typeFilterCombo;
    QComboBox* m_categoryFilterCombo;
    QSpinBox* m_durationMinSpin;
    QPushButton* m_resetFilterButton;
    
    // Список событий
    QListView* m_eventListView;
    
    // Кнопка добавления события
    QPushButton* m_addEventButton;
};

#endif // MAINWINDOW_H
