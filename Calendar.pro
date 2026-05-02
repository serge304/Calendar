QT += core gui widgets sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Calendar
TEMPLATE = app

# Версии C++20 для современных возможностей
CONFIG += c++20

# Исходные файлы
SOURCES += \
    src/main.cpp \
    src/models/event.cpp \
    src/models/eventmodel.cpp \
    src/models/eventfilterproxymodel.cpp \
    src/database/dbmanager.cpp \
    src/views/mainwindow.cpp \
    src/views/eventdialog.cpp \
    src/views/eventlistview.cpp \
    src/delegates/eventdelegate.cpp

# Заголовочные файлы
HEADERS += \
    src/models/event.h \
    src/models/eventmodel.h \
    src/models/eventfilterproxymodel.h \
    src/database/dbmanager.h \
    src/views/mainwindow.h \
    src/views/eventdialog.h \
    src/views/eventlistview.h \
    src/delegates/eventdelegate.h

# Файлы форм (если будут использоваться .ui файлы)
FORMS += \
    src/views/mainwindow.ui \
    src/views/eventdialog.ui

# Ресурсы
RESOURCES += \
    resources.qrc

# Пути для поиска заголовочных файлов
INCLUDEPATH += $$PWD/src

# Настройки компиляции
QMAKE_CXXFLAGS += -Wall -Wextra

# Для Windows дополнительные настройки
win32 {
    DEFINES += _CRT_SECURE_NO_WARNINGS
}

# Для macOS
macx {
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15
}

# Директория для объектов и временных файлов
MOC_DIR = moc
UI_DIR = ui
RCC_DIR = rcc
OBJECTS_DIR = obj

# База данных будет создаваться в директории приложения
DEFINES += DB_PATH=\"\\\"$$PWD/events.db\\\"\"
