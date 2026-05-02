#include "views/eventdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QBuffer>
#include <QPixmap>
#include <QLabel>

EventDialog::EventDialog(QWidget *parent, const Event& event)
    : QDialog(parent)
    , m_titleEdit(nullptr)
    , m_startDateEdit(nullptr)
    , m_durationSpin(nullptr)
    , m_descriptionEdit(nullptr)
    , m_typeCombo(nullptr)
    , m_categoryCombo(nullptr)
    , m_iconCombo(nullptr)
    , m_imageListWidget(nullptr)
    , m_addImageButton(nullptr)
    , m_removeImageButton(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_eventId(event.id)
{
    setWindowTitle(event.id >= 0 ? "Редактировать событие" : "Добавить событие");
    setMinimumWidth(500);
    setMinimumHeight(600);
    
    setupUi();
    
    if (event.id >= 0) {
        loadEventData(event);
    } else {
        // Значения по умолчанию для нового события
        m_startDateEdit->setDate(QDate::currentDate());
        m_durationSpin->setValue(1);
    }
}

EventDialog::~EventDialog() {
}

void EventDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Основная форма
    QGroupBox* mainGroup = new QGroupBox("Основная информация");
    QFormLayout* formLayout = new QFormLayout(mainGroup);
    formLayout->setSpacing(10);
    formLayout->setLabelAlignment(Qt::AlignRight);
    
    // Название
    m_titleEdit = new QLineEdit();
    m_titleEdit->setPlaceholderText("Введите название события");
    m_titleEdit->setMinimumWidth(300);
    formLayout->addRow("Название*", m_titleEdit);
    
    // Дата начала
    m_startDateEdit = new QDateEdit();
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setDisplayFormat("dd.MM.yyyy");
    m_startDateEdit->setMinimumDate(QDate(2000, 1, 1));
    m_startDateEdit->setMaximumDate(QDate(2100, 12, 31));
    formLayout->addRow("Дата начала*", m_startDateEdit);
    
    // Длительность
    m_durationSpin = new QSpinBox();
    m_durationSpin->setRange(1, 365);
    m_durationSpin->setSuffix(" дн.");
    formLayout->addRow("Длительность*", m_durationSpin);
    
    // Тип события
    m_typeCombo = new QComboBox();
    m_typeCombo->setEditable(false);
    formLayout->addRow("Тип события*", m_typeCombo);
    
    // Категория
    m_categoryCombo = new QComboBox();
    m_categoryCombo->setEditable(false);
    formLayout->addRow("Категория*", m_categoryCombo);
    
    // Описание
    m_descriptionEdit = new QTextEdit();
    m_descriptionEdit->setPlaceholderText("Описание события...");
    m_descriptionEdit->setMaximumHeight(100);
    formLayout->addRow("Описание", m_descriptionEdit);
    
    mainLayout->addWidget(mainGroup);
    
    // Группа иконки и изображений
    QGroupBox* mediaGroup = new QGroupBox("Иконка и изображения");
    QVBoxLayout* mediaLayout = new QVBoxLayout(mediaGroup);
    
    // Выбор иконки
    QHBoxLayout* iconLayout = new QHBoxLayout();
    QLabel* iconLabel = new QLabel("Иконка:");
    m_iconCombo = new QComboBox();
    m_iconCombo->addItem("📅 Стандартная", "default");
    m_iconCombo->addItem("🎉 Праздник", "celebration");
    m_iconCombo->addItem("💼 Работа", "work");
    m_iconCombo->addItem("🏠 Дом", "home");
    m_iconCombo->addItem("❤️ Личное", "personal");
    m_iconCombo->addItem("📚 Учеба", "study");
    m_iconCombo->addItem("⚽ Спорт", "sport");
    connect(m_iconCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EventDialog::onIconSelected);
    iconLayout->addWidget(iconLabel);
    iconLayout->addWidget(m_iconCombo);
    iconLayout->addStretch();
    mediaLayout->addLayout(iconLayout);
    
    // Список изображений
    QHBoxLayout* imageListLayout = new QHBoxLayout();
    QLabel* imageListLabel = new QLabel("Изображения:");
    m_imageListWidget = new QListWidget();
    m_imageListWidget->setMaximumHeight(100);
    m_imageListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    imageListLayout->addWidget(imageListLabel);
    imageListLayout->addWidget(m_imageListWidget, 1);
    mediaLayout->addLayout(imageListLayout);
    
    // Кнопки управления изображениями
    QHBoxLayout* imageButtonsLayout = new QHBoxLayout();
    m_addImageButton = new QPushButton("+ Добавить изображение");
    m_removeImageButton = new QPushButton("- Удалить");
    m_removeImageButton->setEnabled(false);
    
    connect(m_addImageButton, &QPushButton::clicked,
            this, &EventDialog::onAddImageButtonClicked);
    connect(m_removeImageButton, &QPushButton::clicked,
            this, &EventDialog::onRemoveImageButtonClicked);
    connect(m_imageListWidget, &QListWidget::itemSelectionChanged,
            this, [this]() {
                m_removeImageButton->setEnabled(m_imageListWidget->currentRow() >= 0);
            });
    
    imageButtonsLayout->addWidget(m_addImageButton);
    imageButtonsLayout->addWidget(m_removeImageButton);
    imageButtonsLayout->addStretch();
    mediaLayout->addLayout(imageButtonsLayout);
    
    mainLayout->addWidget(mediaGroup);
    
    // Заполняем комбобоксы
    populateTypeAndCategoryCombos();
    
    // Кнопки OK/Cancel
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_okButton = new QPushButton("OK");
    m_okButton->setDefault(true);
    m_okButton->setMinimumWidth(80);
    connect(m_okButton, &QPushButton::clicked,
            this, &EventDialog::validateAndAccept);
    
    m_cancelButton = new QPushButton("Отмена");
    m_cancelButton->setMinimumWidth(80);
    connect(m_cancelButton, &QPushButton::clicked,
            this, &QDialog::reject);
    
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
}

