project(AAACombatSimulator)
cmake_minimum_required(VERSION 2.8)

set(HEADER_FILES
    combatthread.h
    combatwidget.h
    controlwidget.h
    factionwidget.h
    focusspinbox.h
    settingswidget.h
    simulatorapplication.h
    unit.h
    unitwidget.h)
    
set(SOURCE_FILES
    combatthread.cpp
    combatwidget.cpp
    controlwidget.cpp
    factionwidget.cpp
    focusspinbox.cpp
    main.cpp
    settingswidget.cpp
    simulatorapplication.cpp
    unit.cpp
    unitwidget.cpp)

# set(QT_USE_QTXML TRUE)  
# set(QT_USE_QTNETWORK TRUE)  
find_package(Qt4 REQUIRED QtCore QtGui QtXML QtNetwork)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
qt4_wrap_cpp(HEADER_MOC_FILES ${HEADER_FILES})

add_executable(AAACombatSimulator
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${HEADER_MOC_FILES})
    
target_link_libraries(AAACombatSimulator ${QT_LIBRARIES})