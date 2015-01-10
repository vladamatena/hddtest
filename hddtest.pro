# -------------------------------------------------
# Project created by QtCreator 2010-04-12T15:07:24
# -------------------------------------------------
TARGET = hddtest
TEMPLATE = app
SOURCES += main.cpp \
    hddtest.cpp \
    randomgenerator.cpp \
    seeker.cpp \
    readrnd.cpp \
    device.cpp \
    readcont.cpp \
    readblock.cpp \
    filerw.cpp \
    filestructure.cpp \
	smallfiles.cpp \
    testwidget.cpp \
    definitions.cpp \
    file.cpp \
    timer.cpp \
    testthread.cpp \
	about.cpp
HEADERS += hddtest.h \
    randomgenerator.h \
    seeker.h \
    readrnd.h \
    device.h \
    readcont.h \
    readblock.h \
    filerw.h \
    filestructure.h \
    filestructure.h \
    smallfiles.h \
    testwidget.h \
    definitions.h \
    file.h \
    timer.h \
    testthread.h \
	about.h
FORMS += hddtest.ui \
    testwidget.ui \
    about.ui
QT += xml
#QT += dbus

RESOURCES += \
    resource.qrc

target.path = $$PREFIX/bin/

INSTALLS += target

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += udisks2
