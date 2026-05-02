#ifndef EVENTDIALOG_H
#define EVENTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDateEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QSpinBox>
#include "models/event.h"

class EventDialog : public QDialog {
    Q_OBJECT

public:
    explicit EventDialog(QWidget *parent = nullptr, const Event& event = Event());
    ~EventDialog();

    // Получение данных события из диалога
    Event getEvent() const;

private slots:
    void onAddImageButtonClicked();
    void onRemoveImageButtonClicked();
    void onIconSelected();
    void validateAndAccept();

private:
    void setupUi();
    void loadEventData(const Event& event);
    void populateTypeAndCategoryCombos();

    // Поля ввода
    QLineEdit* m_titleEdit;
    QDateEdit* m_startDateEdit;
    QSpinBox* m_durationSpin;
    QTextEdit* m_descriptionEdit;
    QComboBox* m_typeCombo;
    QComboBox* m_categoryCombo;
    
    // Выбор иконки
    QComboBox* m_iconCombo;
    
    // Список изображений
    QListWidget* m_imageListWidget;
    QPushButton* m_addImageButton;
    QPushButton* m_removeImageButton;
    
    // Кнопки OK/Cancel
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    
    // Внутреннее хранение изображения иконки
    QByteArray m_currentIconData;
    QList<QByteArray> m_imagesData;
    
    // ID существующего события (если редактирование)
    int64_t m_eventId;
};

#endif // EVENTDIALOG_H
