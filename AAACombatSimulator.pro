TEMPLATE    = app
CONFIG      += qt console
TARGET      = AAACombatSimulator
QT          += xml network

# install
target.path = ./
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS AAACombatSimulator.pro
sources.path = ./
INSTALLS += target sources

#RESOURCES = combatally.qrc

#RC_FILE = combatally.rc

#ICON = images/swords.icns

HEADERS = \
    combatthread.h \
    combatwidget.h \
    controlwidget.h \
    factionwidget.h \
    settingswidget.h \
    simulatorapplication.h \
    unit.h \
    unitwidget.h

SOURCES = \
    combatthread.cpp \
    combatwidget.cpp \
    controlwidget.cpp \
    factionwidget.cpp \
    settingswidget.cpp \
    main.cpp \
    simulatorapplication.cpp \
    unit.cpp \
    unitwidget.cpp