void EventDialog::populateTypeAndCategoryCombos() {
    // Типы событий
    m_typeCombo->addItem("Выберите тип...", "");
    m_typeCombo->addItem("Встреча", "meeting");
    m_typeCombo->addItem("Задача", "task");
    m_typeCombo->addItem("Праздник", "holiday");
    m_typeCombo->addItem("Напоминание", "reminder");
    m_typeCombo->addItem("Событие", "event");
    m_typeCombo->addItem("Другое", "other");
    
    // Категории
    m_categoryCombo->addItem("Выберите категорию...", "");
    m_categoryCombo->addItem("Работа", "work");
    m_categoryCombo->addItem("Личное", "personal");
    m_categoryCombo->addItem("Семья", "family");
    m_categoryCombo->addItem("Учеба", "education");
    m_categoryCombo->addItem("Спорт", "sports");
    m_categoryCombo->addItem("Здоровье", "health");
    m_categoryCombo->addItem("Финансы", "finance");
    m_categoryCombo->addItem("Другое", "other");
}

void EventDialog::loadEventData(const Event& event) {
    m_titleEdit->setText(event.title);
    m_startDateEdit->setDate(event.startDate);
    m_durationSpin->setValue(event.durationDays);
    m_descriptionEdit->setPlainText(event.description);
    
    // Выбираем тип
    int typeIndex = m_typeCombo->findData(event.eventType);
    if (typeIndex >= 0) {
        m_typeCombo->setCurrentIndex(typeIndex);
    }
    
    // Выбираем категорию
    int catIndex = m_categoryCombo->findData(event.category);
    if (catIndex >= 0) {
        m_categoryCombo->setCurrentIndex(catIndex);
    }
    
    // Загружаем иконку (по умолчанию)
    m_currentIconData = event.iconData;
    
    // Загружаем изображения
    m_imagesData = event.imageDataList;
    for (int i = 0; i < m_imagesData.size(); ++i) {
        QPixmap pix;
        if (pix.loadFromData(m_imagesData[i])) {
            QListWidgetItem* item = new QListWidgetItem(
                QIcon(pix.scaled(32, 32, Qt::KeepAspectRatio)), 
                QString("Изображение %1").arg(i + 1)
            );
            m_imageListWidget->addItem(item);
        }
    }
}

