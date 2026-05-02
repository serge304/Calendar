cmake_minimum_required(VERSION 3.16)
project(CalendarApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)

set(SOURCES
    src/main.cpp
    src/database/dbmanager.cpp
    src/models/eventmodel.cpp
    src/models/eventfilterproxymodel.cpp
    src/views/mainwindow.cpp
    src/views/eventdialog.cpp
    src/views/eventlistview.cpp
    src/delegates/eventdelegate.cpp
)

set(HEADERS
    src/database/dbmanager.h
    src/models/event.h
    src/models/eventmodel.h
    src/models/eventfilterproxymodel.h
    src/views/mainwindow.h
    src/views/eventdialog.h
    src/views/eventlistview.h
    src/delegates/eventdelegate.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