Event EventDialog::getEvent() const {
    Event event;
    event.id = m_eventId;
    event.title = m_titleEdit->text();
    event.startDate = m_startDateEdit->date();
    event.durationDays = m_durationSpin->value();
    event.description = m_descriptionEdit->toPlainText();
    event.eventType = m_typeCombo->currentData().toString();
    event.category = m_categoryCombo->currentData().toString();
    event.iconData = m_currentIconData;
    event.imageDataList = m_imagesData;
    
    return event;
}

void EventDialog::onAddImageButtonClicked() {
    QStringList filters;
    filters << "Images (*.png *.xpm *.jpg *.jpeg *.gif *.bmp)"
            << "All files (*)";
    
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilters(filters);
    
    if (dialog.exec() == QDialog::Accepted) {
        QStringList filePaths = dialog.selectedFiles();
        
        for (const QString& filePath : filePaths) {
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray data = file.readAll();
                
                // Проверяем, что это действительно изображение
                QPixmap pix;
                if (pix.loadFromData(data)) {
                    m_imagesData.append(data);
                    
                    QListWidgetItem* item = new QListWidgetItem(
                        QIcon(pix.scaled(32, 32, Qt::KeepAspectRatio)),
                        QString("Изображение %1").arg(m_imagesData.size())
                    );
                    m_imageListWidget->addItem(item);
                } else {
                    QMessageBox::warning(this, "Ошибка", 
                        QString("Не удалось загрузить изображение: %1").arg(filePath));
                }
            }
        }
    }
}

void EventDialog::onRemoveImageButtonClicked() {
    int currentRow = m_imageListWidget->currentRow();
    if (currentRow >= 0 && currentRow < m_imagesData.size()) {
        m_imagesData.removeAt(currentRow);
        delete m_imageListWidget->takeItem(currentRow);
    }
}

void EventDialog::onIconSelected() {
    // В реальной реализации здесь можно загружать разные иконки
    // Для простоты просто сохраняем выбор
    QString iconType = m_iconCombo->currentData().toString();
    
    // Генерируем простую иконку на основе выбора
    // В реальном приложении можно загружать из ресурсов
    if (iconType == "celebration") {
        m_currentIconData = QByteArrayLiteral("celebration_icon");
    } else if (iconType == "work") {
        m_currentIconData = QByteArrayLiteral("work_icon");
    } else if (iconType == "home") {
        m_currentIconData = QByteArrayLiteral("home_icon");
    } else if (iconType == "personal") {
        m_currentIconData = QByteArrayLiteral("personal_icon");
    } else if (iconType == "study") {
        m_currentIconData = QByteArrayLiteral("study_icon");
    } else if (iconType == "sport") {
        m_currentIconData = QByteArrayLiteral("sport_icon");
    } else {
        m_currentIconData = QByteArrayLiteral("default_icon");
    }
}

void EventDialog::validateAndAccept() {
    // Валидация обязательных полей
    if (m_titleEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка валидации", 
            "Название события обязательно для заполнения");
        m_titleEdit->setFocus();
        return;
    }
    
    if (!m_startDateEdit->date().isValid()) {
        QMessageBox::warning(this, "Ошибка валидации", 
            "Укажите корректную дату начала");
        m_startDateEdit->setFocus();
        return;
    }
    
    if (m_durationSpin->value() <= 0) {
        QMessageBox::warning(this, "Ошибка валидации", 
            "Длительность должна быть больше 0");
        m_durationSpin->setFocus();
        return;
    }
    
    if (m_typeCombo->currentData().toString().isEmpty()) {
        QMessageBox::warning(this, "Ошибка валидации", 
            "Выберите тип события");
        m_typeCombo->setFocus();
        return;
    }
    
    if (m_categoryCombo->currentData().toString().isEmpty()) {
        QMessageBox::warning(this, "Ошибка валидации", 
            "Выберите категорию события");
        m_categoryCombo->setFocus();
        return;
    }
    
    accept();
}
